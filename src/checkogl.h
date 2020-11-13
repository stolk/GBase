// For run-time checking of OpenGL errors, using glError()
// Enabled in DEBUG builds only.

#ifndef CHECKOGL_H
#define CHECKOGL_H

#include "baseconfig.h"
#include "logx.h"

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES 1
#endif

#if defined( USEES2 )
#	if defined( IPHN )
#		include <TargetConditionals.h>
#		include <OpenGLES/ES2/gl.h>
#		include <OpenGLES/ES2/glext.h>
#		define glGenVertexArrays glGenVertexArraysOES
#		define glDeleteVertexArrays glDeleteVertexArraysOES
#		define glBindVertexArray glBindVertexArrayOES
#	else
#		include <GLES2/gl2.h>
#		include <GLES2/gl2ext.h>
#	endif
#endif

#if defined( USEES3 ) && !defined( OSX ) && !defined( USEES31 )
#	if defined( IPHN ) || defined( APTV )
#		include <TargetConditionals.h>
#		include <OpenGLES/ES3/gl.h>
#		include <OpenGLES/ES3/glext.h>
#	else
#		include <GLES3/gl3.h>
#		include <GLES3/gl3ext.h>
#	endif
#endif

#if defined( USEES31 )
#	include <GLES3/gl31.h>
#	include <GLES3/gl3ext.h>
#endif

#if defined(MAC) || defined(OSX)
#	include <OpenGL/gl3.h>
#	include <OpenGL/gl3ext.h>
#endif

#if defined(GLFW)
#	define GLFW_INCLUDE_GLCOREARB
#	include <GLFW/glfw3.h>
#endif

#if defined(XWIN)
#	if defined( USEES2 )
#		define GLFW_INCLUDE_ES2
#		include <GLFW/glfw3.h>
#		include <GLES2/gl2ext.h>
#	endif
#	if defined( USEES3 )
#		define GLFW_INCLUDE_ES3
#		include <GLFW/glfw3.h>
#	endif
#	if defined( USECOREPROFILE )
#		include <GL/glcorearb.h>
#	endif
#endif


#if defined(JS)
#	include <GL/gl.h>
#	include <GL/glext.h>
#endif

#if defined(MSWIN)
#	include <GL/gl3w.h> // We need GL3W to give us the function pointers.
//#	define GLFW_INCLUDE_GLCOREARB
//#	include <GLFW/glfw3.h>
#endif

// NOTE: AMD does not have glPushGroupMarkerEXT so do not put it in production builds!
#	if defined( IPHN ) && defined( USEES3 )
#		define PUSHGROUPMARKER(A) glPushGroupMarkerEXT( 0, #A );
#		define POPGROUPMARKER     glPopGroupMarkerEXT();
#	elif defined( XWIN ) && defined( DEBUG )
//#		define PUSHGROUPMARKER(A) glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, #A );
//#		define POPGROUPMARKER     glPopDebugGroup();
#		define PUSHGROUPMARKER(A) glPushGroupMarkerEXT( 0, #A );
#		define POPGROUPMARKER     glPopGroupMarkerEXT();
#	else
#		define PUSHGROUPMARKER(A)
#		define POPGROUPMARKER
#	endif


#define ERR2STR( ERR, STR ) \
	switch( ERR ) \
	{ \
	case GL_NO_ERROR: \
		STR = "GL_NO_ERROR"; break; \
	case GL_INVALID_ENUM: \
		STR = "GL_INVALID_ENUM"; break; \
	case GL_INVALID_VALUE: \
		STR = "GL_INVALID_VALUE"; break; \
	case GL_INVALID_OPERATION: \
		STR = "GL_INVALID_OPERATION"; break; \
	case GL_OUT_OF_MEMORY: \
		STR = "GL_OUT_OF_MEMORY"; break; \
	case GL_INVALID_FRAMEBUFFER_OPERATION: \
		STR = "GL_INVALID_FRAMEBUFFER_OPERATION"; break; \
	default: \
		STR = "GL_UNKNOWN_ERROR"; break; \
	}



#define CHECK_OGL_FUNC \
{ \
	GLenum err = glGetError(); \
	if ( err != GL_NO_ERROR ) \
	{ \
		const char* _s = 0; \
		ERR2STR( err, _s ); \
		ASSERTM( err == GL_NO_ERROR, "OpenGL Error %s:%d (0x%x) %s", __FILE__, __LINE__, err, _s ); \
	} \
}

#if defined(DEBUG)
#	define CHECK_OGL CHECK_OGL_FUNC
#else
#	define CHECK_OGL
#endif

#define CHECK_OGL_RELEASE CHECK_OGL_FUNC


#if defined( OSX )
#	define USE_DXT		// S3 Texture Compression.
#	define COMPRESSED_TEXTURE_FILE_EXTENSION ".dds"
#else
#	define USE_ETC		// Ericsson Texture Compression.
#	define COMPRESSED_TEXTURE_FILE_EXTENSION ".pkm"
#endif


#endif // CHECK OGL

