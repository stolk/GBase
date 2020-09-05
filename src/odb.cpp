// Object DataBase.
//
// Functionality: 
//   updates render transformation matrices from the physics state.
//   draw light's view into shadow map.
//   draws objects in the world (triangles and edges.)
//
// (c)2017 Abraham Stolk

#include "odb.h"	// Our own interface.

#include "vmath.h"	// vector math.
#include "categories.h"	// categories for the physics collision.
#include "logx.h"	// logging functionality.
#include "geomdb.h"	// geometry database that maintains vbo collection.
#include "glpr.h"	// GL Program management.

#include "ode/ode.h"	// third party physics library: OpenDE.
#include "checkogl.h"	// macro for checking for GL errors, also GL configuration.

dWorldID odb_world=0;		//!< Physics world.
dSpaceID odb_rootspace=0;	//!< Root of physics collision space.

int odb_objCnt;	//!< How many world objects have been created? (size of this db.)
int odb_jntCnt;	//!< How many physics joints have been created?
int odb_spcCnt;	//!< How many physics spaces have been created?


const char*	odb_names[ WORLD_MAX_OBJS ];		//!< name, mainly for debugging purpose.

// Visual properties

geomdesc_t*	odb_descriptions[ WORLD_MAX_OBJS ];	//!< references to the render models.
mat44_t		odb_trfs[ WORLD_MAX_OBJS ];		//!< 4x4 transformation matrices.
float		odb_sqdist[ WORLD_MAX_OBJS ];		//!< square of the distance to camera.
bool		odb_culled[ WORLD_MAX_OBJS ];		//!< distance culled?
float		odb_cullmargin[ WORLD_MAX_OBJS ];	//!< for large objects, use larger cull dist.

// Physics properties

dBodyID		odb_bodies[ WORLD_MAX_OBJS ];		//!< optional rigid body associated with object.
dBodyID		odb_master_bodies[ WORLD_MAX_OBJS ];	//!< optional other rigid body that overrides body-disabling.

dJointID	odb_joints[ WORLD_MAX_JOINTS ];		//!< all physics joints used in world (n.b: not per object.)
const char*	odb_spacenames[ WORLD_MAX_SPACES ];	//!< names of the collision spaces (n.b: not per object.)

float		odb_timesincecoll[ WORLD_MAX_OBJS ];	//!< how long ago did this object last collide?


static const float adjustValues[ 16 ] =
{0.5, 0.0, 0.0, 0.0,
0.0, 0.5, 0.0, 0.0,
0.0, 0.0, 0.5, 0.0,
0.5, 0.5, 0.5, 1.0};
//! Auxiliary matrix used to adjust light view -1..1 to 0..1 texure coord.
mat44_t odb_adjustTrf = mat44_t( adjustValues );


//! Creates a physics world, the root collision space, and clears out the object database.
void odb_create( dWorldID worldid, dSpaceID spaceid )
{
	int numgeoms = dSpaceGetNumGeoms( spaceid );
	LOGI( "odb_create(%p) with space containing %d geoms.", (void*)spaceid, numgeoms );
	odb_world = worldid;
	odb_rootspace = spaceid;
	odb_objCnt = 0;
	odb_jntCnt = 0;
	odb_spcCnt = 0;
	memset( odb_names, 0, sizeof(char*) * WORLD_MAX_OBJS );
	memset( odb_bodies, 0, sizeof(dBodyID) * WORLD_MAX_OBJS );
	memset( odb_master_bodies, 0, sizeof(dBodyID) * WORLD_MAX_OBJS );
	memset( odb_trfs, 0, sizeof(mat44_t) * WORLD_MAX_OBJS );
	memset( odb_descriptions, 0, sizeof(geomdesc_t*) * WORLD_MAX_OBJS );
	memset( odb_cullmargin, 0, sizeof(float) * WORLD_MAX_OBJS );
	memset( odb_timesincecoll, 0, sizeof(float) * WORLD_MAX_OBJS );
	memset( odb_spacenames, 0, sizeof(const char*) * WORLD_MAX_SPACES );
	odb_spacenames[ odb_spcCnt++ ] = "odb_rootspace";
	dGeomSetData( (dGeomID) odb_rootspace, 0 );
}


//! Clear out the object database.
void odb_destroy( void )
{
	// We no longer need all those VBOs stored in the GPU's memory.
	int numremoved=0;
	for ( int i=0; i<odb_objCnt; ++i )
	{
		geomdesc_t* dsc = odb_descriptions[ i ];
		bool removed = geomdb_rmv( dsc );
		if ( removed )
			numremoved++;
		odb_names[ i ] = 0;
		odb_descriptions[ i ] = 0;
	}
	LOGI( "Removed %d descriptions from geomdb.", numremoved );
	odb_rootspace = 0;
	odb_world = 0;
	odb_objCnt = 0;
	odb_spcCnt = 0;
}


//! Look up entry in object database, using a name.
int odb_lookup( const char* name )
{
	for ( int i=0; i<odb_objCnt; ++i )
		if ( !strcmp( name, odb_names[ i ] ) )
			return i;
	return -1;
}


dSpaceID odb_create_hash_space( const char* spacename, dSpaceID parentSpace, int minlevel, int maxlevel )
{
	dSpaceID spc = dHashSpaceCreate( parentSpace );
	dHashSpaceSetLevels( spc, minlevel, maxlevel );
	odb_spacenames[ odb_spcCnt ] = spacename;
	odb_spcCnt++;
	return spc;
}


dSpaceID odb_create_simple_space( const char* spacename, dSpaceID parentSpace )
{
	dSpaceID spc = dSimpleSpaceCreate( parentSpace );
	dGeomSetData( (dGeomID)spc, (void*) (long) -odb_spcCnt );
	odb_spacenames[ odb_spcCnt ] = spacename;
	odb_spcCnt++;
	return spc;
}


dSpaceID odb_create_quadtree_space( const char* spacename, dSpaceID parentSpace, dReal* ctr, dReal* ext, int depth )
{
	dSpaceID spc = dQuadTreeSpaceCreate( parentSpace, ctr, ext, depth );
	dGeomSetData( (dGeomID)spc, (void*) (long) -odb_spcCnt );
	odb_spacenames[ odb_spcCnt ] = spacename;
	odb_spcCnt++;
	return spc;
}


//! Keep visual transformation matrices in sync with physics state (for those objects that have a rigid body.)
static void odb_update_transforms( void )
{
	for ( int i=0; i<odb_objCnt; ++i )
	{
		dBodyID body = odb_bodies[ i ];
		if ( body )
		{
			const dReal* pos = dBodyGetPosition( body );
			const dReal* R   = dBodyGetRotation( body );
			mat44_t& trf = odb_trfs[ i ];
			float* matrix = trf.data;
			matrix[0]=R[0];
			matrix[1]=R[4];
			matrix[2]=R[8];
			matrix[3]=0;
			matrix[4]=R[1];
			matrix[5]=R[5];
			matrix[6]=R[9];
			matrix[7]=0;
			matrix[8]=R[2];
			matrix[9]=R[6];
			matrix[10]=R[10];
			matrix[11]=0;
			matrix[12]=pos[0];
			matrix[13]=pos[1];
			matrix[14]=pos[2];
			matrix[15]=1;
#if 0
			const bool disabled = !dBodyIsEnabled( body );
			if ( disabled )
			{
				for (int j=0; j<12; ++j)
					matrix[j] *= 0.5f;
			}
#endif
		}
	}
}


//! Updates the transforms, and does a distance culling.
void odb_update( float dt, vec3_t focuspt, float culldist )
{
	odb_update_transforms();
	// Update times since collision
	for ( int i=0; i<odb_objCnt; ++i )
		odb_timesincecoll[ i ] += dt;
	// See how far away from focus pt, for culling purposes, and phyics-disabling.
	for ( int i=0; i<odb_objCnt; ++i )
	{
		const vec3_t p = odb_trfs[ i ].getRow( 3 );
		odb_sqdist[ i ] = 
			( p[0] - focuspt[0] ) * ( p[0] - focuspt[0] ) + 
			( p[1] - focuspt[1] ) * ( p[1] - focuspt[1] );
		bool was_culled = odb_culled[ i ];
		// We disable physics for distant objects, and avoid drawing them.
		const float culldistsq = ( culldist + odb_cullmargin[i] ) * ( culldist + odb_cullmargin[i] );
		odb_culled[ i ] = odb_sqdist[ i ] > culldistsq;
		if ( odb_bodies[ i ] )
		{
			if ( odb_culled[ i ] )
				dBodyDisable( odb_bodies[ i ] );
			else
				if ( was_culled )	// freshly unculled!
					dBodyEnable( odb_bodies[ i ] );
		}
	}
	// If there is a masterbody set, then this overrides our body disabling.
	for ( int i=0; i<odb_objCnt; ++i )
	{
		dBodyID mb   = odb_master_bodies[ i ];
		dBodyID body = odb_bodies[ i ];
		if ( mb )
		{
			if (!body)
				continue;
			const int mbe = dBodyIsEnabled( mb );
			const int e   = dBodyIsEnabled( odb_bodies[ i ] );
			if ( e != mbe )
			{
				if ( mbe )
				{
					dBodyEnable( body );
					odb_culled[ i ] = false;
				}
				else
				{
					dBodyDisable( body );
					odb_culled[ i ] = true;
				}
			}
		}
	}
}


//! Draw triangles of the objects in the db.
void odb_draw_main( const rendercontext_t& rc )
{
	const mat44_t camViewProjMat    = rc.camproj    * rc.camview;
	const mat44_t lightViewProjMat  = odb_adjustTrf * rc.lightproj  * rc.lightview;
#if defined(TWOLIGHTS) || defined(THREELIGHTS)
	const mat44_t auxil0ViewProjMat = odb_adjustTrf * rc.auxil0proj * rc.auxil0view;
#endif
#if defined(THREELIGHTS)
	const mat44_t auxil1ViewProjMat = odb_adjustTrf * rc.auxil1proj * rc.auxil1view;
#endif
	for ( int i=0; i<odb_objCnt; ++i )
	{
		geomdesc_t* geomdesc = odb_descriptions[ i ];
		mat44_t& trf = odb_trfs[ i ];
		if ( geomdesc && geomdesc->numt && !odb_culled[ i ] )
		{
			mat44_t modelCamViewProjMat	= camViewProjMat * trf;
			static int mcvpUniform = glpr_uniform( "modelcamviewprojmat" );
			glUniformMatrix4fv( mcvpUniform, 1, false, modelCamViewProjMat.data );

			mat44_t modelLightViewMat	= rc.lightview * trf;
			mat44_t modelLightViewProjMat	= lightViewProjMat * trf;
			static int mlvpUniform = glpr_uniform( "modellightviewprojmat" );
			static int mlvUniform  = glpr_uniform( "modellightviewmat" );
			glUniformMatrix4fv( mlvpUniform, 1, false, modelLightViewProjMat.data );
			glUniformMatrix4fv( mlvUniform,  1, false, modelLightViewMat.data );

#if defined(TWOLIGHTS) || defined(THREELIGHTS)
			mat44_t modelAuxil0ViewMat	= rc.auxil0view * trf;
			mat44_t modelAuxil0ViewProjMat	= auxil0ViewProjMat * trf;
			static int ma0vpUniform = glpr_uniform( "modelauxil0viewprojmat" );
			static int ma0vUniform  = glpr_uniform( "modelauxil0viewmat" );
			glUniformMatrix4fv( ma0vpUniform, 1, false, modelAuxil0ViewProjMat.data );
			glUniformMatrix4fv( ma0vUniform,  1, false, modelAuxil0ViewMat.data );
#endif
#if defined(THREELIGHTS)
			mat44_t modelAuxil1ViewMat	= rc.auxil1view * trf;
			mat44_t modelAuxil1ViewProjMat	= auxil1ViewProjMat * trf;
			static int ma1vpUniform = glpr_uniform( "modelauxil1viewprojmat" );
			static int ma1vUniform  = glpr_uniform( "modelauxil1viewmat" );
			glUniformMatrix4fv( ma1vpUniform, 1, false, modelAuxil1ViewProjMat.data );
			glUniformMatrix4fv( ma1vUniform,  1, false, modelAuxil1ViewMat.data );
#endif

#ifdef USE_VAO
			glBindVertexArray( geomdesc->vaos[0] );
#else
			// set the ptrs!
			glBindBuffer( GL_ARRAY_BUFFER, geomdesc->vbos[0] );
			glVertexAttribPointer( ATTRIB_VERTEX, 3, GL_FLOAT, 0, 9 * sizeof(float), (void*) (  0 * sizeof(float) ) );
			glVertexAttribPointer( ATTRIB_NORMAL, 3, GL_FLOAT, 0, 9 * sizeof(float), (void*) (  3 * sizeof(float) ) );
			glVertexAttribPointer( ATTRIB_RGB,    3, GL_FLOAT, 0, 9 * sizeof(float), (void*) (  6 * sizeof(float) ) );
			glEnableVertexAttribArray( ATTRIB_VERTEX );
			glEnableVertexAttribArray( ATTRIB_NORMAL );
			glEnableVertexAttribArray( ATTRIB_RGB );
#endif
			const int offset = 0;
			glDrawArrays( GL_TRIANGLES, offset, geomdesc->numt * 3 );
#ifdef USE_VAO
			glBindVertexArray( 0 );
#else
			glDisableVertexAttribArray( ATTRIB_VERTEX );
			glDisableVertexAttribArray( ATTRIB_NORMAL );
			glDisableVertexAttribArray( ATTRIB_RGB );
			glBindBuffer( GL_ARRAY_BUFFER, 0 );
#endif
		}

	}
}


//! Draw the triangles of the objects in the db, into the shadow map.
void odb_draw_shdw( const rendercontext_t& rc, int lightnr )
{
#if defined(TWOLIGHTS) || defined(THREELIGHTS)
	const mat44_t& lightview = lightnr==2 ? rc.auxil1view : ( lightnr==1 ? rc.auxil0view : rc.lightview );
	const mat44_t& lightproj = lightnr==2 ? rc.auxil1proj : ( lightnr==1 ? rc.auxil0proj : rc.lightproj );
	const mat44_t  lightViewProjMat = lightproj * lightview;
#else
	ASSERT(lightnr==0);
	const mat44_t lightViewProjMat = rc.lightproj * rc.lightview;
#endif

	for ( int i=0; i<odb_objCnt; ++i )
	{
		geomdesc_t* geomdesc = odb_descriptions[ i ];
		mat44_t& trf = odb_trfs[ i ];
		if ( geomdesc && geomdesc->numt && !odb_culled[ i ] )
		{
			mat44_t modelLightViewProjMat = lightViewProjMat * trf;
			static int mlvpUniform = glpr_uniform( "modellightviewprojmat" );
			glUniformMatrix4fv( mlvpUniform, 1, false, modelLightViewProjMat.data );
#ifdef USE_VAO
			glBindVertexArray( geomdesc->vaos[0] );
#else
			// set the ptrs!
			glBindBuffer( GL_ARRAY_BUFFER, geomdesc->vbos[0] );
			glVertexAttribPointer( ATTRIB_VERTEX, 3, GL_FLOAT, 0, 9 * sizeof(float), (void*) (  0 * sizeof(float) ) );
			glEnableVertexAttribArray( ATTRIB_VERTEX );
#endif
			//LOGI( "drawing nr %02d: numt=%d vbo=%d (%s) %f,%f,%f", i, geomdesc->numt, geomdesc->vbos[0], odb_names[i], modelLightViewProjMat.getTranslation()[0], modelLightViewProjMat.getTranslation()[1], modelLightViewProjMat.getTranslation()[2] );
			const int offset = 0;
			glDrawArrays( GL_TRIANGLES, offset, geomdesc->numt * 3 );
#ifdef USE_VAO
			glBindVertexArray( 0 );
#else
			glDisableVertexAttribArray( ATTRIB_VERTEX );
			glBindBuffer( GL_ARRAY_BUFFER, 0 );
#endif
		}

	}
}


//! Draw the edges of all objects in the db.
void odb_draw_edge( const rendercontext_t& rc )
{
	const mat44_t camViewProjMat = rc.camproj * rc.camview;
	for ( int i=0; i<odb_objCnt; ++i )
	{
		geomdesc_t* geomdesc = odb_descriptions[ i ];
		mat44_t& trf = odb_trfs[ i ];
		if ( geomdesc && geomdesc->nume && !odb_culled[ i ] )
		{
			mat44_t modelCamViewProjMat = camViewProjMat * trf;
			
			static int mcvpUniform = glpr_uniform( "modelcamviewprojmat" );
			glUniformMatrix4fv( mcvpUniform, 1, false, modelCamViewProjMat.data);
#ifdef USE_VAO
			glBindVertexArray( geomdesc->vaos[1] );
#else
			glBindBuffer( GL_ARRAY_BUFFER, geomdesc->vbos[1] );
			CHECK_OGL
			glVertexAttribPointer( ATTRIB_VERTEX, 3, GL_FLOAT, 0, 3 * sizeof(float), (void*) ( 0 * sizeof(float) ) );
			CHECK_OGL
			glEnableVertexAttribArray( ATTRIB_VERTEX );
			CHECK_OGL
#endif
			const int offset = 0;
			glDrawArrays( GL_LINES, offset, geomdesc->nume * 2 );
#ifdef USE_VAO
			glBindVertexArray( 0 );
#else
			glDisableVertexAttribArray( ATTRIB_VERTEX );
			glBindBuffer( GL_ARRAY_BUFFER, 0 );
#endif
		}
	}
}

