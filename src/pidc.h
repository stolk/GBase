// pidc.h
// Propotional Integral Differential Control.
//
// (c)2012-2017 Abraham Stolk

#ifndef PIDC_H
#define PIDC_H

#include "vmath.h"

//! Scalar PID controller
typedef struct
{
	float P;
	float I;
	float D;
	float previousError; // Last error
	float integralError; // Historic error
	bool  fresh; // If set, we have no 'last error' yet.
	bool  angular; // Angular PIDs have errors wrap around at -pi and +pi.
} pid1_t;


//! Reset a PID controller. Clears historial error.
extern void pid1_reset( pid1_t& p );

//! Calculate the steering force based on current value (ist) and desired value (soll).
extern float pid1_update( pid1_t& p, float dt, float ist, float soll );


//! Vector PID controller
typedef struct
{
	float P;
	float I;
	float D;
	vec3_t previousError; // Last error
	vec3_t integralError; // Historic error
	bool  fresh; // If set, we have no 'last error' yet.
	bool  angular; // Angular PIDs have errors wrap around at -pi and +pi.
} pid3_t;


//! Reset a PID3 controller. Clears historial error.
extern void pid3_reset( pid3_t& p );

//! Calculate the steering force based on current value (ist) and desired value (soll).
extern vec3_t pid3_update( pid3_t& p, float dt, vec3_t ist, vec3_t soll );

#endif

