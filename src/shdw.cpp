// shdw.cpp
//
// Maintains the shadow buffer (bound to depth texture)

#include "shdw.h"

#include "checkogl.h"

#include <stdlib.h>


#include "logx.h"


#define SHADOWBUFFERSIZE	2048


unsigned int shdw_texture=0;
bool shdw_completeframebufferfix=
#if defined( JS ) || defined( OCVR )
	true;
#else
	false;
#endif

static GLuint	shadowFramebuffer=0;
static GLuint	shadowDepth=0;		// Only used if GL implementation does not support depth texture


static unsigned int* colorbuffermem=0;
static unsigned int* depthbuffermem=0;

bool shdw_createFramebuffer( bool supportsDepthTexture )
{
	shadowDepth = 0;

	// create the framebuffer
	glGenFramebuffers( 1, &shadowFramebuffer );
	CHECK_OGL_RELEASE
	glBindFramebuffer( GL_FRAMEBUFFER, shadowFramebuffer );
	CHECK_OGL_RELEASE
	//glPixelStorei( GL_UNPACK_ALIGNMENT, 1 ); // ES analyzer told me to do this.

	// create the texture
	glGenTextures( 1, &shdw_texture );
	CHECK_OGL_RELEASE
	glBindTexture( GL_TEXTURE_2D, shdw_texture );
	CHECK_OGL_RELEASE
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	CHECK_OGL_RELEASE
	if ( !supportsDepthTexture )
	{
#if defined( MSWIN ) || defined( MAC ) || defined( JS ) || defined( IPHN ) || defined( APTV ) || ( defined( USEES2 ) && defined( XWIN ) )
		LOGE( "No depth texture support." );
		ASSERT( supportsDepthTexture );
#else
		colorbuffermem = (unsigned int*)malloc( 2 * sizeof(unsigned int) * SHADOWBUFFERSIZE * SHADOWBUFFERSIZE );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, SHADOWBUFFERSIZE, SHADOWBUFFERSIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, colorbuffermem );
		CHECK_OGL_RELEASE
#endif
	}
	else
	{
		GLvoid* pixels = 0;
#if defined( IPHNX )
		pixels = (unsigned int*)malloc( sizeof(int) * SHADOWBUFFERSIZE * SHADOWBUFFERSIZE );
#endif

#if defined( USEES2 )
		// OpenGLES 2 does not have GL_DEPTH_COMPONENT24.
		GLint internal_fmt = GL_DEPTH_COMPONENT;
#elif defined( USECOREPROFILE ) || defined( USEES3 )
		GLint internal_fmt = GL_DEPTH_COMPONENT32F;
#else
		GLint internal_fmt = GL_DEPTH_COMPONENT24;
#endif

#if ( defined( IPHN ) || defined( APTV ) || defined( ANDROID ) ) && defined( USEES3 )
		internal_fmt = GL_DEPTH_COMPONENT24;
#endif

		glTexImage2D
		(
			GL_TEXTURE_2D,				// target
			0,					// level
			internal_fmt,				// internal format
		 	SHADOWBUFFERSIZE, SHADOWBUFFERSIZE,	// width, height
		 	0,					// border
		 	GL_DEPTH_COMPONENT,			// format
		 	GL_UNSIGNED_INT,			// type
		 	pixels					// pixels
		);
		CHECK_OGL_RELEASE
	}
	glBindTexture( GL_TEXTURE_2D, 0 );
	CHECK_OGL_RELEASE

#if defined( MSWIN ) || defined( MAC )
	// Depth buffer only, so we will not be drawing or reading colours.
	glReadBuffer( GL_NONE );
	CHECK_OGL_RELEASE
	glDrawBuffer( GL_NONE );
	CHECK_OGL_RELEASE
#endif

	// attach the texture
	if ( !supportsDepthTexture )
	{
		glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, shdw_texture, 0 );
		CHECK_OGL_RELEASE
		LOGI( "Attached shadow texture with id 0x%x as color to shadowFramebuffer with id 0x%x", shdw_texture, shadowFramebuffer );
	}
	else
	{
		glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shdw_texture, 0 );
		CHECK_OGL_RELEASE
		LOGI( "Attached shadow texture with id 0x%x as depth to shadowFramebuffer with id 0x%x", shdw_texture, shadowFramebuffer );
		if ( shdw_completeframebufferfix )
		{
			// Workaround for incomplete framebuffer thing in browsers. May not always work though :-(
			// The real bug may be the missing glDrawBuffer() symbol when emscripten links.
			GLuint colr_texture=0;
			glGenTextures( 1, &colr_texture );
			glBindTexture( GL_TEXTURE_2D, colr_texture );
			CHECK_OGL_RELEASE
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,  GL_CLAMP_TO_EDGE );
			CHECK_OGL_RELEASE
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, SHADOWBUFFERSIZE, SHADOWBUFFERSIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0 );
			CHECK_OGL_RELEASE
			glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colr_texture, 0 );
			LOGI( "Attached colour texture with id 0x%x as colour to shadowFramebuffer with id 0x%x", colr_texture, shadowFramebuffer );
		}
	}
 
	// check for success
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER) ;

	switch( status )
	{
	case GL_FRAMEBUFFER_COMPLETE:
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT :
		LOGE( "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT" );
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT  :
		LOGE( "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT" );
		break;
	case GL_FRAMEBUFFER_UNSUPPORTED :
		LOGE( "GL_FRAMEBUFFER_UNSUPPORTED" );
		break;
#if defined( MSWIN ) || defined( JS ) || defined( MAC )
	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER :
		LOGE( "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER" );
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER :
		LOGE( "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER" );
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE :
		LOGE( "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE" );
		break;
	case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS :
		LOGE( "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS" );
		break;
	case GL_FRAMEBUFFER_UNDEFINED :
		LOGE( "GL_FRAMEBUFFER_UNDEFINED" );
		break;
#endif
	default:
		LOGE( "UNKNOWN FRAMEBUFFER STATUS %x", status );
		break;
	}
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		LOGE( "failed to make complete shadowFramebuffer object for shadow. (%x)", status );
#if defined( JS )
		return true;
#else
		return false;
#endif
	}
	else
	{
		LOGI( "Got a complete shadowFramebuffer object with id %x", shadowFramebuffer );
		return true;
	}
}


void shdw_destroyFramebuffer( void )
{
	if ( !shadowFramebuffer ) 
	{
		LOGE( "There is no shadowFramebuffer to destroy." );
		return;
	}
	glBindFramebuffer( GL_FRAMEBUFFER, shadowFramebuffer );
	// unbind texture
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0 );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_2D, 0, 0 );

	// delete the texture
	glDeleteTextures( 1, &shdw_texture );
	CHECK_OGL
	if ( depthbuffermem ) free( depthbuffermem );
	depthbuffermem = 0;
	if ( colorbuffermem ) free( colorbuffermem );
	colorbuffermem = 0;
	shdw_texture = 0;

	if ( shadowDepth )
	{
		glBindRenderbuffer( GL_RENDERBUFFER, 0 ) ;
		glDeleteRenderbuffers( 1, &shadowDepth );
		CHECK_OGL
		shadowDepth = 0;
	}

	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	CHECK_OGL
	glDeleteFramebuffers( 1, &shadowFramebuffer );
	CHECK_OGL

	LOGI( "Destroyed shadow framebufer with id %x", shadowFramebuffer );
	shadowFramebuffer=0;
}


void shdw_use( void )
{
	glBindFramebuffer( GL_FRAMEBUFFER, shadowFramebuffer );
	CHECK_OGL

	// If depth textures are not supported, we need to bind a separate depth buffer to the rgba texture.
	if ( shadowDepth )
		glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, shadowDepth );

	glViewport(0,0,SHADOWBUFFERSIZE,SHADOWBUFFERSIZE);
	CHECK_OGL
}


void shdw_invalidate( void )
{
#if defined(USEES3)
	glBindFramebuffer( GL_FRAMEBUFFER, shadowFramebuffer );
	GLenum attachments[ 1 ] = { GL_DEPTH_ATTACHMENT };
	glInvalidateFramebuffer( GL_FRAMEBUFFER, 1, attachments );
	CHECK_OGL
#endif
}


void shdw_dump( void )
{
	const int w = SHADOWBUFFERSIZE;
	const int h = w;
	const GLuint t0 = shdw_texture;
	shdw_use();
	char fname[256];
	// write out depth map
	if (t0)
	{
		snprintf(fname, sizeof(fname), "shdw-depth.pgm");
		FILE* f = fopen(fname, "wb");
		fprintf(f, "P2\n%d %d\n65535\n", w, h);
		float* vals = new float[w*h];
		glReadPixels(0, 0, w, h, GL_DEPTH_COMPONENT, GL_FLOAT, vals);
		for (int y=0; y<h; ++y)
		{
			float* reader = vals + (h-1-y)*w;
			for (int x=0; x<w; ++x)
			{
				float v = *reader++;
				//v = 5*(v - 0.8);
				v = v < 0 ? 0 : v;
				unsigned int b = (int)(65535.99 * v);
				fprintf(f, "%d ", b);
			}
			fprintf(f,"\n");
		}
		delete [] vals;
		fclose(f);
		LOGI("Wrote depth map to %s", fname);
	}
}
