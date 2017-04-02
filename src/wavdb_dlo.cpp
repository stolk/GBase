// WavDB that uses dlopen on shared objects.

#include "wavdb.h"

#include <math.h>
#include <stdio.h>	// for snprintf()
#include <dlfcn.h>	// for dlopen()
#include <string.h>
#include <assert.h>

#include "logx.h"

static const int WAVDB_MAX_SZ=64;
static int wavdb_sz=0;
static const char* wavdb_names[ WAVDB_MAX_SZ ];
static int wavdb_lengths[ WAVDB_MAX_SZ ]; // in nr of shorts
static short* wavdb_waves[ WAVDB_MAX_SZ ];

const char* wavdb_path=0;

int wavdb_load( const char* pkgname, const char* lname, const char** names, int* lengths, int count )
{
    void* h = 0;
    char path[128];
#if defined( RASP )
    snprintf( path, 128, "./lib%s.so", lname );
#elif defined( JS )
    snprintf( path, 128, "/lib%s.so.js", lname );
#elif defined( XWIN )
    snprintf( path, 128, "./lib%s.so", lname );
#else
    snprintf( path, 128, "%s/lib%s.so", wavdb_path ? wavdb_path : ".", lname );
#endif

    h = dlopen( path, RTLD_NOW );
    if ( !h )
    {
        LOGE( "Could not load %s (%s)", path, dlerror() );
        snprintf( path, 128, "./lib%s.so", lname );
        h = dlopen( path, RTLD_NOW );
        if ( !h ) return 0;
    }

    int numLoaded = 0;
    for ( int i=0; i<count; ++i )
    {
        const char* name = names[ i ];
        char lname[128];
        char dname[128];

        snprintf( lname,128, "%s_len", name );
        snprintf( dname,128, "%s_dat", name );

        wavdb_waves[ wavdb_sz ] = (short*)dlsym(h, dname);
        if ( !wavdb_waves[ wavdb_sz ] )
        {
            LOGE( "Could not get symbol %s from %s (%s)", dname, path, dlerror() );
        }
        else
        {
            int* lp = (int*)  dlsym(h, lname);
            ASSERT( lp );
            wavdb_lengths[ wavdb_sz ] = *lp;
            wavdb_names[ wavdb_sz ] = name;
            wavdb_sz++;
            numLoaded++;
        }
    }
    // DO NOT CLOSE! WE KEEP USING THE POINTERS
    //dlclose(h);

    return numLoaded;
}


void wavdb_init(void)
{
}


void wavdb_lookup(const char* name, int* length, short** data)
{
    for ( int i=0; i<wavdb_sz; ++i )
        if ( !strcmp( wavdb_names[ i ], name ) )
        {
            *length = wavdb_lengths[ i ];
            *data = wavdb_waves[ i ];
            return;
        }
    LOGE( "Wave '%s' not stored in wavdb.", name );
}


void wavdb_prt(void)
{
	char s[1024];
	s[0] = 0;
	for ( int i=0; i<wavdb_sz; ++i )
	{
		char t[80];
		snprintf( t, 80, "%s(%d)%c", wavdb_names[ i ], wavdb_lengths[ i ], i==wavdb_sz-1?'.':',' );
		strncat( s, t, 1023 - strlen(s) );
	}
	LOGI( "wavdb contains %d waves: %s", wavdb_sz, s );
}

