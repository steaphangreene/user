#ifndef	INSOMNIA_USER_ENGINE_H
#define	INSOMNIA_USER_ENGINE_H

#include <stdio.h>

#include "config.h"

extern int USER_ARGC;
extern char **USER_ARGV;

void U2_Exit(int, const char *, ...)
	__attribute__ ((noreturn, format (printf, 2, 3)));
void U2_Exit(int) __attribute__ ((noreturn));

void U2_Init(int argc, char **argv);//  __attribute__ ((constructor));

FILE *U2_FOpenRead(const char *);
FILE *U2_FOpenSystemRead(const char *);
FILE *U2_FOpenLocalRead(const char *);
FILE *U2_FOpenHomeRead(const char *);
FILE *U2_FOpenWrite(const char *);
FILE *U2_FOpenSystemWrite(const char *);
FILE *U2_FOpenLocalWrite(const char *);
FILE *U2_FOpenHomeWrite(const char *);

#ifdef USER_DEBUG
  extern int debug_index;
  extern char *debug_position[16];
  #define Debug(x) { debug_position[debug_index++] = x; debug_index&=15; }
  #ifdef USER_ENGINE_DEBUG
    #define UserDebug(x) { debug_position[debug_index++] = x; debug_index&=15; }
  #else
    #define UserDebug(x) {}
  #endif
#else
  #define Debug(x) {}
  #define UserDebug(x) {}
#endif

#endif
