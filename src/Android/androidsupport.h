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

struct android_app;

typedef struct
{
	struct android_app* app;
	int animating;
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
extern int androidsupport_initDisplay( void );

//! Terminate the display.
extern void androidsupport_termDisplay( void );

