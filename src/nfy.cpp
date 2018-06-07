// nfy.cpp
//
// (c)2012-2017 by Abraham Stolk


#include "nfy.h"


#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <float.h>


static const int NFY_MAX_OBS = 96;
static int nfy_num_obs = 0;
static notificationcallback_t nfy_cbk[ NFY_MAX_OBS ];
static const char* nfy_lbl[ NFY_MAX_OBS ];
static const int NFY_MAX_QUEUE_LEN = 16;
static const char* nfy_queued_messages[ NFY_MAX_QUEUE_LEN ];
static int nfy_queue_len = 0;


bool nfy_obs_add( const char* lbl, notificationcallback_t cbk )
{
    for ( int i=0; i<nfy_num_obs; ++i )
        if ( nfy_cbk[ i ] == cbk && !strcmp( nfy_lbl[ i ], lbl ) )
            return false; // We already have this observer recorded.

    if ( nfy_num_obs < NFY_MAX_OBS )
    {
        int i = nfy_num_obs++;
        nfy_cbk[ i ] = cbk;
        nfy_lbl[ i ] = lbl;
        return true;
    }
    return false;
}


bool nfy_obs_rmv( const char* lbl, notificationcallback_t cbk )
{
    int i=0;
    while ( i < nfy_num_obs && ( cbk != nfy_cbk[ i ] || strcmp( lbl, nfy_lbl[ i ] ) ) )
        ++i;
    if ( i == nfy_num_obs )
        return false;   // not found.
    if ( i == nfy_num_obs-1 )
    {
        nfy_num_obs--;
    }
    else
    {
        const int j = nfy_num_obs-1;
        nfy_cbk[ i ] = nfy_cbk[ j ];
        nfy_lbl[ i ] = nfy_lbl[ j ];
        nfy_num_obs--;
    }
    return true;
}


int nfy_msg( const char* msg )
{
    int invokation_count=0;
    const char* s = msg;
    while ( *s && *s!=' ' && *s!='\n' ) ++s;
    const long len = s-msg;
    if ( !len ) return invokation_count;
    for ( int i=0; i<nfy_num_obs; ++i )
    {
        const int rv = strncmp( msg, nfy_lbl[ i ], len );
        if ( rv == 0 && nfy_lbl[ i ][ len ] == 0 )
        {
            nfy_cbk[ i ]( msg );
            ++invokation_count;
        }
    }
    return invokation_count;
}


int nfy_int( const char* msg, const char* key )
{
	const char* s = msg;
	while ( *s!=0 && *s!=' ' && *s!='\n' ) ++s;
	s = strstr( s, key );
	if ( s )
	{
		while ( *s && *s!='=' ) ++s;
		if ( s[ 0 ] && s[ 1 ] )
			return atoi( s+1 );
	}
	return INT_MIN;
}


float nfy_flt( const char* msg, const char* key )
{
    const char* s = msg;
    while ( *s!=0 && *s!=' ' && *s!='\n' ) ++s;
	s = strstr( s, key );
	if ( s )
	{
		while ( *s && *s!='=' ) ++s;
		if ( s[ 0 ] && s[ 1 ] )
			return (float) atof( s+1 );
	}
	return -FLT_MAX;
}


void nfy_str( const char* msg, const char* key, char* dst, int dstlen )
{
	const char* s = msg;
	while ( *s!=0 && *s!=' ' && *s!='\n' ) ++s;
	s = strstr( s, key );
	if ( s )
	{
		while ( *s && *s!='=' ) ++s;
		if ( s[ 0 ] && s[ 1 ] )
		{
			++s;
			const char *t = s;
			while ( *t && *t!=' ' && *t!='\n' ) ++t;
			long len = t-s;
			if ( len )
			{
				if ( len >= dstlen )
					len = dstlen-1;
				memcpy( dst, s, len );
				dst[ len ] = 0;
				return;
			}
		}
	}
	// return empty string, if there is place for it.
	if ( dstlen ) dst[ 0 ] = 0;
}


bool nfy_match( const char* msg, const char* lbl )
{
	return strncmp( msg, lbl, strlen(lbl) ) == 0;
}


static void nfy_tst_cb0( const char* msg )
{
}

static void nfy_tst_cb1( const char* msg )
{
}


int nfy_queue_msg( const char* msg )
{
	if ( nfy_queue_len < NFY_MAX_QUEUE_LEN )
	{
		nfy_queued_messages[ nfy_queue_len++ ] = msg;
		return 1;
	}
	else
		return 0;
}


int nfy_process_queue( void )
{
	int rv=0;
	for ( int i=0; i<nfy_queue_len; ++i )
	{
		const char* msg = nfy_queued_messages[ i ];
		rv += nfy_msg( msg );
	}
	nfy_queue_len = 0;
	return rv;
}


bool nfy_tst( void )
{
	const char* m0 = "start levelname=court nr=2 height=3.14 missing=";
	const int i0 = nfy_int( m0, "nr" );
	if ( i0 != 2 )
        return false;
	const int i1 = nfy_int( m0, "missing" );
	if ( i1 != INT_MIN )
        return false;
	const float f0 = nfy_flt( m0, "height" );
	if ( f0 != 3.14f )
        return false;
	const bool b0 = nfy_match( m0, "stop" );
	if ( b0 == true )
        return false;
	const bool b1 = nfy_match( m0, "start" );
	if ( b1 == false )
        return false;
	char s0[ 5 ];
	nfy_str( m0, "levelname", s0, 5 );
	if ( strcmp( s0, "cour" ) )
        return false;
    
	const bool ok0 = nfy_obs_add( "start", nfy_tst_cb0 );
	if ( !ok0 )
		return false;
	const bool ok1 = nfy_obs_add( "stop", nfy_tst_cb1 );
	if ( !ok1 )
		return false;
	const int c0 = nfy_msg( m0 );
	if ( c0 != 1 )
		return false;
	const bool ok2 = nfy_obs_rmv( "start", nfy_tst_cb0 );
	if ( !ok2 )
		return false;
	const bool ok3 = nfy_obs_rmv( "stop", nfy_tst_cb1 );
	if ( !ok3 )
		return false;

	return true;
}


