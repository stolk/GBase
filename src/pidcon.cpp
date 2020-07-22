// pid.cpp
//
// (c)2012-2017 Abraham Stolk

#include "pidcon.h"
#include "logx.h"


void pid1_reset( pid1_t &p )
{
	p.head = p.tail = 0;
	p.integral = 0.0f;
}


int pid1_histsz( pid1_t& p )
{
	int rv = p.tail - p.head;
	if ( rv < 0 )
		rv += PIDHISTSZ;
	return rv;
}


float pid1_update( pid1_t &p, float dt, float ist, float soll )
{
	if ( dt <= 0.0f ) return 0.0f;

	float error = ist - soll;
	if ( p.angular )
	{
		// normalize angular error
		error = ( error < -M_PI ) ? error + 2 * M_PI : error;
		error = ( error >  M_PI ) ? error - 2 * M_PI : error;
	}

	const bool histempt = (p.head==p.tail);
	const bool histfull = ((p.head+1)&PIDHISTMSK) == p.tail;
	if (histfull)
	{
		// remove oldest sample.
		p.integral -= p.hist[ p.head ];
		// advance head.
		p.head = ( p.head + 1 ) & PIDHISTMSK;
	}

	float previousError = histempt ? error : p.hist[ (p.tail-1) & PIDHISTMSK ];

	// add new sample to history at the tail.
	p.hist[ p.tail ] = error;
	p.tail = ( p.tail + 1 ) & PIDHISTMSK;
	p.integral += error;

	const float historicError = p.integral / pid1_histsz( p );

	const float derivativeError = ( error - previousError ) / dt;
	return p.P * error + p.I * historicError + p.D * derivativeError;
}


void pid3_reset( pid3_t &p )
{
	p.integral = vec3_t(0,0,0);
	p.head = 0;
	p.tail = 0;
}


static int pid3_histsz( pid3_t& p )
{
	int rv = p.tail - p.head;
	if ( rv < 0 )
		rv += PIDHISTSZ;
	return rv;
}


vec3_t pid3_update( pid3_t &p, float dt, vec3_t ist, vec3_t soll )
{
	if ( dt <= 0.0f ) return vec3_t(0,0,0);
	vec3_t error = ist - soll;

	const bool histempt = (p.head==p.tail);
	const bool histfull = ((p.head+1)&PIDHISTMSK) == p.tail;
	if (histfull)
	{
		// remove oldest sample.
		p.integral -= p.hist[ p.head ];
		// advance head.
		p.head = ( p.head + 1 ) & PIDHISTMSK;
	}

	vec3_t previousError = histempt ? error : p.hist[ (p.tail-1) & PIDHISTMSK ];

	// add new sample to history at the tail.
	p.hist[ p.tail ] = error;
	p.tail = ( p.tail + 1 ) & PIDHISTMSK;
	p.integral += error;

	const vec3_t historicError = p.integral * ( 1.0f /  pid3_histsz( p ) );

	const vec3_t derivativeError = ( error - previousError ) * ( 1.0f / dt );
	return error * p.P + historicError * p.I + derivativeError * p.D;
}

