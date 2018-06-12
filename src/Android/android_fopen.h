#ifndef ANDROID_FOPEN_H
#define ANDROID_FOPEN_H

#include <stdio.h>			// for funopen()

#include <android/asset_manager.h>	// for AAssetManager

#ifdef __cplusplus
extern "C" {
#endif

// Java code needs to tell us about where to find the asset manager.
extern void android_fopen_set_asset_manager(AAssetManager* manager);

extern FILE* android_fopen(const char* fname, const char* mode);
extern fpos_t android_seek(void* cookie, fpos_t offset, int whence);
extern void android_list_assets( const char* dirName );

#ifdef __cplusplus
}
#endif

#endif
