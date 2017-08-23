// quad.h
//
// simple textured quad drawing

#include "vmath.h"
#include "rendercontext.h"

#ifndef QUAD_H
#define QUAD_H

extern void quad_init( void );

extern void quad_exit( void );

extern void quad_draw( const char* tag, vec3_t xlat, vec3_t rotx, vec3_t roty );

extern void quad_draw_set( const char* tag, int cnt, const vec3_t* xlat, const vec3_t* rotx, const vec3_t* roty );

extern void quad_draw_dof( void );

extern void quad_prepare( void );

#endif

