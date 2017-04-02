// txdb.h
//
// Texture DataBase.
// For loading images, and creating GL textures.
// (c)2017 Abraham Stolk.

#ifndef TXDB_H
#define TXDB_H

//! Once per process lifetime.
extern void txdb_init(void);

//! Delete all entries in txdb, and destroy the textures.
extern void txdb_clear(void);

//! Load all named images.
extern int  txdb_load( const char* pkgname, const char* lname, const char** names, unsigned int* values, int count );

//! Create texture from image with data in memory.
extern unsigned int txdb_load_from_memory( const char* name, const unsigned int* raw, int szw, int szh, bool compressed=false );

//! Load compressed images.
extern int txdb_load_compressed( const char** names, unsigned int* values, int count );

//! Use named texture.
extern void txdb_use( const char* name );

//! Print the contents of the txdb.
extern void txdb_prt( void );

//! Directory where the images reside.
extern const char* txdb_path;

#endif
