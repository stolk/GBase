
extern void sticksignal_update( float dt, int steps );

//! Gets an extrapolated stick value.
extern void sticksignal_sample( int nr, float* stickx, float* sticky );

//! Gets the current stick value.
extern void sticksignal_latest( int nr, float* stickx, float* sticky );

extern void sticksignal_init( void );

