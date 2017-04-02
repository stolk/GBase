#include "wavdb.h"

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "logx.h"

static const int WAVDB_MAX_SZ=64;
static int wavdb_sz=0;
static const char* wavdb_names[ WAVDB_MAX_SZ ];
static int wavdb_lengths[ WAVDB_MAX_SZ ]; // in nr of short ints
static short* wavdb_waves[ WAVDB_MAX_SZ ];

const char* wavdb_path = "./";

int wavdb_load( const char* pkgname, const char* lname, const char** names, int* lengths, int count )
{
	int numLoaded = 0;
	DIR* dir = opendir( wavdb_path );
	if ( !dir )
	{
		LOGE( "Cannot open dir %s", wavdb_path );
		return 0;
	}
	struct dirent* entry;
	while ( ( entry = readdir( dir ) ) != 0 )
	{
		const char* n = entry->d_name;
		for ( int i=0; i<count; ++i )
			if ( !strncmp( names[ i ], n, strlen( names[ i ] ) ) )
			{
				ASSERT( wavdb_sz < WAVDB_MAX_SZ );
				struct stat filestats;
				char fname[256];
				snprintf( fname, sizeof(fname), "%s/%s", wavdb_path, n );
				const int rv = stat( fname, &filestats );
				if ( rv )
				{
					perror( "stat() failed" );
					LOGE( "Could not stat() %s", fname );
				}
				else
				{
					int length = (int) filestats.st_size / sizeof( short );
					FILE *f = fopen( fname, "rb" );
					ASSERT( f );
					wavdb_lengths[ wavdb_sz ] = length;
					wavdb_waves[ wavdb_sz ] = (short*) malloc( (int) filestats.st_size );
					const int numread = (int) fread( wavdb_waves[ wavdb_sz ], sizeof(short), length, f );
					ASSERT( numread == length );
					fclose( f );
					wavdb_names[ wavdb_sz ] = names[ i ];
					if ( lengths ) lengths[ i ] = length;
					//LOGI("Loaded %s(%d)", names[ i ], length );
					wavdb_sz += 1;
					numLoaded += 1;
				}
				break;
			}
	}
	closedir( dir );
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
	size_t space = sizeof( s );
	s[0] = 0;
	for ( int i=0; i<wavdb_sz; ++i )
	{
		char t[80];
		int sz = snprintf( t, 80, "%s(%d)%c", wavdb_names[ i ], wavdb_lengths[ i ], i==wavdb_sz-1?'.':',' );
		strncat( s, t, space-1 );
		space -= sz;
	}
	LOGI( "wavdb contains %d waves: %s", wavdb_sz, s );
}

