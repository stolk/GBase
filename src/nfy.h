// nfy.h
//
// A lightweight notification system.
// Observers can be called back with messages that match the message label.
//
// Messages have a label and any number of key=value fields.
// They are in the following form:
//   label foo=123 bar=3.14 baz=somestringwithoutwhitespace
//
// To extract values from the message, the callback functions can use:
// nfy_int, nfy_flt, nfy_str.
//
// (c)2012 by Abraham Stolk


#ifndef NFY_H
#define NFY_H

#include <float.h> // for FLT_MAX
#include <limits.h> // for INT_MAX

typedef void (*notificationcallback_t) ( const char* msg );


//! Add an observer for a message type. Returns false if no space.
extern bool nfy_obs_add( const char* lbl, notificationcallback_t cbk );

//! Remove an observer for a message type. Returns false if not found.
extern bool nfy_obs_rmv( const char* lbl, notificationcallback_t cbk );

//! Posts a notification message, returns the nr of observers notified.
extern int nfy_msg( const char* msg );

//! Queue a notification to be processed at a later time.
extern int nfy_queue_msg( const char* msg );

//! Process all queued messages.
extern int nfy_process_queue( void );

//! Extracts the integer value associated with the key from the message.
extern int nfy_int( const char* msg, const char* key );

//! Extracts a float value associated with the key from the message.
extern float nfy_flt( const char* msg, const char* key );

//! Extracts a string value associated with the key from the message.
extern void nfy_str( const char* msg, const char* key, char* dst, int dstlen );

//! Tests whether the label matches the message tag.
extern bool nfy_match( const char* msg, const char* lbl );


//! Unit test.
extern bool nfy_tst( void );

#endif
