#include <user/os_defs.h>
#include <user/mouse.h>
#include <user/screen.h>
#include <string.h>

#ifdef DOS
#include <dos.h>
#endif

#ifdef X_WINDOWS
#define Screen X_Screen
#define Window X_Window
#define Cursor X_Cursor

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#ifdef SVGALIB
#include <vgamouse.h>
#endif

#undef Screen
#undef Window
#undef Cursor

extern int IN_X;
extern unsigned char *Frame;
extern int ___myscreen;
extern Display *___mydisplay;
extern X_Window ___mywindow;
extern XSizeHints ___myhint;
extern XWMHints ___mywmhint;
extern GC ___mygc;
extern XEvent ___myevent;       
extern Colormap ___mymap;
extern XColor ___mypal[256];
extern XImage *___mypic;
#endif

extern User * __Da_User;
extern Screen * __Da_Screen;
Mouse * __Da_Mouse = NULL;

Mouse::Mouse()  {
  InitMouse();
  }

Mouse::Mouse(int ixp, int iyp)  {
  InitMouse();
  SetPosition(ixp, iyp);
  }

Mouse::Mouse(const Graphic *icur)  {
  InitMouse();
  SetCursor(icur);
  }

Mouse::Mouse(const Graphic &icur)  {
  InitMouse();
  SetCursor(icur);
  }

Mouse::Mouse(const Graphic *icur, int ixp, int iyp)  {
  InitMouse();
  SetPosition(ixp, iyp);
  SetCursor(icur);
  }

Mouse::Mouse(const Graphic &icur, int ixp, int iyp)  {
  InitMouse();
  SetPosition(ixp, iyp);
  SetCursor(icur);
  }

Mouse::~Mouse()  {
  __Da_Mouse = NULL;
  delete Cursor;
  }

void Mouse::InitMouse()  {
  if(__Da_User == NULL)  {
    Exit(1, "Need a User to control the mouse (Must create User first).\n");
    }
  int ctr;
  curbutt = NULL;
  for(ctr=1; ctr<MAX_PANEL; ctr++)  {
    behavior[ctr][0] = MOUSE_UNDEFINED;
    behavior[ctr][1] = MOUSE_UNDEFINED;
    behavior[ctr][2] = MOUSE_UNDEFINED;
    }
  behavior[0][0] = MOUSE_BOX;
  behavior[0][1] = MOUSE_BOX;
  behavior[0][2] = MOUSE_IGNORE;
  if(__Da_Mouse != NULL)  {
    Exit(1, "Two Mice!?  That's nuts!\n");
    }
  __Da_Mouse = this;
  xend = -1;
  yend = -1;
  xstart = -1;
  ystart = -1;
  bstart = 0;
  lbutton = 0;
  rbutton = 0;

#ifdef DOS
  union REGS inregs;
  union REGS outregs;
  inregs.w.ax = 0x0000;
  int86(0x33, &inregs, &outregs);
  if(outregs.w.ax != 0xFFFF)  {
    Exit(1, "Mouse Driver Not installed!\n");
    }
  numbuttons = outregs.w.bx;
  if((outregs.w.bx > 3) || (outregs.w.bx < 2))  numbuttons = 2;
#endif

  shown = 0;
  Dot = new Graphic(8, 8);
  Dot->DefLinH("FFFFFFFFFFFFFFFF");
  Dot->DefLinH("FFFFFFFFFFFFFFFF");
  Dot->DefLinH("FFFFFFFFFFFFFFFF");
  Dot->DefLinH("FFFFFFFFFFFFFFFF");
  Dot->DefLinH("FFFFFFFFFFFFFFFF");
  Dot->DefLinH("FFFFFFFFFFFFFFFF");
  Dot->DefLinH("FFFFFFFFFFFFFFFF");
  Dot->DefLinH("FFFFFFFFFFFFFFFF");
  Box[0].DisableCollisions();
  Box[0].SetPriority(1);
  Box[1].DisableCollisions();
  Box[1].SetPriority(1);
  Box[2].DisableCollisions();
  Box[2].SetPriority(1);
  Box[3].DisableCollisions();
  Box[3].SetPriority(1);
  Cursor = new Sprite;
  Cursor->DisableCollisions();
  Cursor->SetPriority(0);

#ifdef X_WINDOWS
  X_Window rt, ot;
  int ay, ax, wy, wx;
  unsigned int knb;
  XQueryPointer(___mydisplay, ___mywindow, &rt, &ot, &ax, &ay, &wx, &wy, &knb);
  xpos = wx; ypos = wy;
  if(IN_X)  {
    XGrabPointer(___mydisplay, ___mywindow, True,
      PointerMotionMask | ButtonPressMask | ButtonReleaseMask,
      GrabModeAsync, GrabModeAsync, ___mywindow,  None, CurrentTime);
    }
  else if(Frame != NULL)  {
    XGrabPointer(___mydisplay, ___mywindow, True,
      PointerMotionMask | ButtonPressMask | ButtonReleaseMask,
      GrabModeAsync, GrabModeAsync, None,  None, CurrentTime);
    }
#endif
  }

void Mouse::SetPanelBehavior(Panel p, int l, int m, int r)  {
  behavior[p][0] = l;
  behavior[p][2] = m;
  behavior[p][1] = r;
  }

void Mouse::SetCursor(const Graphic *icur)  {
  Cursor->UseImage(icur);
  }

void Mouse::SetCursor(const Graphic &icur)  {
  Cursor->SetImage(icur);

#ifdef X_WINDOWS
  if(IN_X)  {
    XColor fgc, bgc;
    Pixmap fg, msk;

    fg = XCreatePixmapFromBitmapData(___mydisplay, ___mywindow, "\0",
        1, 1, 0, 0, 1);
    msk = XCreatePixmapFromBitmapData(___mydisplay, ___mywindow, "\0",
        1, 1, 0, 0, 1);
    bzero(&fgc, sizeof(XColor));
    bzero(&bgc, sizeof(XColor));
    XDefineCursor(___mydisplay, ___mywindow,
        XCreatePixmapCursor(___mydisplay, fg, msk, &fgc, &bgc, 1, 1));
    }
#endif
  }
	
void Mouse::ShowCursor(int c1, int c2)  {
  if(Cursor->image != NULL)  delete Cursor->image;
  ShowCursor();
#ifdef X_WINDOWS
  c1=c2;  //UNUSED!!!
#endif
#ifdef DOS
  unsigned int ctrx, ctry;
  for(ctry=0; ctry<Cursor->image->ysize; ctry++)  {
    for(ctrx=0; ctrx<Cursor->image->xsize; ctrx++)  {
      if(Cursor->image->image[ctry][ctrx] == 0xFF)
	Cursor->image->image[ctry][ctrx] = c1;
      else if(Cursor->image->image[ctry][ctrx] == 0xFE)
	Cursor->image->image[ctry][ctrx] = c2;
      }
    }
#endif
  }

void Mouse::ShowCursor()  {
  shown = 1;
  SetXRange(0, __Da_Screen->pxend[0]-1);
  SetYRange(0, __Da_Screen->pyend[0]-1);
  SetPosition((__Da_Screen->pxend[0]/2), (__Da_Screen->pyend[0]/2));

#ifdef X_WINDOWS
  if(!IN_X)  {
#endif
   if(Cursor->image == NULL)  {
    Cursor->DefSize(12,12);
    Cursor->DefLinH("FFFF--------------------");
    Cursor->DefLinH("FFFEFFFF----------------");
    Cursor->DefLinH("--FFFEFEFFFF------------");
    Cursor->DefLinH("--FFFEFEFEFEFFFF--------");
    Cursor->DefLinH("----FFFEFEFEFEFEFFFF----");
    Cursor->DefLinH("----FFFEFEFEFEFEFF------");
    Cursor->DefLinH("------FFFEFEFEFF--------");
    Cursor->DefLinH("------FFFEFEFFFEFF------");
    Cursor->DefLinH("--------FFFF--FFFEFF----");
    Cursor->DefLinH("--------FF------FFFEFF--");
    Cursor->DefLinH("------------------FFFEFF");
    Cursor->DefLinH("--------------------FFFF");
    }
#ifdef X_WINDOWS
   }
#endif
  }

void Mouse::HideCursor()  {
  shown = 0;
  }

void Mouse::Update()  {
  int xp = xpos, yp = ypos;
  UpdatePos();
  if(xp == xpos && yp == ypos && xp == Cursor->xpos && yp == Cursor->ypos &&
	Cursor->visible)
		return;
  if(shown == 0)  return;
  if(ystart >= 0 && behavior[pstart][bstart-1] != MOUSE_DRAW)  {
    int tx = xstart, ty = ystart;
    int bx = xpos, by = ypos;
    int quad = 0;
    if(xpos < xstart)  {  tx = xpos;  bx = xstart; quad = 1-quad; }
    if(ypos < ystart)  {  ty = ypos;  by = ystart; quad = 1-quad; }
    int xs = (bx-tx)+1, ys = (by-ty)+1;

    if(behavior[pstart][bstart-1] == MOUSE_BOX)  {
      Box[0].SetPanel(pstart);
      Box[0].SetImage(Dot->Scaled(1, ys));
      Box[2].SetPanel(pstart);
      Box[2].SetImage(Dot->Scaled(1, ys));
      Box[1].SetPanel(pstart);
      Box[1].SetImage(Dot->Scaled(xs, 1));
      Box[3].SetPanel(pstart);
      Box[3].SetImage(Dot->Scaled(xs, 1));

      Box[0].Move(tx,ty);
      Box[1].Move(tx,ty);
      Box[2].Move(bx,ty);
      Box[3].Move(tx,by);
      }
    else  {
      char tmpc[xs+1];
      bzero(tmpc, xs+1);
      Graphic lng(xs, ys);
      int ctr, xp, nxp;
      if(quad)  {
	xp=xs-1;
	for(ctr=0; ctr<ys; ctr++)  {
	  nxp = (xs-1) - (((ctr+1)*xs)/ys);
	  memset(&tmpc[nxp], 255, (xp-nxp)+1);
	  lng.DefLin(tmpc);
	  memset(&tmpc[nxp], 0, (xp-nxp)+1);
	  xp=nxp;
	  }
	}
      else  {
	xp=0;
	for(ctr=0; ctr<ys; ctr++)  {
	  nxp = ((ctr+1)*xs)/ys;
	  memset(&tmpc[xp], 255, (nxp-xp)+1);
	  lng.DefLin(tmpc);
	  memset(&tmpc[xp], 0, (nxp-xp)+1);
	  xp=nxp;
	  }
	}
      Box[0].SetPanel(pstart);
      Box[0].SetImage(lng);
      Box[0].Move(tx,ty);
      }
    }
  Cursor->Move(xpos, ypos);
  }

void Mouse::UpdatePos()  {
  char justpressed;
  int ctr, ctr2;

#ifdef DOS
  union REGS inregs;
  union REGS outregs;
  inregs.w.ax = 0x0003;
  int86(0x33, &inregs, &outregs);
  xpos = outregs.w.cx/2;
  ypos = outregs.w.dx;
  lbutton = ((outregs.w.bx) & 1);
  rbutton = ((outregs.w.bx) & 2)/2;
  if((numbuttons > 2) && ((outregs.w.bx) & 4))  { lbutton = 1; rbutton = 1; }
#endif

#ifdef X_WINDOWS
#ifdef SVGALIB
  if(IN_X || (Frame != NULL))  {
#endif
    XFlush(___mydisplay);
    while(XCheckMaskEvent(___mydisplay, PointerMotionMask, &___myevent))  {
      if(Frame != NULL)  {
	xpos += ___myevent.xmotion.x_root;
	ypos += ___myevent.xmotion.y_root;
	}
      else {
	xpos = ___myevent.xmotion.x;
	ypos = ___myevent.xmotion.y;
	}
      __Da_Screen->ClipToPanel(xpos, ypos, 0);
      }

//    X_Window rt, ot;
//    int ay, ax, wy, wx;
//    unsigned int knb;
//    XQueryPointer(___mydisplay, ___mywindow, &rt, &ot, &ax, &ay, &wx, &wy, &knb);
//    xpos = wx; ypos = wy;

//    lbutton = (knb & Button1Mask)/Button1Mask;
//    rbutton = (knb & Button3Mask)/Button3Mask;
//    if(knb & Button2Mask){lbutton = 1; rbutton = 1;}
#ifdef SVGALIB
    }
  else  {
   int tmpb;
   mouse_update();
   xpos = mouse_getx();
   ypos = mouse_gety();
   tmpb = mouse_getbutton();
   lbutton = ((MOUSE_LEFTBUTTON & tmpb) == MOUSE_LEFTBUTTON);
   rbutton = ((MOUSE_RIGHTBUTTON & tmpb) == MOUSE_RIGHTBUTTON);
// ***************************************
    }
#endif
#endif

  if(!shown)  return;
  if(ystart >= 0)  {
    __Da_Screen->ClipToPanel(xpos, ypos, pstart);
    SetPosition(xpos, ypos);
    }
  justpressed = 0;

#ifdef DOS
  if((!bstart) && (lbutton+rbutton) >0 && ystart < 0)  {
    justpressed = 1;
    pstart = 0;
    bstart = (lbutton+(rbutton*2));
    inregs.w.ax = 0x0005;
    inregs.w.bx = bstart-1;
    if(bstart == 3 && numbuttons < 3)  inregs.w.bx = 0;
    int86(0x33, &inregs, &outregs);
    xstart = outregs.w.cx/2;
    ystart = outregs.w.dx;
    for(ctr=(MAX_PANEL-1); ctr > 0 && pstart == 0; ctr--)  {
      if(ystart >= __Da_Screen->pystart[ctr] &&
		xstart >= __Da_Screen->pxstart[ctr] &&
		ystart < __Da_Screen->pyend[ctr] &&
		xstart < __Da_Screen->pxend[ctr] &&
		behavior[ctr][bstart-1] != MOUSE_UNDEFINED)
	pstart = ctr;
      }
    if(bstart > 1 && behavior[pstart][bstart-1] == MOUSE_IGNORE)
      bstart=1;
    if(bstart == 1 && behavior[pstart][0] == MOUSE_IGNORE)  {
      bstart=0;
      ystart=-1;
      xstart=-1;
      pstart = 0;
      }
    }
#endif

#ifdef X_WINDOWS
  if(bstart == 0)  {
    XFlush(___mydisplay);
//    while(XCheckMaskEvent(___mydisplay, ButtonReleaseMask, &___myevent));
    while((XCheckMaskEvent(___mydisplay, ButtonPressMask, &___myevent))
	&& (xpos >= __Da_Screen->pxstart[0] && ypos >= __Da_Screen->pystart[0]
	&& xpos < __Da_Screen->pxend[0]	&& ypos < __Da_Screen->pyend[0]))  {
      justpressed = 1;
      bstart = ___myevent.xbutton.button;
      if(bstart == 3) bstart = 2;
      else if(bstart == 2) bstart = 3;
      lbutton = (bstart & 1);
      rbutton = (bstart & 2) >> 1;
      if(Frame != NULL)  {
	xstart = xpos;
	ystart = ypos;
	}
      else {
	xstart = ___myevent.xbutton.x;
	ystart = ___myevent.xbutton.y;
	}
      pstart = 0;
      for(ctr=(MAX_PANEL-1); ctr > 0 && pstart == 0; ctr--)  {
	if(ystart >= __Da_Screen->pystart[ctr] &&
		xstart >= __Da_Screen->pxstart[ctr] &&
		ystart < __Da_Screen->pyend[ctr] &&
		xstart < __Da_Screen->pxend[ctr] &&
		behavior[ctr][bstart-1] != MOUSE_UNDEFINED)
	  pstart = ctr;
	}
      if(bstart > 1 && behavior[pstart][bstart-1] == MOUSE_IGNORE)  bstart=1;
      }
    }
  else  {
    XFlush(___mydisplay);
    while(XCheckMaskEvent(___mydisplay, ButtonPressMask, &___myevent));
    while(XCheckMaskEvent(___mydisplay, ButtonReleaseMask, &___myevent))  {
      lbutton = 0;
      rbutton = 0;
      if(Frame != NULL)  {
	xend = xpos;
	yend = ypos;
	}
      else {
	xend = ___myevent.xbutton.x;
	yend = ___myevent.xbutton.y;
	}
      __Da_Screen->ClipToPanel(xend, yend, pstart);
      }
    }
	//*****************************************************************

//    while(XCheckMaskEvent(___mydisplay, ButtonReleaseMask | ButtonPressMask,
//        &___myevent))  {
//      if(___myevent.type == ButtonPress)  {
//	printf("Does %p == %p?\n", ___mywindow, ___myevent.xbutton.window);
//	JustPressedKey(___myevent.xkey.keycode);
//	keyboard_map[___myevent.xkey.keycode] = 1;
//        }
//      else  if(___myevent.type == ButtonRelease)  {
//	JustReleasedKey(___myevent.xkey.keycode);
//	keyboard_map[___myevent.xkey.keycode] = 0;
//        }
//      else  printf("Got Unknown event of type %d!\r\n", ___myevent.type);
//      }

	//*****************************************************************

//      X_Window rt, ot;
//      int ay, ax, wy, wx;
//      unsigned int knb;
//
//      XQueryPointer(___mydisplay, DefaultRootWindow(___mydisplay),
//	&rt, &ot, &ax, &ay, &wx, &wy, &knb);
//      printf("Pointer Click in %p, on  %p\n", ot, rt);
#endif

  if(bstart && ystart >= 0 && behavior[pstart][bstart-1] == MOUSE_CLICK)  {
    curbutt = NULL;
    UserAction *tmp = new UserAction(pstart, bstart, xstart, ystart,
	xstart, ystart);
    unsigned long mask = 1;
    mask <<= (31-(xstart%32));
    if(__Da_Screen->spritehere[xstart>>5][ystart] != NULL)
      *tmp+=__Da_Screen->spritehere[xstart>>5][ystart]->
	HitBlock(xstart>>5, ystart, mask, -1);
    for(ctr=0; ctr<tmp->Size() && curbutt == NULL; ctr++)  {
      if((__Da_Screen->SpriteList[(*tmp)[ctr]]->mouseinter == SPRITE_SBUTTON) ||
	(__Da_Screen->SpriteList[(*tmp)[ctr]]->mouseinter == SPRITE_BUTTON)){
		curbutt = __Da_Screen->SpriteList[(*tmp)[ctr]];
		((Button *)curbutt)->Click();
		}
      }
    ystart = -1;
    xstart = -1;
    if(curbutt == NULL)  __Da_User->ActionOccured(tmp);
    else delete tmp;
    }
  else if(bstart && behavior[pstart][bstart-1] == MOUSE_DRAW
	&& (ystart != ypos || xstart != xpos || justpressed))  {
    UserAction *tmp = new UserAction(pstart, bstart, xpos, ypos,
	xpos, ypos);
//    unsigned long mask = 1;
//    mask <<= (31-(xstart%32));
//    if(__Da_Screen->spritehere[xstart/32][ystart] != NULL)
//      *tmp+=__Da_Screen->spritehere[xstart/32][ystart]->
//	HitBlock(xstart/32, ystart, mask, -1);
    ystart = ypos;
    xstart = xpos;
    __Da_User->ActionOccured(tmp);
    }
  else if(bstart && ystart >= 0 && (lbutton+rbutton) == 0
	&& behavior[pstart][bstart-1] != MOUSE_DRAW)  {		//Got A BOX!!!

#ifdef DOS
    inregs.w.ax = 0x0006;
    inregs.w.bx = bstart-1;
    if(bstart == 3 && numbuttons < 3)  inregs.w.bx = 0;
    int86(0x33, &inregs, &outregs);
    xend = outregs.w.cx/2;
    yend = outregs.w.dx;
#endif

#ifdef X_WINDOWS
//    xend = xpos;
//    yend = ypos;
#endif

    __Da_Screen->ClipToPanel(xend, yend, pstart);

    Box[0].Erase();	Box[1].Erase();
    Box[2].Erase();	Box[3].Erase();
    UserAction *tmp = new UserAction(pstart, bstart, xstart, ystart, xend, yend);
    int xs=xstart, xe=xpos, ys=ystart, ye=ypos;
    if(xs>xe)  { ctr=xs; xs=xe; xe=ctr; }
    if(ys>ye)  { ctr=ys; ys=ye; ye=ctr; }
    unsigned long mask; 
    for(ctr=(xs>>5); ctr<=(xe>>5); ctr++)  {
      mask = 0xFFFFFFFF;
      if(ctr==(xs>>5) && (xs%32))  mask >>= (xs%32);
      if(ctr==(xe>>5))  { mask >>= (31-(xe%32)); mask <<= (31-(xe%32)); }
      for(ctr2=ys; ctr2<=ye; ctr2++)  if(__Da_Screen->spritehere[ctr][ctr2] != NULL)  {
        *tmp+=__Da_Screen->spritehere[ctr][ctr2]->HitBlock(ctr, ctr2, mask, -1);
        }
      }
    ystart = -1;
    xstart = -1;
    bstart = 0;
    __Da_User->ActionOccured(tmp);
    }
  if(bstart && (lbutton+rbutton) == 0)  {
    bstart = 0;
    ystart = -1;
    xstart = -1;
    if(curbutt != NULL && curbutt->mouseinter == SPRITE_BUTTON)
	((Button *)curbutt)->Click();
    curbutt = NULL;
    }
  }

void Mouse::SetXRange(int min, int max)  {
#ifdef X_WINDOWS
  min=max; //UNUSED!!!
#endif

#ifdef DOS
  union REGS inregs;
  union REGS outregs;
  inregs.w.ax = 0x0007;
  inregs.w.cx = min*2;
  inregs.w.dx = max*2;
  int86(0x33, &inregs, &outregs);
#endif
  }

void Mouse::SetYRange(int min, int max)  {
#ifdef X_WINDOWS
  min=max; //UNUSED!!!
#endif

#ifdef DOS
  union REGS inregs;
  union REGS outregs;
  inregs.w.ax = 0x0008;
  inregs.w.cx = min;
  inregs.w.dx = max;
  int86(0x33, &inregs, &outregs);
#endif
  }

void Mouse::SetPosition(int X, int Y)  {

#ifdef DOS
  union REGS inregs;
  union REGS outregs;
  inregs.w.ax = 0x0004;
  inregs.w.cx = X*2;
  inregs.w.dx = Y;
  int86(0x33, &inregs, &outregs);
#endif

#ifdef X_WINDOWS
  X=Y; //UNUSED!!!
#endif

  Cursor->Erase();
  }

int Mouse::DrawingBox()  {
  return (ystart >= 0);
  }

