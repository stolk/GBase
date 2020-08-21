// odb.h
//
// Object DataBase.
// Does book keeping of physics and visual attributes of objects in the world.
// Capable of distance culling and drawing the objects (shadow, lines, faces) in the database.
//
// TODO: reinstate the recording and playback of the database.

#ifndef ODB_H
#define ODB_H

#include "vmath.h"
#include "rendercontext.h"
#include "geomdb.h"

#include "ode/ode.h"

const int WORLD_MAX_OBJS=256;
const int WORLD_MAX_JOINTS=160;
const int WORLD_MAX_SPACES=16;

const int REPLAYBUFFERSIZE = 8192;
const int REPLAYBUFFERMASK = ( REPLAYBUFFERSIZE-1 );

const int ODB_TYPE_MASK		= 0xf0000;
const int ODB_TYPE_REGULAR	= 0x00000;
const int ODB_TYPE_DIRT		= 0x10000;
const int ODB_TYPE_TERRAIN	= 0x20000;
const int ODB_TYPE_STRUCTURE	= 0x30000;
const int ODB_TYPE_LEVELBLOCK	= 0x40000;
const int ODB_TYPE_AMMO		= 0x50000;
const int ODB_TYPE_LASER	= 0x60000;

#define ASSIGN_DATA_TO_GEOM( GEOM, TYPE, NR ) \
	dGeomSetData( GEOM, (void*)(long)( NR | TYPE ) )

#define ASSIGN_DATA_TO_BODY( BODY, TYPE, NR ) \
	dBodySetData( BODY, (void*)(long)( NR | TYPE ) )

extern int odb_objCnt;						//!< The nr of objects (dynamic or not) in the world.
extern int odb_jntCnt;						//!< The nr of physics joints in the world.
extern int odb_spcCnt;						//!< The nr of spaces in the world (root space plus sub spaces.)

extern dWorldID		odb_world;				//!< World in which we do physics simulation.
extern dSpaceID		odb_rootspace;				//!< The root space for our world, holding collision geometry.
extern mat44_t		odb_adjustTrf;				//!< Auxiliary matrix used for rendering.

extern const char*	odb_names[ WORLD_MAX_OBJS ];		//!< Object names.
extern dBodyID		odb_bodies[ WORLD_MAX_OBJS ];		//!< Rigid bodies.
extern dBodyID		odb_master_bodies[ WORLD_MAX_OBJS ];	//!< Body enable status is inheritted from master body.
extern mat44_t		odb_trfs[ WORLD_MAX_OBJS ];		//!< Transformation matrix to render this object with.
extern geomdesc_t*	odb_descriptions[ WORLD_MAX_OBJS ];	//!< Visual models for objects.
extern bool		odb_culled[ WORLD_MAX_OBJS ];		//!< Is this object culled for rendering?
extern float		odb_cullmargin[ WORLD_MAX_OBJS ];	//!< A margin that is added to cull distance.
extern float		odb_timesincecoll[ WORLD_MAX_OBJS ];	//!< How long ago did this object collide with something?

extern dJointID		odb_joints[ WORLD_MAX_JOINTS ];		//!< Collection of joints working on the objects.

extern const char*	odb_spacenames[ WORLD_MAX_SPACES ];	//!< Names associated with the collision spaces.

//! Creates an empty db.
extern void odb_create( dWorldID worldid, dSpaceID spaceid );

//! Clear the db of all objects, joints and spaces.
extern void odb_destroy( void );

//! Updates transforms, culling and enabling.
extern void odb_update( float dt, vec3_t focuspt, float culldist );

//! Draws the faces of all objects in the db.
extern void odb_draw_main( const rendercontext_t& rc );

//! Draws the lines of all objects in the db.
extern void odb_draw_edge( const rendercontext_t& rc );

//! Draws the faces in the lightview.
extern void odb_draw_shdw( const rendercontext_t& rc );

//! Find named object in the db and return its index.
extern int  odb_lookup( const char* name );

extern dSpaceID odb_create_simple_space( const char* spacename, dSpaceID parentSpace );
extern dSpaceID odb_create_hash_space( const char* spacename, dSpaceID parentSpace, int minlevel, int maxlevel );
extern dSpaceID odb_create_quadtree_space( const char* spacename, dSpaceID parentSpace, dReal* ctr, dReal* ext, int depth );

#endif
