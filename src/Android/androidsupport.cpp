#include "androidsupport.h"

#include <jni.h>
#include <errno.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <EGL/egl.h>

#include <android/log.h>

#include "android_native_app_glue.h"

#include "logx.h"
#include "assertreport.h"


//! Helper function to lookup an EGL error string.
static inline const char* eglErrorString( EGLint error )
{
	switch( error )
	{
		case EGL_SUCCESS: return "No error";
		case EGL_NOT_INITIALIZED: return "EGL not initialized or failed to initialize";
		case EGL_BAD_ACCESS: return "Resource inaccessible";
		case EGL_BAD_ALLOC: return "Cannot allocate resources";
		case EGL_BAD_ATTRIBUTE: return "Unrecognized attribute or attribute value";
		case EGL_BAD_CONTEXT: return "Invalid EGL context";
		case EGL_BAD_CONFIG: return "Invalid EGL frame buffer configuration";
		case EGL_BAD_CURRENT_SURFACE: return "Current surface is no longer valid";
		case EGL_BAD_DISPLAY: return "Invalid EGL display";
		case EGL_BAD_SURFACE: return "Invalid surface";
		case EGL_BAD_MATCH: return "Inconsistent arguments";
		case EGL_BAD_PARAMETER: return "Invalid argument";
		case EGL_BAD_NATIVE_PIXMAP: return "Invalid native pixmap";
		case EGL_BAD_NATIVE_WINDOW: return "Invalid native window";
		case EGL_CONTEXT_LOST: return "Context lost";
	}
	return "Unknown EGL error";
}



androidsupport_engine_t androidsupport_engine;



// If we cause an exception in JNI, we print the exception info to
// the log, we clear the exception to avoid a pending-exception
// crash, and we force the function to return.
#define EXCEPTION_RETURN(env) \
	if (env->ExceptionOccurred()) { \
		env->ExceptionDescribe(); \
		env->ExceptionClear(); \
		return false; \
	}

#define CHECKEGL( F ) \
	{ \
		const EGLint eglerr = eglGetError(); \
		ASSERTM \
		( \
			eglerr==EGL_SUCCESS, \
			#F " failed with %s", \
			eglErrorString(eglerr) \
		); \
	}

#define CHECKEGLV( F ) \
	{ \
		const EGLint eglerr = eglGetError(); \
		ASSERTM \
		( \
			eglerr==EGL_SUCCESS, \
			#F " failed with %s (VENDOR:%s VERSION:%s)", \
			eglErrorString(eglerr), eglQueryString(display, EGL_VENDOR), eglQueryString(display, EGL_VERSION) \
		); \
	}


bool androidsupport_assertDialogDismissed=false;


char androidsupport_manufacturer[80];
char androidsupport_model[80];



bool androidsupport_launchUrl( const char* url )
{
	struct android_app* app = androidsupport_engine.app;
	if ( !app ) return false;

	JNIEnv* env = app->appThreadEnv;
	if ( !env ) return false;

	jstring jniText = env->NewStringUTF( url );
	EXCEPTION_RETURN( env );

	jclass thisClass = env->GetObjectClass( app->appThreadThis );
	EXCEPTION_RETURN( env );

	jmethodID launchMethod = env->GetMethodID( thisClass, "launchURL", "(Ljava/lang/String;)V" );
	EXCEPTION_RETURN( env );

	app->appThreadEnv->CallVoidMethod( app->appThreadThis, launchMethod, jniText );
	EXCEPTION_RETURN( env );

	return true;
}


//! Helper function to open a java-side dialog window with a message.
static bool alertFatal( const char* msg )
{
	if ( !androidsupport_engine.app ) return false;

	JNIEnv* env = androidsupport_engine.app->appThreadEnv;
	if ( !env ) return false;

	jstring jniText = env->NewStringUTF( msg );
	EXCEPTION_RETURN( env );

	jclass thisClass = env->GetObjectClass( androidsupport_engine.app->appThreadThis );
	EXCEPTION_RETURN( env );

	jmethodID alertMethod = env->GetMethodID( thisClass, "alertFatal", "(Ljava/lang/String;)V" );
	EXCEPTION_RETURN( env );

	env->CallVoidMethod( androidsupport_engine.app->appThreadThis, alertMethod, jniText );
	EXCEPTION_RETURN( env );

	return true;
}


//! Helper function to open a java-side dialog window with a message.
bool androidsupport_reportFailedLaunch( const char* msg )
{
	if ( !androidsupport_engine.app ) return false;

	JNIEnv* env = androidsupport_engine.app->appThreadEnv;
	if ( !env ) return false;

	jstring jniText = env->NewStringUTF( msg );
	EXCEPTION_RETURN( env );

	jclass thisClass = env->GetObjectClass( androidsupport_engine.app->appThreadThis );
	EXCEPTION_RETURN( env );

	jmethodID alertMethod = env->GetMethodID( thisClass, "reportFailedLaunch", "(Ljava/lang/String;)V" );
	EXCEPTION_RETURN( env );

	env->CallVoidMethod( androidsupport_engine.app->appThreadThis, alertMethod, jniText );
	EXCEPTION_RETURN( env );

	return true;
}


static void androidsupport_presentAssert( const char* condition, const char* file, int line )
{
	LOGE( "ASSERT FAILED FAILED FAILED!" );
	LOGE( "COND %s", condition );
	LOGE( "FILE %s", file );
	LOGE( "LINE %d", line );

	char logm[1024];	// message for the log.
	char usrm[1024];	// message for the user.
	snprintf
	(
		logm, sizeof(logm),
		"ASSERT FAILED %s v%s(%s/%s): %s (%s:%d)",
		TOSTRING(LOGTAG), TOSTRING(APPVER),
		androidsupport_manufacturer, androidsupport_model,
	       	condition, file, line
	);
	snprintf
	(
		usrm, sizeof(usrm),
		"FATAL ERROR:\n"
		"%s (%s:%d)\n"
		"%s v%s",
		condition, file, line,
		TOSTRING(LOGTAG), TOSTRING(APPVER)
	);

	if ( !strstr( condition, "[NOREP]" ) )
	{
		const char* server = "45.79.100.67";	// stolk.org
		assertreport_init( server, 2323 );
		assertreport_send( logm, strlen( logm )+1 );
		assertreport_exit();
	}

	alertFatal( usrm );

	// Wait a maximum of 16 seconds for user to acknowledge the assertion failure.
	// After which we should terminate regardless.
	for ( int i=0; i<16 && !androidsupport_assertDialogDismissed; ++i )
		sleep( 1 );

	// This will terminate the process.
	abort();
}


//! Hooks an assert handler.
void androidsupport_init( struct android_app* app )
{
	memset( &androidsupport_engine, 0, sizeof( androidsupport_engine) );
	androidsupport_engine.app = app;
	asserthook = androidsupport_presentAssert;	// If an assert is triggered, call this function.
}


/**
 * Initialize an EGL context for the current display.
 * After this, all GL resources have to be re-created: so call ctrl_create() afterwards.
 */
int androidsupport_initDisplay( bool withDepthBuffer )
{
	// initialize OpenGL ES and EGL
	androidsupport_engine_t* engine = &androidsupport_engine;
	ASSERT( engine->app->window );

	/*
	 * Here specify the attributes of the desired configuration.
	 * Below, we select an EGLConfig with at least 8 bits per color
	 * component compatible with on-screen windows and 4x MSAA.
	 */
	const EGLint attribs[] = {
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_DEPTH_SIZE, withDepthBuffer ? 16 : 0,
		EGL_BLUE_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_RED_SIZE, 8,
#if 0
		// MSAA is super slow on some Android devices, so only use it with utmost care.
		EGL_SAMPLE_BUFFERS, 1,
		EGL_SAMPLES, 4,
#endif
		EGL_NONE
	};
	/*
	 * If we can't get that, then RGB565 will work as well.
	 */
	const EGLint attribs_fallback[] = {
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_DEPTH_SIZE, withDepthBuffer ? 16 : 0,
		EGL_BLUE_SIZE, 5,
		EGL_GREEN_SIZE, 5,
		EGL_RED_SIZE, 5,
		EGL_NONE
	};
	EGLint w, h, dummy, format;
	EGLint numConfigs=0;
	EGLConfig config=0;
	EGLSurface surface=0;
	EGLContext context=0;

	(void) eglGetError();
	EGLDisplay display = eglGetDisplay( EGL_DEFAULT_DISPLAY );
	CHECKEGL( eglGetDisplay )
	ASSERTM( display != EGL_NO_DISPLAY, "No default EGL display found. eglGetDisplay returned %p", EGL_NO_DISPLAY );

	const EGLBoolean eglinitialized = eglInitialize(display, 0, 0);
	CHECKEGL( eglInitialize )
	ASSERT( eglinitialized )

	/* Here, the application chooses the configuration it desires. In this
	 * sample, we have a very simplified selection process, where we pick
	 * the first EGLConfig that matches our criteria */
	eglChooseConfig(display, attribs, &config, 1, &numConfigs);
	const EGLint egle = eglGetError();
	if ( egle == EGL_BAD_CONFIG )
	{
		LOGE( "Preferred EGL configuration was deemed to be a bad config?" );
	}
	else
	{
		ASSERTM( egle == EGL_SUCCESS, " eglChooseConfig failed with %s", eglErrorString(egle) );
		LOGI("number of EGL configurations that match our preferred criteria: %d", numConfigs);
	}

	if ( numConfigs < 1 )
	{
		LOGE( "Cannot get EGL configuration. Trying fallback (16bit colour)..." );
		eglChooseConfig( display, attribs_fallback, &config, 1, &numConfigs);
		CHECKEGLV( eglChooseConfig )
		if ( numConfigs < 0 )
			return -1;
	}

	/* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
	 * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
	 * As soon as we picked a EGLConfig, we can safely reconfigure the
	 * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
	eglGetConfigAttrib( display, config, EGL_NATIVE_VISUAL_ID, &format );
	CHECKEGLV( eglGetConfigAttrib )

	const int32_t setbuffers_result = ANativeWindow_setBuffersGeometry( engine->app->window, 0, 0, format );
	if ( setbuffers_result ) LOGE( "ANativeWindow_setBuffersGeometry() failed." );

	surface = eglCreateWindowSurface( display, config, engine->app->window, NULL );
	EGLint eglerr = eglGetError();
	if ( eglerr == EGL_BAD_ALLOC ) return -2;
	ASSERTM( eglerr==EGL_SUCCESS, "eglCreateWindowSurface() failed with %s", eglErrorString(eglerr) );

	const EGLint contextAttribsES3[] =
	{
		EGL_CONTEXT_CLIENT_VERSION, 3,
		EGL_NONE
	};
	const EGLint contextAttribsES2[] =
	{
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};
#if defined(USEES2)
	context = eglCreateContext( display, config, NULL, contextAttribsES2 );
	eglerr = eglGetError();
	if ( eglerr != EGL_SUCCESS )
	{
		LOGE( "Couldn't create ES2 context: this device is incompatible!" );
		return -1;
	}
#else
	context = eglCreateContext( display, config, NULL, contextAttribsES3 );
	eglerr = eglGetError();
	if ( eglerr != EGL_SUCCESS )
	{
		context = eglCreateContext( display, config, NULL, contextAttribsES2 );
		CHECKEGLV( eglCreateContext );
		LOGE( "Couldn't create ES3 context, can only create ES2 context: this device is incompatible!" );
		return -1;
	}
#endif

	const EGLBoolean madecur = eglMakeCurrent(display, surface, surface, context);
	CHECKEGLV( elgMakeCurrent )
	ASSERTM( madecur != EGL_FALSE, "format=%d, numConfigs=%d", format, numConfigs );

	eglQuerySurface(display, surface, EGL_WIDTH,  &w);
	eglQuerySurface(display, surface, EGL_HEIGHT, &h);

	engine->display = display;
	engine->context = context;
	engine->surface = surface;
	engine->width = w;
	engine->height = h;

	LOGI("Created surface of size %dx%d", w, h);

	return 0;
}



/**
 * Tear down the EGL context currently associated with the display.
 * Call ctrl_destroy() before this.
 */
void androidsupport_termDisplay( void )
{
	androidsupport_engine_t* engine = &androidsupport_engine;
	if (engine->display != EGL_NO_DISPLAY) 
	{
		eglMakeCurrent( engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
		if ( engine->context != EGL_NO_CONTEXT ) {
			eglDestroyContext( engine->display, engine->context );
		}
		if ( engine->surface != EGL_NO_SURFACE ) {
			eglDestroySurface( engine->display, engine->surface );
		}
		eglTerminate( engine->display );
	}
	engine->display = EGL_NO_DISPLAY;
	engine->context = EGL_NO_CONTEXT;
	engine->surface = EGL_NO_SURFACE;
}

