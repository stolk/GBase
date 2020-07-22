// pidcon.h
// Propotional Integral Differential Control.
//
// (c)2020 Abraham Stolk

#ifndef PIDCON_H
#define PIDCON_H

#include "vmath.h"

#define PIDHISTSZ	64		// Must be power of 2.
#define PIDHISTMSK	(PIDHISTSZ-1)

//! Scalar PID controller
typedef struct
{
	float P;
	float I;
	float D;
	bool  angular; // Angular PIDs have errors wrap around at -pi and +pi.
	int   head;
	int   tail;
	float hist[PIDHISTSZ];
	float integral;
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
	bool  angular; // Angular PIDs have errors wrap around at -pi and +pi.
	int   head;
	int   tail;
	vec3_t hist[PIDHISTSZ];
	vec3_t integral;
} pid3_t;


//! Reset a PID3 controller. Clears historial error.
extern void pid3_reset( pid3_t& p );

//! Calculate the steering force based on current value (ist) and desired value (soll).
extern vec3_t pid3_update( pid3_t& p, float dt, vec3_t ist, vec3_t soll );

#endif

