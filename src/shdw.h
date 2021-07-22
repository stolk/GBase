// shdw.cpp
//
// Maintains the shadow buffer (bound to depth texture)

#ifndef SHDW_H
#define SHDW_H

#ifndef SHADOWBUFFERSIZE
#	define SHADOWBUFFERSIZE		2048
#endif

extern bool shdw_createFramebuffer( bool supportsDepthTexture, int nr=0, int w=SHADOWBUFFERSIZE, int h=SHADOWBUFFERSIZE );

extern void shdw_destroyFramebuffer( int nr=0 );

extern void shdw_use( int nr=0 );

extern int  shdw_usable( int nr=0 );

extern void shdw_invalidate( int nr=0 );

extern void shdw_dump( int nr=0 );

extern unsigned int shdw_texture( int nr=0);

extern bool shdw_completeframebufferfix;

extern bool shdw_use_hardware_depth_compare;

extern bool shdw_use_hardware_pcf;

extern bool shdw_verbose;

#endif
