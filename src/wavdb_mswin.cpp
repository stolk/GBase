#include "wavdb.h"

#include <math.h>
#include <stdio.h>	// for snprintf()
#include <windows.h>	// for DLL opening.
#include <string.h>
#include <assert.h>

#include "logx.h"

static const int WAVDB_MAX_SZ=64;
static int wavdb_sz=0;
static const char* wavdb_names[ WAVDB_MAX_SZ ];
static int wavdb_lengths[ WAVDB_MAX_SZ ];
static short* wavdb_waves[ WAVDB_MAX_SZ ];
const char* wavdb_path = ".";


int wavdb_load( const char* pkgname, const char* libname, const char** names, int* lengths, int count )
{
	wchar_t path[128];
	wchar_t nm[80];
	size_t rv;
	mbstowcs_s( &rv, nm, 80, libname, 80 );
	swprintf_s( path, 128, L"%s.dll", nm );

	HMODULE h = LoadLibrary( path );
    if ( !h )
    {
        LOGI("Could not load %s", libname);
        return 0;
    }

    int numLoaded = 0;
    for ( int i=0; i<count; ++i )
    {
        const char* name = names[ i ];
        char lname[128];
        char dname[128];

        snprintf( lname,128, "%s_len", name );
        snprintf( dname,128, "%s_dat", name );

        wavdb_waves[ wavdb_sz ] = (short*) GetProcAddress( h, dname );
        if ( !wavdb_waves[ wavdb_sz ] )
        {
            LOGE( "Could not get symbol %s from %s", dname, libname );
        }
        else
        {
			unsigned int* lp = (unsigned int*) GetProcAddress( h, lname );
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
	snprintf( s, 1024, "wavdb contains %d waves: ", wavdb_sz );
	for ( int i=0; i<wavdb_sz; ++i )
	{
		char t[80];
		snprintf( t, 80, "%s(%d)%c", wavdb_names[ i ], wavdb_lengths[ i ], i==wavdb_sz-1?'.':',' );
		strncat_s( s, t, 1024 );
	}
	LOGI( s );
}

