// rendercontex.h
//
// Contains the current camera transform, camera view transform, camera projection transform, and two light transforms.
//
// (c) Bram Stolk


#ifndef RENDERCONTEXT_H
#define RENDERCONTEXT_H


typedef struct
{
	// Camera
	mat44_t camtrf;
	mat44_t	camview; 
	mat44_t camproj;

	// First light
	mat44_t lightview;
	mat44_t lightproj;

	// Second light
	mat44_t auxil0view;
	mat44_t auxil0proj;

	// Third light
	mat44_t auxil1view;
	mat44_t auxil1proj;
} rendercontext_t;


// Values used to identify vertex attributes.
// Kinda game-specific, probably shouldn't be defined here.

const int ATTRIB_VERTEX = 0;
const int ATTRIB_NORMAL = 1;
const int ATTRIB_RGB    = 2;
const int ATTRIB_TRF    = 3;
const int ATTRIB_TRFX   = 3;
const int ATTRIB_TRFY   = 4;
const int ATTRIB_TRFZ   = 5;
const int ATTRIB_TRFP   = 6;
const int ATTRIB_UVOFF  = 7;

const int ATTRIB_UV     = 2;

// For profile shader
const int ATTRIB_RGB0	= 3;
const int ATTRIB_RGB1	= 4;

// For cloud shader
const int ATTRIB_TMOFFS = 3;
const int ATTRIB_SSDELT = 4;
const int ATTRIB_PDRIFT = 5;

// For flm shader
const int ATTRIB_AGE	= 1;
const int ATTRIB_BNESS  = 2;

// For flsh shader, line shader
const int ATTRIB_CLR	= 1;

// For crowd shader
const int ATTRIB_LIMB	= 7;

// For tracks shader
const int ATTRIB_INTENS	= 1;

// For font shader
const int ATTRIB_OPACIT = 1;

// For prts shader
const int ATTRIB_DISPLACEMENT = 2;
const int ATTRIB_TANGENT      = 3;

// For wall shader
const int ATTRIB_HUE	= 3;

// For clouds shader (Armor)
const int ATTRIB_UVSHIFT = 4;

#endif
