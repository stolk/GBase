#include "glpr.h"

#include <stdlib.h>
#include <stdio.h>

#include "checkogl.h"
#include "vmath.h"
#include "rendercontext.h"
#include "logx.h"


#define MAXUNIFORMS	512
#define MAXGLPROGS	18

static int		glpr_unif[ MAXUNIFORMS ];	//!< uniform values.
static const char*	glpr_name[ MAXUNIFORMS ];	//!< uniform names.
static unsigned int	glpr_unif_prog[ MAXUNIFORMS ];	//!< programs associated with uniforms.
static int		glpr_numu;			//!< nr of uniforms.

static unsigned int	glpr_programs[ MAXGLPROGS ];
static int		glpr_numprograms;


// To avoid name-clashes between different programs, we use a start index where we search for a uniform name.
static int		glpr_searchindex;
static unsigned int	glpr_usedprogram;

const char*		glpr_last_compile_log;		//!< the last GLSL compile log.
const char*		glpr_last_link_log;		//!< the last GLSL link log.

void glpr_init( void )
{
	if ( glpr_numprograms || glpr_numu )
		LOGE( "glpr is leaking! numprograms: %d numuniforms: %d", glpr_numprograms, glpr_numu );
	glpr_numprograms = 0;
	glpr_numu = 0;
	glpr_last_compile_log = "";
	glpr_last_link_log = "";
}


void glpr_clear( void )
{
	LOGI( "Clearing out %d programs and %d uniforms...", glpr_numprograms, glpr_numu );
	// Clear out the uniforms.
	for ( int i=0; i<glpr_numu; ++i )
	{
		glpr_unif[ i ] = -1;
		if ( glpr_name[ i ] )
			free( (void*) glpr_name[ i ] );
		glpr_name[ i ] = 0;
		glpr_unif_prog[ i ] = 0;
	}
	glpr_numu = 0;

	// Clear out the programs.
	for ( int i=0; i<glpr_numprograms; ++i )
	{
		glDeleteProgram( glpr_programs[i] );
		CHECK_OGL
		glpr_programs[i] = 0;
	}
	glpr_numprograms = 0;

	glpr_usedprogram = 0;
	glpr_last_compile_log = "";
	glpr_last_link_log = "";
}


void glpr_dump( void )
{
	unsigned int program = 0xffffffff;
	for ( int i=0; i<glpr_numu; ++i )
	{
		unsigned int pg = glpr_unif_prog[ i ];
		if ( pg != program )
		{
			LOGI( "PROGRAM %u:", pg );
			program = pg;
		}
		LOGI( "%d %s", glpr_unif[ i ], glpr_name[ i ] );
	}
}


void glpr_use( unsigned int program )
{
	ASSERTM( program > 0, "Invalid program specified in glpr_use( %u )", program );

	bool found=0;
	for ( int i=0; i<glpr_numprograms; ++i )
		if ( glpr_programs[i] == program )
			found=true;

	ASSERTM( found, "Program %u was not found amoung %d loaded programs.", program, glpr_numprograms );

	glUseProgram( program );
	CHECK_OGL
	for ( glpr_searchindex = 0; glpr_searchindex < glpr_numu; ++glpr_searchindex )
		if ( glpr_unif_prog[ glpr_searchindex ] == program )
			break;
	glpr_usedprogram = program;
}


int glpr_uniform( const char* nm )
{
	ASSERTM( glpr_usedprogram > 0, "Cannot get uniform %s if glpr_use was not called.", nm );
	int i;
	for ( i=glpr_searchindex; i<glpr_numu; ++i )
	{
		if ( !strcmp( nm, glpr_name[ i ] ) )
			return glpr_unif[ i ];
		if ( glpr_unif_prog[ i ] != glpr_usedprogram )
			break;
	}
	ASSERTM( 0, "uniform '%s' for program %u not found. Searched [%d,%d). Program Count: %d", nm, glpr_usedprogram, glpr_searchindex, i, glpr_numprograms );
	return -1;
}


static int glpr_add( const char* nm, unsigned int program )
{
	ASSERTM( glIsProgram( program ), "Program %u is not a GL program, so can't get uniform location for '%s'", program, nm );
	ASSERTM( glpr_numprograms > 0, "Cannot add uniform '%s' for program %d is we have 0 programs loaded.", nm, program );
	ASSERT( glpr_numu < MAXUNIFORMS );
	glpr_name[ glpr_numu ] = nm;
	glpr_unif_prog[ glpr_numu ] = program;
	glpr_unif[ glpr_numu ] = glGetUniformLocation( program, nm );
	CHECK_OGL_RELEASE
	if ( glpr_unif[ glpr_numu ] < 0 )
		LOGE( "Failed to get uniform location of '%s' for program %u", nm, program );
	return glpr_unif[ glpr_numu++ ];
}


static bool glpr_compile( GLuint* shader, GLenum type, const GLchar* source )
{
	ASSERT( source );
	*shader = glCreateShader(type);
	CHECK_OGL_RELEASE
	ASSERT( *shader > 0 );
	glShaderSource( *shader, 1, &source, NULL );
	CHECK_OGL_RELEASE
	glCompileShader( *shader );
	CHECK_OGL_RELEASE
	GLint logLength=0;
	glGetShaderiv( *shader, GL_INFO_LOG_LENGTH, &logLength );
	CHECK_OGL_RELEASE
	if ( logLength > 1 )
	{
		GLchar *log = (GLchar *)malloc( (unsigned int)logLength );
		glGetShaderInfoLog( *shader, logLength, &logLength, log );
		CHECK_OGL_RELEASE
		LOGI( "Shader compile log:\n%s", log );
		glpr_last_compile_log = log;
	}
	GLint status=0;
	glGetShaderiv( *shader, GL_COMPILE_STATUS, &status );
	CHECK_OGL_RELEASE

	if ( status == 0 )
	{
		// We failed compilation.
		return false;
	}
	return true;
}


static bool glpr_link( GLuint prog )
{
	glLinkProgram( prog );
	CHECK_OGL_RELEASE
	GLint logLength=0;
	glGetProgramiv( prog, GL_INFO_LOG_LENGTH, &logLength );
	CHECK_OGL_RELEASE
	if ( logLength > 1 )
	{
		GLchar *log = (GLchar*)malloc( (unsigned int)logLength );
		glGetProgramInfoLog( prog, logLength, &logLength, log );
		CHECK_OGL_RELEASE
		LOGE( "Program link log:\n%s", log );
		glpr_last_link_log = log;
	}
	GLint status=0;
	glGetProgramiv( prog, GL_LINK_STATUS, &status );
	CHECK_OGL_RELEASE
	if ( status == 0 )
		return false;
	return true;
}

bool glpr_validate( GLuint prog )
{
	ASSERTM( prog>0, "Program 0 is not valid, so cannot validate. Number of programs loaded: %d", glpr_numprograms );
	ASSERTM( glIsProgram( prog ), "Program %d is not a valid GL program, so cannot validate it. Number of programs loaded: %d", prog, glpr_numprograms );
	GLint logLength, status;
	glValidateProgram(prog);
	glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
	if ( logLength > 1 )
	{
		GLchar *log = (GLchar *)malloc( (unsigned int) logLength );
		glGetProgramInfoLog(prog, logLength, &logLength, log);
		if ( !strstr( log, "Validation successful." ) )	// This is what AMD GPUPRO reports. It is not interesting.
			LOGE( "Program(%d) validate log:\n%s", prog, log );
		free(log);
	}
	glGetProgramiv(prog, GL_VALIDATE_STATUS, &status);
	if ( status == 0 )
		return false;
	return true;
}


static int bind_attribute( unsigned int program, const char* attributes, const char* name, int loc )
{
	const char* s = strstr( attributes, name );
	if ( !s ) return 0;
	char c = s[ strlen( name ) ];
	if ( c > ',' )
		return 0;
	glBindAttribLocation( program, (GLuint) loc, name );
	CHECK_OGL_RELEASE
	return 1;
}


unsigned int glpr_load( const char* name, const char* src_vsh, const char* src_fsh, const char* attributes, const char* uniforms )
{
	GLuint vertShader=0, fragShader=0;
	GLuint program = 0;
#if !defined(DEBUG)
	glGetError();	// reset the error flag.
#endif
	bool vshOk = glpr_compile( &vertShader, GL_VERTEX_SHADER, src_vsh );
	ASSERTM( vshOk, "Failed to compile vertex shader for %s: %s", name, glpr_last_compile_log );
	if ( !vshOk )
	{
		LOGE( "Failed to compile vertex shader %s", name );
		return 0;
	}
	else
	{
		LOGI( "Compiled vertex shader %s", name );
	}

	bool fshOk = glpr_compile( &fragShader, GL_FRAGMENT_SHADER, src_fsh );
	ASSERTM( fshOk, "Failed to compile fragment shader for %s: %s", name, glpr_last_compile_log );
	if ( !fshOk )
	{
		LOGE( "Failed to compile fragment shader %s", name );
		return 0;
	}
	else
	{
		LOGI( "Compiled fragment shader %s", name );
	}

	program = glCreateProgram();
	CHECK_OGL_RELEASE
	ASSERT( program > 0 );

	ASSERTM( glIsProgram( program ), "Program %u for %s not a GL program", program, name );
	CHECK_OGL_RELEASE
	// Attach vertex shader to program.
	ASSERTM( glIsShader( vertShader ), "%s not a vert shader.", name );
	glAttachShader(program, vertShader);
	CHECK_OGL_RELEASE
	// Attach fragment shader to program.
	ASSERTM( glIsShader( fragShader ), "%s not a frag shader.", name );
	glAttachShader(program, fragShader);
	CHECK_OGL_RELEASE

	// Bind attribute locations.
	// This needs to be done prior to linking.
	int numbound=0;
	numbound += bind_attribute( program, attributes, "position", ATTRIB_VERTEX );
	numbound += bind_attribute( program, attributes, "surfacenormal", ATTRIB_NORMAL );
	numbound += bind_attribute( program, attributes, "tangent", ATTRIB_TANGENT );
	numbound += bind_attribute( program, attributes, "displacement", ATTRIB_DISPLACEMENT );
	numbound += bind_attribute( program, attributes, "rgb", ATTRIB_RGB );
	numbound += bind_attribute( program, attributes, "rgb0", ATTRIB_RGB0 );
	numbound += bind_attribute( program, attributes, "rgb1", ATTRIB_RGB1 );
	numbound += bind_attribute( program, attributes, "uv", ATTRIB_UV );
	numbound += bind_attribute( program, attributes, "trf",  ATTRIB_TRF );
	// specific to imhotep pyramid builder.
	numbound += bind_attribute( program, attributes, "limb", ATTRIB_LIMB );	// specific to crowdsim shader.
	// specific to the little plane that could.
	numbound += bind_attribute( program, attributes, "uvoff", ATTRIB_UVOFF );
	numbound += bind_attribute( program, attributes, "positiondrift", ATTRIB_PDRIFT );
	numbound += bind_attribute( program, attributes, "ssdelta", ATTRIB_SSDELT );
	numbound += bind_attribute( program, attributes, "timeoffset", ATTRIB_TMOFFS );
	// specific to the little bike that could.
	numbound += bind_attribute( program, attributes, "intensity", ATTRIB_INTENS );
	// specific to font rendering.
	numbound += bind_attribute( program, attributes, "opacity", ATTRIB_OPACIT );
	// specific to fragger
	numbound += bind_attribute( program, attributes, "hue", ATTRIB_HUE );
	// specific to FTT
	numbound += bind_attribute( program, attributes, "uvshift", ATTRIB_UVSHIFT );
	CHECK_OGL_RELEASE
	// specific to Ring Miner
	numbound += bind_attribute( program, attributes, "boardpos", ATTRIB_BOARDPOS );
	numbound += bind_attribute( program, attributes, "ore", ATTRIB_ORE );

	//LOGI( "bound %d attributes for shader %s", numbound, name );
	(void)numbound;

	// Link program.
	const bool linkOk = glpr_link( program );
	ASSERTM( linkOk, "Linking of %s failed: %s", name, glpr_last_link_log );
	if ( !linkOk )
	{
		LOGE( "Failed to link program %s", name );
		return 0;
	}

	ASSERT( glpr_numprograms < MAXGLPROGS );
	glpr_programs[ glpr_numprograms++ ] = program;
	glpr_searchindex = glpr_numu;
	const char* s = uniforms;
	while ( *s >= 32 )
	{
		const char* e = s;
		while ( *e != ',' && *e > 32 )
			++e;
		size_t sz = 1UL + (size_t) (e - s);
		char* unifname = (char*) malloc( sz );
		memcpy( unifname, s, sz-1 );
		unifname[ sz-1 ] = 0;
		int u = glpr_add( unifname, program );
		(void) u;
		s = e;
		if ( *s != 0 )
			s++;
	}

	// Release vertex and fragment shaders.
	if (vertShader) glDeleteShader(vertShader);
	if (fragShader) glDeleteShader(fragShader);
	CHECK_OGL_RELEASE

	LOGI( "Loaded program %s as handle %u in slot %d", name, program, glpr_numprograms-1 );

	return program;
}

