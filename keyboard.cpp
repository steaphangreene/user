#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <ctype.h>

#include "config.h"
#include "screen.h"
#include "keyboard.h"

extern Screen *__Da_Screen;

Keyboard::Keyboard() {
  if(__Da_Screen == NULL) Exit(-1, "Must create Screen before Keyboard!\n");
  memset(key_stat, 0, 65536);
  crit = 0;
  }

void Keyboard::Update() {
  if(crit) return;
  XEvent e;
  XFlush(__Da_Screen->_Xdisplay);
  while(XCheckMaskEvent(__Da_Screen->_Xdisplay,
	KeyPressMask|KeyReleaseMask, &e))  {
    if(e.type == KeyPress) key_stat[XKeycodeToKeysym(
	__Da_Screen->_Xdisplay, e.xkey.keycode, 0)] = 1;
    else key_stat[XKeycodeToKeysym(
	__Da_Screen->_Xdisplay, e.xkey.keycode, 0)] = 0;
    XFlush(__Da_Screen->_Xdisplay);
    }
  }

int Keyboard::IsPressed(int k) {
  Update();
  return(key_stat[k] != 0);
  }

int Keyboard::WaitForKey() {
  crit = 1;
  int ret = GetAKey();
  if(__Da_Screen == NULL)
    while(ret==0) { ret = GetAKey(); }
  else
    while(ret==0) { __Da_Screen->RefreshFast(); ret = GetAKey(); }
  crit = 0;
  return ret;
  }

int Keyboard::GetAKey(int t) {
  crit = 1;
  int ret = GetAKey();
  if(__Da_Screen == NULL)
    while(t>0&&ret==0) { t-=100; usleep(100000); ret = GetAKey(); }
  else  {
    long dest, udest;
    timeval tv;
    gettimeofday(&tv, NULL);
    dest = t/1000;
    udest = (t-dest*1000)*1000;
    dest += tv.tv_sec;
    udest += tv.tv_usec;
    if(udest > 1000000) { udest -= 1000000; dest += 1; }
    while((tv.tv_sec<dest || tv.tv_usec<udest) && ret==0) {
      __Da_Screen->RefreshFast(); ret = GetAKey(); gettimeofday(&tv, NULL);
      }
    }
  crit = 0;
  return ret;
  }

char Keyboard::WaitForChar() {
  crit = 1;
  char ret = GetAChar();
  if(__Da_Screen == NULL)
    while(ret==0) { ret = GetAChar(); }
  else
    while(ret==0) { __Da_Screen->RefreshFast(); ret = GetAChar(); }
  crit = 0;
  return ret;
  }

char Keyboard::GetAChar(int t) {
  crit = 1;
  char ret = GetAChar();
  if(__Da_Screen == NULL)
    while(t>0&&ret==0) { t-=100; usleep(100000); ret = GetAChar(); }
  else  {
    long dest, udest;
    timeval tv;
    gettimeofday(&tv, NULL);
    dest = t/1000;
    udest = (t-dest*1000)*1000;
    dest += tv.tv_sec;
    udest += tv.tv_usec;
    if(udest > 1000000) { udest -= 1000000; dest += 1; }
    while((tv.tv_sec<dest || tv.tv_usec<udest) && ret==0) {
      __Da_Screen->RefreshFast(); ret = GetAChar(); gettimeofday(&tv, NULL);
      }
    }
  crit = 0;
  return ret;
  }

int Keyboard::GetAKey() {
  XEvent e;
  XFlush(__Da_Screen->_Xdisplay);
  if(!XCheckMaskEvent(__Da_Screen->_Xdisplay, KeyPressMask, &e)) return 0;
  return XKeycodeToKeysym(__Da_Screen->_Xdisplay, e.xkey.keycode, 0);
  }


char Keyboard::GetAChar() {
  int key = GetAKey();
  return KeyToChar(key);
  }

char Keyboard::KeyToChar(int key) {
  int ret;
  switch(key)  {
    case(0): return 0; break;
    case(SCAN_INSERT):  ret = KEY_INSERT;  break;
    case(SCAN_DEL):  ret = KEY_DELETE;  break;
    case(SCAN_BACKSP):  ret = KEY_BACKSPACE;  break;
    case(SCAN_UP):  ret = KEY_UP;  break;
    case(SCAN_DOWN):  ret = KEY_DOWN;  break;
    case(SCAN_LEFT):  ret = KEY_LEFT;  break;
    case(SCAN_RIGHT):  ret = KEY_RIGHT;  break;
    case(SCAN_ESC):  ret = KEY_ESCAPE;  break;
    case(SCAN_END):  ret = KEY_END;  break;
    case(SCAN_HOME):  ret = KEY_HOME;  break;
    case(SCAN_PGUP):  ret = KEY_PGUP;  break;
    case(SCAN_PGDN):  ret = KEY_PGDN;  break;
#ifdef DOS
    default:  ret=__Char_Lookup[which];  break;
#endif
#ifdef X_WINDOWS
    case(SCAN_RETURN):
    case(SCAN_ENTER):  ret = '\n'; break;
    case(SCAN_TAB):  ret = '\t'; break;
    case(SCAN_SPACE):  ret = ' '; break;
    case(SCAN_QUOTE):  ret = '\''; break;
    case(SCAN_BQUOTE):  ret = '`'; break;
    case(SCAN_DOT):  ret = '.'; break;
    case(SCAN_COMA):  ret = ','; break;
    case(SCAN_BSLASH):  ret = '\\'; break;
    case(SCAN_SLASH):  ret = '/'; break;
    case(SCAN_EQUALS):  ret = '='; break;
    case(SCAN_MINUS):  ret = '-'; break;
    case(SCAN_CAPS):
    case(SCAN_LCTRL):
    case(SCAN_RCTRL):
    case(SCAN_RSHIFT):
    case(SCAN_LSHIFT):
    case(SCAN_LALT):
    case(SCAN_RALT):  ret = 0; break;
    default:  {
      ret = XKeysymToString(key)[0];
      if(key_stat[SCAN_LSHIFT]	|| key_stat[SCAN_RSHIFT])  {
	ret = toupper(ret);
	}
      }break;
    }
#endif
  return ret;
  }
