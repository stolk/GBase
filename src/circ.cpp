#include "circ.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "checkogl.h"
#include "txdb.h"
#include "glpr.h"

static unsigned int vbo_size=0;
static unsigned int ibo_size=0;
static unsigned int vbo=0;
static unsigned int ibo=0;
#if defined( USE_VAO )
static unsigned int vao=0;
#endif
static int numv=0;
static float* vdata=0;
static unsigned int* idata=0;


void circ_init( int numtria )
{
	if ( !vbo )
	{
		float da = M_PI * 2.0f / numtria;
		numv = numtria + 2;
		const int stride = 4;
		vbo_size = numv * sizeof(float) * stride;
		ibo_size = numtria * 3 * sizeof( unsigned int );
		vdata = (float*) malloc( vbo_size );
		idata = (unsigned int*) malloc( ibo_size );
		float* writer = vdata;

		*writer++ = 0.0f;	// x
		*writer++ = 0.0f;	// y
		*writer++ = 0.0f;	// u
		*writer++ = 0.0f;	// v

		for ( int i=0; i<= numtria; ++i )
		{
			float x = cosf( da * i );
			float y = sinf( da * i );
			*writer++ = x;	// x
			*writer++ = y;	// y
			*writer++ = x;	// u
			*writer++ = y;	// v
		}

		unsigned int* iwriter = idata;
		for ( int i=0; i<numtria; ++i )
		{
			*iwriter++ = 0;
			*iwriter++ = i+1;
			*iwriter++ = i+2;
		}

#if defined( USE_VAO )
		glGenVertexArrays( 1, &vao );
		glBindVertexArray( vao );
		CHECK_OGL
#endif

		glGenBuffers( 1, &vbo );
		assert( vbo );
		glGenBuffers( 1, &ibo );
		assert( ibo );

		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo );
		glBufferData
		(
			GL_ELEMENT_ARRAY_BUFFER,
			ibo_size,
			idata,
			GL_STATIC_DRAW
		);
		CHECK_OGL

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
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
#endif
		CHECK_OGL
	}
}


void circ_exit( void )
{
	if ( vbo )
	{
#if defined( USE_VAO )
		glDeleteVertexArrays( 1, &vao );
#endif
		glDeleteBuffers( 1, &vbo );
		glDeleteBuffers( 1, &ibo );
		CHECK_OGL
		vbo = 0;
		ibo = 0;
	}
}


void circ_draw( const rendercontext_t& rc, vec3_t xlat, vec3_t rotx, vec3_t roty, vec3_t xlatuv, vec3_t rotu, vec3_t rotv )
{
	static int rotxUniform = glpr_uniform( "rotx" );
	static int rotyUniform = glpr_uniform( "roty" );
	static int rotuUniform = glpr_uniform( "rotu" );
	static int rotvUniform = glpr_uniform( "rotv" );
	static int translationUniform = glpr_uniform( "translation" );
	static int translationuvUniform = glpr_uniform( "translationuv" );
	static int texturemapUniform = glpr_uniform( "texturemap" );

	glUniform2f( rotxUniform, rotx.x, rotx.y );
	glUniform2f( rotyUniform, roty.x, roty.y );
	glUniform2f( translationUniform, xlat.x, xlat.y );
	glUniform2f( rotuUniform, rotu.x, rotu.y );
	glUniform2f( rotvUniform, rotv.x, rotv.y );
	glUniform2f( translationuvUniform, xlatuv.x, xlatuv.y );
	glUniform1i( texturemapUniform, 0 ); // use texture unit 0
	CHECK_OGL

#if defined( USE_VAO )
	glBindVertexArray( vao );
	CHECK_OGL
#else
	glBindBuffer( GL_ARRAY_BUFFER, vbo );
	CHECK_OGL
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo );
	CHECK_OGL

	const int stride = 4;

	glVertexAttribPointer( ATTRIB_VERTEX, 2, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*) 0 );
	glEnableVertexAttribArray( ATTRIB_VERTEX );
    
	glVertexAttribPointer( ATTRIB_UV, 2, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*) ( 2 * sizeof(float) ) );
	glEnableVertexAttribArray( ATTRIB_UV );
#endif

	const int totaltria = ibo_size / (3*sizeof(uint32_t));
	glDrawElements
	(
		GL_TRIANGLES,
		totaltria*3,
		GL_UNSIGNED_INT,
		0
	);
	CHECK_OGL

#if defined( USE_VAO )
	glBindVertexArray( 0 );
#else
	glDisableVertexAttribArray( ATTRIB_VERTEX );
	glDisableVertexAttribArray( ATTRIB_UV );

	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
#endif

	CHECK_OGL
}


