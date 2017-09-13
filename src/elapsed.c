#include "elapsed.h"


#if defined( __APPLE__ )
#	include <mach/mach_time.h>
#endif

#if defined( XWIN ) || defined( ANDROID ) || defined( JS )
#	include <time.h>
#endif


double elapsed_ms_since_start( void )
{
	static int virgin=1;
#if defined( IPHN ) || defined( APTV ) ||  defined( OSX )
	static uint64_t c0;
	static double scale = 0.0;
	if ( virgin )
	{
		struct mach_timebase_info tbi;
		mach_timebase_info( &tbi );
		scale = tbi.numer / tbi.denom;
		c0 = mach_absolute_time();
		c0 = scale * c0;
		virgin = 0;
	}
	uint64_t  c1 = mach_absolute_time();
	c1 = scale * c1;
	const uint64_t diff = c1-c0;
	const double ms = diff / 1000000.0;
	return ms;
#elif defined( XWIN ) || defined( ANDROID ) || defined( JS )
	static struct timespec res0;
	struct timespec res1;
	if ( virgin )
	{
		clock_gettime( CLOCK_MONOTONIC, &res0 );
		virgin = 0;
	}
	clock_gettime( CLOCK_MONOTONIC, &res1 );
	const double delta_sec  = res1.tv_sec  - res0.tv_sec;
	const double delta_nsec = res1.tv_nsec - res0.tv_nsec;
	const double delta = delta_sec + delta_nsec/1e9;
	return delta * 1000.0;
#else
#error "elapsed_ms_since_start() has not been implemented for this architecture."
#endif
}

