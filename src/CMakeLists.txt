
cmake_minimum_required(VERSION 3.4.1)

# build libgbase as a static lib
add_library(gbase STATIC
logx.c
assertreport.c
kv.cpp
elapsed.c
pseudorand.c
nfy.cpp
pidc.cpp
glpr.cpp
dbd.cpp
camera.cpp
light.cpp
geomdb.cpp
meshdb.cpp
#offsc.cpp
quad.cpp
shdw.cpp
tty.cpp
txdb_stb.cpp
wavdb.cpp

Android/android_native_app_glue.c
Android/androidsupport.cpp
Android/android_fopen.c 
)

target_include_directories(gbase PRIVATE
	./
	$ENV{HOME}/src/opende/include/
	$ENV{HOME}/src/stb/
)

