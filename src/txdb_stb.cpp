// texture database, where images are loaded from file, using stb (and android_fopen.)

// Our own interface.
#include "txdb.h"

#if defined(ANDROID)
#	include "android_fopen.h"
#	define fopen android_fopen
#endif

// Sigh, not defined on MacOS?
#if !defined(GL_COMPRESSED_RGB8_ETC2)
#      define GL_COMPRESSED_RGB8_ETC2           0x9274
#endif

#include <math.h>
#include <stdio.h>	// for snprintf()
#include <sys/types.h>	// req'd by VisualC for off_t
#include <string.h>
#include <assert.h>

#include "logx.h"
#include "checkogl.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb_image.h"

static const int TXDB_MAX_SZ=96;
static int txdb_sz=0;
static const char* txdb_names[ TXDB_MAX_SZ ];
static GLuint txdb_values[ TXDB_MAX_SZ ];

const char* txdb_path=".";

unsigned int txdb_bgcolour = 0x00000000;

unsigned int txdb_load_from_memory( const char* name, const unsigned int* raw, int szw, int szh, bool compressed )
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

#if !defined( USEES3 ) && !defined( USECOREPROFILE )
	ASSERT( !compressed );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, szw, szh, 0, GL_RGBA, GL_UNSIGNED_BYTE, raw );
#else
	// Specify a 2D texture image, providing the a pointer to the image data in memory
	const int bpp = 4; // assume 4 bits per pixel (ETC2)
	if ( compressed )
		glCompressedTexImage2D(	GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB8_ETC2, szw, szh, 0, szw*szh*bpp/8, raw );
	else
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, szw, szh, 0, GL_RGBA, GL_UNSIGNED_BYTE, raw );
#endif
	CHECK_OGL

	// Reuse a slot?
	for ( int i=0; i<txdb_sz; ++i )
	{
		if ( !strcmp( txdb_names[ i ], name ) )
		{
			// Refresh the old texture at this slot.
			glDeleteTextures( 1, txdb_values+i );
			txdb_values[ i ] = texture;
			return texture;
		}
	}

	// No. Put in a new slot.
	ASSERT( txdb_sz < TXDB_MAX_SZ );
	txdb_names[ txdb_sz ] = name;
	txdb_values[ txdb_sz ] = texture;
	txdb_sz += 1;

	return texture;
}


int txdb_load( const char* pkgname, const char* lname, const char** names, unsigned int* values, int count )
{
	int numLoaded = 0;
	for ( int i=0; i<count; ++i )
	{
		const char* name = names[ i ];
		char fname[128];
		snprintf(fname, sizeof(fname), "%s/%s/%s.png", txdb_path, lname, name);
		int szw=0;
		int szh=0;
		int numch=0;
		unsigned int* p = (unsigned int*) stbi_load(fname, &szw, &szh, &numch, 4);
		if (!p)
		{
			LOGE("Failed to find asset %s", fname);
			if ( values ) values[ i ] = 0;
		}
		else
		{
			ASSERT(szw && szh);
			//LOGI( "%s has pixel(0,0) %x", name, * ( (unsigned int*)p ) );
			for ( int j=0; j<szw*szh; ++j )
				if ( p[j] == 0x00000000 )
					p[j] = txdb_bgcolour;
			const bool compressed = false;
			unsigned int texture = txdb_load_from_memory( name, (unsigned int*)p, szw, szh, compressed );
			numLoaded += 1;
			stbi_image_free(p);
			//LOGI( "Loaded %s(%dx%d) as %02x at %p with numch=%d\n", name, szw, szh, texture, p, numch );
			if ( values ) values[ i ] = texture;
		}
	}
	return numLoaded;
}


void txdb_init(void)
{
	stbi_set_flip_vertically_on_load(1);
	//stbi_set_unpremultiply_on_load(1);
}


void txdb_clear(void)
{
	glDeleteTextures( txdb_sz, txdb_values );
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
    LOGE( "Texture '%s' not stored in txdb of size %d.", name, txdb_sz );
}


void txdb_prt(void)
{
	char s[2048];
	snprintf( s, 2048, "txdb contains %d textures: ", txdb_sz );
	for ( int i=0; i<txdb_sz; ++i )
	{
		char t[ 80 ];
		snprintf( t, 80, "%s(%d)%c", txdb_names[ i ], txdb_values[ i ], i==txdb_sz-1?'.':',' );
		strncat( s, t, 2047 - strlen(s) );
	}
	LOGI( "%s", s );
}

