// Deal with those microsoft peculiarities.

#ifndef BASECONFIG_H
#	define BASECONFIG_H

#	if defined( MSWIN )

#		include <math.h>
#		include <string.h>


#		if _MSC_VER < 1900
#			define roundf(X) floorf((X) + 0.5f)
#			define snprintf _snprintf
#			define strncpy	strncpy_s
#			define strncat	strncat_s
#		endif

#		define isnanf isnan

#		define __BASE_FILE__ __FILE__

#		undef NDEBUG // We want asserts please.

#		pragma warning( disable : 4305 )	// truncation from 'double' to 'const float'
#		pragma warning( disable : 4244 )	// truncation from 'float ' to 'int', possible loss of data
#		pragma warning( disable : 4800 )	// forcing value to bool 'true' or 'false' (performance warning)
#		pragma warning( disable : 4996 )	// Consider using fopen_s instead.

#		define _CRT_SECURE_NO_WARNINGS

#		define __restrict__ __restrict
#		define __inline__ __inline
#		define aligned_alloc( AL, SZ ) malloc( SZ )
#	endif // MSWIN


// Not all platforms garantee Vertex Array Objects.
#	if defined( ANDROID ) || defined( RASP ) || defined( JS )
#		undef USE_VAO
#	else
#		define USE_VAO // On iPhone or android simulator:
#	endif

#	if defined( USEES3 )
#		define USE_VAO	// Android OpenGL ES3.x does have them.
#	endif


#	if defined( ANDROID ) || defined( RASP ) || defined( MSWIN )
#		define EMBEDGEOMS
#	endif

#	if defined( PLAY ) || defined( IPHN )
#		define USETOUCH
#	endif

#	if defined( OSX ) || defined( IPHN ) || defined( APTV )
#		define isnanf isnan
#	endif


// We use the GL DEBUG PROC extension on XWin and MSWin.
#	if !defined( OSX ) && !defined( IPHN ) && !defined( APTV ) && !defined( PLAY ) && !defined( JS )
#		define USEGLDEBUGPROC
#	endif

#endif
