#include "config.h"

#include <stdio.h>
#include <string.h>
#include <algorithm>
using std::min;
using std::max;

#ifdef DOS
#include <pc.h>
#include <bios.h>
#include <dpmi.h>
#include <sys/segments.h>
#endif

#include "list.h"
#include "mouse.h"
#include "engine.h"
#include "screen.h"
#include "sprite.h"
#include "graphic.h"
#include "control.h"
#include "keyboard.h"
#include "input.h"

extern Mouse *__Da_Mouse;
extern Screen *__Da_Screen;
extern Keyboard *__Da_Keyboard;
extern InputQueue *__Da_InputQueue;

Mouse::Mouse() {
  int ctr, ctr2;
  cvis = 1;
  crit = 0;
  cursor = NULL;
  selector = NULL;
  selg = NULL;
  selcolor = 0;
  curcont = NULL;
  mtype = MOUSE_NONE;
  bdown = -1;
  pin = 0;
  xpos = -666; ypos = -666;
  if(__Da_Screen == NULL) U2_Exit(-1, "Need to create Screen before Mouse!\n");
  memset(butt_stat, 0, MAX_MBUTTONS);
  for(ctr=0; ctr<MAX_PANELS; ctr++)
	memset(pb[ctr], 0, MAX_MBUTTONS*sizeof(int));
  switch(__Da_Screen->VideoType())  {
    #ifdef DOS
    case(VIDEO_DOS): {
      mtype=MOUSE_DOS;

      __dpmi_regs regs;
      regs.x.ax = 0x0000;
      __dpmi_int(0x33, &regs);
      if(regs.x.ax != 0xFFFF)  {
	U2_Exit(1, "Mouse Driver Not installed!\n");
	}
      num_butt = regs.x.bx;
      }break;
    #endif
    #ifdef X_WINDOWS
    case(VIDEO_XWINDOWS): {
      mtype=MOUSE_XWINDOWS;
      }break;
    #ifdef XF86_DGA
    #ifdef X_DGA2
    case(VIDEO_XDGA2): {
      mtype=MOUSE_XDGA2;
      }break;
    #endif
    case(VIDEO_XF86DGA): {
      mtype=MOUSE_XF86DGA;
      }break;
    #endif
    #endif
    }
  SetRange(0, 0, __Da_Screen->XSize(), __Da_Screen->YSize());
  
  for(ctr=0; ctr<MAX_MBUTTONS; ++ctr) {
    for(ctr2=0; ctr2<MAX_PANELS; ++ctr2) {
      pb[ctr2][ctr] = MB_CLICK;
      }
    }
  __Da_Mouse = this;
  cursor = new Sprite;
  cursor->SetPriority(-10000);
  cursor->DisableCollisions();
  selector = new Sprite;
  selector->SetPriority(-10000);
  selector->DisableCollisions();
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
  switch(mtype)  {
#ifdef DOS
    case(MOUSE_DOS): {
      __dpmi_regs regs;
      regs.x.ax = 0x0003;
      __dpmi_int(0x33, &regs);
      xpos = regs.x.cx/2;
      ypos = regs.x.dx;
      butt_stat[0] = (regs.x.bx & 1);
      butt_stat[2] = (regs.x.bx & 2)>>1;
      if(num_butt > 2)
	butt_stat[1] = (regs.x.bx & 4)>>2;
      else
	butt_stat[1] = (butt_stat[0] && butt_stat[2]);
      }break;
#endif

#ifdef X_WINDOWS
#ifdef XF86_DGA
#ifdef X_DGA2
    case(MOUSE_XDGA2): {
      int xp=xpos, yp=ypos;
      XEvent e;
      XFlush(__Da_Screen->_Xdisplay);
      while(XCheckMaskEvent(__Da_Screen->_Xdisplay,
		PointerMotionMask|ButtonPressMask|ButtonReleaseMask, &e))  {
	if(e.type == ButtonPress) {
//	  xp+=e.xbutton.x; yp+=e.xbutton.y;
	  Pressed(e.xbutton.button, xp, yp);
	  }
	else if(e.type == ButtonRelease) {
//	  xp+=e.xbutton.x; yp+=e.xbutton.y;
	  Released(e.xbutton.button, xp, yp);
	  }
	else  {
	  xp+=e.xmotion.x; yp+=e.xmotion.y;
	  if(xp<rngxs) xp=rngxs;
	  if(yp<rngys) yp=rngys;
	  if(xp>=rngxe) xp=rngxe-1;
	  if(yp>=rngye) yp=rngye-1;
	  }
	XFlush(__Da_Screen->_Xdisplay);
	}
      Moved(xp, yp);
      }break;
#endif
    case(MOUSE_XF86DGA): {
      int xp=xpos, yp=ypos;
      XEvent e;
      XFlush(__Da_Screen->_Xdisplay);
      while(XCheckMaskEvent(__Da_Screen->_Xdisplay,
		PointerMotionMask|ButtonPressMask|ButtonReleaseMask, &e))  {
	if(e.type == ButtonPress) {
//	  xp+=e.xbutton.x; yp+=e.xbutton.y;
	  Pressed(e.xbutton.button, xp, yp);
	  }
	else if(e.type == ButtonRelease) {
//	  xp+=e.xbutton.x; yp+=e.xbutton.y;
	  Released(e.xbutton.button, xp, yp);
	  }
	else  {
	  xp+=e.xmotion.x; yp+=e.xmotion.y;
	  if(xp<rngxs) xp=rngxs;
	  if(yp<rngys) yp=rngys;
	  if(xp>=rngxe) xp=rngxe-1;
	  if(yp>=rngye) yp=rngye-1;
	  }
	XFlush(__Da_Screen->_Xdisplay);
	}
      Moved(xp, yp);
      }break;
#endif
    case(MOUSE_XWINDOWS): {
      int xp=xpos, yp=ypos;
      XEvent e;
      XFlush(__Da_Screen->_Xdisplay);
      while(XCheckMaskEvent(__Da_Screen->_Xdisplay,
		PointerMotionMask|ButtonPressMask|ButtonReleaseMask, &e))  {
	if(e.type == ButtonPress) {
	  xp=e.xbutton.x; yp=e.xbutton.y;
	  Pressed(e.xbutton.button, xp, yp);
	  }
	else if(e.type == ButtonRelease) {
	  xp=e.xbutton.x; yp=e.xbutton.y;
	  Released(e.xbutton.button, xp, yp);
	  }
	else  {
	  xp=e.xmotion.x; yp=e.xmotion.y;
	  }
	XFlush(__Da_Screen->_Xdisplay);
	}
      Moved(xp, yp);
      }break;
#endif
    }
  }

void Mouse::SetCursor(Graphic *g) {
  SetCursor(*g);
  }

void Mouse::SetCursor(Graphic &g) {
  UserDebug("User:Mouse:SetCursor() Begin");
#ifdef X_WINDOWS
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
#endif
  UserDebug("User:Mouse:SetCursor() Middle");
  cursor->SetImage(g);
  UserDebug("User:Mouse:SetCursor() End");
  }

void Mouse::ShowCursor() {
  if(cvis || cursor == NULL) return;
  xpos=0; ypos=0;
  cvis = 1;  Update();
  }

void Mouse::HideCursor() {
  if(cvis == 0 || cursor == NULL) return;
  cvis = 0;  cursor->Erase();
  }

void Mouse::SetRange(int x1, int y1, int x2, int y2) {
  rngxs = x1;
  rngxe = x2;
  rngys = y1;
  rngye = y2;
#ifdef DOS
  __dpmi_regs regs;
  regs.x.ax = 0x0007;
  regs.x.cx = x1*2;
  regs.x.dx = x2*2;
  __dpmi_int(0x33, &regs);
  regs.x.ax = 0x0008;
  regs.x.cx = y1;
  regs.x.dx = y2;
  __dpmi_int(0x33, &regs);
#endif  
  }

void Mouse::Pressed(int b, int x, int y) {
  butt_stat[b] = 1;
  IntList clk = __Da_Screen->CollideRectangle(x, y, 1, 1);
  if(clk.Size() > 0)  {
    int ctr; Sprite *s;
    for(ctr=0; ctr<clk.Size(); ctr++)  {
      s = __Da_Screen->GetSpriteByNumber(clk[ctr]);
      if(s!=NULL && s->IsControl())  {
	((Control *)s)->Click(b); 
	if(curcont == NULL) {
	  curcont = (Control *)s;
	  contx = x; conty = y; contb = b;
	  }
	return;
	}
      }
    }
  if(__Da_InputQueue == NULL) return;
  Panel p = __Da_Screen->WhichPanel(x, y);
  switch(pb[p][b]) {
    case(MB_IGNORE): break;
    case(MB_CLICKDRAW):
    case(MB_DRAW): {
      if(bdown < 0) bdown = b;
      startx = x; starty = y; pin = p;
      if(pb[p][b] != MB_CLICKDRAW) break;
      }
    case(MB_CLICK): {
      InputAction a;
      a.g.type = INPUTACTION_MOUSEDOWN;
      a.m.button = b;
      a.m.x = x;
      a.m.y = y;
      a.m.xs = 1;
      a.m.ys = 1;
      a.m.panel = __Da_Screen->WhichPanel(x, y);
      if(__Da_Keyboard != NULL) a.m.modkeys = __Da_Keyboard->ModKeys();
      else a.m.modkeys = 0;
      __Da_InputQueue->ActionOccurs(&a);
      }break;
    case(MB_LINE): {
      if(bdown < 0) bdown = b;
      startx = x; starty = y; pin = p;
      if(selg == NULL) selg = new Graphic;
      selg->SetFillRect(1, 1, __Da_Screen->GetApparentDepth(), selcolor);
      if(selector == NULL) selector = new Sprite;
      selector->Erase();
      selector->UseImage(selg);
      selector->Draw(x, y);
      }break;
    case(MB_BOX): {
      if(bdown < 0) bdown = b;
      startx = x; starty = y; pin = p;
      if(selg == NULL) selg = new Graphic;
      selg->SetFillRect(1, 1, __Da_Screen->GetApparentDepth(), selcolor);
      if(selector == NULL) selector = new Sprite;
      selector->Erase();
      selector->UseImage(selg);
      selector->Draw(x, y);
      }break;
    }
  }

void Mouse::Released(int b, int x, int y) {
  butt_stat[b] = 0;
  if(curcont != NULL) {
    curcont->UnClick(b);
    if(b == contb) curcont = NULL;
    }
  if(__Da_InputQueue == NULL) return;
  if(x < __Da_Screen->PanelXStart(pin)) x = __Da_Screen->PanelXStart(pin);
  if(y < __Da_Screen->PanelYStart(pin)) y = __Da_Screen->PanelYStart(pin);
  if(x >= __Da_Screen->PanelXEnd(pin)) x = __Da_Screen->PanelXEnd(pin)-1;
  if(y >= __Da_Screen->PanelYEnd(pin)) y = __Da_Screen->PanelYEnd(pin)-1;
  switch(pb[pin][bdown]) {
    case(MB_IGNORE): break;
    case(MB_DRAW):
    case(MB_CLICKDRAW): {
      pin = 0; bdown = -1;
      }break;
    case(MB_CLICK): {
      InputAction a;
      a.g.type = INPUTACTION_MOUSEUP;
      a.m.button = b;
      a.m.x = x;
      a.m.y = y;
      a.m.xs = 1;
      a.m.ys = 1;
      a.m.panel = __Da_Screen->WhichPanel(x, y);
      if(__Da_Keyboard != NULL) a.m.modkeys = __Da_Keyboard->ModKeys();
      else a.m.modkeys = 0;
      __Da_InputQueue->ActionOccurs(&a);
      }break;
    case(MB_LINE): {
      if(b == bdown) {
	bdown = -1;
	InputAction a;
	a.g.type = INPUTACTION_MOUSEDOWN;
	a.m.button = b;
	a.m.x = startx;
	a.m.y = starty;
	a.m.xs = x-startx;
	a.m.ys = y-starty;
	a.m.panel = __Da_Screen->WhichPanel(x, y);
	if(__Da_Keyboard != NULL) a.m.modkeys = __Da_Keyboard->ModKeys();
	else a.m.modkeys = 0;
	__Da_InputQueue->ActionOccurs(&a);
	if(selector != NULL) selector->Erase();
	}
      }break;
    case(MB_BOX): {
      if(b == bdown) {
	bdown = -1;
	InputAction a;
	a.g.type = INPUTACTION_MOUSEDOWN;
	a.m.button = b;
	a.m.x = min(x, startx);
	a.m.y = min(y, starty);
	a.m.xs = abs(x-startx)+1;
	a.m.ys = abs(y-starty)+1;
	a.m.panel = __Da_Screen->WhichPanel(x, y);
	if(__Da_Keyboard != NULL) a.m.modkeys = __Da_Keyboard->ModKeys();
	else a.m.modkeys = 0;
	__Da_InputQueue->ActionOccurs(&a);
	if(selector != NULL) selector->Erase();
	}
      }break;
    }
  }

void Mouse::Moved(int x, int y) {
  if(x==xpos && y==ypos) return;
  xpos = x; ypos = y;
  if(curcont != NULL) curcont->Drag(contb, x-contx, y-conty);
  contx = x; conty = y;
  if(bdown >= 0) {
    if(x < __Da_Screen->PanelXStart(pin)) x = __Da_Screen->PanelXStart(pin);
    if(y < __Da_Screen->PanelYStart(pin)) y = __Da_Screen->PanelYStart(pin);
    if(x >= __Da_Screen->PanelXEnd(pin)) x = __Da_Screen->PanelXEnd(pin)-1;
    if(y >= __Da_Screen->PanelYEnd(pin)) y = __Da_Screen->PanelYEnd(pin)-1;
    xpos = x; ypos = y;
    switch(pb[pin][bdown]) {
      case(MB_IGNORE): break;
      case(MB_DRAW):
      case(MB_CLICKDRAW): {
	InputAction a;
	a.g.type = INPUTACTION_MOUSEDOWN;
	a.m.button = bdown;
	a.m.x = x;
	a.m.y = y;
	a.m.xs = startx;
	a.m.ys = starty;
	a.m.panel = pin;
	if(__Da_Keyboard != NULL) a.m.modkeys = __Da_Keyboard->ModKeys();
	else a.m.modkeys = 0;
	__Da_InputQueue->ActionOccurs(&a);
	}break;
      case(MB_LINE): {
	if(selg == NULL) selg = new Graphic;
	selector->Erase();
	selg->SetLine(x-startx, y-starty,
		__Da_Screen->GetApparentDepth(), selcolor);
	if(selector == NULL) selector = new Sprite;
	selector->UseImage(selg);
	selector->Draw(startx, starty);
	}break;
      case(MB_BOX): {
	if(selg == NULL) selg = new Graphic;
	selector->Erase();
	selg->SetRect(abs(x-startx)+1, abs(y-starty)+1,
		__Da_Screen->GetApparentDepth(), selcolor);
	if(selector == NULL) selector = new Sprite;
	selector->UseImage(selg);
	selector->Draw(min(x, startx), min(y, starty));
	}break;
      }
    }
  if(cvis && cursor != NULL && xpos != -666 && ypos != -666)  {
    cursor->Move(xpos, ypos);
    }
  }

void Mouse::SetBehavior(Panel p, int butt, int mb) {
  pb[p][butt] = mb;
  }

void Mouse::SetSelColor(color c) {
  selcolor = c;
  }

int Mouse::DrawingSelector() {
  return (bdown >= 0);
  }
