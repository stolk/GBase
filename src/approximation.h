#include <math.h>

// low precision atan2f
static inline float atan2_approximation( float y, float x )
{
	//http://pubs.opengroup.org/onlinepubs/009695399/functions/atan2.html
	//Volkan SALMA

	const float ONEQTR_PI = M_PI / 4.0f;
	const float THRQTR_PI = 3.0f * M_PI / 4.0f;
	float r, angle;
	float abs_y = fabsf(y) + 1e-10f;      // kludge to prevent 0/0 condition
	if ( x < 0.0f )
	{
		r = (x + abs_y) / (abs_y - x);
		angle = THRQTR_PI;
	}
	else
	{
		r = (x - abs_y) / (x + abs_y);
		angle = ONEQTR_PI;
	}
	angle += (0.1963f * r * r - 0.9817f) * r;
	if ( y < 0.0f )
		return( -angle );     // negate if in quad III or IV
	else
		return( angle );
}

#ifdef __cplusplus
// low precision sine/cosine (currently for C++ only due to use of references.)
// can be used outside of -M_PI .. M_PI range.
static inline void sincos_approximation( float x, float& sine, float& cosine )
{
	const float fouroverpi = 4.0f / M_PI;
	const float fouroverpisquare = 4.0f / ( M_PI * M_PI );

	x = fmodf( x, M_PI * 2.0f );
	x = x > M_PI ? x - 2.0f * M_PI : x;	// map to -pi .. pi

	if ( x < 0.0f )
		sine = fouroverpi * x + fouroverpisquare * x * x;
	else
		sine = fouroverpi * x - fouroverpisquare * x * x;

	// do cosine now.
	x += ( 0.5f * M_PI );
	x = x > M_PI ? x - 2.0f * M_PI : x;	// map to -pi .. pi

	if ( x < 0.0f )
		cosine = fouroverpi * x + fouroverpisquare * x * x;
	else
		cosine = fouroverpi * x - fouroverpisquare * x * x;
}
#endif

static inline float sin_approximation(float x)
{
	const float fouroverpi = 4.0f / M_PI;
	const float fouroverpisquare = 4.0f / (M_PI * M_PI);

	x = fmodf(x, M_PI * 2.0f);
	x = x > M_PI ? x - 2.0f * M_PI : x; // map to -pi .. pi

	if (x < 0.0f)
		return fouroverpi * x + fouroverpisquare * x * x;
	else
		return fouroverpi * x - fouroverpisquare * x * x;
}

// natural log on [0x1.f7a5ecp-127, 0x1.fffffep127]. Maximum relative error 9.4529e-5
// From: https://stackoverflow.com/a/39822314/301166
static inline int __float_as_int( float in )
{
	union fi { int i; float f; } conv;
	conv.f = in;
	return conv.i;
}
static inline float __int_as_float( int in )
{
	union fi { int i; float f; } conv;
	conv.i = in;
	return conv.f;
}
float logf_approximation( float a )
{
	float m, r, s, t, i, f;
	int32_t e;

	e = (__float_as_int (a) - 0x3f2aaaab) & 0xff800000;
	m = __int_as_float (__float_as_int (a) - e);
	i = (float)e * 1.19209290e-7f; // 0x1.0p-23
	/* m in [2/3, 4/3] */
	f = m - 1.0f;
	s = f * f;
	/* Compute log1p(f) for f in [-1/3, 1/3] */
	r = fmaf (0.230836749f, f, -0.279208571f); // 0x1.d8c0f0p-3, -0x1.1de8dap-2
	t = fmaf (0.331826031f, f, -0.498910338f); // 0x1.53ca34p-2, -0x1.fee25ap-2
	r = fmaf (r, s, t);
	r = fmaf (r, s, f);
	r = fmaf (i, 0.693147182f, r); // 0x1.62e430p-1 // log(2)
	return r;
}

