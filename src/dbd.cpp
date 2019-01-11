//  dbd.cpp
//
//  (c)2012-2017 Abraham Stolk


#include "dbd.h"

#include "checkogl.h"
#include "vmath.h"
#include "rendercontext.h"
#include "glpr.h"


static const int maxv = 32768;
static int numv = 0;
static float vdata[ maxv ][ 3 ];

void dbd_init(void)
{
	dbd_clear();
}


void dbd_clear(void)
{
	numv = 0;
}


void dbd_line( const vec3_t fr, const vec3_t to )
{
	if ( numv < maxv )
	{
		vdata[ numv ][ 0 ] = fr.x;
		vdata[ numv ][ 1 ] = fr.y;
		vdata[ numv ][ 2 ] = fr.z;
		++numv;
		vdata[ numv ][ 0 ] = to.x;
		vdata[ numv ][ 1 ] = to.y;
		vdata[ numv ][ 2 ] = to.z;
		++numv;
	}
}


void dbd_box( const vec3_t& p, const vec3_t& x, const vec3_t& y, const vec3_t& z )
{
	dbd_line( p, p+x );
	dbd_line( p+x, p+x+y );
	dbd_line( p+x+y, p+y );
	dbd_line( p+y, p );

	dbd_line( p+z, p+z+x );
	dbd_line( p+z+x, p+z+x+y );
	dbd_line( p+z+x+y, p+z+y );
	dbd_line( p+z+y, p+z );

	dbd_line( p, p+z );
	dbd_line( p+x, p+x+z );
	dbd_line( p+x+y, p+x+y+z );
	dbd_line( p+y, p+y+z );
}


void dbd_box( const mat44_t& m )
{
	vec3_t x = m.getRow( 0 );
	vec3_t y = m.getRow( 1 );
	vec3_t z = m.getRow( 2 );
	vec3_t p = m.getRow( 3 );
	dbd_box( p - x*0.5f - y*0.5f - z*0.5f, x, y, z );
}


void dbd_circlez( const mat44_t& m )
{
	const int nump = 12;
	vec4_t pts[ nump ];
	for ( int i=0; i<nump; ++i )
	{
		const float a = M_PI * 2.0f * i / (nump-1.0f);
		pts[ i ] = vec4_t( cosf(a), sinf(a), 0.0f, 1.0f );
	}
	for ( int i=0; i<nump-1; ++i )
	{
		const vec4_t xp0 = m * pts[ i+0 ];
		const vec4_t xp1 = m * pts[ i+1 ];
		dbd_line( vec3_t(xp0[0],xp0[1],xp0[2]), vec3_t(xp1[0],xp1[1],xp1[2]) );
	}
}


void dbd_sphere( const vec3_t pos, float r )
{
	mat44_t m;

	m.setRows
	(
		vec3_t( r,0,0 ),
		vec3_t( 0,r,0 ),
		vec3_t( 0,0,r ),
		pos
	);
	dbd_circlez( m );

	m.setRows
	(
		vec3_t( r,0,0 ),
		vec3_t( 0,0,r ),
		vec3_t( 0,r,0 ),
		pos
	);
	dbd_circlez( m );

	m.setRows
	(
		vec3_t( 0,r,0 ),
		vec3_t( 0,0,r ),
		vec3_t( r,0,0 ),
		pos
	);
	dbd_circlez( m );
}


void dbd_crosshairs( const vec3_t& p, float sz )
{
	dbd_line( p-vec3_t(sz,0,0), p+vec3_t(sz,0,0) );
	dbd_line( p-vec3_t(0,sz,0), p+vec3_t(0,sz,0) );
	dbd_line( p-vec3_t(0,0,sz), p+vec3_t(0,0,sz) );
}


void dbd_vector( const vec3_t& fr, const vec3_t& to )
{
	const vec3_t d = to-fr;
	const float l = d.length();
	if ( l <= 0.0f )
		return;
	vec3_t d0 = d * ( 1.0f / l );
	vec3_t d1;
	if ( fabsf( d[0] ) >= fabsf( d[1] ) && fabsf( d[0] ) >= fabsf( d[2] ) )
		d1 = vec3_t( 0,1,0 );	// mainly in x: choose y.
	else if ( fabsf( d[1] ) >= fabsf( d[0] ) && fabsf( d[1] ) >= fabsf( d[2] ) )
		d1 = vec3_t(0,0,1);	// mainly in y: choose z.
	else
		d1 = vec3_t(1,0,0);	// mainly in z: choose x.
	vec3_t d2 = d0.crossProduct( d1 );
	d2.normalize();
	d1 = d0.crossProduct( d2 );
	const float m = 0.1f * l;
	const vec3_t p0 = fr + d*0.8f + d1*m;
	const vec3_t p1 = fr + d*0.8f - d1*m;
	const vec3_t p2 = fr + d*0.8f + d2*m;
	const vec3_t p3 = fr + d*0.8f - d2*m;
	dbd_line( fr, to );
	dbd_line( to, p0 );
	dbd_line( to, p1 );
	dbd_line( to, p2 );
	dbd_line( to, p3 );
	dbd_line( p0, p1 );
	dbd_line( p2, p3 );
}


int dbd_draw_edge( const rendercontext_t& rc )
{
	static int mcvpUniform = glpr_uniform( "modelcamviewprojmat" );
	if ( numv )
	{
		GLuint vbo=0;
#ifdef USE_VAO
		GLuint vao=0;
		glGenVertexArrays( 1, &vao );
		CHECK_OGL
		glBindVertexArray( vao );
		CHECK_OGL
#endif
		glGenBuffers( 1, &vbo );
		CHECK_OGL
		glBindBuffer( GL_ARRAY_BUFFER, vbo );
		CHECK_OGL
		glEnableVertexAttribArray( ATTRIB_VERTEX );
		CHECK_OGL
		glBufferData( GL_ARRAY_BUFFER, numv*3*sizeof(float), (void*)vdata, GL_STREAM_DRAW );
		glVertexAttribPointer( ATTRIB_VERTEX, 3, GL_FLOAT, 0, 3 * sizeof(float), (void*) 0 );
		CHECK_OGL

		mat44_t modelCamViewProjMat = rc.camproj * rc.camview;
		glUniformMatrix4fv( mcvpUniform, 1, false, modelCamViewProjMat.data);
		CHECK_OGL

		glDrawArrays( GL_LINES, 0, numv );
		CHECK_OGL

#ifdef USE_VAO
		glBindVertexArray( 0 );
		CHECK_OGL
		glDeleteVertexArrays( 1, &vao );
		CHECK_OGL
#endif
		glBindBuffer( GL_ARRAY_BUFFER, 0 );
		CHECK_OGL

		glDeleteBuffers( 1, &vbo );
		CHECK_OGL
	}
	return numv;
}


