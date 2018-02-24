//  tty.cpp
//
//  (c)2012 Abraham Stolk


#include "tty.h"

#include "nfy.h"

#include "checkogl.h"
#include "vmath.h"
#include "rendercontext.h"
#include "logx.h"
#include "glpr.h"

#define NUMTTYLINES 4
#define TTYLINELEN 37
#define LASTTTYLINE (NUMTTYLINES-1)

static char lines[ NUMTTYLINES ][ TTYLINELEN ];
static float outputCompletion=0.0;
static bool autoHidden=false;
static float timeSinceWrite=0;

static float fntdata[] =
{
	1,2, 0,1,  0,1, 0,-1,  0,-1, 1,-2,	// ( (40)
	-1,2, 0,1,  0,1, 0,-1,  0,-1, -1,-2,	// ) (41)
	0,2, 0,-2,  2,0, -2,0,  -1,-1, 1,1,  -1,1, 1,-1,	// * (42)
	0,2, 0,-2,  2,0, -2,0,	// + (43)
	-1,-2, 0,-2,  0,-2, 1,-1,  1,-1, 0,-1,  0,-1, -1,-2,   // , (44)

	-2,0, 2,0,	// - (45)
	0,-2, 1,-2,  1,-2, 1,-1,  1,-1, 0,-1,  0,-1, 0,-2,   // . (46)
	-2,-2, 2,2,	// / (47)
	-1,2, 1,2,  1,2, 2,-2,  2,-2, -2,-2,  -2,-2, -1,2,   // 0 (48)
	0,2, 0,-2,   // 1 (49)
	-2,2, 2,0,  2,0, -2,0,  -2,0, -2,-2,  -2,-2, 2,-2,   // 2 (50)
	-2,2, 0,2,  0,2, 0,0,  0,0, 2,0,  2,0, 2,-2,  2,-2, -2,-2,   // 3 (51)
	-2,2, -2,0,  -2,0, 2,0,  2,0, 2,-2,   // 4 (52)
	2,2, -2,2,  -2,2, -2,0,  -2,0, 2,0,  2,0, -2,-2,  -2,-2, -2,-2,  -2,-2, -2,-2,   // 5 (53)
	0,2, -2,-2,  -2,-2, 2,-2,  2,-2, 1,0,  1,0, -1,0,   // 6 (54)
	-2,2, 2,2,  2,2, 0,-2,   // 7 (55)
	-2,0, -2,-2,  -2,-2, 2,-2,  2,-2, 2,0,  2,0, -1,0,  -1,0, -1,2,  -1,2, 1,2,  1,2, 1,0,   // 8 (56)
	0,-2, 2,2,  2,2, -2,2,  -2,2, -1,0,  -1,0, 1,0,   // 9 (57)
	0,-2, 1,-2,  1,-2, 1,-1,  1,-1, 0,-1,  0,-1, 0,-2,  0,1, 1,1,  1,1, 1,2,  1,2, 0,2,  0,2, 0,1,   // : (58)
	1,2, -1,0,  -1,0, 1,-2,   // < (60)
	-1,2, 1,0,  1,0, -1,-2,   // > (62)
	-2,1, -1,2,  -1,2, 1,2,  1,2, 2,1,  2,1, 0,-1,  0,-1, 0,-2,   // ? (63)
	-2,-2, 0,2,  0,2, 2,-2,  2,-2, 1,0,  1,0, -1,0,   // A (65)
	-2,0, 1,0,  1,0, 0,2,  0,2, -2,2,  -2,2, -2,-2,  -2,-2, 2,-2,  2,-2, 1,0,   // B (66)
	2,2, 0,2,  0,2, -2,1,  -2,1, -2,-1,  -2,-1, 0,-2,  0,-2, 2,-2,   // C (67)
	-2,2, -2,-2,  -2,-2, 1,-2,  1,-2, 2,-1,  2,-1, 2,1,  2,1, 1,2,  1,2, -2,2,   // D (68)
	1,2, -2,2,  -2,2, -2,0,  -2,0, 0,0,  0,0, -2,0,  -2,0, -2,-2,  -2,-2, 2,-2,   // E (69)
	-2,-2, -2,0,  -2,0, 0,0,  0,0, -2,0,  -2,0, -2,2,  -2,2, 2,2,   // F (70)
	2,2, -2,1,  -2,1, -2,-2,  -2,-2, 2,-2,  2,-2, 2,0,  2,0, 0,0,   // G (71)
	-2,2, -2,-2,  -2,-2, -2,0,  -2,0, 2,0,  2,0, 2,2,  2,2, 2,-2,   // H (72)
	-1,2, 1,2,  1,2, 0,2,  0,2, 0,-2,  0,-2, 1,-2,  1,-2, -1,-2,   // I (73)
	2,2, 2,-1,  2,-1, 1,-2,  1,-2, -1,-2,  -1,-2, -2,-1,   // J (74)
	-2,2, -2,-2,  -2,-2, -2,0,  -2,0, 0,2,  0,2, -2,0,  -2,0, 2,-2,   // K (75)
	-2,2, -2,-2,  -2,-2, 2,-2,   // L (76)
	-2,-2, -2,2,  -2,2, 0,0,  0,0, 2,2,  2,2, 2,-2,   // M (77)
	-2,-2, -2,2,  -2,2, 2,-2,  2,-2, 2,2,   // N (78)
	-1,2, 1,2,  1,2, 2,1,  2,1, 2,-1,  2,-1, 1,-2,  1,-2, -1,-2,  -1,-2, -2,-1,  -2,-1, -2,1,  -2,1, -1,2,   // O (79)
	-2,-2, -2,2,  -2,2, 2,2,  2,2, 2,1,  2,1, -2,0,   // P (80)
	1,-1, 2,-2,  2,-2, 2,2,  2,2, -2,2,  -2,2, -2,-2,  -2,-2, 2,-2,   // Q (81)
	-2,-2, -2,2,  -2,2, 2,0,  2,0, -2,0,  -2,0, 2,-2,   // R (82)
	2,2, -2,0,  -2,0, 2,0,  2,0, 2,-2,  2,-2, -2,-2,   // S (83)
	-2,2, 2,2,  2,2, 0,2,  0,2, 0,-2,   // T (84)
	-2,2, -2,-2,  -2,-2, 2,-2,  2,-2, 2,2,   // U (85)
	-2,2, 0,-2,  0,-2, 2,2,   // V (86)
	-2,2, -1,-2,  -1,-2, 0,0,  0,0, 1,-2,  1,-2, 2,2,   // W (87)
	-2,2, 2,-2,  2,-2, 0,0,  0,0, 2,2,  2,2, -2,-2,  -2,-2, -2,-2,  -2,-2, -2,-2,   // X (88)
	-2,2, 0,0,  0,0, 2,2,  2,2, -2,-2,   // Y (89)
	-2,2, 2,2,  2,2, -2,-2,  -2,-2, 2,-2,   // Z (90)
	1,2, -1,2,  -1,2, -1,-2,  -1,-2, 1,-2,   // [ (91)
	-2,2, 2,-2,   // \ (92)
	-1,2, 1,2,  1,2, 1,-2,  1,-2, -1,-2,   // ] (93)
	-1,1, 0,2,  0,2, 1,1,   // ^ (94)
	-2,-2, 2,-2,   // _ (95)
};

static int fntdelimeters[] =
{
	0, 6, 12, 20, 24, 

32, 34, 42, 44, 52, 54, 62, 72, 78, 90, 98, 102, 116, 124, 140, 140, 144, 144, 148, 158, 158, 166, 178, 188, 200, 212, 222, 232, 242, 252, 260, 270, 274, 282, 288, 304, 312, 322, 330, 338, 344, 350, 354, 362, 374, 380, 386, 392, 394, 400, 404, 406

//	0, 2, 10, 12, 20, 22, 30, 40, 46, 58, 66, 70, 84, 92, 108, 108, 112, 112, 116, 126, 126, 134, 146, 156, 168, 180, 190, 200, 210, 220, 228, 238, 242, 250, 256, 272, 280, 290, 298, 306, 312, 318, 322, 330, 342, 348, 354, 360, 362, 368, 372, 374
};


static void on_tty( const char* msg )
{
	//tty_write( text );
}


bool tty_init(void)
{
	tty_reset();
	const bool ok = nfy_obs_add( "TTY", on_tty );
	return ok;
}


static void tty_linefeed(void)
{
	for ( int i=0; i<LASTTTYLINE; ++i )
		memcpy( lines[ i ], lines[ i + 1 ], TTYLINELEN );
	memset( lines[ LASTTTYLINE ], ' ', TTYLINELEN );
	outputCompletion = 0.0;
}


void tty_update(float dt)
{
	outputCompletion += 0.5f * dt;
	timeSinceWrite += dt;
	if ( timeSinceWrite > 7.0 )
		autoHidden = true;
}


void tty_reset(void)
{
	tty_clear();
	timeSinceWrite = 1000.0f;
	autoHidden = true;
}


void tty_write( const char* s )
{
	tty_linefeed();
	int len = (int) strlen( s );
	len = len > TTYLINELEN ? TTYLINELEN : len;
	memcpy( lines[ LASTTTYLINE ], s, len );
	autoHidden = false;
	timeSinceWrite = 0;
}


void tty_clear( void )
{
	for ( int l=0; l<NUMTTYLINES; ++l )
		 memset( lines[ l ], ' ', TTYLINELEN );
}


void tty_draw_edge( const char* tag, const rendercontext_t& rc )
{
	static int mcvpUniform = glpr_uniform( "modelcamviewprojmat" );
	if ( autoHidden ) return;

	static float drawbuf[ 4 * 2048 ];
	float* writer = drawbuf;
	int numv=0;

	const float terminalWidth = TTYLINELEN;

	const float charw = 2.0f / terminalWidth;
	const float charh = 2.0f / NUMTTYLINES;

	const float marginx = -1.0f + 1.0f/terminalWidth;
	const float marginy = -1.0f + 1.0f/NUMTTYLINES;

	for ( int line=0; line<NUMTTYLINES; ++line )
		for ( int x=0; x<TTYLINELEN; ++x )
		{
			char c = lines[ line ][ x ];
#if defined( MSWIN )
			const bool showChar = line != LASTTTYLINE || x < (int) ( outputCompletion * TTYLINELEN +0.5f );
#else
			const bool showChar = line != LASTTTYLINE || x < (int) roundf( outputCompletion * TTYLINELEN );
#endif
			if ( c >= 40 && c < 96 && showChar )
			{
				c -= 40;
				int fr = fntdelimeters[ c+0 ];
				int to = fntdelimeters[ c+1 ];
				if ( fr < to )
				{
					const float px = marginx + x * charw;
					const float py = marginy + ( LASTTTYLINE - line ) * charh;
					while ( fr < to )
					{
						const float xx = fntdata[ fr*2 + 0 ];
						const float yy = fntdata[ fr*2 + 1 ];
						*writer++ = px + ( xx/6 ) * charw;
						*writer++ = py + ( yy/6 ) * charh;
						*writer++ = 0.0;
						*writer++ = 1.0;
						++numv;
						++fr;
					}
				}
			}
		}

	ASSERT( numv < 2048 );
	if ( !numv )
		return;

	GLuint vbo=0;
#ifdef USE_VAO
	GLuint vao=0;
	glGenVertexArrays( 1, &vao );
	glBindVertexArray( vao );
#endif
	glGenBuffers( 1, &vbo );
	CHECK_OGL
	glBindBuffer( GL_ARRAY_BUFFER, vbo );
	CHECK_OGL
	glEnableVertexAttribArray( ATTRIB_VERTEX );
	CHECK_OGL
	glBufferData( GL_ARRAY_BUFFER, numv*4*sizeof(float), (void*)drawbuf, GL_STREAM_DRAW );
	CHECK_OGL
	glVertexAttribPointer( ATTRIB_VERTEX, 4, GL_FLOAT, 0, 4 * sizeof(float), (void*) 0 );
	CHECK_OGL

	mat44_t trf;
	trf.identity();

	glUniformMatrix4fv( mcvpUniform, 1, false, trf.data);
	CHECK_OGL

	glLineWidth( 2.0f );
	glDrawArrays( GL_LINES, 0, numv );
	CHECK_OGL

#ifdef USE_VAO
	glBindVertexArray( 0 );
	glDeleteVertexArrays( 1, &vao );
	CHECK_OGL
#endif

	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glDeleteBuffers( 1, &vbo );
}


static int numlines( const char* str, size_t* maxlen )
{
	int retval=0;
	const char* s=str;
	const char* prv=s;
	if ( maxlen )
		*maxlen = 0;
	while ( s && *s )
	{
		retval++;
		const char* nxt = strchr( s, '\n' );
		size_t len = nxt ? ( nxt - prv ) : strlen( s );
		if ( maxlen )
			*maxlen = *maxlen < len ? len : *maxlen;
		s = nxt ? nxt+1 : 0;
		prv = s;
	}
	return retval;
}


void tty_draw_string( const char* str, const rendercontext_t& rc, vec3_t pos, vec3_t sz, const char* align, const mat44_t* texttrf )
{
	static int mcvpUniform = glpr_uniform( "modelcamviewprojmat" );
	size_t maxlinelen=0;
	const int linecount = numlines( str, &maxlinelen );
	const bool rightAlign = !strcmp( align, "right" );
	const bool centerAlign= !strcmp( align, "center" );
	const int l = (int) strlen( str );
	if ( rightAlign  ) pos.x -= maxlinelen * sz.x;
	if ( centerAlign ) pos.x -= 0.5f * maxlinelen * sz.x;
	static float drawbuf[ 8192 ];
	float* writer = drawbuf;
	int numv=0;
	const float charw=sz.x;
	const float charh=sz.y;
	const float linespacing = 1.2f * charh;
	pos.y += ( linespacing * 0.5f * ( linecount - 1.0f ) ); 
	int charx=0;
	for ( int x=0; x<l; ++x, ++charx )
	{
		int c = str[ x ];
		if ( c == '\n' )
		{
			pos.y -= linespacing;
			charx = -1;
		}
		if ( c < 40 || c >= 96 ) continue;
		c -= 40;
		int fr = fntdelimeters[ c+0 ];
		int to = fntdelimeters[ c+1 ];
		if ( fr < to )
		{
			const float px = pos.x + charx * charw + 0.5f * charw;
			const float py = pos.y; // + 0.5f * charh;
			while ( fr < to )
			{
				const float xx = fntdata[ fr*2 + 0 ];
				const float yy = fntdata[ fr*2 + 1 ];
				if ( writer < drawbuf + 8192 - 4 )
				{
					*writer++ = px + ( xx/6 ) * charw;
					*writer++ = py + ( yy/6 ) * charh;
					*writer++ = 0.0;
					*writer++ = 1.0;
				}
				++numv;
				++fr;
			}
		}
	}
	if ( !numv ) return;

	GLuint vbo=0;
#ifdef USE_VAO
	GLuint vao=0;
	glGenVertexArrays( 1, &vao );
	glBindVertexArray( vao );
#endif
	glGenBuffers( 1, &vbo );
	glBindBuffer( GL_ARRAY_BUFFER, vbo );
	glBufferData( GL_ARRAY_BUFFER, numv*4*sizeof(float), (void*)drawbuf, GL_STREAM_DRAW );
	CHECK_OGL
	glVertexAttribPointer( ATTRIB_VERTEX, 4, GL_FLOAT, 0, 4 * sizeof(float), (void*) 0 /* offset in vbo */ );
	CHECK_OGL
	glEnableVertexAttribArray( ATTRIB_VERTEX );
	CHECK_OGL

	mat44_t trf;
	if ( texttrf )
	{
		trf = rc.camproj * rc.camview * (*texttrf);
	}
	else
	{
		trf.identity();
	}
	glUniformMatrix4fv( mcvpUniform, 1, false, trf.data);

	glDrawArrays( GL_LINES, 0, numv );
	CHECK_OGL

#ifdef USE_VAO
	glBindVertexArray( 0 );
	glDeleteVertexArrays( 1, &vao );
	CHECK_OGL
#endif
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glDeleteBuffers( 1, &vbo );
	CHECK_OGL
}

