#include "config.h"

#include "input.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/joystick.h>
#include <ctype.h>

#include "screen.h"
#include "joystick.h"

extern Screen *__Da_Screen;
extern Joystick *__Da_Joystick[16];
extern InputQueue *__Da_InputQueue;

extern char *U2_JoyDev[16];

Joystick::Joystick() {
  for(jnum = 0; jnum < 16; ++jnum) {
    if(__Da_Joystick[jnum] == NULL) break;
    }
  Create(U2_JoyDev[jnum]);
  }

Joystick::Joystick(const char *devfl) {
  Create(devfl);
  }

void Joystick::Create(const char *devfl) {
  if(__Da_Screen == NULL) U2_Exit(-1, "Must create Screen before Joystick!\n");
  memset(AxisRemap, 0, 16*(sizeof(Sprite *)));
  memset(ButtonRemap, 0, 16*(sizeof(Sprite *)));
  memset((char *)AxisStats, 0, 16*sizeof(int));
  memset((char *)ButtonStats, 0, 16*sizeof(char));

  jdev = open(devfl, O_RDONLY|O_NONBLOCK);
  if(jdev < 0) {
    printf("Can not open Joystick device \"%s\"\n", devfl);
    printf("There will be no Joystick support.\n");
    return;
    }

  crit = 0;
  for(jnum = 0; jnum < 16; ++jnum) {
    if(__Da_Joystick[jnum] == NULL) {
      __Da_Joystick[jnum] = this;
      break;
      }
    }
  if(jnum >= 16) {
    U2_Exit(-1, "More then 16 Joysticks unsupported!\n");
    }
  }

Joystick::~Joystick() {
  __Da_Joystick[jnum] = NULL;
  }

void Joystick::Update() {
  if(crit) return;
#ifdef X_WINDOWS
  struct js_event je;
  while(read(jdev, &je, sizeof(struct js_event)) > 0) {
    if(je.type == JS_EVENT_BUTTON) {
      ButtonStats[je.number] = je.value;
      }
    else if(je.type == JS_EVENT_AXIS) {
      AxisStats[je.number] = je.value;
      }
//    else if(je.type == JS_EVENT_INIT|JS_EVENT_BUTTON) {
//      ButtonStats[je.number] = je.value;
//      }
//    else if(je.type == JS_EVENT_INIT|JS_EVENT_AXIS) {
//      AxisStats[je.number] = je.value;
//      }
    }
#endif
  }

int Joystick::IsPressed(int k) {
  Update();
  if(k >= JS_BUTTON && k < JS_BUTTON_MAX) {
    return ButtonStats[k-JS_BUTTON];
    }
  else if(k >= JS_AXIS && k < JS_AXIS_MAX) {
    return AxisStats[k-JS_AXIS];
    }
  else return 0;
  }

unsigned int Joystick::Buttons() {
  int ctr;
  unsigned long ret = 0;
  for(ctr = 0; ctr < (JS_BUTTON_MAX-JS_BUTTON); ++ctr) {
    if(ButtonStats[ctr]) ret |= (1<<ctr);
    }
  return ret;
  }

int Joystick::WaitForAction() {
//  crit = 1;
//  int ret = GetAKey();
//  if(__Da_Screen == NULL)
//    while(ret==0) { ret = GetAKey(); }
//  else
//    while(ret==0) { __Da_Screen->RefreshFast(); ret = GetAKey(); }
//  crit = 0;
//  return ret;
  return 0;
  }

int Joystick::GetAction(int t) {
  crit = 1;
//  int ret = GetAKey();
//  if(__Da_Screen == NULL)
//    while(t>0&&ret==0) { t-=100; usleep(100000); ret = GetAKey(); }
//  else  {
//    time_t dest; long udest;
//    timeval tv;
//    gettimeofday(&tv, NULL);
//    dest = t/1000;
//    udest = (t-dest*1000)*1000;
//    dest += tv.tv_sec;
//    udest += tv.tv_usec;
//    if(udest > 1000000) { udest -= 1000000; dest += 1; }
//    while((tv.tv_sec<dest ||(tv.tv_sec==dest && tv.tv_usec<udest)) && ret==0) {
//      __Da_Screen->RefreshFast(); ret = GetAKey(); gettimeofday(&tv, NULL);
//      }
//    }
//  crit = 0;
//  return ret;
  return 0;
  }

int Joystick::GetAction() {
  int ret=0;
//#ifdef X_WINDOWS
//  XEvent e;
//  XFlush(__Da_Screen->_Xdisplay);
//  while(ret == 0 && XCheckMaskEvent(__Da_Screen->_Xdisplay,
//	KeyPressMask|KeyReleaseMask, &e)) {
//    if(e.type == KeyPress)  {
//      ret = XKeycodeToKeysym(__Da_Screen->_Xdisplay, e.xkey.keycode, 0);
//      key_stat[ret] = 1;
//      }
//    else  {
//      key_stat[XKeycodeToKeysym(__Da_Screen->_Xdisplay, e.xkey.keycode, 0)]=0;
//      }
//    }
//#endif
  return ret;
  }

void Joystick::DisableQueue()  {
  queue_actions=0;
  }

void Joystick::EnableQueue()  {
  queue_actions=1;
  }

void Joystick::MapActionToControl(int k, Control &c)  {
  MapActionToControl(k, &c);
  }

void Joystick::MapActionToControl(int k, Control *c)  {
  if(k >= JS_BUTTON && k < JS_BUTTON_MAX) ButtonRemap[k-JS_BUTTON] = c;
  else if(k >= JS_AXIS && k < JS_AXIS_MAX) AxisRemap[k-JS_AXIS] = c;
  }

void Joystick::MapActionToControl(int k, int c)  {
  if(__Da_Screen != NULL)
	MapActionToControl(k, (Control *)__Da_Screen->GetSpriteByNumber(c));
  }

/*
void Joystick::Down(int k)  {
  if(key_stat[k] == 1) return;
  key_stat[k] = 1;
  InputAction a;
  a.k.type = INPUTACTION_KEYDOWN;
  a.k.modkeys = ModKeys();
  a.k.key = k;
  a.k.chr = KeyToChar(k);
  if(__Da_InputQueue != NULL) __Da_InputQueue->ActionOccurs(&a);
  }

void Joystick::Up(int k)  {
  if(key_stat[k] == 0) return;
  key_stat[k] = 0;
  InputAction a;
  a.k.type = INPUTACTION_KEYUP;
  a.k.modkeys = ModKeys();
  a.k.key = k;
  a.k.chr = 0;
  if(__Da_InputQueue != NULL) __Da_InputQueue->ActionOccurs(&a);
  }
*/

