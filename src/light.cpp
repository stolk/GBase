//
//  light.cpp
//
// (c)2012 Abraham Stolk

#include "light.h"


static mat44_t trf;
static mat44_t view;		// view transformation is the inverse of the light transformation.
static mat44_t proj;		// projection transformation (ortho cam)
	
static float  projectionSize;
static vec3_t pos;
static vec3_t coi;
	
static bool validViewMat;	// is the view transformation valid, or does it need recalculation from transform mat?

void light_init(void)
{
	trf.identity();
	view.identity();
	proj.identity();

	pos = vec3_t( 0, 0, 50 );
	coi = vec3_t( 0, 0, 0 );

	light_setProjectionSize( 40, 10, 290 );
}


void light_setProjectionSize( float projsz, float near, float far )
{
	projectionSize = projsz;
	proj.identity();

	const float left  = -projectionSize;
	const float right =  projectionSize;
	const float bottom= -projectionSize;
	const float top   =  projectionSize;
	
	const float r_l = right - left;
	const float t_b = top - bottom;
	const float f_n = far - near;
	const float tx = - (right + left) / (right - left);
	const float ty = - (top + bottom) / (top - bottom);
	const float tz = - (far + near) / (far - near);
	
	float* mout = proj.data;
	
	mout[0] = 2.0f / r_l;
	mout[1] = 0.0f;
	mout[2] = 0.0f;
	mout[3] = 0.0f;
	
	mout[4] = 0.0f;
	mout[5] = 2.0f / t_b;
	mout[6] = 0.0f;
	mout[7] = 0.0f;
	
	mout[8] = 0.0f;
	mout[9] = 0.0f;
	mout[10] = -2.0f / f_n;
	mout[11] = 0.0f;
	
	mout[12] = tx;
	mout[13] = ty;
	mout[14] = tz;
	mout[15] = 1.0f;	
	
	validViewMat = false;
}


void light_getProjTransform(mat44_t& mat)
{
	memcpy( mat, proj, sizeof(mat44_t) );
}


void light_getViewTransform(mat44_t& mat)
{	
	memcpy( mat, view, sizeof(mat44_t) );
}


void light_setPos(const vec3_t& p)
{
	pos = p;
	validViewMat = false;
}


vec3_t light_pos(void)
{
	return pos;
}


vec3_t light_dir(void)
{
	vec3_t d = coi - pos;
	d.normalize();
	return d;
}


void light_setCOI(const vec3_t& c)
{
	coi = c;
	validViewMat = false;
}


void light_update(float dt)
{	
	if ( !validViewMat )
	{
		vec3_t z = pos - coi;

		z.normalize();
		vec3_t up( 0,1,0 );
		vec3_t x = up.crossProduct( z );
		x.normalize();
		vec3_t y = z.crossProduct( x );
		const vec3_t& p = pos;
		float m[16] = 
		{
			x.x, x.y, x.z, 0,
			y.x, y.y, y.z, 0,
			z.x, z.y, z.z, 0,
			p.x, p.y, p.z, 1
		};
		trf = mat44_t( m );

		view = trf.inverse();

		validViewMat = true;
	}
}

