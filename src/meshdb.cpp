#include "meshdb.h"
#include "logx.h"
#include "checkogl.h"

#define MAXSZ 16
static int cnt = 0;
static const char* names[ MAXSZ ];
static geomdesc_t* descriptions[ MAXSZ ];
static dTriMeshDataID meshes[ MAXSZ ];
static float* block_vdata[ MAXSZ ];
static float* block_ndata[ MAXSZ ];
static unsigned int* block_indices[ MAXSZ ];


dTriMeshDataID meshdb_add( const char* name, geomdesc_t* desc )
{
	// Lookup to see if it is already loaded in the meshdb.
	for ( int i=0; i<cnt; ++i )
	{
		if ( !strcmp( name, names[ i ] ) )
			return meshes[ i ];
	}
	if ( cnt == MAXSZ )
		return 0;
	// Create the mesh and add it to the meshdb.
	names[ cnt ] = name;
	descriptions[ cnt ] = desc;
	const int numtria = desc->numt;
	unsigned int* indices = (unsigned int*)malloc(numtria*3*sizeof(unsigned int));
	float* vdata = (float*)malloc(numtria*9*sizeof(float));
	float* ndata = (float*)malloc(numtria*9*sizeof(float));
	for ( int i=0; i<numtria*3; i++ )
	{
		memcpy(vdata + i * 3, desc->vdata + i*9 + 0, 3*sizeof(float) );
		memcpy(ndata + i * 3, desc->vdata + i*9 + 3, 3*sizeof(float) );
		indices[ i ] = i;
	}
	meshes[ cnt ] = dGeomTriMeshDataCreate();
	dGeomTriMeshDataBuildSingle1
	(
		meshes[ cnt ],
		vdata,
		3*sizeof(float),
		3*numtria,
		indices,
		numtria*3,
		3*sizeof(int),
		ndata
	);
	block_indices[ cnt ] = indices;
	block_vdata[ cnt ] = vdata;
	block_ndata[ cnt ] = ndata;
	return meshes[ cnt++ ];
}


void meshdb_clear( void )
{
	for ( int i=0; i<cnt; ++i )
	{
		free( block_vdata[ i ] );
		free( block_ndata[ i ] );
		free( block_indices[ i ] );
		dGeomTriMeshDataDestroy( meshes[ i ] );
		meshes[ i ] = 0;
	}
	LOGI( "Cleared meshdb of size %d.", cnt );
	cnt = 0;
}


int meshdb_count( void )
{
	return cnt;
}
