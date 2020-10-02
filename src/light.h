// light.h
//
// (c)2012 Abraham Stolk

#ifndef LIGHT_H
#define LIGHT_H

#include "vmath.h"

extern void light_init(void);

extern void light_setProjectionSize( float projsz, float near, float far );

extern void light_getProjTransform(mat44_t& mat);

extern void light_getTransform(mat44_t& mat);

extern void light_getViewTransform(mat44_t& mat);

extern void light_setPos(const vec3_t& p);

extern vec3_t light_pos(void);

extern vec3_t light_dir(void);

extern void light_setCOI(const vec3_t& c);

extern void light_update(float dt);

#endif

