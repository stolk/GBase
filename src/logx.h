#ifndef LOGX_H
#define LOGX_H

#define STRINGIFY(X) #X
#define TOSTRING(X) STRINGIFY(X)

#if defined(__unix__)
#	define WRNFMT "\033[33mWRN\033[0m "
#	define ERRFMT "\033[31mERR\033[0m "
#else
#	define WRNFMT "WRN "
#	define ERRFMT "ERR "
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#include "baseconfig.h"
#include <stdio.h>

typedef void(*asserthook_t) (const char* condition, const char* file, int line);
extern asserthook_t asserthook;
extern FILE* logx_file;

/*
 * Define LOGI, LOGW, LOGE macros.
 */

#if defined(ANDROID)

#	include <android/log.h>
#	include <assert.h>

#	if !defined(LOGTAG)
#		define LOGTAG base
#	endif

#	define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO,  TOSTRING(LOGTAG), __VA_ARGS__))
#	define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN,  TOSTRING(LOGTAG), __VA_ARGS__))
#	define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, TOSTRING(LOGTAG), __VA_ARGS__))

#else

#	include <assert.h>

#	define LOGI(...) \
	{ \
		printf( __VA_ARGS__ ); \
		printf("\n"); \
		fflush( stdout ); \
		if ( logx_file ) { fprintf( logx_file, __VA_ARGS__ ); fprintf( logx_file, "\n" ); fflush(logx_file); } \
	}

#	define LOGW(...) \
	{ \
		printf( WRNFMT __VA_ARGS__ ); \
		printf("\n"); \
		fflush( stdout ); \
		if ( logx_file ) { fprintf( logx_file, "WRN " __VA_ARGS__ ); fprintf( logx_file, "\n" ); fflush(logx_file); } \
	}

#	define LOGE(...) \
	{ \
		printf( ERRFMT __VA_ARGS__ ); \
		printf("\n"); \
		fflush( stdout ); \
		if ( logx_file ) { fprintf( logx_file, "ERR " __VA_ARGS__ ); fprintf( logx_file, "\n" ); fflush(logx_file); } \
	}
#endif


/*
 * Define ASSERT macros.
 */


#define ASSERTM(C, FMT, ...) \
{ \
	if ( !(C) ) \
	{\
		char assertmessage[1024]; \
		snprintf( assertmessage, sizeof(assertmessage), "%s / " FMT, #C, __VA_ARGS__ ); \
		LOGE("ASSERT FAILED at %s(%d): %s", __BASE_FILE__, __LINE__, assertmessage); \
		if ( asserthook ) asserthook( assertmessage, __BASE_FILE__, __LINE__ ); \
	}\
	assert( C ); \
}


#define ASSERT(C) \
{ \
	if ( !(C) ) \
	{ \
		LOGE( "ASSERT FAILED at %s(%d):  %s", __BASE_FILE__, __LINE__, #C ); \
		if ( asserthook ) asserthook( #C, __BASE_FILE__, __LINE__ ); \
	} \
	assert( C ); \
}


#ifdef __cplusplus
}
#endif


#endif

