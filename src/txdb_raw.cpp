#include "txdb.h"

#include <math.h>
#include <stdio.h>	// for snprintf()
#include <string.h>
#include <assert.h>
#include <dirent.h>
#include <stdlib.h>
#include <errno.h>

#include "logx.h"
#include "checkogl.h"

static const int TXDB_MAX_SZ=96;
static int txdb_sz=0;
static const char* txdb_names[ TXDB_MAX_SZ ];
static GLuint txdb_values[ TXDB_MAX_SZ ];
static const int32_t* txdb_images[ TXDB_MAX_SZ ];

const char* txdb_path = "/";

unsigned int txdb_load_from_memory( const char* name, const int32_t* raw, int szw, int szh, bool compressed )
{
	unsigned int texture=0;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	CHECK_OGL

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );  // REQ'D FOR NON SQUARE TEXTURES!
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );  // REQ'D FOR NON SQUARE TEXTURES!
	CHECK_OGL

	// Specify a 2D texture image, providing the a pointer to the image data in memory
	const int bpp = 4; // assume 4 bits per pixel (ETC2)
	if ( compressed )
#if defined( USE_ETC )
		glCompressedTexImage2D(	GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB8_ETC2, szw, szh, 0, szw*szh*bpp/8, raw );
#else
		glCompressedTexImage2D( GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, szw, szh, 0, szw*szh*bpp/8, raw );
#endif
	else
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, szw, szh, 0, GL_RGBA, GL_UNSIGNED_BYTE, raw );
	CHECK_OGL

	ASSERT( txdb_sz < TXDB_MAX_SZ );
	txdb_names [ txdb_sz ] = name;
	txdb_values[ txdb_sz ] = texture;
	txdb_images[ txdb_sz ] = raw;
	txdb_sz += 1;

	return texture;
}


static GLuint txdb_load_from_file( const char* name, int szw, int szh, FILE* f, bool compressed )
{
	int32_t* img = new int32_t[ szh * szw ];
	const size_t numr = fread( img, szw*szh*4, 1, f );
	if ( numr < 1 )
		LOGE( "Incomplete read from texture file of size %dx%d", szw, szh );
	return txdb_load_from_memory( name, img, szw, szh, compressed );
}


int txdb_load_compressed( const char** names, unsigned int* values, int count )
{
	int numLoaded = 0;
	char fname[ 256 ];
	unsigned char hdr[ 128 ];
	for ( int i=0; i<count; ++i )
	{
		const char* name = names[ i ];
		snprintf( fname, sizeof(fname), "%s/%s", txdb_path, name );
		FILE* f = fopen( fname, "rb" );
		if ( !f )
		{
			LOGE( "Could not open compressed texture '%s'", name );
		}
		else
		{
#if defined( USE_ETC )
			size_t rv = fread( hdr, 1, 16, f );
			ASSERT( rv == 16 );
			int szw = hdr[ 8]*256 + hdr[ 9];
			int szh = hdr[10]*256 + hdr[11];
			ASSERT( szw );
			ASSERT( szh );
#else
			size_t rv = fread( hdr,1, 128, f );
			ASSERT( rv == 128 );
			int szw;
			int szh;	
			szh = *( (int*) ( hdr+4+ 8 ) );
			szw = *( (int*) ( hdr+4+12 ) );
			ASSERT( szh );
			ASSERT( szw );
#endif
			const int bpp = 4;
			const int imsz = szw * szh * bpp / 8;
			int32_t* img = new int32_t[ imsz ];
			rv = fread( img, 1, imsz, f );
			ASSERT( rv == (size_t)imsz );
			const bool compressed = true;
			unsigned int texture = txdb_load_from_memory( name, img, szw, szh, compressed );
			numLoaded += 1;
			LOGI("Loaded compressed %s(%dx%d) as %02x", name, szw, szh, texture );
			if ( values )
				values[ i ] = texture;
		}
	}
	return numLoaded;
}


int txdb_load( const char* pkgname, const char* lname, const char** names, unsigned int* values, int count )
{
	int numLoaded = 0;
	DIR* dir = opendir( txdb_path );
	if ( !dir )
	{
		LOGE( "Cannot open dir %s to read textures. (%s)", txdb_path, strerror( errno ) );
		return 0;
	}
	struct dirent* entry;
	while ( ( entry = readdir( dir ) ) != 0 )
	{
		const char* n = entry->d_name;
		char fname[ 256 ];
		for ( int i=0; i<count; ++i )
		{
			const char* p = strchr( n, '.' );
			size_t nmlen = p ? p - n : -1;
			if ( nmlen > 0 && !strncmp( names[ i ], n, nmlen ) )
			{
				int w = atoi( p+1 );
				p = strchr( p, 'x' );
				if ( !p )
					continue;
				int h = atoi( p+1 );
				snprintf( fname, sizeof(fname), "%s/%s", txdb_path, n );
				FILE *f = fopen( fname, "rb" );
				ASSERT( f );
				const bool compressed = false;
				GLuint texture = txdb_load_from_file( names[ i ], w, h, f, compressed );
				fclose( f );
				numLoaded += 1;
				LOGI("Loaded %s(%dx%d) as %02x at %p", names[ i ], w, h, texture, p );
				if ( values )
					values[ i ] = texture;
				break;
			}
		}
	}
	closedir( dir );
	return numLoaded;
}


void txdb_init(void)
{
}


void txdb_clear(void)
{
	glDeleteTextures( txdb_sz, txdb_values );
	for ( int i=0; i<txdb_sz; ++i )
	{
		delete [] txdb_images[ i ];
		txdb_images[ i ] = 0;
	}
	LOGI( "Cleared %d textures from txdb", txdb_sz );
	CHECK_OGL
	txdb_sz = 0;
}


void txdb_use(const char* name)
{
    for ( int i=0; i<txdb_sz; ++i )
        if ( !strcmp( txdb_names[ i ], name ) )
        {
            glBindTexture( GL_TEXTURE_2D, txdb_values[ i ] );
            CHECK_OGL
            return;
        }
    LOGE( "Texture '%s' not stored in txdb.", name );
}


void txdb_prt(void)
{
	char s[2048];
	size_t space = sizeof(s);
	snprintf( s, 2048, "txdb contains %d textures: ", txdb_sz );
	for ( int i=0; i<txdb_sz; ++i )
	{
		char t[ 80 ];
		int sz = snprintf( t, 80, "%s(%d)%c", txdb_names[ i ], txdb_values[ i ], i==txdb_sz-1?'.':',' );
		strncat( s, t, space-1 );
		space -= sz;
	}
	LOGI( "%s", s );
}

