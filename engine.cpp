#include "config.h"

#include <stdio.h>
#include <stdarg.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "engine.h"
#include "screen.h"
#include "speaker.h"
#include "mouse.h"
#include "keyboard.h"
#include "joystick.h"
#include "input.h"
#include "net.h"
#include "chunk.h"

#ifdef DOS
#include <time.h>
#endif

Screen *__Da_Screen = NULL;
Speaker *__Da_Speaker = NULL;
Mouse *__Da_Mouse = NULL;
Keyboard *__Da_Keyboard = NULL;
Joystick *__Da_Joystick[16] = { NULL, NULL, NULL, NULL,
				NULL, NULL, NULL, NULL,
				NULL, NULL, NULL, NULL,
				NULL, NULL, NULL, NULL };
InputQueue *__Da_InputQueue = NULL;
Network *__Da_Network = NULL;

int USER_ARGC = 0;
char **USER_ARGV = NULL;

// Configuration Options
char *U2_JoyDev[16] = { "/dev/js0", "/dev/js1", "/dev/js2", "/dev/js3",
			"/dev/js4", "/dev/js5", "/dev/js6", "/dev/js7",
			"/dev/js8", "/dev/js9", "/dev/js10","/dev/js11",
			"/dev/js12","/dev/js13","/dev/js14","/dev/js15" };

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
  Chunk *tmp;
  tmp->DeleteAll();
  fprintf(stderr, "User 2.0: Engine Shut Down.\n");
  }

int CheckArgFlag(const char *flag, char **target, int c) {
  if(c < USER_ARGC-1 && (!strcmp(flag, USER_ARGV[c]))) {
    (*target) = USER_ARGV[c+1];
    return 2;
   }
  else if((!strncmp(flag, USER_ARGV[c], strlen(flag)))
	&& (USER_ARGV[c][strlen(flag)] == '=')) {
    (*target) = USER_ARGV[c] + strlen(flag) + 1;
    return 1;
    }
  return 0;
  }

void U2_Init(int argc, char **argv)  {
  fprintf(stderr, "User 2.0: Engine Initializing....\n\r");
  signal(SIGABRT, SIG_DFL);  // Make sure abort can still generate core dumps
  signal(SIGKILL, SigHand);
  signal(SIGINT, SigHand);
  signal(SIGILL, SigHand);
  signal(SIGQUIT, SigHand);
  signal(SIGSEGV, SigHand);
  signal(SIGHUP, SigHand);
  signal(SIGPIPE, SIG_IGN);

  USER_ARGC = argc;
  USER_ARGV = argv;

  if(USER_ARGC > 1) {
    int arg = 1, num = 1;
    while(arg < USER_ARGC && num > 0) {
     num = 0;
     num += CheckArgFlag("--joy1dev", &U2_JoyDev[0], arg);
     num += CheckArgFlag("--joy2dev", &U2_JoyDev[1], arg);
     num += CheckArgFlag("--joy3dev", &U2_JoyDev[2], arg);
     num += CheckArgFlag("--joy4dev", &U2_JoyDev[3], arg);
     num += CheckArgFlag("--joy5dev", &U2_JoyDev[4], arg);
     num += CheckArgFlag("--joy6dev", &U2_JoyDev[5], arg);
     num += CheckArgFlag("--joy7dev", &U2_JoyDev[6], arg);
     num += CheckArgFlag("--joy8dev", &U2_JoyDev[7], arg);
     num += CheckArgFlag("--joy9dev", &U2_JoyDev[8], arg);
     num += CheckArgFlag("--joy10dev", &U2_JoyDev[9], arg);
     num += CheckArgFlag("--joy11dev", &U2_JoyDev[10], arg);
     num += CheckArgFlag("--joy12dev", &U2_JoyDev[11], arg);
     num += CheckArgFlag("--joy13dev", &U2_JoyDev[12], arg);
     num += CheckArgFlag("--joy14dev", &U2_JoyDev[13], arg);
     num += CheckArgFlag("--joy15dev", &U2_JoyDev[14], arg);
     num += CheckArgFlag("--joy16dev", &U2_JoyDev[15], arg);
     arg += num;
     }
    }

#ifdef DOS
  uclock();
#endif /*DOS*/
  umask(022);
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

#ifndef DOS
  extern "C" { int strsignal(int); }
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
  abort();
#else
#ifdef DOS
  U2_Exit(1, "\nReceived signal #%d.\n", sn);
#else
  U2_Exit(1, "Received signal \"%s\" (#%d)\n", (char *)strsignal(sn), sn);
#endif
#endif
  }

void U2_Exit(int code)  {
  U2_Exit(code, "%s", "");
  }

void U2_Exit(int code, const char *out, ...)  {
  UserDebug("In U2_Exit Function!");
  StopUserEngine();

  va_list stuff;  
  va_start(stuff, out);
  vprintf(out, stuff);
  va_end(stuff);
  exit(0);
  }

FILE *U2_FOpenRead(const char *fn)  {
  if(fn[0] == '/') return fopen(fn, "rb");
  FILE *file = NULL;
  file = fopen(fn, "rb");
  if(file == NULL && USER_ARGC)  {
    char buffer[256], prep[256];
    int ctr;
    sprintf(prep, "%s%c", USER_ARGV[0], 0);
    for(ctr=strlen(prep); ctr > 0 && prep[ctr-1] != '/'; --ctr);
    prep[ctr] = 0;
    sprintf(buffer, "%s/%s%c", prep, fn, 0);
    file = fopen(buffer, "rb");

//    if(file == NULL) { 
//      char *tmpp1, *tmpp2, *path, delim = ':'; 
//      int fail = 0; 
//      tmpp2 = getenv("PATH"); 
//      path = new char[strlen(tmpp2)+1]; 
//      tmpp1 = path; 
//      strcpy(path, tmpp2); 
//      while((!fail) && file == NULL) {
//        tmpp2 = tmpp1; 
//        while(*tmpp2 != delim && *tmpp2 != 0) tmpp2++;
//        if(*tmpp2 == delim) { 
//          char tmpc = tmpp2[1]; 
//          *tmpp2 = '/';
//          tmpp2[1] = 0; 
//          TryToOpen(filen, tmpp1); 
//          tmpp2[1] = tmpc; 
//          } 
//        else { 
//          tmpp2 = tmpp1; 
//          fail = 1; 
//          } 
//        tmpp1 = ++tmpp2;
//        } 
//      delete path; 
//      } 
    }
  return file;
  }

FILE *U2_FOpenHomeRead(const char *fn)  {
  if(fn[0] == '/') return fopen(fn, "rb");
  FILE *file;
  if(getenv("HOME"))  {
    char buffer[256];
    sprintf(buffer, "%s/%s%c", getenv("HOME"), fn, 0);
    file = fopen(buffer, "rb");
    }
  else {
    file = NULL;
    fprintf(stderr, "Warning: U2_FOpenHomeRead Called with no $HOME set!\n");
    }
  return file;
  }

FILE *U2_FOpenSystemRead(const char *fn)  {
  if(fn[0] == '/') return fopen(fn, "rb");
  FILE *file;
  if(USER_ARGC)  {
    char buffer[256], prep[256];
    int ctr;
    sprintf(prep, "%s%c", USER_ARGV[0], 0);
    for(ctr=strlen(prep); ctr > 0 && prep[ctr-1] != '/'; --ctr);
    prep[ctr] = 0;
    sprintf(buffer, "%s/%s%c", prep, fn, 0);
    file = fopen(buffer, "rb");
    }
  else {
    file = NULL;
    fprintf(stderr, "Warning: U2_FOpenSystemWrite Called before U2_Init!\n");
    }
  return file;
  }

FILE *U2_FOpenWrite(const char *fn)  {
  return fopen(fn, "wb");
  }

FILE *U2_FOpenLocalWrite(const char *fn)  {
  return fopen(fn, "wb");
  }

FILE *U2_FOpenSystemWrite(const char *fn)  {
  if(fn[0] == '/') return fopen(fn, "wb");
  FILE *file;
  if(USER_ARGC)  {
    char buffer[256], prep[256];
    int ctr;
    sprintf(prep, "%s%c", USER_ARGV[0], 0);
    for(ctr=strlen(prep); ctr > 0 && prep[ctr-1] != '/'; --ctr);
    prep[ctr] = 0;
    sprintf(buffer, "%s/%s%c", prep, fn, 0);
    file = fopen(buffer, "wb");
    }
  else {
    file = NULL;
    fprintf(stderr, "Warning: U2_FOpenSystemWrite Called before U2_Init!\n");
    }
  return file;
  }

FILE *U2_FOpenHomeWrite(const char *fn)  {
  if(fn[0] == '/') return fopen(fn, "wb");
  FILE *file;
  if(getenv("HOME"))  {
    char buffer[256];
    sprintf(buffer, "%s/%s%c", getenv("HOME"), fn, 0);
    file = fopen(buffer, "wb");
    }
  else {
    file = NULL;
    fprintf(stderr, "Warning: U2_FOpenHomeWrite Called with no $HOME set!\n");
    }
  return file;
  }
