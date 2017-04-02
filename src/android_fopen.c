// android fopen() trick by netguy204
// http://www.50ply.com/blog/2013/01/19/loading-compressed-android-assets-with-file-pointer/

#include <stdio.h>

#include "android_fopen.h"
#include <errno.h>
#include <android/asset_manager.h>

#include "logx.h"

static int android_read(void* cookie, char* buf, int size)
{
	return AAsset_read((AAsset*)cookie, buf, size);
}

static int android_write(void* cookie, const char* buf, int size)
{
	return EACCES; // can't provide write access to the apk
}


fpos_t android_seek(void* cookie, fpos_t offset, int whence)
{
	return AAsset_seek((AAsset*)cookie, offset, whence);
}


static int android_close(void* cookie)
{
	AAsset_close((AAsset*)cookie);
	return 0;
}


// must be established by someone else...
static AAssetManager* android_asset_manager;
void android_fopen_set_asset_manager(AAssetManager* manager) 
{
	android_asset_manager = manager;
	LOGI("Asset Manager has been set to %p.", manager);
}


FILE* android_fopen(const char* fname, const char* mode) 
{
	if(mode[0] == 'w')
	{
		LOGE("Cannot write to Android assets %s", fname);
		return NULL;
	}
	if(!android_asset_manager)
	{
		LOGE("Manager has not been set. Was android_fopen_set_asset_manager() called?");
		return NULL;
	}
	AAsset* asset = AAssetManager_open(android_asset_manager, fname, 0);
	if(!asset)
	{
		LOGE("Asset '%s' not found by manager.", fname);
		return NULL;
	}
	return funopen(asset, android_read, android_write, android_seek, android_close);
}

