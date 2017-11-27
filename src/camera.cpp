// camera.cpp
//
// (c)2012-2017 Abraham Stolk

#include "camera.h"

#include "pidc.h"
#include "logx.h"
#include "nfy.h"

#include <float.h>

static	mat44_t trf;
static	mat44_t view;		// view transformation is the inverse of the camera transformation.
static	mat44_t proj;		// projection transformation (perspective cam)
static	mat44_t viewproj;	// view * proj tranform (cached multiplication)
	
static	mat44_t trfNoShift; // the transformation without shift to coi.
    
static	vec3_t desiredCoi;  // where do we want cam to point?
static	vec3_t actualCoi;   // where does cam actually point to?
    
static	float  desiredPan;
static	float  actualPan;

static	pid3_t coiPid;      // vector PID for coi control
static	pid1_t panPid;      // angular PID for pan control
    
static	float  alignVel;
static	pid1_t alignPid;

static	vec3_t actualPos;
static	vec3_t desiredPos;

static	float dist;
static	float orbitAngle;
static	float elevationAngle;

static float relaxOrbit;
static float relaxElevation = -FLT_MAX;
static float relaxFactor = -FLT_MAX;
	
static	bool validViewMat;	// is the view transformation valid, or does it need recalculation from transform mat?

static  float fovy_radians = 0.3f * M_PI;
static  float fovx_radians = 0.3f * M_PI;

static float minZ = -0.12f;

// for frustum clips
static  vec3_t tnorml(-1, 0, 0);	// normal for left wall
static  vec3_t tnormr( 1, 0, 0);	// normal for right wall
static  vec3_t tnormt( 0, 0,-1);	// normal for top wall
static  vec3_t tnormb( 0, 0, 1);	// normal for bottom wall

float camera_minDist = 10;
float camera_maxDist = 1100;

static float camera_rollangle = 0.0f;

float camera_dist(void) { return dist; }

void camera_setDist( float d ) { dist = d; }

static void camera_scaleDist(float factor)
{
	dist = factor * dist;
	dist = ( dist < camera_minDist ) ? camera_minDist : dist;
	dist = ( dist > camera_maxDist ) ? camera_maxDist : dist;
}


static void camera_changeOrbit(float delta)
{
	orbitAngle += delta;
	orbitAngle = orbitAngle >  M_PI ? orbitAngle - 2*M_PI : orbitAngle;
	orbitAngle = orbitAngle < -M_PI ? orbitAngle + 2*M_PI : orbitAngle;
}


static void camera_changeElevation(float delta)
{
	elevationAngle += delta;
	elevationAngle = elevationAngle >  0.4999f * M_PI ?  0.4999f * M_PI : elevationAngle;
	elevationAngle = elevationAngle < -0.4999f * M_PI ? -0.4999f * M_PI : elevationAngle;
}


static void onCameraControl(const char* msg)
{
	const float distScale = nfy_flt( msg, "distScale" );
	if ( distScale > -FLT_MAX ) camera_scaleDist( distScale );

	const float orbitDelta = nfy_flt( msg, "orbitDelta" );
	if ( orbitDelta > -FLT_MAX ) camera_changeOrbit( orbitDelta );

	const float elevationDelta = nfy_flt( msg, "elevationDelta" );
	if ( elevationDelta > -FLT_MAX ) camera_changeElevation( elevationDelta );

	const float distSetting = nfy_flt( msg, "distSetting" );
	if ( distSetting > -FLT_MAX ) dist = distSetting;

	const float orbitSetting = nfy_flt( msg, "orbitSetting" );
	if ( orbitSetting > -FLT_MAX ) orbitAngle = orbitSetting;

	const float elevationSetting = nfy_flt( msg, "elevationSetting" );
	if ( elevationSetting > -FLT_MAX ) elevationAngle = elevationSetting;

	const float aspectRatio = nfy_flt( msg, "aspectRatio" );
	const int offaxis = nfy_int( msg, "offaxis" );
	if ( aspectRatio > -FLT_MAX ) camera_setAspectRatio( aspectRatio, 0.1f, 150.0f, offaxis>0 );

	const float ro  = nfy_flt( msg, "relaxOrbit" );
	const float re  = nfy_flt( msg, "relaxElevation" );
	const float rf  = nfy_flt( msg, "relaxFactor" );
	const int   rso = nfy_int( msg, "relaxSnapOrbit" );
	if ( ro > -FLT_MAX ) relaxOrbit = ro;
	if ( re > -FLT_MAX ) relaxElevation = re;
	if ( rf > -FLT_MAX ) relaxFactor = rf;
	if ( rf == 0.0f ) { relaxOrbit = -FLT_MAX; relaxElevation = -FLT_MAX; }
	if ( rso )
	{
		if      ( orbitAngle < -0.75f*M_PI )
		relaxOrbit = -1.00*-M_PI;
		else if ( orbitAngle < -0.25f*M_PI )
		relaxOrbit = -0.50*M_PI;
		else if ( orbitAngle <  0.25f*M_PI )
		relaxOrbit =  0.00*M_PI;
		else if ( orbitAngle <  0.75f*M_PI )
		relaxOrbit =  0.50*M_PI;
		else
		relaxOrbit =  1.00*M_PI;
	}
}


void camera_init( float fovy )
{
	fovy_radians = fovy;
	actualPos = desiredPos = vec3_t( -4, 0, 1.4f );
	desiredCoi = actualCoi = vec3_t( 0, 0, 0 );
    
	desiredPan = actualPan = 0.0;
	
	dist = 8.0f;
	orbitAngle = 0.0f;
	elevationAngle = 0.16f;
	relaxOrbit = -FLT_MAX;
	relaxElevation = -FLT_MAX;
	relaxFactor = 0.0f;

	proj.identity();
	validViewMat = false;
    
	coiPid.P = -0.070f;
	coiPid.I = -0.060f;
	coiPid.D = -0.008f;
    
	panPid.P = -0.05f;
	panPid.I = -0.05f;
	panPid.D = -0.01f;
	panPid.angular = true;
    
	alignPid.P = -0.03f;
	alignPid.I = -0.06f;
	alignPid.D = -0.05f;
	alignPid.angular = false;
	
	pid3_reset( coiPid );
	pid1_reset( panPid );
	pid1_reset( alignPid );
    
	alignVel = 0.0f;

	nfy_obs_add( "cameraControl", onCameraControl );
}


void camera_setCoiPid( float p, float i, float d )
{
	coiPid.P = p;
	coiPid.I = i;
	coiPid.D = d;
}


void camera_setPanPid( float p, float i, float d )
{
	panPid.P = p;
	panPid.I = i;
	panPid.D = d;
}


void camera_setAspectRatio(float aspect, float zNear, float zFar, bool offaxis)
{
	// create a projection matrix
	const float f = 1.0f / tanf(fovy_radians/2.0f);
	fovx_radians = 2.0f * atanf( aspect * tanf(fovy_radians/2.0f) );
	float* mout = proj.data;
    
	mout[0] = f / aspect;
	mout[1] = 0.0f;
	mout[2] = 0.0f;
	mout[3] = 0.0f;
    
	mout[4] = 0.0f;
	mout[5] = f;
	mout[6] = 0.0f;
	mout[7] = 0.0f;

	mout[8] = 0.0f;
	mout[9] = offaxis ? -0.25f : 0.0f; // off axis projection!
	mout[10] = (zFar+zNear) / (zNear-zFar);
	mout[11] = -1.0f;

	mout[12] = 0.0f;
	mout[13] = 0.0f;
	mout[14] = 2 * zFar * zNear /  (zNear-zFar);
	mout[15] = 0.0f;
}


vec3_t camera_screenToWorld(vec3_t spos)
{
    float sx = 1.0f / proj.data[0];
    float sy = 1.0f / proj.data[5];
    const float* m = trf.data;
    vec3_t camx( m[0], m[1], m[2] );
    vec3_t camy( m[4], m[5], m[6] );
    vec3_t camz( m[8], m[9], m[10] );
    vec3_t camo( m[12], m[13], m[14] );
    return camo + camx * sx * spos[ 0 ] + camy * sy * spos[ 1 ] - camz;
}



void camera_getViewTransform(mat44_t& mat)
{
	if ( !validViewMat )
	{
		view = trf.inverse();
		validViewMat = true;
	}
	memcpy( mat.data, view.data, sizeof(mat.data) );
}


void camera_getCameraTransform(mat44_t &mat)
{
    memcpy( mat.data, trf.data, sizeof(mat.data) );
}


void camera_getCameraTransformNoShift(mat44_t &mat)
{
    memcpy( mat.data, trfNoShift.data, sizeof(mat.data) );
}


void camera_getProjTransform(mat44_t& mat)
{
	memcpy( mat.data, proj.data, sizeof(mat.data) );
}


void camera_setPos(const vec3_t& p)
{
	actualPos = desiredPos = p;
	validViewMat = false;
}


void camera_forceCOI(const vec3_t& c)
{
	desiredCoi = actualCoi = c;
	pid3_reset( coiPid );
}


void camera_setCOI(const vec3_t& c)
{
	desiredCoi = c;
	validViewMat = false;
}


vec3_t camera_getCOI( void )
{
	return actualCoi;
}


void camera_forcePan(float p)
{
	desiredPan = actualPan = p;
	pid1_reset( panPid );
}


void camera_setPan(float p)
{
	desiredPan = p;
	validViewMat = false;
}


vec3_t camera_pos(void)
{
	return actualPos;
}


vec3_t camera_viewX(void)
{
	return vec3_t (view.data[0], view.data[1], view.data[2]);
}

vec3_t camera_viewY(void)
{
	return vec3_t(view.data[4], view.data[5], view.data[6]);
}

vec3_t camera_viewZ(void)
{
	return vec3_t(view.data[8], view.data[9], view.data[10]);
}

vec3_t camera_viewP(void)
{
	return vec3_t(view.data[12], view.data[13], view.data[14]);
}

vec3_t camera_camX(void)
{
	return vec3_t(trf.data[0], trf.data[1], trf.data[2]);
}

vec3_t camera_camY(void)
{
	return vec3_t(trf.data[4], trf.data[5], trf.data[6]);

}

vec3_t camera_camZ(void)
{
	return vec3_t(trf.data[8], trf.data[9], trf.data[10]);

}

void camera_alignTo(vec3_t d, float margin, float dt)
{
    vec3_t camz = camera_camZ();
    float dp = camz.dotProduct( d );
    if ( fabsf( dp ) < margin )
    {
        dp = 0.0f;
    }

    const float steer = pid1_update( alignPid, dt, dp, 0 );
    alignVel -= steer;
    orbitAngle += alignVel * dt;
}


static float dist_point_to_plane( vec3_t p, vec3_t pln, vec3_t plo, vec3_t* projected_point )
{
	const float sb = pln.dotProduct( p - plo );
	if ( projected_point ) *projected_point = p + pln * -sb;
	return sb;
}


/*
 * This culling probably does not work if there is camera roll.
 * Use with care.
 */
bool camera_should_cull( vec3_t p, float margin )
{
	const vec3_t plo = trf.getTranslation();
	float dl = dist_point_to_plane( p, tnorml, plo, 0 );
	float dr = dist_point_to_plane( p, tnormr, plo, 0 );
	return dl > margin || dr > margin;
}


void camera_update( float dt )
{
	// Run PID to update our actual COI/DOI values
	const vec3_t coiSteer = pid3_update( coiPid, dt, actualCoi, desiredCoi );
    
	actualCoi = actualCoi + coiSteer;
    
	const float panSteer = pid1_update( panPid, dt, actualPan, desiredPan );
	actualPan = actualPan + panSteer;
	actualPan = ( actualPan < -M_PI ) ? actualPan + 2 * M_PI : actualPan;
	actualPan = ( actualPan >  M_PI ) ? actualPan - 2 * M_PI : actualPan;
    
	const vec3_t doi( cosf( actualPan ), sinf( actualPan ), 0 );
	
	if ( relaxFactor > 0.0f )
	{
		if ( relaxOrbit > -FLT_MAX )
		{
			float deltao = relaxOrbit - orbitAngle;
			deltao = deltao >  M_PI ? deltao - 2*M_PI : deltao;
			deltao = deltao < -M_PI ? deltao + 2*M_PI : deltao;
			camera_changeOrbit( deltao * relaxFactor );
		}
		if ( relaxElevation > -FLT_MAX )
		{
			float deltae = relaxElevation - elevationAngle;
			deltae = deltae >  M_PI ? deltae - 2*M_PI : deltae;
			deltae = deltae < -M_PI ? deltae + 2*M_PI : deltae;
			camera_changeElevation( deltae * relaxFactor );
		}
	}
    
	const vec3_t toCam
	(
		-doi[ 0 ] * cosf( orbitAngle ) * cosf( elevationAngle ) + doi[ 1 ] * sinf( orbitAngle ) * cosf( elevationAngle ),
		-doi[ 1 ] * cosf( orbitAngle ) * cosf( elevationAngle ) - doi[ 0 ] * sinf( orbitAngle ) * cosf( elevationAngle ),
		sinf( elevationAngle )
	);
	
	desiredPos =  actualCoi + ( toCam * dist );
	actualPos = desiredPos;

	// clip camera against ground
	if ( actualPos.z < minZ )
	{
		vec3_t p = actualPos;
		p.z = minZ;
		float distxy = sqrtf( (actualCoi.x-p.x) * (actualCoi.x-p.x) + (actualCoi.y-p.y)*(actualCoi.y-p.y) );
		elevationAngle = atan2f( (p.z-actualCoi.z), distxy );
		// Never look straight up or down!
		elevationAngle = CLAMPED( elevationAngle, -0.49f*M_PI, 0.44f*M_PI );
		actualPos.z = minZ;
	}

	vec3_t z = actualPos - actualCoi;

	z.normalize();
	
	vec3_t up( 0,0,1 );
	
	vec3_t x = up.crossProduct( z );
	if ( x.lengthSq() < 1.0e-20f )
		x = vec3_t(1,0,0);
	else
		x.normalize();
	vec3_t y = z.crossProduct( x );
	vec3_t p = actualPos;

	float m[16] = 
	{
		x.x, x.y, x.z, 0,
		y.x, y.y, y.z, 0,
		z.x, z.y, z.z, 0,
		p.x, p.y, p.z, 1
	};
	trf = mat44_t( m );
	
	if ( fabsf( camera_rollangle ) > 0.0f )
	{
		quat_t ar = quat_t::fromAxisRot( z, 180.0f * camera_rollangle / (float) M_PI );
		mat44_t mar;
		mar.setRotation( ar.rotMatrix() );
		trf = mar * trf;
		trf.setTranslation( p );
	}
	
	view = trf.inverse();
	viewproj = proj * view;
	
	// caculate frustum clipping info.
	vec3_t boundl( -tanf( fovx_radians*0.5f ), 0, -1 );
	vec3_t boundr(  tanf( fovx_radians*0.5f ), 0, -1 );
	boundl.normalize();
	boundr.normalize();

#if 0
	const vec3_t tboundl = trf * boundl;
	const vec3_t tboundr = trf * boundr;
	LOGI( "tboundl %f,%f,%f   tboundr %f,%f,%f", tboundl.x, tboundl.y, tboundl.z, tboundr.x, tboundr.y, tboundr.z );
#endif

	const vec3_t yplus(0,1,0);
	const vec3_t norml = yplus.crossProduct( boundl );
	const vec3_t normr = boundr.crossProduct( yplus );
	tnorml = trf * norml;
	tnormr = trf * normr;

	// now do the same calc, without a COI, which we will use in the library view
    
	z = ( toCam * dist );
	z.normalize();
	x = up.crossProduct( z );
	x.normalize();
	y = z.crossProduct( x );
	p = toCam * dist;
	float m2[16] = 
	{
		x.x, x.y, x.z, 0,
		y.x, y.y, y.z, 0,
		z.x, z.y, z.z, 0,
		p.x, p.y, p.z, 1
	};
	trfNoShift = mat44_t( m2 );
}


vec3_t camera_aim( void )
{
	const float e = elevationAngle - 0.40f;
	const vec3_t aim
	(
		cosf( orbitAngle ) * cosf( e ) ,
		sinf( orbitAngle ) * cosf( e ),
		-sinf( e )
	);
	return aim;
}


float camera_elevationAngle( void )
{
	return elevationAngle;
}


void camera_setElevationAngle( float a )
{
	elevationAngle = a;
}



float camera_orbitAngle( void )
{
	return orbitAngle;
}


void camera_setMinZ( float height )
{
	minZ = height;
}


void camera_setRollAngle( float a )
{
	camera_rollangle = a;
}


float camera_depthInZBuffer(vec3_t pt)
{
	vec4_t p(pt.x, pt.y, pt.z, 1.0f);
	vec4_t xp = viewproj * p;
	xp = xp * (1.0f / xp.w);
	return (1.0f + xp.z) * 0.5f;	// map from -1..1 to 0..1
}

