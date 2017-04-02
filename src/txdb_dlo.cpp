#include "txdb.h"

#include <math.h>
#include <stdio.h>	// for snprintf()
#include <dlfcn.h>	// for dlopen()
#include <string.h>
#include <assert.h>

#include "logx.h"
#include "checkogl.h"

static const int TXDB_MAX_SZ=96;
static int txdb_sz=0;
static const char* txdb_names[ TXDB_MAX_SZ ];
static GLuint txdb_values[ TXDB_MAX_SZ ];

const char* txdb_path=0;


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

	ASSERT( txdb_sz < TXDB_MAX_SZ );
	txdb_names[ txdb_sz ] = name;
	txdb_values[ txdb_sz ] = texture;
	txdb_sz += 1;

	return texture;
}


int txdb_load( const char* pkgname, const char* lname, const char** names, unsigned int* values, int count )
{
    void* h = 0;
    char path[128];

#if defined( RASP )
    snprintf( path, 128, "./lib%s.so", lname );
#elif defined( XWIN )
    snprintf( path, 128, "./lib%s.so", lname );
#elif defined( OSX )
    snprintf( path, 128, "./lib%s.dylib", lname );
#else
    // Android
    snprintf( path, 128, "%s/lib%s.so", txdb_path ? txdb_path : ".", lname );
#endif

    h = dlopen( path, RTLD_NOW );
    if ( !h )
    {
        LOGI( "Could not load %s (%s)", path, dlerror() );
        snprintf( path, 128, "./lib%s.so", lname );
        h = dlopen( path, RTLD_NOW );
        if ( !h ) return 0;
    }

    int numLoaded = 0;
    for ( int i=0; i<count; ++i )
    {
        const char* name = names[ i ];
        char iname[128];
        char wname[128];
        char hname[128];
        snprintf( iname,128, "%s", name );
        snprintf( wname,128, "%s_w", name );
        snprintf( hname,128, "%s_h", name );
        unsigned int* p = (unsigned int*)dlsym(h,iname);
        if ( !p )
        {
            if ( values ) values[ i ] = 0;
            LOGI( "Could not get symbol %s from %s (%s)", name, path, dlerror() );
        }
        else
        {
            int* szw = (int*)dlsym(h,wname);
            int* szh = (int*)dlsym(h,hname);
            ASSERT( szw );
            ASSERT( szh );

            const bool compressed = false;
            unsigned int texture = txdb_load_from_memory( name, p, *szw, *szh, compressed );
            numLoaded += 1;

            //LOGI("Loaded %s(%dx%d) as %02x at %p\n", name, *szw, *szh, texture, p );
            if ( values )
                values[ i ] = texture;
        }
    }
    dlclose(h);

    return numLoaded;
}


void txdb_init(void)
{
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
    LOGE( "Texture '%s' not stored in txdb.", name );
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

