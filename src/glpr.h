// glpr.h
//
// GL program for shader, vertex/fragment.
//
// (c)2012 Abraham Stolk

#ifndef GLPR_H
#define GLPR_H

#include "vmath.h"
#include "rendercontext.h"

extern void glpr_dump( void );

extern void glpr_init( void );

extern void glpr_clear( void );

extern unsigned int glpr_load( const char* name, const char* src_vsh, const char* src_fsh, const char* attributes, const char* uniforms );

extern unsigned int glpr_load_from_files( const char* name, const char* fname_vsh, const char* fname_fsh, const char* attributes, const char* uniforms );

extern bool glpr_validate( unsigned int prog );

extern int  glpr_uniform( const char* nm );

extern void glpr_use( unsigned int prog );

extern const char*	glpr_last_compile_log;		//!< the last GLSL compile log.
extern const char*	glpr_last_link_log;		//!< the last GLSL link log.

#endif
