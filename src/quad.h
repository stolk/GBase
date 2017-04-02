// quad.h
//
// simple textured quad drawing

#include "vmath.h"
#include "rendercontext.h"

#ifndef QUAD_H
#define QUAD_H

extern void quad_init( void );

extern void quad_exit( void );

extern void quad_draw( const char* tag, const rendercontext_t& rc, vec3_t xlat, vec3_t rotx, vec3_t roty );

extern void quad_draw_dof( void );

extern void quad_prepare( const rendercontext_t& rc );

#endif

