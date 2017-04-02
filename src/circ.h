// circ.h
//
// simple textured circ drawing

#include "vmath.h"
#include "rendercontext.h"

#ifndef CIRC_H
#define CIRC_H

extern void circ_init( int numtria );

extern void circ_exit( void );

extern void circ_draw( const rendercontext_t& rc, vec3_t xlat, vec3_t rotx, vec3_t roty, vec3_t xlatuv, vec3_t rotu, vec3_t rotv );

#endif

