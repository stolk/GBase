#include "txdb.h"

#include <math.h>
#include <stdio.h>	// for snprintf()
#include <iostream>

#include <windows.h>	// for LoadLibrary()

#include <string.h>
#include <assert.h>

#include "logx.h"
#include "checkogl.h"

static const int TXDB_MAX_SZ=96;
static int txdb_sz=0;
static const char* txdb_names[ TXDB_MAX_SZ ];
static GLuint txdb_values[ TXDB_MAX_SZ ];
const char* txdb_path = ".";


int txdb_load( const char* pkgname, const char* lname, const char** names, unsigned int* values, int count )
{
	wchar_t path[128];
	wchar_t nm[80];
	size_t rv;
	mbstowcs_s( &rv, nm, 80, lname, 80 );
	swprintf_s( path, 128, L"%s.dll", nm );

	HMODULE h = LoadLibrary( path );
	if ( !h )
	{
		LOGI("Could not load %s", lname);
		return 0;
	}

	int numLoaded = 0;
	for ( int i=0; i<count; ++i )
	{
        const char* name = names[ i ];
        char iname[128];
        char wname[128];
        char hname[128];
        _snprintf_s( iname,128, "%s", name );
        _snprintf_s( wname,128, "%s_w", name );
        _snprintf_s( hname,128, "%s_h", name );
        GLuint texture=0;
		unsigned int* p = (unsigned int*) GetProcAddress( h, iname );

        if ( !p )
        {
            if ( values ) values[ i ] = 0;
            LOGI("Could not get symbol %s from %s", name, lname);
        }
        else
        {
            int* szw = (int*)GetProcAddress(h,wname);
            int* szh = (int*)GetProcAddress(h,hname);
            ASSERT( szw );
            ASSERT( szh );

            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            CHECK_OGL

            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );  // REQ'D FOR NON SQUARE TEXTURES!
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );  // REQ'D FOR NON SQUARE TEXTURES!
            CHECK_OGL

            // Specify a 2D texture image, providing the a pointer to the image data in memory
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, *szw, *szh, 0, GL_RGBA, GL_UNSIGNED_BYTE, p);
            CHECK_OGL

            ASSERT( txdb_sz < TXDB_MAX_SZ );
            txdb_names[ txdb_sz ] = name;
            txdb_values[ txdb_sz ] = texture;
            txdb_sz += 1;
            numLoaded += 1;
            //LOGI("Loaded %s(%dx%d) as %02x at %p\n", name, *szw, *szh, texture, p );
            if ( values )
                values[ i ] = texture;
        }
	}
	FreeLibrary( h );

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
	_snprintf_s( s, 2048, "txdb contains %d textures: ", txdb_sz );
	for ( int i=0; i<txdb_sz; ++i )
	{
		char t[ 80 ];
		_snprintf_s( t, 80, "%s(%d)%c", txdb_names[ i ], txdb_values[ i ], i==txdb_sz-1?'.':',' );
		strncat_s( s, t, 2048 );
	}
	LOGI( "%s", s );
}

