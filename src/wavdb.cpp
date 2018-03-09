#include "wavdb.h"

#include <math.h>
#include <stdio.h>	// for snprintf()
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/types.h>	// req'd for visualC.

#if defined(ANDROID)
#	include "Android/android_fopen.h"
#	define fopen android_fopen
#endif

#include "logx.h"

static const int WAVDB_MAX_SZ=64;
static int wavdb_sz=0;
static const char* wavdb_names[ WAVDB_MAX_SZ ];
static int wavdb_lengths[ WAVDB_MAX_SZ ]; // in nr of shorts
static short* wavdb_waves[ WAVDB_MAX_SZ ];

const char* wavdb_path=0;

int wavdb_load( const char* pkgname, const char* lname, const char** names, int* lengths, int count )
{
	int numLoaded = 0;
	for ( int i=0; i<count; ++i )
	{
		const char* name = names[ i ];
		char fname[128];
		snprintf(fname, sizeof(fname), "%s/%s/%s.wav", wavdb_path, lname, name);

		FILE* f = fopen(fname, "rb");
		if (f)
		{
			const size_t hdrsize = 44;	// Assume 44 byte header - DANGEROUS!
			const off_t endpos = fseek(f, 0, SEEK_END);
			if (endpos < 0)
				perror("fseek");
			const size_t filelength = ftell(f);
			wavdb_names[ wavdb_sz ] = name;
			wavdb_lengths[ wavdb_sz ] = (int) ((filelength - hdrsize)/2);
			fseek(f, hdrsize, SEEK_SET);
			wavdb_waves[wavdb_sz] = (short*)malloc(wavdb_lengths[wavdb_sz] * sizeof(short));
			const size_t res = fread(wavdb_waves[wavdb_sz], sizeof(short), wavdb_lengths[wavdb_sz], f);
			ASSERT((int)res == wavdb_lengths[wavdb_sz]);
			fclose(f);
			wavdb_sz++;
			numLoaded++;
		}
		else
		{
			LOGE( "Failed to load %s", fname );
		}
	}
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

