// key value store


#include "kv.h"
#include "logx.h"
#include "nfy.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <emscripten.h>

static const char* filesPath=0;
static bool readyforuse = false;

extern "C" 
{
EMSCRIPTEN_KEEPALIVE
void kv_mounted( void )
{
	readyforuse = true;
	char fname[128];
	snprintf( fname, sizeof(fname), "/kv/%s", "virgin" );
	nfy_queue_msg( "keyvalue ready=1 sync=0" );
}
EMSCRIPTEN_KEEPALIVE
void kv_synced( void )
{
	LOGI( "Wrote keyvalues to persistent storage." );
}
}


bool kv_sync( void )
{
	if ( !readyforuse )
	{
		LOGE( "Cannot sync keyvalues yet: we have not read from persistant storage yet." );
		return false;
	}

	EM_ASM(
		FS.syncfs( false, function(err) {
			assert(!err);
			ccall('kv_synced');
		});
	);
	return true;
}


void kv_init( const char* fp )
{
	filesPath = fp;

	LOGI( "Mounting /kv filesystem..." );

	EM_ASM(
		FS.mkdir('/kv');
		FS.mount(IDBFS, {}, '/kv');
		// sync from persisted state into memory and then
		// run the 'test' function
		FS.syncfs(true, function (err) {
			assert(!err);
			ccall('kv_mounted');
		});
	);
}


void kv_set_int( const char* key, const int v )
{
	ASSERT( filesPath );
	char fname[ 256 ];
	snprintf( fname, sizeof(fname), "/kv/.%s_%s", filesPath, key );
	FILE* f = 0;
	f = fopen( fname, "w" );
	if ( !f )
		LOGE( "Cannot write key-value pair to '%s'", fname );
	ASSERTM( f, "Failed to open %s for writing.", fname );	// Has triggered in the wild, once.
	fprintf( f, "%d", v );
	fclose( f );
}


void kv_set_flt( const char* key, const float v )
{
	ASSERT( filesPath );
	char fname[ 256 ];
	snprintf( fname, sizeof(fname), "/kv/.%s_%s", filesPath, key );
	FILE* f = 0;
	f = fopen( fname, "w" );
	if ( !f )
		LOGE( "Cannot write key-value pair to '%s'", fname );
	ASSERT( f );
	fprintf( f, "%f", v );
	fclose( f );
}


void kv_set_str( const char* key, const char* str )
{
	ASSERT( filesPath );
	char fname[ 256 ];
	snprintf( fname, sizeof(fname), "/kv/.%s_%s", filesPath, key );
	FILE* f = 0;
	f = fopen( fname, "w" );
	if ( !f )
		LOGE( "Cannot write key-value pair to '%s'", fname );
	ASSERT( f );
	fprintf( f, "%s", str );
	fclose( f );

}


void kv_set_blob( const char* key, const char* blob, size_t sz )
{
	ASSERT( filesPath );
	char fname[ 256 ];
	snprintf( fname, sizeof(fname), "/kv/.%s_%s", filesPath, key );
	FILE *f = 0;
	f = fopen( fname, "w" );
	if ( !f )
		LOGE( "Cannot write key-value pair to '%s'", fname );
	ASSERT( f );
	size_t numw = fwrite( blob, 1, sz, f );
	ASSERT( numw == sz );
	fclose( f );
}


int kv_get_int( const char* key, int defaultvalue )
{
	ASSERT( filesPath );
	char fname[ 256 ];
	snprintf( fname, sizeof(fname), "/kv/.%s_%s", filesPath, key );
	FILE* f = 0;
	f = fopen( fname, "r" );
	if ( f )
	{
		char line [ 128 ];
		char* rv = fgets( line, 128, f );
		(void) rv;
		fclose( f );
		return atoi( line );
	}
	else
		return defaultvalue;
}


float kv_get_flt( const char* key, float defaultvalue )
{
	ASSERT( filesPath );
	char fname[ 256 ];
	snprintf( fname, sizeof(fname), "/kv/.%s_%s", filesPath, key );
	FILE* f = 0;
	f = fopen( fname, "r" );
	if ( f )
	{
		char line [ 128 ];
		char* rv = fgets( line, 128, f );
		(void) rv;
		fclose( f );
		return atof( line );
	}
	else
		return defaultvalue;
}


int kv_get_str( const char* key, char* str, int len )
{
	ASSERT( filesPath );
	char fname[ 256 ];
	snprintf( fname, sizeof(fname), "/kv/.%s_%s", filesPath, key );
	FILE* f = 0;
	f = fopen( fname, "r" );
	if ( f )
	{
		char* rv = fgets( str, len, f );
		(void) rv;
		fclose( f );
		return (int) strlen( str );
	}
	else
	{
		str[ 0 ] = 0;
		return 0;
	}
}


size_t kv_get_blob( const char* key, char* blob, size_t maxsz )
{
	ASSERT( filesPath );
	char fname[ 256 ];
	snprintf( fname, sizeof(fname), "/kv/.%s_%s", filesPath, key );
	FILE* f = 0;
	f = fopen( fname, "r" );
	if ( f )
	{
		size_t numr = fread( blob, 1, maxsz, f );
		fclose( f );
		return numr;
	}
	else
		return 0;
}

