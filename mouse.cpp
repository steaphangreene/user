#include <stdio.h>
#include <string.h>

#include "mouse.h"
#include "engine.h"
#include "screen.h"
#include "sprite.h"
#include "graphic.h"

extern Screen *__Da_Screen;
extern Mouse *__Da_Mouse;

Mouse::Mouse() {
  cvis = 1;
  cursor = NULL;
  mtype = MOUSE_NONE;
  xpos = -666; ypos = -666;
  if(__Da_Screen == NULL) Exit(-1, "Need to create Screen before Mouse!\n");
  memset(butt_stat, 0, 8);
  switch(__Da_Screen->VideoType())  {
    #ifdef X_WINDOWS
    case(VIDEO_XWINDOWS): { mtype=MOUSE_XWINDOWS; } break;
    #ifdef XF86_DGA
    case(VIDEO_XF86DGA): { mtype=MOUSE_XF86DGA; } break;
    #endif
    #endif
    }
  __Da_Mouse = this;
  }

Mouse::~Mouse() {
  __Da_Mouse = NULL; 
  }

int Mouse::IsPressed(int k) {
  Update();
  return(butt_stat[k] != 0);
  }

int Mouse::XPos() { Update(); return xpos; }

int Mouse::YPos() { Update(); return ypos; }

void Mouse::Update() {
  if(crit) return;
  XEvent e;
  XFlush(__Da_Screen->_Xdisplay);
  while(XCheckMaskEvent(__Da_Screen->_Xdisplay,
	PointerMotionMask|ButtonPressMask|ButtonReleaseMask, &e))  {
    if(e.type == ButtonPress) {
      butt_stat[e.xbutton.button] = 1;
//      printf("Press %d\n", e.xbutton.button);
      }
    else if(e.type == ButtonRelease) {
      butt_stat[e.xbutton.button] = 0;
//      printf("Release %d\n", e.xbutton.button);
      }
    else  {
      xpos = e.xmotion.x; ypos = e.xmotion.y;
      }
    XFlush(__Da_Screen->_Xdisplay);
    }
  if(cvis && cursor != NULL && xpos != -666 && ypos != -666)  {
    cursor->Move(xpos, ypos);
//    printf("Moved to (%d, %d)\n", xpos, ypos);
    }
  }

void Mouse::SetCursor(Graphic &g) {
  Debug("User:Mouse:SetCursor() Begin");
  if(mtype == MOUSE_XWINDOWS)  {
    XColor fgc, bgc;
    Pixmap fg, msk;

    fg = XCreatePixmapFromBitmapData(__Da_Screen->_Xdisplay,
	__Da_Screen->_Xwindow, "\0", 1, 1, 0, 0, 1);
    msk = XCreatePixmapFromBitmapData(__Da_Screen->_Xdisplay,
	__Da_Screen->_Xwindow, "\0",1, 1, 0, 0, 1);
    memset(&fgc, 0, sizeof(XColor));
    memset(&bgc, 0, sizeof(XColor));
    XDefineCursor(__Da_Screen->_Xdisplay, __Da_Screen->_Xwindow,
      XCreatePixmapCursor(__Da_Screen->_Xdisplay, fg, msk, &fgc, &bgc, 1, 1));

    }
  if(cursor != NULL) delete cursor;
  Debug("User:Mouse:SetCursor() Middle");
  cursor = new Sprite(g);
  Debug("User:Mouse:SetCursor() End");
  }

void Mouse::ShowCursor() {
  if(cvis || cursor == NULL) return;
  cvis = 1;  Update();
  }

void Mouse::HideCursor() {
  if(cvis == 0 || cursor == NULL) return;
  cvis = 0;  cursor->Erase();
  }

