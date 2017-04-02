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


void quad_prepare( const rendercontext_t& rc )
{
	static int texturemapUniform = glpr_uniform( "texturemap" );
	glActiveTexture ( GL_TEXTURE0 );
	glUniform1i( texturemapUniform, 0 ); // Use texture unit 0
	CHECK_OGL
}


void quad_draw( const char* tag, const rendercontext_t& rc, vec3_t xlat, vec3_t rotx, vec3_t roty )
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

