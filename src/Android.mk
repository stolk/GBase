# Incoming from Application.mk is APP_CFLAGS
# The following defines are optionally taken from there:
#   -DDEBUG
#   -DUSEES2
#   -DUSEES3
#   -DUSECOREPROFILE
#   -DLOGTAG=sometag

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := base

LOCAL_SRC_FILES := \
logx.c \
kv.cpp \
elapsed.c \
pseudorand.c \
nfy.cpp \
pidc.cpp \
glpr.cpp \
dbd.cpp \
camera.cpp \
light.cpp \
geomdb.cpp \
meshdb.cpp \
offsc.cpp \
quad.cpp \
shdw.cpp \
tty.cpp \
worldobj.cpp \
staticworldobj.cpp \
dynamicworldobj.cpp \
odb.cpp \


LOCAL_C_INCLUDES := $(HOME)/src/opende/include


ifdef $(USE_ASSET_MANAGER)
  LOCAL_SRC_FILES += android_fopen.c txdb_stb.cpp wavdb.cpp	# Use asset manager to get assets from archive.
  LOCAL_C_INCLUDES += $(HOME)/src/stb/
else
  LOCAL_SRC_FILES += txdb_dlo.cpp wavdb_dlo.cpp			# Use dlopen() to get assets from .so file.
endif


LOCAL_ARM_NEON := true
LOCAL_ARM_MODE := arm

include $(BUILD_STATIC_LIBRARY)

