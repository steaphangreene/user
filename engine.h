#ifndef	INSOMNIA_ENGINE_H
#define	INSOMNIA_ENGINE_H

#define main Main

void Exit(int, const char *, ...)
	__attribute__ ((noreturn, format (printf, 2, 3)));
void Exit(int) __attribute__ ((noreturn));

extern int debug_position;

#endif
