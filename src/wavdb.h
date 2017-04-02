#ifndef WAVDB_H
#define WAVDB_H

extern void wavdb_init(void);

extern int  wavdb_load( const char* pkgname, const char* lname, const char** names, int* lengths, int count );

extern void wavdb_lookup( const char* name, int* length, short** data );

extern void wavdb_prt( void );

extern const char* wavdb_path;

#endif
