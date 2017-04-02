// meshdb.h
//
// Database for physics meshes.
// Responsible for creating OpenDE meshes.
//
// (c)2012-2017 Game Studio Abraham Stolk Inc.

#include "vmath.h"
#include "geomdb.h"
#include <ode/ode.h>

extern dTriMeshDataID meshdb_add( const char* name, geomdesc_t* desc );

extern void meshdb_clear( void );

extern int meshdb_count( void );
