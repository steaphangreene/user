#ifndef INSOMNIA_USER_CONFIG_H
#define INSOMNIA_USER_CONFIG_H


/*****************
* Config Options *
*****************/

#define	CONFIG_FILE		"/etc/u2.conf"

/*****************
* FileIO Options *
*****************/

#define USE_ZLIB

/*****************
* Sprite Options *
*****************/

#define MAX_SPRITES             16384
#define REDRAW_RECTS            8
#define BIN_SIZE                32  // Must Agree
#define BIN_FACTOR              5   // (eg: 2^BIN_FACTOR = BIN_SIZE)
#define LARGE_BIN_SIZE          128  // Must Agree
#define LARGE_BIN_FACTOR        7   // (eg: 2^LARGE_BIN_FACTOR = LARGE_BIN_SIZE)

/***********************
* Memory Chunk Options *
***********************/

#define CHUNK_MAX		256

/********************
* Debugging Options *
********************/

#define USER_DEBUG
#define USER_ENGINE_DEBUG

/****************************************
* Operating System Defines for Graphics *
****************************************/

#define MAX_PANELS      64

#ifdef __DJGPP__
	#define DOS
#else
	#define X_WINDOWS
	#define XF86_DGA // Must also define X_WINDOWS (above)

	//#define SVGALIB // Not yet implemented
#endif

/*************************************
* Operating System Defines for Sound *
*************************************/

#ifdef __DJGPP__
	#define DOS_SOUND
#else
	#define OSS_SOUND
	#define OSS_MMAP_SOUND
//	#define ESD_SOUND
#endif

/*************************************
* Operating System Defines for Input *
*************************************/

#define KB_BUF_SIZE	128
#define U2_KEY_MAX	65536

#endif
