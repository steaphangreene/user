#include "config.h"

#include <stdio.h>
#include <stdarg.h>
#include <signal.h>
#include <string.h>

#include "engine.h"
#include "screen.h"
#include "speaker.h"
#include "mouse.h"
#include "keyboard.h"
#include "input.h"

#ifdef DOS
#include <time.h>
#endif

Screen *__Da_Screen = NULL;
Speaker *__Da_Speaker = NULL;
Mouse *__Da_Mouse = NULL;
Keyboard *__Da_Keyboard = NULL;
InputQueue *__Da_InputQueue = NULL;

int USER_ARGC;
char **USER_ARGV;

void SigHand(int);

#ifdef USER_DEBUG
int debug_index=0;
char *debug_position[16]
	= {"","","","","","","","","","","","","","","",":USER DEBUG BEGIN:"};
#endif

void StopUserEngine()  __attribute__ ((destructor));

void StopUserEngine()  {
  if(__Da_Keyboard != NULL) delete __Da_Keyboard;
  if(__Da_Speaker != NULL)  delete __Da_Speaker;
  if(__Da_Screen != NULL)  delete __Da_Screen;
  __Da_Keyboard = NULL;
  __Da_Speaker = NULL;
  __Da_Screen = NULL;
  fprintf(stderr, "User 2.0: Engine Shut Down.\n");
  }

void InitUserEngine(int argc, char **argv)  {
  printf("User 2.0: Engine Initializing....\n\r");
  signal(SIGKILL, SigHand);
  signal(SIGABRT, SigHand);
  signal(SIGINT, SigHand);
  signal(SIGILL, SigHand);
  signal(SIGQUIT, SigHand);
  signal(SIGSEGV, SigHand);
  signal(SIGHUP, SigHand);
  signal(SIGPIPE, SIG_IGN);

  USER_ARGC = argc;
  USER_ARGV = argv;

#ifdef DOS
  uclock();
#endif
  }

#ifdef X_WINDOWS
#define Screen X_Screen
#define Window X_Window

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#undef Screen
#undef Window

extern Display *___mydisplay;
#endif

void SigHand(int sn)  {
#ifdef USER_DEBUG
  StopUserEngine();
#ifdef DOS
  fprintf(stderr, "\nReceived signal #%d, debug traceback follows:\n", sn);
#else
  fprintf(stderr, "\nReceived signal \"%s\" (#%d), debug traceback follows:\n",
	(char *)strsignal(sn), sn);
#endif
  int ctr, ctr2;
  for(ctr=0; ctr<16; ctr++)  {
    for(ctr2=-1; ctr2<ctr; ctr2++) fprintf(stderr, "-");
    fprintf(stderr, "> ");
    fprintf(stderr, "%s\n", debug_position[(debug_index+(15-ctr))&15]);
    }
  _exit(1);
#else
#ifdef DOS
  Exit(1, "\nReceived signal #%d.\n", sn);
#else
  Exit(1, "Received signal \"%s\" (#%d)\n", (char *)strsignal(sn), sn);
#endif
#endif
  }

void Exit(int code)  {
  Exit(code, "");
  }

void Exit(int code, const char *out, ...)  {
  Debug("In Exit Function!");
  StopUserEngine();

  va_list stuff;  
  va_start(stuff, out);
  vprintf(out, stuff);
  va_end(stuff);
  _exit(code);
  }
