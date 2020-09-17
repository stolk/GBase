// categories.h
// Different physics object categories.

#ifndef CATEGORIES_H
#define CATEGORIES_H

#define CATEGORY_TERR	(1<<0)	// terrain
#define CATEGORY_PROP	(1<<1)	// props
#define CATEGORY_PRLO	(1<<2)	// props low res
#define CATEGORY_PRHI	(1<<3)	// props high res
#define CATEGORY_TRUK	(1<<4)	// truck
#define CATEGORY_CRAN	(1<<5)	// crane
#define CATEGORY_GRAP	(1<<6)	// grapple
#define CATEGORY_CVYR	(1<<7)	// conveyerbelt
#define CATEGORY_SKID   (1<<8)  // tyres of skid loader
#define CATEGORY_TRAC   (1<<9)  // track links
#define CATEGORY_TYRE   (1<<10) // tyres
#define CATEGORY_HOOK   (1<<11) // helicopter hook
#define CATEGORY_BLAD   (1<<12) // helicopter blades
#define CATEGORY_ICON   (1<<13) // icons and powerups
#define CATEGORY_PROX   (1<<14) // proximity sensors
#define CATEGORY_DOOR   (1<<15) // door panels
#define CATEGORY_WATR   (1<<16) // water
#define CATEGORY_FLOT   (1<<17) // floatation device
#define CATEGORY_PERS   (1<<18) // person
#define CATEGORY_HORS   (1<<19) // horse feet
#define CATEGORY_RINK   (1<<20) // horse rink
#define CATEGORY_SNOW   (1<<21) // ice and snow
#define CATEGORY_SNSR	(1<<22) // sensor
#define CATEGORY_DIGR	(1<<23) // digger
#define CATEGORY_DIRT	(1<<24) // dirt
#define CATEGORY_SPKT	(1<<25) // sprocket of crawler
#define CATEGORY_AERO	(1<<26) // aeroplane
#define CATEGORY_AMMO	(1<<27)	// ammunition
#define CATEGORY_BOAT	(1<<28) // boat
#define CATEGORY_DUST	(1<<29) // dust
#define CATEGORY_OTHR	(1<<30)	// other

#define CATEGORIES_DYNAMIC \
  ( CATEGORY_PROP | CATEGORY_TRUK | CATEGORY_HOOK | CATEGORY_BLAD | CATEGORY_SKID | CATEGORY_TRAC | CATEGORY_TYRE | CATEGORY_CRAN | CATEGORY_GRAP | CATEGORY_DIGR | CATEGORY_DIRT | CATEGORY_PERS )

#define CATEGORIES_DYNAMIC_LO_RES \
  ( CATEGORY_TERR | CATEGORY_TRUK | CATEGORY_HOOK | CATEGORY_BLAD | CATEGORY_SKID | CATEGORY_TRAC | CATEGORY_TYRE | CATEGORY_CRAN | CATEGORY_PROP | CATEGORY_PERS | CATEGORY_DIRT | CATEGORY_PRLO )

#endif

