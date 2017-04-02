// offsc.h
//
// Maintains the off screen buffers

#ifndef OFFSC_H
#define OFFSC_H

#define MAXFRAMEBUFFERS	8

// Call this before creating any offscreen framebuffers.
extern void offsc_init( void );

// Returns a handle to a buffer, or -1 on failure.
extern int offsc_createFramebuffer( int w, int h, bool withdepth );

#if defined( USECOREPROFILE ) // OpenGLES3.x has no glTexImage2DMultisample
// Returns a handle to a 4x msaa buffer, or -1 on failure.
extern int offsc_createMultiSampleFramebuffer( int w, int h, bool withdepth );
#endif

// Destroys framebuffer with specified handle.
extern void offsc_destroyFramebuffer( int idx );

// Use the specified framebuffer.
extern void offsc_use( int idx );

// Tag an off screen framebuffer with a name, for debugging.
extern void offsc_tag( int idx, const char* nm );

// Resolve an MSAA buffer.
extern void offsc_resolve_msaa( int src, int dst );

// Debug aid.
extern void offsc_dump( int first=0, int count=-1 );
extern void offsc_dump_framebuffer( const char* rgbname, const char* aname, const char* zname, int w, int h );

extern unsigned int offsc_color_texture[ MAXFRAMEBUFFERS ];
extern unsigned int offsc_depth_texture[ MAXFRAMEBUFFERS ];

#endif
