// key value store

#include <stdlib.h>	// for size_t

extern void kv_init( const char* filesPath );

extern bool kv_sync( void );

extern void kv_set_int( const char* key, const int v );

extern void kv_set_flt( const char* key, const float v );

extern void kv_set_str( const char* key, const char* str );

extern void kv_set_blob( const char* key, const char* blob, size_t sz );

extern int kv_get_int( const char* key, int defaultvalue=0 );

extern float kv_get_flt( const char* key, float defaultvalue=0.0f );

extern int kv_get_str( const char* key, char* str, int len );

extern size_t kv_get_blob( const char* key, char* blob, size_t maxsz );

