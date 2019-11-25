// androidsupport.h
//
// Provides basic support functions on Android platform:
//
// * Hooks assert handler that shows asserts in a dialog.
// * Can launch URLs.
// * Initializes and terminates the EGL display.
//
// (c)2018 Game Studio Abraham Stolk Inc.

#include <EGL/egl.h>

#define LAUNCH_FAILURE_NONE			 0
#define LAUNCH_FAILURE_INSUFFICIENT_RESOURCES	-1
#define LAUNCH_FAILURE_NO_GLES2_CONTEXT		-2
#define LAUNCH_FAILURE_NO_GLES3_CONTEXT		-3
#define LAUNCH_FAILURE_NO_MATCHING_EGL_CONFIG	-4
#define LAUNCH_FAILURE_NO_WINDOW		-5


struct android_app;

typedef struct
{
	struct android_app* app;
	int animating;		//!< If non-zero, we are between RESUME and PAUSE events.
	int ready;		//!< If non-zero, the window has been created.
	int focused;		//!< If non-zero, our window has focus.
	EGLDisplay display;
	EGLSurface surface;
	EGLContext context;
	int32_t width;
	int32_t height;
} androidsupport_engine_t;


//! Represents the state of our engine.
extern androidsupport_engine_t androidsupport_engine;

//! Signals that the assert dialog has been dismissed, java-side.
extern bool androidsupport_assertDialogDismissed;

//! Calls into java to open a URL.
extern bool androidsupport_launchUrl( const char* url );

//! Hooks an assert handler.
extern void androidsupport_init( struct android_app* app );

//! Initialize the display.
extern int androidsupport_initDisplay( bool withDepthBuffer=true );

//! Terminate the display.
extern void androidsupport_termDisplay( void );

//! Report (java-side) that we failed to launch.
extern bool androidsupport_reportFailedLaunch( int failcode );

//! The name of the manufacturer.
extern char androidsupport_manufacturer[80];

//! The name of the device model.
extern char androidsupport_model[80];

