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

typedef void *U2_File;

U2_File U2_FOpenRead(const char *);
U2_File U2_FOpenSystemRead(const char *);
U2_File U2_FOpenLocalRead(const char *);
U2_File U2_FOpenHomeRead(const char *);
U2_File U2_FOpenWrite(const char *);
U2_File U2_FOpenSystemWrite(const char *);
U2_File U2_FOpenLocalWrite(const char *);
U2_File U2_FOpenHomeWrite(const char *);
int U2_FGetC(U2_File fl);
size_t U2_FRead(void *buf, int size, int num, U2_File fl);
int U2_FPutC(int c, U2_File fl);
size_t U2_FWrite(const void *buf, int size, int num, U2_File fl);
int U2_FFlush(U2_File fl);
int U2_FClose(U2_File fl);

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
