// shdw.cpp
//
// Maintains the shadow buffer (bound to depth texture)

#include "shdw.h"

#include "checkogl.h"

#include <stdlib.h>


#include "logx.h"


#if !defined(MAXSHADOWBUFS)
#	define MAXSHADOWBUFS			2
#endif

static int	shdw_w[MAXSHADOWBUFS];
static int	shdw_h[MAXSHADOWBUFS];

static GLuint	shdw_textures[MAXSHADOWBUFS] = { 0,0 };

static GLuint	shadowFramebuffers[MAXSHADOWBUFS] = { 0,0, };

bool shdw_use_hardware_depth_compare = false;
bool shdw_use_hardware_pcf = false;
bool shdw_verbose = true;

int shdw_create_count = 0;
int shdw_destroy_count = 0;
int shdw_valid_count = 0;

//unsigned int shdw_texture=0;


bool shdw_completeframebufferfix=
#if defined( JS ) || defined( OCVR )
	true;
#else
	false;
#endif


bool shdw_createFramebuffer( bool supportsDepthTexture, int nr, int shadoww, int shadowh )
{
	ASSERTM( nr>=0 && nr<MAXSHADOWBUFS, "Shadow buffer nr %d is not in range [0,%d]", nr, MAXSHADOWBUFS );
	GLuint& shadowFramebuffer = shadowFramebuffers[nr];
	GLuint& shdw_texture = shdw_textures[nr];

	shdw_w[nr] = shadoww;
	shdw_h[nr] = shadowh;

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
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	if ( shdw_use_hardware_depth_compare )
	{
#if defined(ANDROID)
        ASSERT( !shdw_use_hardware_depth_compare );
#else
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_GREATER );
#endif
	}
	const GLint filter = shdw_use_hardware_pcf ? GL_LINEAR : GL_NEAREST;
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter );
	CHECK_OGL_RELEASE
	if ( !supportsDepthTexture )
	{
#if defined( MSWIN ) || defined( MAC ) || defined( JS ) || defined( IPHN ) || defined( APTV ) || ( defined( USEES2 ) && defined( XWIN ) )
		LOGE( "No depth texture support." );
		ASSERT( supportsDepthTexture );
#else
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, shadoww, shadowh, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0 );
		CHECK_OGL_RELEASE
#endif
	}
	else
	{
		GLvoid* pixels = 0;
#if defined( USEES2 )
		// OpenGLES 2 does not have GL_DEPTH_COMPONENT24.
		GLint internal_fmt = GL_DEPTH_COMPONENT;
		GLint type = GL_UNSIGNED_INT;
#elif defined( USECOREPROFILE ) || defined( USEES3 )
		GLint internal_fmt = GL_DEPTH_COMPONENT32F;
		GLint type = GL_FLOAT;
#else
		GLint internal_fmt = GL_DEPTH_COMPONENT24;
		GLint type = GL_UNSIGNED_INT;
#endif

#if ( defined( IPHN ) || defined( APTV ) || defined( ANDROID ) ) && defined( USEES3 )
		internal_fmt = GL_DEPTH_COMPONENT24;
#endif

		glTexImage2D
		(
			GL_TEXTURE_2D,				// target
			0,					// level
			internal_fmt,				// internal format
		 	shadoww, shadowh,			// width, height
		 	0,					// border
		 	GL_DEPTH_COMPONENT,			// format
		 	type,					// type
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
		if ( shdw_verbose )
			LOGI( "Attached shadow texture with id 0x%x as color to shadowFramebuffer with id 0x%x", shdw_texture, shadowFramebuffer );
	}
	else
	{
		glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shdw_texture, 0 );
		CHECK_OGL_RELEASE
		if ( shdw_verbose )
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
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
			CHECK_OGL_RELEASE
			glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, shadoww, shadowh, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0 );
			CHECK_OGL_RELEASE
			glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colr_texture, 0 );
			if ( shdw_verbose )
				LOGI( "Attached colour texture with id 0x%x as colour to shadowFramebuffer with id 0x%x", colr_texture, shadowFramebuffer );
		}
	}

	shdw_create_count += 1;
 
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
		if ( shdw_verbose )
			LOGI( "Got a complete shadowFramebuffer object with id %x of size %dx%d", shadowFramebuffer, shadoww, shadowh );
		shdw_valid_count += 1;
		return true;
	}
}


void shdw_destroyFramebuffer( int nr )
{
	GLuint& shadowFramebuffer = shadowFramebuffers[nr];
	GLuint& shdw_texture = shdw_textures[nr];

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
	shdw_texture = 0;

	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	CHECK_OGL
	glDeleteFramebuffers( 1, &shadowFramebuffer );
	CHECK_OGL

	if ( shdw_verbose )
		LOGI( "Destroyed shadow framebufer with id %x", shadowFramebuffer );
	shadowFramebuffer=0;

	shdw_destroy_count += 1;
}


void shdw_use( int nr )
{
	GLuint& shadowFramebuffer = shadowFramebuffers[nr];

	ASSERTM
	(
		shadowFramebuffer>0,
		"No valid framebuffer(%d) for shdw slot %d. create_count %d destroy_count %d valid_count %d",
		shadowFramebuffer, nr, shdw_create_count, shdw_destroy_count, shdw_valid_count
	);

	glBindFramebuffer( GL_FRAMEBUFFER, shadowFramebuffer );
	CHECK_OGL

	glViewport(0,0,shdw_w[nr],shdw_h[nr]);
	CHECK_OGL
}


int shdw_usable( int nr )
{
	GLuint& shadowFramebuffer = shadowFramebuffers[nr];
	return shadowFramebuffer>0 ? 1 : 0;
}


unsigned int shdw_texture(int nr)
{
	return shdw_textures[nr];
}


void shdw_invalidate( int nr )
{
#if defined(USEES3)
	GLuint& shadowFramebuffer = shadowFramebuffers[nr];

	glBindFramebuffer( GL_FRAMEBUFFER, shadowFramebuffer );
	GLenum attachments[ 1 ] = { GL_DEPTH_ATTACHMENT };
	glInvalidateFramebuffer( GL_FRAMEBUFFER, 1, attachments );
	CHECK_OGL
#endif
}


void shdw_dump( int nr )
{
	ASSERT( nr>=0 && nr<MAXSHADOWBUFS );

	GLuint& shdw_texture = shdw_textures[nr];

	const int w = shdw_w[nr];
	const int h = shdw_h[nr];
	ASSERT(w>0);
	ASSERT(h>0);
	const GLuint t0 = shdw_texture;
	shdw_use( nr );
	char fname[256];
	// write out depth map
	if (t0)
	{
		snprintf(fname, sizeof(fname), "shdw-depth-%d.pgm", nr);
		FILE* f = fopen(fname, "wb");
		fprintf(f, "P2\n%d %d\n65535\n", w, h);
		float* vals = new float[w*h];
		glReadPixels(0, 0, w, h, GL_DEPTH_COMPONENT, GL_FLOAT, vals);
		CHECK_OGL
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
		if ( shdw_verbose )
			LOGI("Wrote depth map with id 0x%x to %s", t0, fname);
	}
}

