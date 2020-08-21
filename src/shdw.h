// shdw.cpp
//
// Maintains the shadow buffer (bound to depth texture)

#ifndef SHDW_H
#define SHDW_H

extern bool shdw_createFramebuffer( bool supportsDepthTexture, int nr=0 );

extern void shdw_destroyFramebuffer( int nr=0 );

extern void shdw_use( int nr=0 );

extern void shdw_invalidate( int nr=0 );

extern void shdw_dump( int nr=0 );

extern unsigned int shdw_texture( int nr=0);

extern bool shdw_completeframebufferfix;

#endif
