// shdw.cpp
//
// Maintains the shadow buffer (bound to depth texture)

#ifndef SHDW_H
#define SHDW_H

extern bool shdw_createFramebuffer( bool supportsDepthTexture );

extern void shdw_destroyFramebuffer( void );

extern void shdw_use( void );

extern void shdw_dump( void );

extern unsigned int shdw_texture;
extern bool shdw_completeframebufferfix;

#endif
