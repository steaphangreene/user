#ifndef INSOMNIA_USER_CONFIG_H
#define INSOMNIA_USER_CONFIG_H

/********************
* Debugging Options *
********************/

#define USER_DEBUG

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
	#define ESD_SOUND
#endif

/*************************************
* Operating System Defines for Input *
*************************************/

#define KB_BUF_SIZE	128
#define KEY_MAX		65536

#endif
