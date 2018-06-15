// A signal from a joystick on a 120Hz device will be smoother than that from a 60Hz device.
// Here, we will deal with that by either interpolating (todo) or extrapolating the 60Hz signal.
// For a description of the problem, see:
// http://thelittleengineerthatcould.blogspot.com/2018/06/joystick-sampling-rate-in-games.html

#include "sticksignal.h"

// From GBase
#include "nfy.h"
#include "logx.h"

#define NUMSTICKS	2

static float joyx[ NUMSTICKS ];
static float joyy[ NUMSTICKS ];

static float prvjoyx[ NUMSTICKS ];
static float prvjoyy[ NUMSTICKS ];

static int   numsteps;
static int   sampleidx[ NUMSTICKS ];
static float predict_dx[ NUMSTICKS ];
static float predict_dy[ NUMSTICKS ];


static void onJoystick( const char* cmd )
{
	const float x  = nfy_flt( cmd, "x" );
	const float y  = nfy_flt( cmd, "y" );
	const float dx = nfy_flt( cmd, "dx" );
	const float dy = nfy_flt( cmd, "dy" );
	const int   left = nfy_int( cmd, "left" );
	const int   nr = left == 1 ? 0 : 1;
	if ( dx > -FLT_MAX && dy > -FLT_MAX )
	{
		joyx[nr] += dx;
		joyx[nr] = joyx[nr] >  1 ?  1 : joyx[nr];
		joyx[nr] = joyx[nr] < -1 ? -1 : joyx[nr];
		joyy[nr] += dy;
		joyy[nr] = joyy[nr] >  1 ?  1 : joyy[nr];
		joyy[nr] = joyy[nr] < -1 ? -1 : joyy[nr];
	}
	else if ( x > -FLT_MAX && y > -FLT_MAX )
	{
		joyx[nr] = x;
		joyy[nr] = y;
	}
}


void sticksignal_update( float dt, int steps )
{
	numsteps = steps;

	for ( int i=0; i<NUMSTICKS; ++i )
	{
		sampleidx[i] = 0;
		predict_dx[i] = ( joyx[i] - prvjoyx[i] ) / numsteps;
		predict_dy[i] = ( joyy[i] - prvjoyy[i] ) / numsteps;
		prvjoyx[i] = joyx[i];
		prvjoyy[i] = joyy[i];
	}
}


void sticksignal_sample( int nr, float* stickx, float* sticky )
{
	if ( nr < NUMSTICKS )
	{
		ASSERT( sampleidx[nr] < numsteps );
		*stickx = prvjoyx[nr] + ( sampleidx[nr] + 1 ) * predict_dx[nr];
		*sticky = prvjoyy[nr] + ( sampleidx[nr] + 1 ) * predict_dy[nr];
		sampleidx[nr]++;
	}
}


void sticksignal_latest( int nr, float* stickx, float* sticky )
{
	if ( nr < NUMSTICKS )
	{
		*stickx = joyx[nr];
		*sticky = joyy[nr];
	}
}


void sticksignal_init( void )
{
	nfy_obs_add( "joystick", onJoystick );
}

