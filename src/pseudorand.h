#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

extern uint64_t pseudorand( void );

extern void pseudoseedrand( uint64_t seed );

extern float pseudorand_float( void );

extern float pseudorand_range( float lo, float hi );

#ifdef __cplusplus
}
#endif

