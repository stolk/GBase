// offsc.cpp
//
// Maintains the offsc buffer (bound to depth texture)
// (c)2016-2017 by Abraham Stolk.

#include "offsc.h"

#include "checkogl.h"

#include <stdlib.h>
#include <string.h>

#include "logx.h"


unsigned int offsc_depth_texture[ MAXFRAMEBUFFERS ];
unsigned int offsc_color_texture[ MAXFRAMEBUFFERS ];

int offsc_w[ MAXFRAMEBUFFERS ];
int offsc_h[ MAXFRAMEBUFFERS ];

static GLuint offscFramebuffer[ MAXFRAMEBUFFERS ];

static const char* offsc_tags[ MAXFRAMEBUFFERS ];


static int offsc_count;

void offsc_init( void )
{
	offsc_count = 0;
}


int offsc_createFramebuffer( int w, int h, bool withdepth )
{
	ASSERT( offsc_count < MAXFRAMEBUFFERS );
	const int idx = offsc_count++;

	offsc_w[idx] = w;
	offsc_h[idx] = h;
	offsc_tags[idx] = 0;

	// create the framebuffer
	glGenFramebuffers( 1, offscFramebuffer+idx );
	glBindFramebuffer( GL_FRAMEBUFFER, offscFramebuffer[idx] );
	//glPixelStorei( GL_UNPACK_ALIGNMENT, 1 ); // ES analyzer told me to do this.

	if ( withdepth )
	{
		// create the texture
		glGenTextures( 1, offsc_depth_texture+idx );
		glBindTexture( GL_TEXTURE_2D, offsc_depth_texture[idx] );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		CHECK_OGL

#if defined( USEES2 )
		GLint internal_fmt = GL_DEPTH_COMPONENT; // OpenGLES 2 does not have GL_DEPTH_COMPONENT24.
#else
		GLint internal_fmt = GL_DEPTH_COMPONENT24;
#endif

		glTexImage2D
		(
			GL_TEXTURE_2D,				// target
			0,					// level
			internal_fmt,				// internal format
		 	w, h,					// width, height
		 	0,					// border
		 	GL_DEPTH_COMPONENT,			// format
		 	GL_UNSIGNED_INT,			// type
		 	0					// pixels
		);
		CHECK_OGL
		glBindTexture( GL_TEXTURE_2D, 0 );
		CHECK_OGL

		//glReadBuffer( GL_NONE );
		//glDrawBuffer( GL_NONE );
		CHECK_OGL
	
		glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, offsc_depth_texture[idx], 0 );
		CHECK_OGL
		LOGI( "Attached depth texture with id 0x%x as depth to offscFramebuffer with id 0x%x", offsc_depth_texture[idx], offscFramebuffer[idx] );
	}

	glGenTextures( 1, offsc_color_texture+idx );
	glBindTexture( GL_TEXTURE_2D, offsc_color_texture[idx] );
	CHECK_OGL
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	//glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
	//glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	//glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	CHECK_OGL
	glTexImage2D
	(
		GL_TEXTURE_2D, 
		0, 
		GL_RGBA, 
		w, h,
		0, 
		GL_RGBA, 
		GL_UNSIGNED_BYTE, 
		0
	);
	CHECK_OGL

	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, offsc_color_texture[idx], 0 );
	LOGI( "Attached colour texture with id 0x%x as colour to offscFramebuffer with id 0x%x", offsc_color_texture[idx], offscFramebuffer[idx] );

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
#if defined( MSWIN ) || defined( JS ) || defined( MAC ) || defined( XWIN )
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
		LOGE( "failed to make complete offscFramebuffer object for offsc. (%x)", status );
		return -1;
	}
	else
	{
		LOGI( "Got a complete offscFramebuffer object with id %x", offscFramebuffer[idx] );
		return idx;
	}
}


#if defined( USECOREPROFILE )
int offsc_createMultiSampleFramebuffer( int w, int h, bool withdepth )
{
	ASSERT( offsc_count < MAXFRAMEBUFFERS );
	const int idx = offsc_count++;

	offsc_w[idx] = w;
	offsc_h[idx] = h;
	offsc_tags[idx] = 0;

	// create the framebuffer
	glGenFramebuffers( 1, offscFramebuffer+idx );
	glBindFramebuffer( GL_FRAMEBUFFER, offscFramebuffer[idx] );

	if ( withdepth )
	{
		// create the texture
		glGenTextures( 1, offsc_depth_texture+idx );
		glBindTexture( GL_TEXTURE_2D_MULTISAMPLE, offsc_depth_texture[idx] );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		CHECK_OGL

#if defined( USEES2 )
		GLint internal_fmt = GL_DEPTH_COMPONENT; // OpenGLES 2 does not have GL_DEPTH_COMPONENT24.
#else
		GLint internal_fmt = GL_DEPTH_COMPONENT24;
#endif

		glTexImage2DMultisample
		(
			GL_TEXTURE_2D_MULTISAMPLE,		// target
			4,					// numsamples
			internal_fmt,				// internal format
		 	w, h,					// width, height
		 	0					// border
		);

		CHECK_OGL
		glBindTexture( GL_TEXTURE_2D_MULTISAMPLE, 0 );
		CHECK_OGL

		//glReadBuffer( GL_NONE );
		//glDrawBuffer( GL_NONE );
		CHECK_OGL
	
		glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, offsc_depth_texture[idx], 0 );
		CHECK_OGL
		LOGI( "Attached msaa depth texture with id 0x%x as depth to offscFramebuffer with id 0x%x", offsc_depth_texture[idx], offscFramebuffer[idx] );
	}

	glGenTextures( 1, offsc_color_texture+idx );
	glBindTexture( GL_TEXTURE_2D_MULTISAMPLE, offsc_color_texture[idx] );
	CHECK_OGL
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	CHECK_OGL
	glTexImage2DMultisample
	(
		GL_TEXTURE_2D_MULTISAMPLE,
		4,
		GL_RGBA8, 
		w, h,
		0
	);
	CHECK_OGL

	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, offsc_color_texture[idx], 0 );
	LOGI( "Attached msaa colour texture with id 0x%x as colour to offscFramebuffer with id 0x%x", offsc_color_texture[idx], offscFramebuffer[idx] );

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
#if defined( MSWIN ) || defined( JS ) || defined( MAC ) || defined( XWIN )
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
		LOGE( "failed to make complete offscFramebuffer object for offsc. (%x)", status );
		return -1;
	}
	else
	{
		LOGI( "Got a complete offscFramebuffer object with id %x size %dx%d", offscFramebuffer[idx], w, h );
		return idx;
	}
}
#endif


void offsc_destroyFramebuffer( int idx )
{
	if ( !offscFramebuffer[idx] ) 
	{
		LOGE( "There is no offscFramebuffer to destroy at idx %d.", idx );
		return;
	}
	glBindFramebuffer( GL_FRAMEBUFFER, offscFramebuffer[idx] );
	// unbind texture
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0 );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_2D, 0, 0 );

	// delete the texture
	glDeleteTextures( 1, offsc_depth_texture+idx );
	CHECK_OGL
	glDeleteTextures( 1, offsc_color_texture+idx );
	CHECK_OGL

	offsc_depth_texture[idx] = 0;
	offsc_color_texture[idx] = 0;

	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	CHECK_OGL

	glDeleteFramebuffers( 1, offscFramebuffer+idx );
	CHECK_OGL

	LOGI( "Destroyed offsc framebufer with id %x", offscFramebuffer[idx] );
	offscFramebuffer[idx]=0;
	offsc_tags[idx] = 0;
}


void offsc_use( int idx )
{
	ASSERT( idx >= 0 && idx < offsc_count );
	glBindFramebuffer( GL_FRAMEBUFFER, offscFramebuffer[idx] );
	CHECK_OGL
}


void offsc_tag( int idx, const char* nm )
{
	ASSERT( idx >= 0 && idx < offsc_count );
	offsc_tags[idx] = nm;
}


void offsc_resolve_msaa( int src, int dst )
{
	GLuint src_buf = offscFramebuffer[ src ];
	GLuint dst_buf = offscFramebuffer[ dst ];
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst_buf);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, src_buf);
	const int srcw = offsc_w[ src ];
	const int srch = offsc_h[ src ];
	const int dstw = offsc_w[ dst ];
	const int dsth = offsc_h[ dst ];
	ASSERT( dstw >= srcw );
	ASSERT( dsth >= srch );
	glBlitFramebuffer(0, 0, srcw, srch, 0, 0, srcw, srch, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
}


void offsc_dump_framebuffer( const char* rgbname, const char* aname, const char* zname, int w, int h )
{
	if (zname && *zname)
	{
		FILE* f = fopen(zname, "wb");
		fprintf(f, "P2\n%d %d\n65535\n", w, h);
		float* vals = new float[w*h];
		glReadPixels(0, 0, w, h, GL_DEPTH_COMPONENT, GL_FLOAT, vals);
		for (int y=0; y<h; ++y)
		{
			float* reader = vals + (h-1-y)*w;
			for (int x=0; x<w; ++x)
			{
				float v = *reader++;
				v = 5*(v - 0.8);
				v = v < 0 ? 0 : v;
				unsigned int b = (int)(65535.99 * v);
				fprintf(f, "%d ", b);
			}
			fprintf(f,"\n");
		}
		delete [] vals;
		fclose(f);
		LOGI("Wrote depth map to %s", zname);
	}

	float* vals = 0;
	vals = new float[w*h*4];
	glReadPixels(0,0, w,h, GL_RGBA, GL_FLOAT, vals);

	if (aname && *aname)
	{
		FILE* f = fopen(aname, "wb");
		fprintf(f, "P6\n%d %d\n255\n", w, h);
		for (int y=0; y<h; ++y)
		{
			float* reader = vals + (h-1-y)*w*4;
			for (int x=0; x<w; ++x)
			{
				float a = reader[3];
				float r = 0.0f;
				float g = a > 0.5f ? (a-0.5f) * 2.0f : 0.0f;
				float b = a > 0.5f ? 0.0f : 2.0f * (0.5f-a);
				fputc( r*255.99f, f );
				fputc( g*255.99f, f );
				fputc( b*255.99f, f );
				reader += 4;
			}
		}
		fclose(f);
		LOGI("Wrote alpha map to %s", aname);
	}

	if (rgbname && *rgbname)
	{
		FILE* f = fopen(rgbname, "wb");
		fprintf(f, "P6\n%d %d\n255\n", w, h);
		for (int y=0; y<h; ++y)
		{
			float* reader = vals + (h-1-y)*w*4;
			for (int x=0; x<w; ++x)
			{
				fputc( reader[0]*255.99f, f);
				fputc( reader[1]*255.99f, f);
				fputc( reader[2]*255.99f, f);
				reader += 4;
			}
		}
		fclose(f);
		LOGI("Wrote colour map to %s", rgbname);
	}

	delete [] vals;
}


void offsc_dump( int first, int cnt )
{
	if ( cnt < 0 ) cnt = offsc_count;
	for (int i=first; i<cnt; ++i)
	{
		char rgbname[128];
		char aname[128];
		char zname[128];

		const int w = offsc_w[i];
		const int h = offsc_h[i];
		//const GLuint t0 = offsc_color_texture[i];
		const GLuint t1 = offsc_depth_texture[i];
		offsc_use(i);

		rgbname[0]=0;
		aname[0]=0;
		zname[0]=0;

		const char* tag = "untagged";
		if ( offsc_tags[i] )
			tag = offsc_tags[i];

		if ( strstr( tag, "msaa" ) )
		{
			LOGI( "Skipping dump of %s", tag );
			continue;
		}

		// write out depth map
		if (t1)
			snprintf(zname, sizeof(zname), "offsc-%s-%d-z.pgm", tag, i);
		
		snprintf(rgbname, sizeof(rgbname), "offsc-%s-%d-rgb.ppm", tag, i);
		snprintf(aname, sizeof(aname), "offsc-%s-%d-a.ppm", tag, i);

		offsc_dump_framebuffer(rgbname, aname, zname, w, h);
	}
}

