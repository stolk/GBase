#ifdef __cplusplus
extern "C" {
#endif

extern double elapsed_ms_since_last_call( void );

extern double elapsed_ms_since_start( void );		//!< Warning - not very exact on iOS and OSX.

#ifdef __cplusplus
}
#endif
