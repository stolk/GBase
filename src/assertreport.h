// assertreport.h

#ifdef __cplusplus
extern "C"
{
#endif

extern int assertreport_send( const char* msg, int len );

extern int assertreport_init( const char* host, int portnr );

extern int assertreport_exit( void );

#ifdef __cplusplus
}
#endif

