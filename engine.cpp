#include <stdio.h>
#include <stdarg.h>
#include "os_defs.h"
#include <user/engine.h>
#include <user/screen.h>
#include <user/net.h>
#include <signal.h>

SoundCard *__Da_SoundCard = NULL;
Screen *__Da_Screen = NULL;
User *__Da_User = NULL;
Network *__Da_Network = NULL;

char * ARGV_0;

int ___DID_WE_EXIT = 0;

void InitUserEngine()  __attribute__ ((constructor));
void StopUserEngine()  __attribute__ ((destructor));
void SigHand(int);

int debug_position=0;

#undef main
int Main(int argc, char **argv);

int main(int argc, char **argv)  {
  ARGV_0 = argv[0];
  Main(argc, argv);
  }
#define main Main

void InitUserEngine()  {
  printf("User Engine Initializing....\n\r");
  signal(SIGKILL, SigHand);
  signal(SIGABRT, SigHand);
  signal(SIGINT, SigHand);
  signal(SIGILL, SigHand);
  signal(SIGQUIT, SigHand);
  signal(SIGSEGV, SigHand);
  signal(SIGHUP, SigHand);
  signal(SIGPIPE, SIG_IGN);

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

void StopUserEngine()  {
  if(___DID_WE_EXIT != 1)  Exit(0, "User Engine Shutting Down....\n\r");
  }


void SigHand(int sn)  {
  Exit(1, "Exiting due to receipt of signal #%d at debug_position %d.\n", sn,
	debug_position);
  }

void Exit(int code)  {
  Exit(code, "");
  }

void Exit(int code, const char *out, ...)  {
  ___DID_WE_EXIT = 1;
  if(__Da_Screen != NULL)  delete __Da_Screen;
  if(__Da_User != NULL)  delete __Da_User;
#ifdef DOS
  if(__Da_SoundCard != NULL)  delete __Da_SoundCard;
  if(__Da_Network != NULL)  delete __Da_Network;
#endif

  va_list stuff;  
  va_start(stuff, out);
  vprintf(out, stuff);
  va_end(stuff);
  _exit(code);
  }
