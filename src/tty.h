// tty.h
//
// (c)2012 Abraham Stolk


#include "vmath.h"
#include "rendercontext.h"

extern void tty_update(float dt);

extern void tty_write(const char* s);

extern void tty_clear(void);

extern void tty_draw_edge(const char* tag, const rendercontext_t& rc);

extern void tty_reset(void);

extern bool tty_init(void);

extern void tty_draw_string( const char* string, const rendercontext_t& rc, vec3_t pos, vec3_t sz, const char* align="left", const mat44_t* texttrf=0 );

