#include "quad.h"

#include <stdio.h>
#include <stdlib.h>

#include "checkogl.h"
#include "txdb.h"
#include "glpr.h"

static unsigned int vbo_size=0;
static unsigned int vbo=0;
#if defined( USE_VAO )
static unsigned int vao=0;
#endif
static int numv=0;
static float* vdata=0;


void quad_init( void )
{
	if ( !vbo )
	{
		numv = 6;
		const int stride = 4;
		vbo_size = numv * sizeof(float) * stride;
		vdata = (float*) malloc( vbo_size );
		float* writer = vdata;

		*writer++ = 1.0;	// x
		*writer++ = 1.0;	// y
		*writer++ = 1.0;	// u
		*writer++ = 1.0;	// v

		*writer++ = -1.0;	// x
		*writer++ = 1.0;	// y
		*writer++ = 0.0;	// u
		*writer++ = 1.0;	// v

		*writer++ = -1.0;	// x
		*writer++ = -1.0;	// y
		*writer++ = 0.0;	// u
		*writer++ = 0.0;	// v

		*writer++ = -1.0;	// x
		*writer++ = -1.0;	// y
		*writer++ = 0.0;	// u
		*writer++ = 0.0;	// v

		*writer++ = 1.0;	// x
		*writer++ = -1.0;	// y
		*writer++ = 1.0;	// u
		*writer++ = 0.0;	// v

		*writer++ = 1.0;	// x
		*writer++ = 1.0;	// y
		*writer++ = 1.0;	// u
		*writer++ = 1.0;	// v

#if defined( USE_VAO )
		glGenVertexArrays( 1, &vao );
		glBindVertexArray( vao );
		CHECK_OGL
#endif

		glGenBuffers( 1, &vbo );
		assert( vbo );

		glBindBuffer( GL_ARRAY_BUFFER, vbo );

		glBufferData( GL_ARRAY_BUFFER, vbo_size, vdata, GL_STATIC_DRAW );

#if defined( USE_VAO )
		glVertexAttribPointer( ATTRIB_VERTEX, 2, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*) 0 );
		glVertexAttribPointer( ATTRIB_UV, 2, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*) ( 2 * sizeof(float) ) );
		
		glEnableVertexAttribArray( ATTRIB_VERTEX );
		glEnableVertexAttribArray( ATTRIB_UV );

		glBindVertexArray( 0 );
#else
		glBindBuffer( GL_ARRAY_BUFFER, 0 );
#endif
		CHECK_OGL
	}
}


void quad_exit( void )
{
	if ( vbo )
	{
#if defined( USE_VAO )
		glDeleteVertexArrays( 1, &vao );
#endif
		glDeleteBuffers( 1, &vbo );
		CHECK_OGL
		vbo = 0;
	}
}


void quad_prepare( void )
{
	static int texturemapUniform = glpr_uniform( "texturemap" );
	glActiveTexture ( GL_TEXTURE0 );
	glUniform1i( texturemapUniform, 0 ); // Use texture unit 0
	CHECK_OGL
}


void quad_draw( const char* tag, vec3_t xlat, vec3_t rotx, vec3_t roty )
{
	txdb_use( tag );
	CHECK_OGL

	static int rotxUniform = glpr_uniform( "rotx" );
	static int rotyUniform = glpr_uniform( "roty" );
	static int translationUniform = glpr_uniform( "translation" );
	glUniform2f( rotxUniform, rotx.x, rotx.y );
	glUniform2f( rotyUniform, roty.x, roty.y );
	glUniform2f( translationUniform, xlat.x, xlat.y );
	CHECK_OGL
	
#if defined( USE_VAO )
	glBindVertexArray( vao );
	CHECK_OGL
#else
	glBindBuffer( GL_ARRAY_BUFFER, vbo );
	CHECK_OGL

	const int stride = 4;

	glVertexAttribPointer( ATTRIB_VERTEX, 2, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*) 0 );
	glEnableVertexAttribArray( ATTRIB_VERTEX );
    
	glVertexAttribPointer( ATTRIB_UV, 2, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*) ( 2 * sizeof(float) ) );
	glEnableVertexAttribArray( ATTRIB_UV );
#endif

	const int offset = 0;
	glDrawArrays( GL_TRIANGLES, offset, numv );
	CHECK_OGL

#if defined( USE_VAO )
	glBindVertexArray( 0 );
#else
	glDisableVertexAttribArray( ATTRIB_VERTEX );
	glDisableVertexAttribArray( ATTRIB_UV );

	glBindBuffer( GL_ARRAY_BUFFER, 0 );
#endif

	CHECK_OGL
}


void quad_draw_set( const char* tag, int cnt, const vec3_t* xlat, const vec3_t* rotx, const vec3_t* roty )
{
	if ( cnt <= 0 ) return;

	txdb_use( tag );
	CHECK_OGL

#if defined( USE_VAO )
	glBindVertexArray( vao );
	CHECK_OGL
#else
	glBindBuffer( GL_ARRAY_BUFFER, vbo );
	CHECK_OGL
	const int stride = 4;

	glVertexAttribPointer( ATTRIB_VERTEX, 2, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*) 0 );
	glEnableVertexAttribArray( ATTRIB_VERTEX );
    
	glVertexAttribPointer( ATTRIB_UV, 2, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*) ( 2 * sizeof(float) ) );
	glEnableVertexAttribArray( ATTRIB_UV );
#endif

	for ( int i=0; i<cnt; ++i )
	{
		static int rotxUniform = glpr_uniform( "rotx" );
		static int rotyUniform = glpr_uniform( "roty" );
		static int translationUniform = glpr_uniform( "translation" );
		glUniform2f( rotxUniform, rotx[i].x, rotx[i].y );
		glUniform2f( rotyUniform, roty[i].x, roty[i].y );
		glUniform2f( translationUniform, xlat[i].x, xlat[i].y );
		CHECK_OGL

		const int offset = 0;
		glDrawArrays( GL_TRIANGLES, offset, numv );
		CHECK_OGL
	}

#if defined( USE_VAO )
	glBindVertexArray( 0 );
#else
	glDisableVertexAttribArray( ATTRIB_VERTEX );
	glDisableVertexAttribArray( ATTRIB_UV );

	glBindBuffer( GL_ARRAY_BUFFER, 0 );
#endif

	CHECK_OGL
}


void quad_draw_dof( void )
{
#if defined( USE_VAO )
	glBindVertexArray( vao );
	CHECK_OGL
#else
	glBindBuffer( GL_ARRAY_BUFFER, vbo );
	CHECK_OGL

	const int stride = 4;

	glVertexAttribPointer( ATTRIB_VERTEX, 2, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*) 0 );
	glEnableVertexAttribArray( ATTRIB_VERTEX );
    
	glVertexAttribPointer( ATTRIB_UV, 2, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*) ( 2 * sizeof(float) ) );
	glEnableVertexAttribArray( ATTRIB_UV );
#endif

	const int offset = 0;
	glDrawArrays( GL_TRIANGLES, offset, numv );
	CHECK_OGL

#if defined( USE_VAO )
	glBindVertexArray( 0 );
#else
	glDisableVertexAttribArray( ATTRIB_VERTEX );
	glDisableVertexAttribArray( ATTRIB_UV );

	glBindBuffer( GL_ARRAY_BUFFER, 0 );
#endif

	CHECK_OGL
}


void quad_mk( unsigned int& vaobj, unsigned int& vbobj, float szx, float szy )
{
	glGenVertexArrays( 1, &vaobj );
	glBindVertexArray( vaobj );
	CHECK_OGL
	glGenBuffers( 1, &vbobj );
	glBindBuffer( GL_ARRAY_BUFFER,  vbobj );
	CHECK_OGL
	const float x0 = -szx;
	const float x1 =  szx;
	const float y0 = -szy;
	const float y1 =  szy;
	// six vertices, of 4 floats each (x,y,u,v)
	float vertdata[ 6 * 4 ] =
	{
		x1,y1,1,1,	// x,y,u,v
		x0,y1,0,1,
		x0,y0,0,0,
		x0,y0,0,0,
		x1,y0,1,0,
		x1,y1,1,1,
	};
	glBufferData( GL_ARRAY_BUFFER, sizeof(vertdata), vertdata, GL_STATIC_DRAW );
	CHECK_OGL
	const int stride = 4;
	glVertexAttribPointer( ATTRIB_VERTEX, 2, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*) 0 );
	glVertexAttribPointer( ATTRIB_UV, 2, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*) ( 2 * sizeof(float) ) );
	glEnableVertexAttribArray( ATTRIB_VERTEX );
	glEnableVertexAttribArray( ATTRIB_UV );
	glBindVertexArray( 0 );
}


void quad_draw_array( unsigned int vaobj )
{
	glBindVertexArray( vaobj );
	const int offset = 0;
	glDrawArrays( GL_TRIANGLES, offset, 6 );
}

