// geomdb.h
//
// Database for render meshes.
// Responsible for creating VAO/VBO for the geometry.
//
// (c)2012-2017 Game Studio Abraham Stolk Inc.

#ifndef GEOMDB_H
#define GEOMDB_H

#include "baseconfig.h"

#define GEOMDEF(A) \
static geomdesc_t dsc_##A = \
{ \
#A, \
nump_##A, \
nume_##A, \
(float*)vdata_##A[0], \
(float*)edges_##A[0], \
{0,0,0,0,0,0}, \
{sizeof(vdata_##A), sizeof(edges_##A)}, \
{0,0,0,0,0,0} \
};

#define GEOMEMPTY(A) \
static geomdesc_t dsc_##A = \
{ \
#A, \
0, 0, \
0, 0, \
{0,0,0,0,0,0}, \
{0,0}, \
{0,0,0,0,0,0}, \
};


typedef struct
{
	const char* tag;		// tag name.
	int numt;			// number of triangles.
	int nume;			// number of edges.
	float* vdata;			// vertex data for triangles.
	float* edata;			// vertex data for edges.
	
	unsigned int vbos[6];		// triple buffered: faces, edges, faces, edges, faces, edges.
	unsigned int vbo_sizes[2];	// size for faces, size for edges.
	unsigned int vaos[6];		// triple buffered: faces, edges, faces, edges, faces, edges.
} geomdesc_t;

extern const char* geomdb_path;

//! Clears out all cached VBO and VAO data, clears optional disk-cache. Returns the nr of geometry entries purged.
extern int geomdb_clear( void );

//! Add a geometry to the db, will cache the geometry as VAO / VBO.
extern bool geomdb_add( geomdesc_t* geomdesc, int numInst = 1, bool textured = false, bool monochrome = false );

//! Remove a geometry fromthe db, will clean up VAO and VBO.
extern bool geomdb_rmv( geomdesc_t* geomdesc );

//! Delete all vbo's
extern int geomdb_unload_vbos(void);

//! Load all vbo's
extern int geomdb_load_vbos(void);

//! Bind the VAO or VBO
extern void geomdb_bind( geomdesc_t* geomdesc, bool forEdge, bool istextured=false );

//! Unbind the VAO or VBO
extern void geomdb_unbind( void );

//! Returns the nr of geoms that have been registered.
extern int geomdb_size( void );

//! Fetch from file
extern float* geomdb_fetch( geomdesc_t* geomdesc );

//! Get a description of db contents.
extern void geomdb_get_desc( char* desc, int sz );

//! Orphan VBO buffer.
extern bool geomdb_orphan_buffer( geomdesc_t* dsc, bool istextured=false, bool ismonochrome=false );

//! Swap the front/back buffers of VBOs that are used for instanced drawing.
extern void geomdb_rotate_buffers( void );

#endif
