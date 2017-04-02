//  camera.h
//
//  User controlled camera that will follow a target.
//  User controls orbitting around target, elevation and distance (track-in/track-out).
//
//  (c)2012-2017 Abraham Stolk

#ifndef CAMERA_H
#define CAMERA_H

#include "vmath.h"

extern float camera_minDist;

extern float camera_maxDist;

extern void camera_init( float fovy = 0.30f * M_PI );

extern void camera_setAspectRatio(float aspect, float zNear=0.1, float zFar=150, bool offaxis=false);

extern vec3_t camera_screenToWorld(vec3_t spos);

extern void camera_getViewTransform(mat44_t& mat);

extern void camera_getCameraTransform(mat44_t &mat);

extern void camera_getCameraTransformNoShift(mat44_t &mat);

extern void camera_getProjTransform(mat44_t& mat);

extern void camera_setPos(const vec3_t& p);

extern void camera_forceCOI(const vec3_t& c);

extern void camera_setCOI(const vec3_t& c);

extern vec3_t camera_getCOI( void );

extern void camera_forcePan(float p);

extern void camera_setPan(float p);

extern vec3_t camera_pos(void);

extern vec3_t camera_viewX(void);

extern vec3_t camera_viewY(void);

extern vec3_t camera_viewZ(void);

extern vec3_t camera_viewP(void);

extern vec3_t camera_camX(void);

extern vec3_t camera_camY(void);

extern vec3_t camera_camZ(void);

extern vec3_t camera_aim( void );

extern float camera_elevationAngle( void );

extern float camera_orbitAngle( void );

extern void camera_alignTo(vec3_t d, float margin, float dt);

extern void camera_update(float dt);

extern float camera_dist(void);

extern void camera_setCoiPid( float p, float i, float d );

extern void camera_setPanPid( float p, float i, float d );

extern bool camera_should_cull( vec3_t p, float margin=0.0f );

extern void camera_setMinZ( float height );

extern void camera_setRollAngle( float a );

extern float camera_depthInZBuffer(vec3_t pt);

#endif

