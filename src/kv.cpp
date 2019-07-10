// key value store


#include "kv.h"
#include "logx.h"
#include "nfy.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static const char* filesPath=0;

void kv_init( const char* fp )
{
	ASSERT( fp );
	filesPath = fp;
	nfy_queue_msg( "keyvalue ready=1 sync=0" );
}


bool kv_sync( void )
{
	return true;
}


void kv_set_int( const char* key, const int v )
{
	ASSERTM( filesPath, "Cannot set key '%s' because filePath has not been set yet.", key );
	char fname[ 256 ];
	snprintf( fname, sizeof(fname), "%s/.%s", filesPath, key );
	FILE* f = 0;
#if defined(MSWIN)
	fopen_s( &f, fname, "w" );
#else
	f = fopen( fname, "w" );
#endif
	if ( !f )
		LOGE( "Cannot write key-value pair to '%s'", fname );
	ASSERTM( f, "Failed to open %s for writing.", fname );	// Has triggered in the wild, once.
	fprintf( f, "%d", v );
	fclose( f );
}


void kv_set_flt( const char* key, const float v )
{
	ASSERTM( filesPath, "Cannot set key '%s' because filePath has not been set yet.", key );
	char fname[ 256 ];
	snprintf( fname, sizeof(fname), "%s/.%s", filesPath, key );
	FILE* f = 0;
#if defined(MSWIN)
	fopen_s( &f, fname, "w" );
#else
	f = fopen( fname, "w" );
#endif
	if ( !f )
		LOGE( "Cannot write key-value pair to '%s'", fname );
	ASSERT( f );
	fprintf( f, "%f", v );
	fclose( f );
}


void kv_set_str( const char* key, const char* str )
{
	ASSERTM( filesPath, "Cannot set key '%s' because filePath has not been set yet.", key );
	char fname[ 256 ];
	snprintf( fname, sizeof(fname), "%s/.%s", filesPath, key );
	FILE* f = 0;
#if defined(MSWIN)
	fopen_s( &f, fname, "w" );
#else
	f = fopen( fname, "w" );
#endif
	if ( !f )
		LOGE( "Cannot write key-value pair to '%s'", fname );
	ASSERT( f );
	fprintf( f, "%s", str );
	fclose( f );

}


void kv_set_blob( const char* key, const char* blob, size_t sz )
{
	ASSERTM( filesPath, "Cannot set key '%s' because filePath has not been set yet.", key );
	char fname[ 256 ];
	snprintf( fname, sizeof(fname), "%s/.%s", filesPath, key );
	FILE *f = 0;
#if defined(MSWIN)
	fopen_s( &f, fname, "w" );
#else
	f = fopen( fname, "w" );
#endif
	if ( !f )
		LOGE( "Cannot write key-value pair to '%s'", fname );
	ASSERT( f );
	size_t numw = fwrite( blob, 1, sz, f );
	ASSERT( numw == sz );
	fclose( f );
}


int kv_get_int( const char* key, int defaultvalue )
{
	ASSERTM( filesPath, "Cannot get key '%s' because filePath has not been set yet.", key );
	char fname[ 256 ];
	snprintf( fname, sizeof(fname), "%s/.%s", filesPath, key );
	FILE* f = 0;
#if defined(MSWIN)
	fopen_s(&f, fname, "r" );
#else
	f = fopen( fname, "r" );
#endif
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
	ASSERTM( filesPath, "Cannot get key '%s' because filePath has not been set yet.", key );
	char fname[ 256 ];
	snprintf( fname, sizeof(fname), "%s/.%s", filesPath, key );
	FILE* f = 0;
#if defined(MSWIN)
	fopen_s(&f, fname, "r" );
#else
	f = fopen( fname, "r" );
#endif
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
	ASSERTM( filesPath, "Cannot get key '%s' because filePath has not been set yet.", key );
	char fname[ 256 ];
	snprintf( fname, sizeof(fname), "%s/.%s", filesPath, key );
	FILE* f = 0;
#if defined(MSWIN)
	fopen_s( &f, fname, "r" );
#else
	f = fopen( fname, "r" );
#endif
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
	ASSERTM( filesPath, "Cannot get key '%s' because filePath has not been set yet.", key );
	char fname[ 256 ];
	snprintf( fname, sizeof(fname), "%s/.%s", filesPath, key );
	FILE* f = 0;
#if defined(MSWIN)
	fopen_s( &f, fname, "r" );
#else
	f = fopen( fname, "r" );
#endif
	if ( f )
	{
		size_t numr = fread( blob, 1, maxsz, f );
		fclose( f );
		return numr;
	}
	else
		return 0;
}

