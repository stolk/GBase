#include "elapsed.h"


#if defined( __APPLE__ )
#	include <mach/mach_time.h>
#endif

#if defined( XWIN ) || defined( ANDROID ) || defined( JS )
#	include <time.h>
#endif

#if defined( MSWIN )
#	define BILLION                             (1E9)
#	define CLOCK_MONOTONIC	-1
#	include <Windows.h>
	struct timespec { long tv_sec; long tv_nsec; };

	static BOOL g_first_time = 1;
	static LARGE_INTEGER g_counts_per_sec;

	int clock_gettime(int dummy, struct timespec *ct)
	{
		LARGE_INTEGER count;

		if (g_first_time)
		{
			g_first_time = 0;

			if (0 == QueryPerformanceFrequency(&g_counts_per_sec))
			{
				g_counts_per_sec.QuadPart = 0;
			}
		}
		if ((NULL == ct) || (g_counts_per_sec.QuadPart <= 0) || (0 == QueryPerformanceCounter(&count))) 
		{
			return -1;
		}

		ct->tv_sec = count.QuadPart / g_counts_per_sec.QuadPart;
		ct->tv_nsec = ((count.QuadPart % g_counts_per_sec.QuadPart) * BILLION) / g_counts_per_sec.QuadPart;
		return 0;
	}
#endif


double elapsed_ms_since_last_call( void )
{
	static int virgin = 1;
#if defined( IPHN ) || defined( APTV ) || defined( OSX )
	static struct mach_timebase_info tbi;
	static uint64_t prev;
	if ( virgin )
	{
		mach_timebase_info( &tbi );
		prev = mach_absolute_time();
		virgin = 0;
	}
	uint64_t curr = mach_absolute_time();
	uint64_t delt = curr - prev;
	prev = curr;
	delt = delt / tbi.denom;
	delt = delt * tbi.numer;
	// now we have the delta in nanoseconds.
	return delt / 1000000.0;
#elif defined( ANDROID ) || defined( XWIN ) || defined( MSWIN )
	static struct timespec prev;
	struct timespec curr;
	if ( virgin )
	{
		clock_gettime( CLOCK_MONOTONIC, &prev );
		virgin = 0;
	}
	clock_gettime( CLOCK_MONOTONIC, &curr );
	const double delta_sec  = curr.tv_sec  - prev.tv_sec;
	const double delta_nsec = curr.tv_nsec - prev.tv_nsec;
	const double delta = delta_sec*1000 + delta_nsec/1e6;
	prev = curr;
	return delta;
#else
#	error "elapsed_ms_since_start() has not been implemented for this architecture."
#endif
}


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
#elif defined( XWIN ) || defined( ANDROID ) || defined( JS ) || defined( MSWIN )
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
	const double delta_ms = delta_sec*1000 + delta_nsec/1e6;
	return delta_ms;
#else
#	error "elapsed_ms_since_start() has not been implemented for this architecture."
#endif
}

