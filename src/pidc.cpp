// pid.cpp
//
// (c)2012-2017 Abraham Stolk

#include "pidc.h"


void pid1_reset( pid1_t &p )
{
	p.previousError = p.integralError = 0.0f;
	p.fresh = true;
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
	p.integralError = ( p.fresh ) ? error : p.integralError;
	p.previousError = ( p.fresh ) ? error : p.previousError;
	p.integralError = ( 1.0f - dt ) * p.integralError +  dt * error;
	float derivativeError = ( error - p.previousError ) / dt;
	p.previousError = error;
	p.fresh = false;
	return p.P * error + p.I * p.integralError + p.D * derivativeError;
}


void pid3_reset( pid3_t &p )
{
	p.previousError = p.integralError = vec3_t( 0,0,0 );
	p.fresh = true;
}


vec3_t pid3_update( pid3_t &p, float dt, vec3_t ist, vec3_t soll )
{
	if ( dt <= 0.0f ) return vec3_t(0,0,0);
	vec3_t error = ist - soll;
	p.integralError = ( p.fresh ) ? error : p.integralError;
	p.previousError = ( p.fresh ) ? error : p.previousError;
	p.integralError = p.integralError * ( 1.0f - dt )   +  error * dt;
	vec3_t derivativeError = ( error - p.previousError ) / dt;
	p.previousError = error;
	p.fresh = false;
	return error * p.P + p.integralError * p.I + derivativeError * p.D;
}

