// For run-time checking of OpenGL errors, using glError()
// Enabled in DEBUG builds only.

#ifndef CHECKOGL_H
#define CHECKOGL_H

#include "baseconfig.h"

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
#		define GL_HALF_FLOAT GL_HALF_FLOAT_OES
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

#	if defined( IPHN ) && defined( USEES3 )
#		define PUSHGROUPMARKER(A) glPushGroupMarkerEXT( 0, #A );
#		define POPGROUPMARKER     glPopGroupMarkerEXT();
#	else
#		define PUSHMARKER(A)
#		define POPMARKER
#	endif

#include <assert.h>
#include "baseconfig.h"

#if defined(DEBUG)
#include "logx.h"
#define CHECK_OGL \
{ \
    GLenum err = glGetError(); \
    if ( err != GL_NO_ERROR ) \
    { \
    switch( err ) \
    { \
        case GL_NO_ERROR: \
            LOGI( "GL_NO_ERROR"); \
            break; \
        case GL_INVALID_ENUM: \
            LOGI( "GL_INVALID_ENUM"); \
            break; \
        case GL_INVALID_VALUE: \
            LOGI( "GL_INVALID_VALUE"); \
            break; \
        case GL_INVALID_OPERATION: \
            LOGI( "GL_INVALID_OPERATION"); \
            break; \
        case GL_OUT_OF_MEMORY: \
            LOGI( "GL_OUT_OF_MEMORY"); \
            break; \
        case GL_INVALID_FRAMEBUFFER_OPERATION: \
            LOGI( "GL_INVALID_FRAMEBUFFER_OPERATION"); \
            break; \
} \
LOGI( "OpenGL Error %s:%d (%x)", __FILE__, __LINE__, err ); \
assert( err == GL_NO_ERROR ); \
} \
}
#else //!DEBUG
#define CHECK_OGL
#endif // DEBUG


#if defined( OSX )
#	define USE_DXT		// S3 Texture Compression.
#	define COMPRESSED_TEXTURE_FILE_EXTENSION ".dds"
#else
#	define USE_ETC		// Ericsson Texture Compression.
#	define COMPRESSED_TEXTURE_FILE_EXTENSION ".pkm"
#endif


#endif // CHECK OGL

