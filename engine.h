#ifndef	INSOMNIA_USER_ENGINE_H
#define	INSOMNIA_USER_ENGINE_H

#include "config.h"

extern int USER_ARGC;
extern char **USER_ARGV;

void Exit(int, const char *, ...)
	__attribute__ ((noreturn, format (printf, 2, 3)));
void Exit(int) __attribute__ ((noreturn));

void InitUserEngine(int argc, char **argv);//  __attribute__ ((constructor));

#ifdef USER_DEBUG
 extern int debug_index;
 extern char *debug_position[16];
 #define Debug(x) { debug_position[debug_index++] = x; debug_index&=15; }
#else
 #define Debug(x) {}
#endif

#endif
