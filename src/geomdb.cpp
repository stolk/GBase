#include "geomdb.h"
#include "checkogl.h"
#include "vmath.h"
#include "logx.h"
#include "rendercontext.h"

#include <stdio.h>
#include <stdlib.h>

#if !defined(MSWIN)
#include <dirent.h>
#include <sys/stat.h>
#endif

static const int GEOMDB_MAX_SZ=64;
static int geomdb_sz=0;
static geomdesc_t *geoms[ GEOMDB_MAX_SZ ];
static int num_instances[ GEOMDB_MAX_SZ ];
static bool textured[ GEOMDB_MAX_SZ ];
static bool monochrome[ GEOMDB_MAX_SZ ];
static float* cached[ GEOMDB_MAX_SZ ];

int geomdb_ringslot=0;
static const int geomdb_numringslots=3;

bool geomdb_evict( int i );

static bool create_vbo( geomdesc_t* geomdesc, bool istextured )
{
	bool created=false;
	if ( !geomdesc->vbos[0] && geomdesc->numt )
	{
		CHECK_OGL
#ifdef USE_VAO
		glGenVertexArrays( 1, &geomdesc->vaos[0] );
		CHECK_OGL

		ASSERT( geomdesc->vaos[0] );
		glBindVertexArray( geomdesc->vaos[0] );
		CHECK_OGL
#else
		geomdesc->vaos[0] = 0;
#endif

		glGenBuffers( 1, &geomdesc->vbos[0] );
		if ( !geomdesc->vbos[0] )
		{
			// This happens for Android customers. I guess the device is out of memory?
			const char* errortext;
			ERR2STR( glGetError(), errortext );
			ASSERTM( geomdesc->vbos[0], "Failed to create a vertex buffer object for %s: %s", geomdesc->tag, errortext );
		}

		glBindBuffer( GL_ARRAY_BUFFER, geomdesc->vbos[0] );
		CHECK_OGL

		glBufferData( GL_ARRAY_BUFFER, geomdesc->vbo_sizes[0], geomdesc->vdata, GL_STATIC_DRAW );

		int stride = 9*sizeof(float);			// vx,vy,vz,nx,ny,nz,r,g,b
		if ( istextured ) stride = 8*sizeof(float);	// vx,vy,vz,nx,ny,nz,u,v

#ifdef USE_VAO
		glVertexAttribPointer( ATTRIB_VERTEX, 3, GL_FLOAT, 0, stride, 0 );
		glVertexAttribPointer( ATTRIB_NORMAL, 3, GL_FLOAT, 0, stride, (void*) ( 3 * sizeof(float) ) );
		if ( istextured )
			glVertexAttribPointer( ATTRIB_UV,  2, GL_FLOAT, 0, stride, (void*) ( 6 * sizeof(float) ) );
		else
			glVertexAttribPointer( ATTRIB_RGB, 3, GL_FLOAT, 0, stride, (void*) ( 6 * sizeof(float) ) );
		glEnableVertexAttribArray( ATTRIB_VERTEX );
		glEnableVertexAttribArray( ATTRIB_NORMAL );
		if ( istextured )
			glEnableVertexAttribArray( ATTRIB_UV );
		else
			glEnableVertexAttribArray( ATTRIB_RGB );
#endif

		glBindBuffer( GL_ARRAY_BUFFER, 0 );

#ifdef USE_VAO
		glBindVertexArray( 0 );
#endif
		created=true;
		CHECK_OGL
		//LOGI( "%s vbo created for '%s' with %zu floats.", istextured ? "textured" : "non-text", geomdesc->tag, geomdesc->vbo_sizes[ 0 ] / sizeof( float ) );
	}
	if ( !geomdesc->vbos[1] && geomdesc->nume )
	{
#ifdef USE_VAO
		glGenVertexArrays( 1, &geomdesc->vaos[1] );
		ASSERT( geomdesc->vaos[1] );
		glBindVertexArray( geomdesc->vaos[1] );
#else
		geomdesc->vaos[1] = 0;
#endif
        
		glGenBuffers( 1, &geomdesc->vbos[1] );
		ASSERT( geomdesc->vbos[1] );
        
		glBindBuffer( GL_ARRAY_BUFFER, geomdesc->vbos[1] );
		
		glBufferData( GL_ARRAY_BUFFER, geomdesc->vbo_sizes[1], geomdesc->edata, GL_STATIC_DRAW );
		
#ifdef USE_VAO
		glVertexAttribPointer( ATTRIB_VERTEX, 3, GL_FLOAT, 0, 3 * sizeof(float), 0 );
		glEnableVertexAttribArray( ATTRIB_VERTEX );
#endif

		glBindBuffer( GL_ARRAY_BUFFER, 0 );
        
#ifdef USE_VAO
		glBindVertexArray( 0 );
#endif
		created=true;
		CHECK_OGL
		//LOGI( "edges vao(%d)/vbo(%d) created for '%s' with %zu floats", geomdesc->vaos[1], geomdesc->vbos[1], geomdesc->tag, geomdesc->vbo_sizes[ 1 ] / sizeof( float ) );
	}
	return created;
}


bool geomdb_orphan_buffer( geomdesc_t* geomdesc, bool istextured, bool ismonochrome )
{
	for ( int i=0; i<geomdb_sz; ++i )
	{
		if ( geoms[ i ] == geomdesc )
		{
			const int b0 = 2*geomdb_ringslot + 0;
			const int b1 = 2*geomdb_ringslot + 1;
			ASSERT( geomdesc->vbos[ b0 ] );
			int numInst = num_instances[ i ];
			ASSERT( numInst );

			glBindBuffer( GL_ARRAY_BUFFER, geomdesc->vbos[ b0 ] );
			unsigned int instdatasz = sizeof(float)*16*numInst;
			if ( istextured )   instdatasz += sizeof(float)*2*numInst;
			if ( ismonochrome ) instdatasz += sizeof(float)*3*numInst;
			const unsigned int totalsz0 = geomdesc->vbo_sizes[ 0 ] + instdatasz;

			glBufferData( GL_ARRAY_BUFFER, totalsz0, NULL, GL_DYNAMIC_DRAW );
			CHECK_OGL
			glBufferSubData( GL_ARRAY_BUFFER, 0, geomdesc->vbo_sizes[ 0 ], geomdesc->vdata );
			CHECK_OGL
			if ( geomdesc->vbos[ b1 ] )
			{
				glBindBuffer( GL_ARRAY_BUFFER, geomdesc->vbos[ b1 ] );
				const unsigned int totalsz1 = geomdesc->vbo_sizes[ 1 ] + instdatasz;
				glBufferData( GL_ARRAY_BUFFER, totalsz1, NULL, GL_DYNAMIC_DRAW );
				CHECK_OGL
				glBufferSubData( GL_ARRAY_BUFFER, 0, geomdesc->vbo_sizes[ 1 ], geomdesc->edata );
				CHECK_OGL
			}
			return true;
		}
	}
	return false;
}


#if !defined( USEES2 )
static bool create_instanced_vbo( geomdesc_t* geomdesc, int numInst, int ringslot, bool istextured, bool ismonochrome )
{
#ifndef USE_VAO
	ASSERT( !numInst );	// not yet implemented for non-vao case.
	return false;
#else
	bool created=false;
	const int b0 = 2*ringslot + 0;
	const int b1 = 2*ringslot + 1;

	// Make vao/vbo for triangles.
	if ( !geomdesc->vbos[ b0 ] && geomdesc->numt )
	{
		glGenVertexArrays( 1, &geomdesc->vaos[ b0 ] );
		CHECK_OGL
		
		ASSERT( geomdesc->vaos[ b0 ] );
		glBindVertexArray( geomdesc->vaos[ b0 ] );
		CHECK_OGL

		glGenBuffers( 1, &geomdesc->vbos[ b0 ] );
		ASSERT( geomdesc->vbos[ b0 ] );
		
		glBindBuffer( GL_ARRAY_BUFFER, geomdesc->vbos[ b0 ] );
		CHECK_OGL
		
		unsigned int instsz = sizeof(float)*16*numInst;
		if ( istextured ) instsz += sizeof(float)*2*numInst;
		if ( ismonochrome ) instsz += sizeof(float)*3*numInst;
		const unsigned int totalsz = geomdesc->vbo_sizes[ 0 ] + instsz;

		glBufferData( GL_ARRAY_BUFFER, totalsz, NULL, GL_DYNAMIC_DRAW );
		CHECK_OGL

		glBufferSubData( GL_ARRAY_BUFFER, 0, geomdesc->vbo_sizes[ 0 ], geomdesc->vdata );
		CHECK_OGL
		
		int stride = 9*sizeof(float);			// vx,vy,vz,nx,ny,nz,r,g,b
		if ( istextured ) stride = 8*sizeof(float);	// vx,vy,vz,nx,ny,nz,u,v
		
		glVertexAttribPointer( ATTRIB_VERTEX, 3, GL_FLOAT, 0, stride, 0 );
		glVertexAttribPointer( ATTRIB_NORMAL, 3, GL_FLOAT, 0, stride, (void*) ( 3 * sizeof(float) ) );

		if ( istextured )
			glVertexAttribPointer( ATTRIB_UV,  2, GL_FLOAT, 0, stride, (void*) ( 6 * sizeof(float) ) );

		if ( !istextured && !ismonochrome )
			glVertexAttribPointer( ATTRIB_RGB, 3, GL_FLOAT, 0, stride, (void*) ( 6 * sizeof(float) ) );
		CHECK_OGL

		glVertexAttribDivisor( ATTRIB_TRF+0, 1 );
		glVertexAttribDivisor( ATTRIB_TRF+1, 1 );
		glVertexAttribDivisor( ATTRIB_TRF+2, 1 );
		glVertexAttribDivisor( ATTRIB_TRF+3, 1 );
		CHECK_OGL

		if ( istextured )
			glVertexAttribDivisor( ATTRIB_UVOFF, 1 );

		if ( ismonochrome )
			glVertexAttribDivisor( ATTRIB_RGB, 1 );

		size_t off = geomdesc->vbo_sizes[ 0 ];
		glVertexAttribPointer( ATTRIB_TRF+0,  4, GL_FLOAT, 0, 16 * sizeof(float), (void*) ( off +  0*sizeof(float) ) );
		glVertexAttribPointer( ATTRIB_TRF+1,  4, GL_FLOAT, 0, 16 * sizeof(float), (void*) ( off +  4*sizeof(float) ) );
		glVertexAttribPointer( ATTRIB_TRF+2,  4, GL_FLOAT, 0, 16 * sizeof(float), (void*) ( off +  8*sizeof(float) ) );
		glVertexAttribPointer( ATTRIB_TRF+3,  4, GL_FLOAT, 0, 16 * sizeof(float), (void*) ( off + 12*sizeof(float) ) );

		if ( istextured )
			glVertexAttribPointer( ATTRIB_UVOFF, 2, GL_FLOAT, 0, 2*sizeof(float), (void*) ( off + sizeof(float)*16*numInst) );

		if  ( ismonochrome )
			glVertexAttribPointer( ATTRIB_RGB, 3, GL_FLOAT, 0, 3*sizeof(float), (void*) ( off + sizeof(float)*16*numInst ) );

		CHECK_OGL

		glEnableVertexAttribArray( ATTRIB_VERTEX );
		glEnableVertexAttribArray( ATTRIB_NORMAL );
		
		if ( ismonochrome )
		{
			glEnableVertexAttribArray( ATTRIB_RGB );
		}
		else if ( istextured )
		{
			glEnableVertexAttribArray( ATTRIB_UV );
			glEnableVertexAttribArray( ATTRIB_UVOFF );
		}
		else
		{
			glEnableVertexAttribArray( ATTRIB_RGB );
		}
		glEnableVertexAttribArray( ATTRIB_TRF+0 );
		glEnableVertexAttribArray( ATTRIB_TRF+1 );
		glEnableVertexAttribArray( ATTRIB_TRF+2 );
		glEnableVertexAttribArray( ATTRIB_TRF+3 );

		CHECK_OGL
		
		glBindBuffer( GL_ARRAY_BUFFER, 0 );
		
		glBindVertexArray( 0 );

		created=true;
		CHECK_OGL
		//LOGI( "instanced vbo of size %d (%d instance data) created for '%s'", totalsz, instsz, geomdesc->tag );
	}

	// Make vao/vbo for edges.
	if ( !geomdesc->vbos[ b1 ] && geomdesc->nume )
	{
		glGenVertexArrays( 1, &geomdesc->vaos[ b1 ] );
		ASSERT( geomdesc->vaos[ b1 ] );
		glBindVertexArray( geomdesc->vaos[ b1 ] );

		glGenBuffers( 1, &geomdesc->vbos[ b1 ] );
		ASSERT( geomdesc->vbos[ b1 ] );
		
		glBindBuffer( GL_ARRAY_BUFFER, geomdesc->vbos[ b1 ] );
		
		unsigned int instdatasz = sizeof(float)*16*numInst;
		unsigned int totalsz = geomdesc->vbo_sizes[ 1 ] + instdatasz;
		glBufferData( GL_ARRAY_BUFFER, totalsz, NULL, GL_DYNAMIC_DRAW );
		glBufferSubData( GL_ARRAY_BUFFER, 0, geomdesc->vbo_sizes[ 1 ], geomdesc->edata );
		
		glVertexAttribDivisor( ATTRIB_TRF+0, 1 );
		glVertexAttribDivisor( ATTRIB_TRF+1, 1 );
		glVertexAttribDivisor( ATTRIB_TRF+2, 1 );
		glVertexAttribDivisor( ATTRIB_TRF+3, 1 );
		CHECK_OGL
		
		glVertexAttribPointer( ATTRIB_VERTEX, 3, GL_FLOAT, 0, 3 * sizeof(float), 0 );
		size_t off = geomdesc->vbo_sizes[ 1 ];
		glVertexAttribPointer( ATTRIB_TRF+0,  4, GL_FLOAT, 0, 16 * sizeof(float), (void*) ( off +  0*sizeof(float) ) );
		glVertexAttribPointer( ATTRIB_TRF+1,  4, GL_FLOAT, 0, 16 * sizeof(float), (void*) ( off +  4*sizeof(float) ) );
		glVertexAttribPointer( ATTRIB_TRF+2,  4, GL_FLOAT, 0, 16 * sizeof(float), (void*) ( off +  8*sizeof(float) ) );
		glVertexAttribPointer( ATTRIB_TRF+3,  4, GL_FLOAT, 0, 16 * sizeof(float), (void*) ( off + 12*sizeof(float) ) );
		CHECK_OGL
		
		glEnableVertexAttribArray( ATTRIB_VERTEX );
		glEnableVertexAttribArray( ATTRIB_TRF+0 );
		glEnableVertexAttribArray( ATTRIB_TRF+1 );
		glEnableVertexAttribArray( ATTRIB_TRF+2 );
		glEnableVertexAttribArray( ATTRIB_TRF+3 );
		
		glBindBuffer( GL_ARRAY_BUFFER, 0 );
		
		glBindVertexArray( 0 );
		created=true;
		CHECK_OGL
	}
	return created;
#endif
}
#endif


int geomdb_unload_vbos(void)
{
	int numUnloaded=0;
	for ( int i=0; i<geomdb_sz; ++i )
	{
		bool unloaded=false;
		geomdesc_t* geomdesc = geoms[ i ];
		for ( int j=0; j<geomdb_numringslots*2; ++j )
			if ( geomdesc && geomdesc->vbos[ j ] )
			{
#ifdef USE_VAO
				glDeleteVertexArrays( 1, &geomdesc->vaos[ j ] );
#endif
				geomdesc->vaos[ j ] = 0;
				glDeleteBuffers( 1, &geomdesc->vbos[ j ] );
				geomdesc->vbos[ j ] = 0;
				unloaded = true;
			}
		if ( unloaded ) numUnloaded++;
	}
	return numUnloaded;
}


int geomdb_load_vbos(void)
{
	int numLoaded=0;
	if ( !geomdb_sz )
		return 0;
	LOGI( "loading VBOs of geomdb containing %d entries:", geomdb_sz );
	char summation[ 2048 ];
	summation[ 0 ] = 0;
	for ( int i=0; i<geomdb_sz; ++i )
	{
		geomdesc_t* geomdesc = geoms[ i ];
		bool created=false;
#if !defined( USEES2 )
		if ( num_instances[ i ] > 1 )
		{
			for ( int s=0; s<geomdb_numringslots; ++s )
				created = create_instanced_vbo( geomdesc, num_instances[ i ], s, textured[ i ], monochrome[ i ] );
		}
		else
#endif
			created = create_vbo( geomdesc, textured[ i ] );
		if ( i ) strncat( summation, " ", sizeof( summation ) - 1 );
		strncat( summation, geomdesc->tag, sizeof( summation ) - 1 );
		if ( created ) numLoaded++;
	}
	LOGI( "%s", summation );
	return numLoaded;
}


int geomdb_clear( void )
{
	int rv = geomdb_unload_vbos();
	int numEvicted=0;
	for ( int i=0; i<geomdb_sz; ++i )
		if ( geomdb_evict( i ) )
			numEvicted++;
	LOGI( "geomdb cleared of %d entries (%d evicted from cache).", geomdb_sz, numEvicted );
	geomdb_sz = 0;
	return rv;
}


bool geomdb_add( geomdesc_t* geomdesc, int numInst, bool istextured, bool ismonochrome )
{
	// db full?
	if ( geomdb_sz == GEOMDB_MAX_SZ )
	{
		LOGE( "geomdb exceeded max capacity of %d entries.", geomdb_sz );
		return false;
	}

	// entry already in db?
	for ( int i=0; i<geomdb_sz; ++i )
		if ( geoms[ i ] == geomdesc )
			return false;

	// do we need to fetch from disk first?
	const bool need_fetch = ( geomdesc->vdata == 0 && geomdesc->edata == 0 );
	if ( need_fetch )
		cached[ geomdb_sz ] = geomdb_fetch( geomdesc );
	else
		cached[ geomdb_sz ] = 0;

	geoms[ geomdb_sz ] = geomdesc;
	num_instances[ geomdb_sz ] = numInst;
	textured[ geomdb_sz ] = istextured;
	monochrome[ geomdb_sz ] = ismonochrome;
	bool created = false;
#if !defined( USEES2 )
	if ( numInst > 1 )
	{
		for ( int s=0; s<geomdb_numringslots; ++s )
			created = create_instanced_vbo( geomdesc, numInst, s, istextured, ismonochrome );
	}
	else
#endif
	created = create_vbo( geomdesc, istextured );
	if ( created )
	{
		//LOGI( "Created vbo %d in slot %d numt=%d", geomdesc->vbos[ 0 ], geomdb_sz, geomdesc->numt );
		geomdb_sz++;
	}
	else
	{
		LOGE( "Failed to create vbo for geomdesc. numt=%d", geomdesc->numt );
	}

	return created;
}


bool geomdb_rmv( geomdesc_t* geomdesc )
{
	for ( int i=0; i<geomdb_sz; ++ i )
		if ( geoms[ i ] == geomdesc )
		{
			// Clean up GL stuff
			for ( int j=0; j<geomdb_numringslots*2; ++j )
				if ( geomdesc->vbos[ j ] )
				{
#ifdef USE_VAO
					glDeleteVertexArrays( 1, &geomdesc->vaos[j] );
#endif
					geomdesc->vaos[j] = 0;
					glDeleteBuffers( 1, &geomdesc->vbos[j] );
					geomdesc->vbos[j] = 0;
					LOGI( "geomdb removed '%s'", geomdesc->tag );
				}
			geomdb_evict( i );
			if ( i == geomdb_sz-1 )
				geomdb_sz--;
			else
			{
				geoms[ i ] = geoms[ geomdb_sz-1 ];
				num_instances[ i ] = num_instances[ geomdb_sz-1 ];
				textured[ i ] = textured[ geomdb_sz-1 ];
				monochrome[ i ] = monochrome[ geomdb_sz-1 ];
				cached[ i ] = cached[ geomdb_sz-1 ];
				geomdb_sz--;
			}
			return true;
		}
	return false;
}


void geomdb_bind( geomdesc_t* geomdesc, bool forEdge, bool istextured )
{
	// Always use ringslot 0 for non instanced VBOs.
	int ringslot = geomdb_ringslot;
	if ( !geomdesc->vbos[ 2 ] && !geomdesc->vbos[ 3 ] )
		ringslot = 0;
	const int buffernr = ( forEdge ? 1 : 0 ) + 2 * ringslot;
	ASSERT( geomdesc->vbos[ buffernr ] );
#ifdef USE_VAO
	ASSERT( geomdesc->vaos[ buffernr ] > 0 );
	glBindVertexArray( geomdesc->vaos[ buffernr ] );
	glBindBuffer( GL_ARRAY_BUFFER, geomdesc->vbos[ buffernr ] );
#else
        // set the ptrs!
	glBindBuffer( GL_ARRAY_BUFFER, geomdesc->vbos[ buffernr ] );
	if ( !forEdge )
	{
		const int stride = ( istextured ? 8 : 9 ) * sizeof(float);
		glVertexAttribPointer( ATTRIB_VERTEX, 3, GL_FLOAT, 0, stride, (void*) (  0 * sizeof(float) ) );
		glVertexAttribPointer( ATTRIB_NORMAL, 3, GL_FLOAT, 0, stride, (void*) (  3 * sizeof(float) ) );
		if ( istextured )
			glVertexAttribPointer( ATTRIB_UV,  2, GL_FLOAT, 0, stride, (void*) (  6 * sizeof(float) ) );
		else
			glVertexAttribPointer( ATTRIB_RGB, 3, GL_FLOAT, 0, stride, (void*) (  6 * sizeof(float) ) );
		glEnableVertexAttribArray( ATTRIB_VERTEX );
		glEnableVertexAttribArray( ATTRIB_NORMAL );
		glEnableVertexAttribArray( ATTRIB_RGB );
		CHECK_OGL
	}
	else
	{
		glBindBuffer( GL_ARRAY_BUFFER, geomdesc->vbos[ buffernr ] );
		glVertexAttribPointer( ATTRIB_VERTEX, 3, GL_FLOAT, 0, 3 * sizeof(float), (void*) ( 0 * sizeof(float) ) );
		glEnableVertexAttribArray( ATTRIB_VERTEX );
		//glDisableVertexAttribArray( ATTRIB_NORMAL );
		//glDisableVertexAttribArray( ATTRIB_RGB );
		CHECK_OGL
	}
#endif
	//LOGI( "Bound %d %s", forEdge ? geomdesc->nume : geomdesc->numt, forEdge ? "edges" : "triangles" );
}


void geomdb_unbind( void )
{
#ifdef USE_VAO
	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
#else
	glDisableVertexAttribArray( ATTRIB_VERTEX );
	glDisableVertexAttribArray( ATTRIB_NORMAL );
	glDisableVertexAttribArray( ATTRIB_RGB );
	glDisableVertexAttribArray( ATTRIB_UV );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
#endif
}


int geomdb_size( void )
{
	return geomdb_sz;
}


const char* geomdb_path = ".";

float* geomdb_fetch( geomdesc_t* geomdesc )
{
	float* cache = 0;
#if !defined(MSWIN)
	DIR* dir = opendir( geomdb_path );
	if ( !dir )
	{
		LOGE( "Cannot open dir %s", geomdb_path );
		return 0;
	}
	struct dirent* entry;
	while ( ( entry = readdir( dir ) ) != 0 )
	{
		const char* n = entry->d_name;
		const int l0 = (const int) strlen( geomdesc->tag );
		const int l1 = (const int) (strchr( n, '_' ) - n);
		if ( l0 == l1 && !strncmp( n, geomdesc->tag, strlen( geomdesc->tag ) ) )
		{
			struct stat filestats;
			char fname[256];
			snprintf( fname, sizeof(fname), "%s/%s", geomdb_path, n );
			const int rv = stat( fname, &filestats );
			if ( rv )
			{
				perror( "stat() failed" );
				LOGE( "Could not stat() %s", fname );
			}
			else
			{
				const char* p = n + strlen(geomdesc->tag);
				ASSERT( *p == '_' );
				p++;
				geomdesc->numt = atoi( p );
				p++;
				while ( *p != '_' ) p++;
				p++;
				geomdesc->nume = atoi( p );
				geomdesc->vbo_sizes[ 0 ] = geomdesc->numt * 3 * 9 * sizeof(float);
				geomdesc->vbo_sizes[ 1 ] = geomdesc->nume * 2 * 3 * sizeof(float);
				LOGI( "Fetching '%s' from disk: numt=%d nume=%d", geomdesc->tag, geomdesc->numt, geomdesc->nume );
				int numfloats = (int) filestats.st_size / sizeof( float );
				ASSERT( numfloats*sizeof(float) == geomdesc->vbo_sizes[ 0 ] + geomdesc->vbo_sizes[ 1 ] );
				FILE *f = fopen( fname, "rb" );
				ASSERT( f );
				cache = (float*) malloc( (int) filestats.st_size );
				const int numread = (int) fread( cache, sizeof(float), numfloats, f );
				ASSERT( numread == numfloats );
				geomdesc->vdata = cache;
				geomdesc->edata = cache + geomdesc->numt * 3 * 9;
				fclose( f );
			}
			break;
		}
	}
	if (!cache)
		LOGE("Failed to fetch %s from %s", geomdesc->tag, geomdb_path);
	closedir( dir );
#endif
	return cache;
}


bool geomdb_evict( int i )
{
	if ( cached[ i ] )
	{
		ASSERT( geoms[ i ] );
		free( cached[ i ] );
		cached[ i ] = 0;
		geoms[ i ]->vdata = geoms[ i ]->edata = 0;
		LOGI( "Evicted %s(%d) from cached geometries.", geoms[i]->tag, i );
		return true;
	}
	return false;
}


void geomdb_get_desc( char* desc, int sz )
{
#if defined(MSWIN)
	*desc = 0;
#else
	desc[ 0 ] = 0;
	char* writer = desc;
	for ( int i=0; i<geomdb_sz; ++i )
	{
		const char* tag = geoms[ i ]->tag;
		size_t curlen = strlen( writer );
		size_t szleft = sz - 1 - curlen;
		writer = strncat( writer, tag, szleft );
		szleft = sz - 1 - curlen;
		if ( i!=geomdb_sz-1 )
			writer = strncat( writer, "/", szleft );
	}
#endif
}


void geomdb_rotate_buffers( void )
{
	geomdb_ringslot = (geomdb_ringslot+1) % geomdb_numringslots;
}

