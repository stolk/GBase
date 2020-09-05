//
// spotlight.cpp
//
// (c)2020 Abraham Stolk

#include "spotlight.h"

#include <math.h>


static mat44_t sl_trf;
static mat44_t sl_view;		// view transformation is the inverse of the light transformation.
static mat44_t sl_proj;		// projection transformation (ortho cam)
	
static vec3_t sl_pos;
static vec3_t sl_coi;


void spotlight_init(void)
{
	sl_trf.identity();
	sl_view.identity();
	sl_proj.identity();

	sl_pos = vec3_t( 0, 0, 50 );
	sl_coi = vec3_t( 0, 0, 0 );
	spotlight_setProjection( 0.04f * M_PI, 1, 1200.0f );
}


void spotlight_setProjection( float fov, float zNear, float zFar )
{
	sl_proj.identity();

#if 1
	// create an orthogonal projection matrix
	const float f = 1.0f / tanf(fov/2.0f);
	const float aspect = 1.0f;
	float* mout = sl_proj.data;

	mout[0] = f / aspect;
	mout[1] = 0.0f;
	mout[2] = 0.0f;
	mout[3] = 0.0f;

	mout[4] = 0.0f;
	mout[5] = f;
	mout[6] = 0.0f;
	mout[7] = 0.0f;

	mout[8] = 0.0f;
	mout[9] = 0.0f;
	mout[10] = (zFar+zNear) / (zNear-zFar);
	mout[11] = -1.0f;

	mout[12] = 0.0f;
	mout[13] = 0.0f;
	mout[14] = 2 * zFar * zNear /  (zNear-zFar);
	mout[15] = 0.0f;
#else
	// create a perspective projection matrix
        const float projectionSize = 50.0f;

        const float left  = -projectionSize;
        const float right =  projectionSize;
        const float bottom= -projectionSize;
        const float top   =  projectionSize;

        const float r_l = right - left;
        const float t_b = top - bottom;
        const float f_n = zFar - zNear;
        const float tx = - (right + left) / (right - left);
        const float ty = - (top + bottom) / (top - bottom);
        const float tz = - (zFar + zNear) / (zFar - zNear);

        float* mout = sl_proj.data;

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
#endif
}


void spotlight_getProjTransform(mat44_t& mat)
{
	memcpy( mat, sl_proj, sizeof(mat44_t) );
}


void spotlight_getViewTransform(mat44_t& mat)
{	
	memcpy( mat, sl_view, sizeof(mat44_t) );
}


void spotlight_setPos(const vec3_t& p)
{
	sl_pos = p;
}


vec3_t spotlight_pos(void)
{
	return sl_pos;
}


vec3_t spotlight_dir(void)
{
	vec3_t d = sl_coi - sl_pos;
	d.normalize();
	return d;
}


void spotlight_setCOI(const vec3_t& c)
{
	sl_coi = c;
}


void spotlight_update(float dt)
{	
	vec3_t z = sl_pos - sl_coi;

	z.normalize();
	vec3_t up( 0,1,0 );
	vec3_t x = up.crossProduct( z );
	x.normalize();
	vec3_t y = z.crossProduct( x );
	const vec3_t& p = sl_pos;
	float m[16] = 
	{
		x.x, x.y, x.z, 0,
		y.x, y.y, y.z, 0,
		z.x, z.y, z.z, 0,
		p.x, p.y, p.z, 1
	};
	sl_trf = mat44_t( m );

	sl_view = sl_trf.inverse();
}

