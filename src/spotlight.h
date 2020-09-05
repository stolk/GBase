// spotlight.h
//
// (c)2020 Abraham Stolk

#ifndef SPOTLIGHT_H
#define SPOTLIGHT_H

#include "vmath.h"

extern void spotlight_init(void);

extern void spotlight_setProjection( float fov, float near, float far );

extern void spotlight_getProjTransform(mat44_t& mat);

extern void spotlight_getViewTransform(mat44_t& mat);

extern void spotlight_setPos(const vec3_t& p);

extern vec3_t spotlight_pos(void);

extern vec3_t spotlight_dir(void);

extern void spotlight_setCOI(const vec3_t& c);

extern void spotlight_update(float dt);

#endif

