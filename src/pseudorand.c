#include <stdint.h>
#include <time.h>

static uint64_t xss[ 2 ] =
{
	0x230280,
	0x141010,
};


void pseudoseedrand( uint64_t seed )
{
	xss[ 0 ] = seed;
	xss[ 1 ] = 0x141010;
}


// xorshift128p
uint64_t pseudorand( void )
{
	uint64_t s1 = xss[ 0 ];
	const uint64_t s0 = xss[ 1 ];
	xss[ 0 ] = s0;
	s1 ^= s1 << 23; // a
	return ( xss[ 1 ] = ( s1 ^ s0 ^ ( s1 >> 17 ) ^ ( s0 >> 26 ) ) ) + s0; // b, c
}


float pseudorand_float( void )
{
	uint64_t r = pseudorand();
	return (float)r / (float)0xffffffffffffffff;
}


float pseudorand_range( float lo, float hi )
{
	const uint64_t r = pseudorand();
	return lo + (float)r / ( (float)0xffffffffffffffff / (hi-lo) );
}
