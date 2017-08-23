#include "glpr.h"

#include <stdlib.h>
#include <stdio.h>

#include "checkogl.h"
#include "vmath.h"
#include "rendercontext.h"
#include "logx.h"


#define MAXUNIFORMS	512
static int		glpr_unif[ MAXUNIFORMS ];	// uniform values.
static const char*	glpr_name[ MAXUNIFORMS ];	// uniform names.
static unsigned int	glpr_prog[ MAXUNIFORMS ];	// programs associated with uniforms.
static int		glpr_numu;			// nr of uniforms.

// To avoid name-clashes between different programs, we use a start index where we search for a uniform name.
static int		glpr_searchindex;
static int		glpr_usedprogram;


void glpr_init( void )
{
	glpr_numu = 0;
	glpr_usedprogram = -1;
	for ( int i=0; i<MAXUNIFORMS; ++i )
	{
		glpr_unif[ i ] = -1;
		if ( glpr_name[ i ] )
			free( (void*)glpr_name[ i ] );
		glpr_name[ i ] = 0;
		glpr_prog[ i ] = 0;
	}
}


void glpr_dump( void )
{
	unsigned int program = 0xffffffff;
	for ( int i=0; i<glpr_numu; ++i )
	{
		unsigned int pg = glpr_prog[ i ];
		if ( pg != program )
		{
			LOGI( "PROGRAM %d:", pg );
			program = pg;
		}
		LOGI( "%d %s", glpr_unif[ i ], glpr_name[ i ] );
	}
}


void glpr_use( unsigned int program )
{
	ASSERTM( program >= 0, "Invalid program specified in glpr_use( %d )", program );
	glUseProgram( program );
	CHECK_OGL
	for ( glpr_searchindex = 0; glpr_searchindex < glpr_numu; ++glpr_searchindex )
		if ( glpr_prog[ glpr_searchindex ] == program )
			break;
	glpr_usedprogram = program;
}


int glpr_uniform( const char* nm )
{
	ASSERTM( glpr_usedprogram >= 0, "Cannot get uniform %s if glpr_use was not called.", nm );
	for ( int i=glpr_searchindex; i<glpr_numu; ++i )
	{
		if ( !strcmp( nm, glpr_name[ i ] ) )
			return glpr_unif[ i ];
	}
	ASSERTM( 0, "uniform '%s' for program %d not found. Searched [%d,%d).", nm, glpr_usedprogram, glpr_searchindex, glpr_numu );
	return -1;
}


int glpr_add( const char* nm, unsigned int program )
{
	ASSERT( glpr_numu < MAXUNIFORMS );
	glpr_name[ glpr_numu ] = nm;
	glpr_prog[ glpr_numu ] = program;
	glpr_unif[ glpr_numu ] = glGetUniformLocation( program, nm );
	if ( glpr_unif[ glpr_numu ] < 0 )
		LOGE( "Failed to get uniform location of '%s' for program nr %d", nm, program );
	return glpr_unif[ glpr_numu++ ];
}


static bool glpr_compile( GLuint* shader, GLenum type, const GLchar* source )
{
	if ( !source )
	{
		LOGE( "Failed to load vertex shader" );
		return false;
	}
	*shader = glCreateShader(type);
	glShaderSource( *shader, 1, &source, NULL );
	glCompileShader( *shader );
	GLint logLength;
	glGetShaderiv( *shader, GL_INFO_LOG_LENGTH, &logLength );
	if ( logLength > 1 )
	{
		GLchar *log = (GLchar *)malloc( logLength );
		glGetShaderInfoLog( *shader, logLength, &logLength, log );
		LOGI( "Shader compile log:\n%s", log );
		free( log );
	}
	GLint status=0;
	glGetShaderiv( *shader, GL_COMPILE_STATUS, &status );
	if ( status == 0 )
	{
		glDeleteShader( *shader );
		return false;
	}
	CHECK_OGL
	return true;
}


static bool glpr_link( GLuint prog )
{
	glLinkProgram( prog );
	CHECK_OGL
	GLint logLength=0;
	glGetProgramiv( prog, GL_INFO_LOG_LENGTH, &logLength );
	CHECK_OGL
	if ( logLength > 1 )
	{
		LOGI( "GL_INFO_LOG_LENGTH = %d", logLength );
		GLchar log[ 2048 ];
		glGetProgramInfoLog( prog, sizeof( log ), &logLength, log );
		CHECK_OGL
		LOGE( "Program link log(sz=%d):\n%s", logLength, log );
	}
	GLint status;
	glGetProgramiv( prog, GL_LINK_STATUS, &status );
	CHECK_OGL
	if ( status == 0 )
		return false;
	CHECK_OGL
	return true;
}


bool glpr_validate( GLuint prog )
{
	GLint logLength, status;
	glValidateProgram(prog);
	glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
	if ( logLength > 1 )
	{
		GLchar *log = (GLchar *)malloc(logLength);
		glGetProgramInfoLog(prog, logLength, &logLength, log);
		LOGI( "Program validate log:\n%s", log );
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
	glBindAttribLocation( program, loc, name );
	return 1;
}


bool glpr_load( const char* name, GLuint& program, const char* src_vsh, const char* src_fsh, const char* attributes, const char* uniforms )
{
	GLuint vertShader, fragShader;

	program = glCreateProgram();
	CHECK_OGL

	bool vshOk = glpr_compile( &vertShader, GL_VERTEX_SHADER, src_vsh );
	if ( !vshOk )
	{
		LOGE( "Failed to compile vertex shader %s", name );
		return false;
	}
	else
	{
		LOGI( "Compiled vertex shader %s", name );
	}

	bool fshOk = glpr_compile( &fragShader, GL_FRAGMENT_SHADER, src_fsh );
	if ( !fshOk )
	{
		LOGE( "Failed to compile fragment shader %s", name );
		return false;
	}
	else
	{
		LOGI( "Compiled fragment shader %s", name );
	}

	// Attach vertex shader to program.
	glAttachShader(program, vertShader);

	// Attach fragment shader to program.
	glAttachShader(program, fragShader);

	// Bind attribute locations.
	// This needs to be done prior to linking.

	int numbound=0;
	numbound += bind_attribute( program, attributes, "position", ATTRIB_VERTEX );
	numbound += bind_attribute( program, attributes, "surfacenormal", ATTRIB_NORMAL );
	numbound += bind_attribute( program, attributes, "rgb", ATTRIB_RGB );
	numbound += bind_attribute( program, attributes, "uv", ATTRIB_UV );
	numbound += bind_attribute( program, attributes, "trf", ATTRIB_TRF );
	// specific to imhotep pyramid builder.
	numbound += bind_attribute( program, attributes, "limb", ATTRIB_LIMB );	// specific to crowdsim shader.
	// specific to the little plane that could.
	numbound += bind_attribute( program, attributes, "uvoff", ATTRIB_UVOFF );
	numbound += bind_attribute( program, attributes, "positiondrift", ATTRIB_PDRIFT );
	numbound += bind_attribute( program, attributes, "ssdelta", ATTRIB_SSDELT );
	numbound += bind_attribute( program, attributes, "timeoffset", ATTRIB_TMOFFS );
	// specific to the little bike that could.
	numbound += bind_attribute( program, attributes, "intensity", ATTRIB_INTENS );

	//LOGI( "bound %d attributes for shader %s", numbound, name );

	// Link program.
	if ( !glpr_link( program ) )
	{
		LOGI ( "Failed to link program %s", name );
		if (vertShader)
		{
			glDeleteShader(vertShader);
			vertShader = 0;
		}
		if (fragShader)
		{
			glDeleteShader(fragShader);
			fragShader = 0;
		}
		if (program)
		{
			glDeleteProgram(program);
			program = 0;
		}
		return false;
	}

	glpr_searchindex = glpr_numu;
	const char* s = uniforms;
	while ( *s >= 32 )
	{
		const char* e = s;
		while ( *e != ',' && *e > 32 )
			++e;
		size_t sz = 1 + e - s;
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
	CHECK_OGL

	LOGI( "Loaded program %s as handle %d", name, program );

	return true;
}

