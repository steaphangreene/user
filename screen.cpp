#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <stdarg.h>

#include "screen.h"
#include "palette.h"
#include "graphic.h"
#include "sprite.h"
#include "speaker.h"
#include "list.h"
#include "resfile.h"
#include "keyboard.h"
#include "mouse.h"

#ifdef X_DGA
#include <X11/extensions/xf86dga.h>
#include <X11/extensions/xf86vmode.h>
#endif

extern Screen *__Da_Screen;
extern Speaker *__Da_Speaker;
extern Mouse *__Da_Mouse;

Screen::~Screen()  {
  Debug("User::Screen::~Screen() Begin");
  if(__Da_Screen != this) { __Da_Screen = NULL; Exit(1, "Phantom Screen!\n"); }
  __Da_Screen = NULL;

  switch(vtype)  {
    #ifdef X_WINDOWS
    case(VIDEO_XWINDOWS): {
      }break;
    #ifdef XF86_DGA
    case(VIDEO_XF86DGA): {
      XF86VidModeSwitchToMode(_Xdisplay, _Xscreen, &oldv);
      XF86DGADirectVideo(_Xdisplay, _Xscreen, 0);
      }break;
    #endif
    #endif
    }
  int ctr;
  for(ctr=0; ctr<256; ctr++)  {
    if(font[ctr] != NULL) delete font[ctr];
    font[ctr] = NULL;
    }
  if(TCursor != NULL) delete TCursor;
  TCursor = NULL;
  Debug("User::Screen::~Screen() End");
  }

Screen::Screen()  {
  Init();
  }

Screen::Screen(int x, int y)  {
  Init();
  SetSize(x, y);
  }

void Screen::Init()  {
  Debug("User::Screen::Init() 1000");
  if(__Da_Screen != NULL)  Exit(1, "One screen at a time!\n");
  __Da_Screen = this;

  int ctr;
  memset(sprites, 0, sizeof(Sprite*)*MAX_SPRITES);
  nextsprite = 0;
  frame.v = NULL;
  shown = 0;
  updated = 0;
  pal = new Palette;
  for(ctr=0; ctr<256; ctr++) font[ctr] = NULL;
  TCursor = NULL; tcx = 1; tcy = 1; AlignCursor();

  DetectVideoType();

  Debug("User::Screen::Init() 1001");
  switch(vtype)  {
    #ifdef X_WINDOWS
    case(VIDEO_XWINDOWS): {
      rowlen = xsize;
      Debug("User::Screen::Init() 1005");
      _Xscreen = DefaultScreen(_Xdisplay);
      depth = DefaultDepth(_Xdisplay, _Xscreen);
//      printf("Colordepth = %d\n", depth);
      if(depth == 24) depth = 32;

      Debug("User::Screen::Init() 1100");
      _Xshints = XAllocSizeHints();
      _Xshints->x = 0; _Xshints->y = 0;
      _Xshints->flags = PSize;
      _Xshints->flags |= USSize;
      _Xshints->flags |= PBaseSize;
      _Xshints->flags |= PMinSize;
      _Xshints->flags |= PMaxSize;
//      _Xshints->flags |= PPosition;
//      _Xshints->flags |= USPosition;
//      _Xshints->flags |= PWinGravity;
//      _Xshints->flags |= PResizeInc;

      Debug("User::Screen::Init() 1200");
      _Xwmhints = XAllocWMHints();
      _Xwmhints->flags = InputHint;
      _Xwmhints->input = True;
      name = "User Engine";
      }break;
    #ifdef XF86_DGA
    case(VIDEO_XF86DGA): {
      Debug("User::Screen::Init() 1300");
      _Xscreen = DefaultScreen(_Xdisplay);
      depth = DefaultDepth(_Xdisplay, _Xscreen);
      if(depth == 24) depth = 32;
      }break;
    #endif
    #endif
    }
  Debug("User::Screen::Init() 1400");
  }


int Screen::SetSize(int x, int y)  {
  xsize = x; ysize = y;
  int ctr, ctr2;
  if(depth==8)  {
    video_buffer.uc = new unsigned char[ysize*xsize];
    background_buffer.uc = new unsigned char[ysize*xsize];
    image = new mfmt[ysize];
    backg = new mfmt[ysize];
    for(ctr=0; ctr<ysize; ctr++)  {
      image[ctr].uc = &video_buffer.uc[xsize*ctr];
      backg[ctr].uc = &background_buffer.uc[xsize*ctr];
      for(ctr2=0; ctr2<xsize; ctr2++)  {
	image[ctr].uc[ctr2] = BlackPixel(_Xdisplay, 0);
	backg[ctr].uc[ctr2] = BlackPixel(_Xdisplay, 0);
	}
      }
    }
  else if(depth==32)  {
    video_buffer.ul = new unsigned long[ysize*xsize];
    background_buffer.ul = new unsigned long[ysize*xsize];
    image = new mfmt[ysize];
    backg = new mfmt[ysize];
    for(ctr=0; ctr<ysize; ctr++)  {
      image[ctr].ul = &video_buffer.ul[xsize*ctr];
      backg[ctr].ul = &background_buffer.ul[xsize*ctr];
      for(ctr2=0; ctr2<xsize; ctr2++)  {
	image[ctr].ul[ctr2] = BlackPixel(_Xdisplay, 0);
	backg[ctr].ul[ctr2] = BlackPixel(_Xdisplay, 0);
	}
      }
    }

  switch(vtype)  {
    #ifdef X_WINDOWS
    case(VIDEO_XWINDOWS): {
      _Xshints->width = xsize;
      _Xshints->height = ysize;
      _Xshints->base_height = _Xshints->height;
      _Xshints->base_width = _Xshints->width;
      _Xshints->min_height = _Xshints->height;
      _Xshints->min_width = _Xshints->width;
      _Xshints->max_height = _Xshints->height;
      _Xshints->max_width = _Xshints->width;

      if(depth == 8)  {
	_Xmap = XCreateColormap(_Xdisplay, DefaultRootWindow(_Xdisplay),
	DefaultVisual(_Xdisplay, _Xscreen), AllocAll);
	}
      else  {
	_Xmap = XCreateColormap(_Xdisplay, DefaultRootWindow(_Xdisplay),
	DefaultVisual(_Xdisplay, _Xscreen), AllocNone);
	}

      XSetWindowAttributes wina;
      wina.backing_store = WhenMapped;
      wina.save_under = True;
      wina.background_pixel = BlackPixel(_Xdisplay, 0);
      wina.colormap = _Xmap;
      wina.event_mask = KeyPressMask | KeyReleaseMask | ExposureMask
//	| SubstructureNotifyMask
	| PointerMotionMask
	| ButtonPressMask
	| ButtonReleaseMask;

      _Xwindow = XCreateWindow(_Xdisplay,
	DefaultRootWindow(_Xdisplay), _Xshints->x, _Xshints->y,
	_Xshints->width, _Xshints->height, 5,
	CopyFromParent, InputOutput, CopyFromParent,
	CWEventMask|CWColormap|CWSaveUnder|CWBackingStore|CWBackPixel, &wina);

      XSetStandardProperties (_Xdisplay, _Xwindow, name, name, None, NULL, 0,
	_Xshints);

      XSetWMHints (_Xdisplay, _Xwindow, _Xwmhints);

      _Xgc = XCreateGC(_Xdisplay, _Xwindow, 0, 0);

      if(depth==8)  {
	_Ximage = XCreateImage(_Xdisplay, None, 8, ZPixmap, 0,
		video_buffer.c, xsize, ysize, 8, 0);
	}
      else if(depth==32)  {
	_Ximage = XCreateImage(_Xdisplay, None,
		DefaultDepth(_Xdisplay, _Xscreen), ZPixmap, 0,
		video_buffer.c, xsize, ysize, 32, 0);
	}

      XPutImage(_Xdisplay, _Xwindow, _Xgc, _Ximage, 0, 0, 0, 0, xsize, ysize);

//      XSetForeground(_Xdisplay, _Xgc, WhitePixel(_Xdisplay, 0));
//      XDrawRectangle(_Xdisplay, _Xwindow, _Xgc, 10, 10, 10, 10);

      XFlush(_Xdisplay);
      while(XCheckMaskEvent(_Xdisplay, ExposureMask, &_Xevent));
      XMapRaised(_Xdisplay, _Xwindow);
      XFlush(_Xdisplay);
      while(!XCheckMaskEvent(_Xdisplay, ExposureMask, &_Xevent));
      }break;
    #ifdef XF86_DGA
    case(VIDEO_XF86DGA): {
      int ctr, n;
      XWarpPointer(_Xdisplay, None, DefaultRootWindow(_Xdisplay),0,0,0,0,0,0);

//      char pt[4];
//      printf("NumButtons = %d\n", XGetPointerMapping(_Xdisplay, pt, 4));
//      printf("%d %d %d %d\n", pt[0], pt[1], pt[2], pt[3]);

      if(depth == 8)  {
	_Xmap = XCreateColormap(_Xdisplay, DefaultRootWindow(_Xdisplay),
	DefaultVisual(_Xdisplay, _Xscreen), AllocAll);
	}
      XGrabKeyboard(_Xdisplay, DefaultRootWindow(_Xdisplay), True,
		GrabModeAsync, GrabModeAsync,  CurrentTime);
      XGrabPointer(_Xdisplay, DefaultRootWindow(_Xdisplay), True,
		PointerMotionMask | ButtonPressMask | ButtonReleaseMask,
		GrabModeAsync, GrabModeAsync, None,  None, CurrentTime);
      XF86DGADirectVideo(_Xdisplay, _Xscreen, XF86DGADirectGraphics
		| XF86DGADirectMouse | XF86DGADirectKeyb);

      XF86VidModeModeInfo curv, **allv;
      XF86VidModeGetAllModeLines(_Xdisplay, _Xscreen, &n, &allv);
      oldv = *(allv[0]);
      curv = oldv;
      for(ctr=0; ctr<n; ctr++)  {
        if(allv[ctr]->hdisplay >= xsize && allv[ctr]->vdisplay >= ysize
		&& allv[ctr]->hdisplay <= curv.hdisplay
		&& allv[ctr]->vdisplay <= curv.vdisplay)
          curv = *(allv[ctr]);
        }
      delete(allv);
      XF86VidModeSwitchToMode(_Xdisplay, _Xscreen, &curv);
      collen = curv.vdisplay;

      XF86DGASetViewPort(_Xdisplay, _Xscreen, 0, 0);
//      XF86DGADirectVideo(_Xdisplay, _Xscreen, 0);
//      memset(frame.v, 0, 1280*1024); // ************************************FIX!
//      XF86DGAInstallColormap(_Xdisplay, _Xscreen, _Xmap);
      if(depth == 8)  {
	for(ctr=0; ctr<collen; ctr++)
	  memset(frame.c+rowlen*ctr, 0, xsize);
	}
      else if(depth == 32)  {
	for(ctr=0; ctr<collen; ctr++)
	  memset(frame.ul+rowlen*ctr, 0, xsize*4);
	}
      else Exit(-1, "Unknown depth error (%d)\n", depth);
      }break;
    #endif
    #endif
    }
  return 1;
  }

void Screen::Hide()  {
  }

void Screen::Show()  {
  updated = 0;
  if(shown) return;
  shown = 1;
  Debug("User::Screen::Show() Entry");

  switch(vtype)  {
    #ifdef X_WINDOWS
    case(VIDEO_XWINDOWS): {
      if(depth == 8)  {
	int ctr;
	for(ctr=0; ctr<256; ctr++)  {
	  _Xpal[ctr].pixel = ctr;
	  _Xpal[ctr].red = ((unsigned short)pal->GetRedEntry(ctr))<<8;
	  _Xpal[ctr].green = ((unsigned short)pal->GetGreenEntry(ctr))<<8;
	  _Xpal[ctr].blue = ((unsigned short)pal->GetBlueEntry(ctr))<<8;
	  _Xpal[ctr].flags = DoRed|DoGreen|DoBlue;
	  }
	XStoreColors(_Xdisplay, _Xmap, _Xpal, 256);
	}
      RefreshFull();
      }break;
    #ifdef XF86_DGA
    case(VIDEO_XF86DGA): {
      if(depth == 8)  {
	int ctr;
	for(ctr=0; ctr<256; ctr++)  {
	  _Xpal[ctr].pixel = ctr;
	  _Xpal[ctr].red = ((unsigned short)pal->GetRedEntry(ctr))<<8;
	  _Xpal[ctr].green = ((unsigned short)pal->GetGreenEntry(ctr))<<8;
	  _Xpal[ctr].blue = ((unsigned short)pal->GetBlueEntry(ctr))<<8;
	  _Xpal[ctr].flags = DoRed|DoGreen|DoBlue;
	  }
	XStoreColors(_Xdisplay, _Xmap, _Xpal, 256);
	XF86DGAInstallColormap(_Xdisplay, _Xscreen, _Xmap);
	}
      RefreshFull();
      }break;
    #endif
    #endif
    }
  Debug("User::Screen::Show() End");
  }

void Screen::Refresh()  {
  if(!shown) return;
  RefreshFast();
  WaitForNextFrame();
  }

void Screen::RefreshFast()  {
  if(__Da_Speaker != NULL) __Da_Speaker->Update();
  if(__Da_Mouse != NULL) __Da_Mouse->Update();
  if((!shown) || updated) return;
  updated = 1;
  switch(vtype)  {
    #ifdef X_WINDOWS
    case(VIDEO_XWINDOWS): {
      Debug("User::Screen::RefreshFull() 1000");
      XPutImage(_Xdisplay, _Xwindow, _Xgc, _Ximage, 0, 0, 0, 0, xsize, ysize);
      }break;
    #ifdef XF86_DGA
    case(VIDEO_XF86DGA): {
      int ctry;
      if(depth == 8) {
	for(ctry=0; ctry<ysize; ctry++) {
	  memcpy(frame.uc + ctry*rowlen, image[ctry].uc, xsize);
	  }
	}
      else if(depth == 32) {
	for(ctry=0; ctry<ysize; ctry++) {
	  memcpy(frame.uc + ctry*rowlen, image[ctry].uc, xsize*4);
	  }
	}
      else Exit(-1, "Unknown depth error (%d)\n", depth);
      }break;
    #endif
    #endif
    }
  Debug("User::Screen::RefreshFull() 1100");
  }

void Screen::RefreshFull()  {
  if(__Da_Speaker != NULL) __Da_Speaker->Update();
  if(__Da_Mouse != NULL) __Da_Mouse->Update();
  if((!shown) || updated) return;
  updated = 1;
  switch(vtype)  {
    #ifdef X_WINDOWS
    case(VIDEO_XWINDOWS): {
      Debug("User::Screen::RefreshFull() 1000");
      XPutImage(_Xdisplay, _Xwindow, _Xgc, _Ximage, 0, 0, 0, 0, xsize, ysize);
      }break;
    #ifdef XF86_DGA
    case(VIDEO_XF86DGA): {
      int ctry;
      if(depth == 8) {
	for(ctry=0; ctry<ysize; ctry++) {
	  memcpy(frame.uc + ctry*rowlen, image[ctry].uc, xsize);
	  }
	}
      else if(depth == 32) {
	for(ctry=0; ctry<ysize; ctry++) {
	  memcpy(frame.uc + ctry*rowlen, image[ctry].uc, xsize*4);
	  }
	}
      else Exit(-1, "Unknown depth error (%d)\n", depth);
      }break;
    #endif
    #endif
    }
  WaitForNextFrame();
  Debug("User::Screen::RefreshFull() 1100");
  }

void Screen::Clear()  {
  updated = 0;
  int ctr, ctr2;
  if(depth==8)  {
    for(ctr=0; ctr<ysize; ctr++)  {
      for(ctr2=0; ctr2<xsize; ctr2++)  {
	((unsigned char **&)image)[ctr][ctr2] = BlackPixel(_Xdisplay, 0);
	((unsigned char **&)backg)[ctr][ctr2] = BlackPixel(_Xdisplay, 0);
	}
      }
    }
  else if(depth==32)  {
    for(ctr=0; ctr<ysize; ctr++)  {
      for(ctr2=0; ctr2<xsize; ctr2++)  {
	((unsigned long **&)image)[ctr][ctr2] = BlackPixel(_Xdisplay, 0);
	((unsigned long **&)backg)[ctr][ctr2] = BlackPixel(_Xdisplay, 0);
	}
      }
    }
  }

void Screen::FadeIn()  {
  Show();
  }

void Screen::FadeOut()  {
  }

void Screen::FadeIn(int n)  {
  n=n;
  Show();
  }

void Screen::FadeOut(int n)  {
  n=n;
  }

void Screen::SetPoint(int x, int y, int c)  {
  updated = 0;
  if(depth==8)  {
    ((unsigned char **&)image)[y][x] = c;
    ((unsigned char **&)backg)[y][x] = c;
    }
  else if(depth==32)  {
    ((unsigned long **&)image)[y][x]
	= (pal->GetRedEntry(c)<<16)
	+ (pal->GetGreenEntry(c)<<8)
	+ (pal->GetBlueEntry(c));
    ((unsigned long **&)backg)[y][x]
	= (pal->GetRedEntry(c)<<16)
	+ (pal->GetGreenEntry(c)<<8)
	+ (pal->GetBlueEntry(c));
    }
  }

void Screen::SetPoint(int x, int y, int r, int g, int b)  {
  }

void Screen::FullScreenBMP(const char *fn) {
  Graphic g((char*)fn);
  FullScreenGraphic(g);
  }

void Screen::DrawTransparentGraphicFG(Graphic &g, int x, int y)  {
  Debug("User:Screen:DrawTransparentGraphicFG Begin");
  updated = 0;
  if(g.depth != depth)  {
    Graphic *g2 = new Graphic(g);
    g2->DepthConvert(depth, *pal);
    DrawTransparentGraphicFG(*g2, x, y);
    delete g2;
    return;
    }
  Debug("User:Screen:DrawTransparentGraphicFG Middle");
  int ctrx, ctry;
  if(depth == 8)  {
    Debug("User:Screen:DrawTransparentGraphicFG Depth 8");
    for(ctry=0; ctry<((ysize-y)<?g.ysize); ctry++)  {
      for(ctrx=0; ctrx<((xsize-x)<?g.xsize); ctrx++)  {
	if(g.image[ctry][ctrx] != g.tcolor)  {
	  ((unsigned char **)image)[ctry+y][ctrx+x] = g.image[ctry][ctrx];
	  }
	}
      }
    }
  else if(depth == 24)  {
    Debug("User:Screen:DrawTransparentGraphicFG Depth 24");
    for(ctry=0; ctry<((ysize-y)<?g.ysize); ctry++)  {
      for(ctrx=0; ctrx<((xsize-x)<?(g.xsize)); ctrx++)  {
	if(g.image[ctry][ctrx*3] != (g.tcolor&255)
		|| g.image[ctry][ctrx*3+1] != ((g.tcolor>>8)&255)
		|| g.image[ctry][ctrx*3+2] != ((g.tcolor>>16)&255))  {
	  image[ctry+y].ul[ctrx+x]
		= (g.image[ctry][ctrx*3]<<16)
		+ (g.image[ctry][ctrx*3+1]<<8)
		+ g.image[ctry][ctrx*3+2];
	  }
	}
      }
    }
  else if(depth == 32)  {
    Debug("User:Screen:DrawTransparentGraphicFG Depth 32");
    for(ctry=0; ctry<((ysize-y)<?g.ysize); ctry++)  {
      for(ctrx=0; ctrx<((xsize-x)<?(g.xsize)); ctrx++)  {
	if(g.image[ctry][(ctrx<<2)+3] != g.tcolor)  {
	  image[ctry+y].ul[ctrx+x] = ((long*)g.image[ctry])[ctrx];
	  }
	}
      }
    }
  else Exit(-1, "Unknown depth error (%d)\n", depth);
  Debug("User:Screen:TransparentGraphicFG End");
  }

void Screen::DrawGraphicFG(Graphic &g, int x, int y)  {
  updated = 0;
  if(g.depth != depth)  {
    Graphic *g2 = new Graphic(g);
    g2->DepthConvert(depth, *pal);
    DrawGraphicFG(*g2, x, y);
    delete g2;
    return;
    }
  int ctrx, ctry;
  if(depth == 8)  {
    for(ctry=0; ctry<((ysize-y)<?g.ysize); ctry++)  {
      for(ctrx=0; ctrx<((xsize-x)<?g.xsize); ctrx++)  {
	((unsigned char **)image)[ctry+y][ctrx+x] = g.image[ctry][ctrx];
	}
      }
    }
  else if(depth == 24)  {
    for(ctry=0; ctry<((ysize-y)<?g.ysize); ctry++)  {
      for(ctrx=0; ctrx<((xsize-x)<?(g.xsize)); ctrx++)  {
	((unsigned long **)image)[ctry+y][ctrx+x]
		= (g.image[ctry][ctrx*3]<<16)
		+ (g.image[ctry][ctrx*3+1]<<8)
		+ g.image[ctry][ctrx*3+2];
	}
      }
    }
  else if(depth == 32)  {
    for(ctry=0; ctry<((ysize-y)<?g.ysize); ctry++)  {
      for(ctrx=0; ctrx<((xsize-x)<?(g.xsize)); ctrx++)  {
	((unsigned long **)image)[ctry+y][ctrx+x] =
		((long*)g.image[ctry])[ctrx];
	}
      }
    }
  else Exit(-1, "Unknown depth error (%d)\n", depth);
  }

void Screen::FullScreenGraphicFG(Graphic &g) {
  updated = 0;
  if(g.depth != depth)  {
    Graphic *g2 = new Graphic(g);
    g2->DepthConvert(depth, *pal);
    FullScreenGraphicFG(*g2);
    delete g2;
    return;
    }
  int ctrx, ctry;
  if(depth == 8)  {
    for(ctry=0; ctry<(ysize<?g.ysize); ctry++)  {
      for(ctrx=0; ctrx<(xsize<?g.xsize); ctrx++)  {
	((unsigned char **)image)[ctry][ctrx] = g.image[ctry][ctrx];
	}
      }
    }
  else if(depth == 24)  {
    for(ctry=0; ctry<(ysize<?g.ysize); ctry++)  {
      for(ctrx=0; ctrx<(xsize<?(g.xsize)); ctrx++)  {
	((unsigned long **)image)[ctry][ctrx]
		= (g.image[ctry][ctrx*3]<<16)
		+ (g.image[ctry][ctrx*3+1]<<8)
		+ g.image[ctry][ctrx*3+2];
	}
      }
    }
  else if(depth == 32)  {
    for(ctry=0; ctry<(ysize<?g.ysize); ctry++)  {
      for(ctrx=0; ctrx<(xsize<?(g.xsize)); ctrx++)  {
	((unsigned long **)image)[ctry][ctrx] = ((long*)g.image[ctry])[ctrx];
	}
      }
    }
  else Exit(-1, "Unknown depth error (%d)\n", depth);
  }

void Screen::DrawTransparentGraphic(Graphic &g, int x, int y)  {
  updated = 0;
  if(g.depth != depth)  {
    Graphic *g2 = new Graphic(g);
    g2->DepthConvert(depth, *pal);
    DrawTransparentGraphic(*g2, x, y);
    delete g2;
    return;
    }
  int ctrx, ctry;
  if(depth == 8)  {
    for(ctry=0; ctry<((ysize-y)<?g.ysize); ctry++)  {
      for(ctrx=0; ctrx<((xsize-x)<?g.xsize); ctrx++)  {
	if(g.image[ctry][ctrx] != g.tcolor)  {
	  ((unsigned char **)image)[ctry+y][ctrx+x] = g.image[ctry][ctrx];
	  ((unsigned char **)backg)[ctry+y][ctrx+x] = g.image[ctry][ctrx];
	  }
	}
      }
    }
  else if(depth == 24)  {
    for(ctry=0; ctry<((ysize-y)<?g.ysize); ctry++)  {
      for(ctrx=0; ctrx<((xsize-x)<?(g.xsize)); ctrx++)  {
	if(g.image[ctry][ctrx*3] != (g.tcolor&255)
		|| g.image[ctry][ctrx*3+1] != ((g.tcolor>>8)&255)
		|| g.image[ctry][ctrx*3+2] != ((g.tcolor>>16)&255))  {
	  ((unsigned long **)image)[ctry+y][ctrx+x]
		= (g.image[ctry][ctrx*3]<<16)
		+ (g.image[ctry][ctrx*3+1]<<8)
		+ g.image[ctry][ctrx*3+2];
	  ((unsigned long **)backg)[ctry+y][ctrx+x]
		= (g.image[ctry][ctrx*3]<<16)
		+ (g.image[ctry][ctrx*3+1]<<8)
		+ g.image[ctry][ctrx*3+2];
	  }
	}
      }
    }
  else Exit(-1, "Unknown depth error (%d)\n", depth);
  }

void Screen::DrawGraphic(Graphic &g, int x, int y)  {
  if(x>=xsize||y>=ysize||x<=-((int)g.xsize)||y<=-((int)g.ysize)) return;
  updated = 0;
  if(g.depth != depth)  {
    Graphic *g2 = new Graphic(g);
    g2->DepthConvert(depth, *pal);
    DrawGraphic(*g2, x, y);
    delete g2;
    return;
    }
  int ctrx, ctry;
  if(depth == 8)  {
    for(ctry=0; ctry<((ysize-y)<?g.ysize); ctry++)  {
      for(ctrx=0; ctrx<((xsize-x)<?g.xsize); ctrx++)  {
	((unsigned char **)image)[ctry+y][ctrx+x] = g.image[ctry][ctrx];
	((unsigned char **)backg)[ctry+y][ctrx+x] = g.image[ctry][ctrx];
	}
      }
    }
  else if(depth == 24)  {
    for(ctry=0; ctry<((ysize-y)<?g.ysize); ctry++)  {
      for(ctrx=0; ctrx<((xsize-x)<?(g.xsize)); ctrx++)  {
	((unsigned long **)image)[ctry+y][ctrx+x]
		= (g.image[ctry][ctrx*3]<<16)
		+ (g.image[ctry][ctrx*3+1]<<8)
		+ g.image[ctry][ctrx*3+2];
	((unsigned long **)backg)[ctry+y][ctrx+x]
		= (g.image[ctry][ctrx*3]<<16)
		+ (g.image[ctry][ctrx*3+1]<<8)
		+ g.image[ctry][ctrx*3+2];
	}
      }
    }
  else if(depth == 32)  {
    for(ctry=0; ctry<((ysize-y)<?g.ysize); ctry++)  {
      for(ctrx=0; ctrx<((xsize-x)<?(g.xsize)); ctrx++)  {
	((unsigned long**)image)[ctry+y][ctrx+x] =((long*)g.image[ctry])[ctrx];
	((unsigned long**)backg)[ctry+y][ctrx+x] =((long*)g.image[ctry])[ctrx];
	}
      }
    }
  else Exit(-1, "Unknown depth error (%d)\n", depth);
  }

void Screen::FullScreenGraphic(Graphic &g) {
  updated = 0;
  if(g.depth != depth)  {
    Graphic *g2 = new Graphic(g);
    g2->DepthConvert(depth, *pal);
    FullScreenGraphic(*g2);
    delete g2;
    return;
    }
  int ctrx, ctry;
  if(depth == 8)  {
    for(ctry=0; ctry<(ysize<?g.ysize); ctry++)  {
      for(ctrx=0; ctrx<(xsize<?g.xsize); ctrx++)  {
	((unsigned char **)image)[ctry][ctrx] = g.image[ctry][ctrx];
	((unsigned char **)backg)[ctry][ctrx] = g.image[ctry][ctrx];
	}
      }
    }
  else if(depth == 24)  {
    for(ctry=0; ctry<(ysize<?g.ysize); ctry++)  {
      for(ctrx=0; ctrx<(xsize<?(g.xsize)); ctrx++)  {
	((unsigned long **)image)[ctry][ctrx]
		= (g.image[ctry][ctrx*3]<<16)
		+ (g.image[ctry][ctrx*3+1]<<8)
		+ g.image[ctry][ctrx*3+2];
	((unsigned long **)backg)[ctry][ctrx]
		= (g.image[ctry][ctrx*3]<<16)
		+ (g.image[ctry][ctrx*3+1]<<8)
		+ g.image[ctry][ctrx*3+2];
	}
      }
    }
  else if(depth == 32)  {
    for(ctry=0; ctry<(ysize<?g.ysize); ctry++)  {
      for(ctrx=0; ctrx<(xsize<?(g.xsize)); ctrx++)  {
	((unsigned long **)image)[ctry][ctrx] = ((long*)g.image[ctry])[ctrx];
	((unsigned long **)backg)[ctry][ctrx] = ((long*)g.image[ctry])[ctrx];
	}
      }
    }
  else Exit(-1, "Unknown depth error (%d)\n", depth);
  }

void Screen::SetPaletteEntry(int c, int r, int g, int b) {
  updated = 0;
  pal->SetPaletteEntry(c, r, g, b);
  if(shown)  {
    switch(vtype)  {
      #ifdef X_WINDOWS
      case(VIDEO_XWINDOWS): {
	if(depth == 8)  {
	  _Xpal[c].pixel = c;
	  _Xpal[c].red = r<<8;
	  _Xpal[c].green = g<<8;
	  _Xpal[c].blue = b<<8;
	  _Xpal[c].flags = DoRed|DoGreen|DoBlue;
	  XStoreColor(_Xdisplay, _Xmap, &_Xpal[c]);
	  }
	else  {
//	  RefreshFull();
	  }
	}break;
      #ifdef XF86_DGA
      case(VIDEO_XF86DGA): {
	if(depth == 8)  {
	  _Xpal[c].pixel = c;
	  _Xpal[c].red = r<<8;
	  _Xpal[c].green = g<<8;
	  _Xpal[c].blue = b<<8;
	  _Xpal[c].flags = DoRed|DoGreen|DoBlue;
	  XStoreColor(_Xdisplay, _Xmap, &_Xpal[c]);
	  XF86DGAInstallColormap(_Xdisplay, _Xscreen, _Xmap);
	  }
	else {
//	  RefreshFull();
	  }
	}break;
      #endif
      #endif
      }
    }
  }

void Screen::GetPalette(const char *fn) {
  pal->GetPalette(fn);
  }

int Screen::RegisterSprite(Sprite *s) {
  sprites[nextsprite] = s;
  return nextsprite++;
  }

void Screen::RemoveSprite(int n, Sprite *s) {
  if(sprites[n] == NULL) Exit(-1, "Tried to remove non-existant sprite!\n");
  if(sprites[n] != s) Exit(-1, "Tried to remove other sprite!\n");
  sprites[n] = NULL;
  }

IntList Screen::CollideRectangle(int s, int x, int y, int xs, int ys)  {
  int ctr; IntList ret;
  for(ctr=0; ctr<nextsprite; ctr++)  {
    if(ctr!=s && sprites[ctr] != NULL && sprites[ctr]->collisions
	&& sprites[ctr]->drawn && sprites[ctr]->image != NULL)  {
      if(x < (sprites[ctr]->xpos + sprites[ctr]->image->xsize)
	    && y < (sprites[ctr]->ypos + sprites[ctr]->image->ysize)
	    && (y+ys) > sprites[ctr]->ypos
	    && (x+xs) > sprites[ctr]->xpos)  {
	if(sprites[s]->Hits(sprites[ctr])) ret += ctr;
	}      
      }
    }
  return ret;
  }

void Screen::MakeFriendly(Sprite *s)  {
  MakeFriendly(s->image);
  }

void Screen::MakeFriendly(Graphic *g)  {
  if(g->depth != depth) g->DepthConvert(depth, *pal);
  }

void Screen::RestoreRectangle(int x, int y, int xs, int ys)  {
  if(x>xsize || y>ysize || x+xs<0 || y+ys<0) return;
  if(x<0) { xs += x; x=0; }
  if(y<0) { ys += y; y=0; }
  if(x+xs > xsize) xs += ((xsize)-(x+xs));
  if(y+ys > ysize) ys += ((ysize)-(y+ys));
//  printf("(%d,%d) D=%d %dx%d\n", x, y, depth, xs, ys);
  updated = 0;
  int ctrx, ctry;
  if(depth == 8)  {
    for(ctry=y; ctry<y+ys; ctry++)  {
      for(ctrx=x; ctrx<x+xs; ctrx++)  {
	((unsigned char **)image)[ctry][ctrx] = 
				((unsigned char **)backg)[ctry][ctrx];
	}
      }
    }
  else if(depth == 32)  {
    for(ctry=y; ctry<y+ys; ctry++)  {
      for(ctrx=x; ctrx<x+xs; ctrx++)  {
	((unsigned long **)image)[ctry][ctrx] = 
				((unsigned long **)backg)[ctry][ctrx];
	}
      }
    }
  else Exit(-1, "Unknown depth error (%d)\n", depth);
  int ctr; Sprite **spp = spbuf;
  for(ctr=0; ctr<MAX_SPRITES; ctr++)  {
    if(sprites[ctr] != NULL) { *spp = sprites[ctr]; ++spp; }
    }
  *spp = NULL;
  qsort(spbuf,(spp-sprites)/sizeof(Sprite*), sizeof(Sprite*), 
	(int (*)(const void *, const void *))&CompareSprites);
  }

void Screen::DetectVideoType()  {
#ifdef X_WINDOWS
  _Xdisplay = XOpenDisplay("");
  if(_Xdisplay != NULL)  {
    vtype = VIDEO_XWINDOWS;
#ifdef XF86_DGA
    if(!access("/dev/mem", W_OK))  {
      int M, m;
      if(XF86DGAQueryVersion(_Xdisplay, &M, &m))  {
	fprintf(stderr, "Got DGA version %d.%d\n", M, m);
	int Xbank, Xmem, Flags;
	XF86DGAQueryDirectVideo(_Xdisplay, _Xscreen, &Flags);
	if(!(Flags & XF86DGADirectPresent)) Exit(0, "Failed DGA Query\n");
	XF86DGAGetVideo(_Xdisplay, _Xscreen,
		&(frame.c), &rowlen, &Xbank, &Xmem);
	fprintf(stderr, "Width %d, Bank %d, Mem %d\n",  rowlen, Xbank, Xmem);
	}
      else { fprintf(stderr, "No DGA support.\n"); return; }
      if(XF86VidModeQueryVersion(_Xdisplay, &M, &m))  {
	fprintf(stderr, "Got VidMode version %d.%d\n", M, m);
	}
      else { fprintf(stderr, "No VidMode support.\n"); return; }
      vtype = VIDEO_XF86DGA;
      }
#endif
    }
#endif
//  printf("Video type = %d\n", vtype);
  if(vtype != VIDEO_NONE) return;
  Exit(-1, "No video capability detected!\n");
  }

void Screen::SetFrameRate(int rt)  {
  if(rt <= 0) { framedelay = 0; }
  else  {
    framedelay = 1000000 / rt;
    timeval tv;
    gettimeofday(&tv, NULL);
    lasttime = tv.tv_sec;
    ulasttime = tv.tv_usec;
    }
  }

void Screen::WaitForNextFrame()  {
  if(framedelay <= 0) return;
  long dest, udest;
  timeval tv;
  dest = 0; udest = framedelay;
  dest += lasttime; udest += ulasttime;
  if(udest > 1000000) { udest -= 1000000; dest += 1; }
  while(tv.tv_sec<dest || (tv.tv_sec==dest && tv.tv_usec<udest)) {
    __Da_Screen->RefreshFast(); gettimeofday(&tv, NULL);
    }
  lasttime = dest;
  ulasttime = udest;
  }

int Screen::SetFont(const char *fn)  {
  ResFile frf(fn, USER_ARGV[0]);
  int ctr;
  for(ctr=0; ctr<256; ctr++)  if(font[ctr] != NULL) delete font[ctr];
  for(ctr=0; ctr<256; ctr++)  font[ctr] = frf.GetGraphic();
  AlignCursor();
  return 0;
  }

int Screen::SetFont(const char *fn, const char *com)  {
  ResFile frf(fn, com);
  if(strcmp(com, USER_ARGV[0])) ResFile frf(fn, USER_ARGV[0]);
  int ctr;
  if(font[' '] != NULL)  {
    for(ctr=0; ctr<256; ctr++)  if(font[ctr] != NULL) delete font[ctr];
    }
  for(ctr=0; ctr<256; ctr++)  {
    font[ctr] = (Graphic *)frf.Get();
    }
  return 0;
  }

void Screen::AlignCursor()  {
  if(tcx < 1) tcx = 1;
  else if(tcx >= xsize) tcx = xsize-1;
  if(tcy < 1) tcy = 1;
  else if(tcy >= ysize) tcy = ysize-1;
  if(font[' '] != NULL)  {
    tcy += ((font[' ']->ysize+2)/2);
    tcy /= (font[' ']->ysize+2);
    tcy *= (font[' ']->ysize+2);
    tcy ++;
    }
  }

void Screen::SetCursor(Graphic &g)  {
  TCursor = new Sprite(g);
  TCursor->Move(tcx, tcy);
  }

int Screen::Printf(long cb, long cf, const char *text, ...)  {
  Debug("User::Screen::Printf(...) Begin");
  int ret;
  va_list stuff;
  va_start(stuff, text);
  char buf[4096];
  bzero(buf, 4096);
  vsprintf(buf, text, stuff);
  ret = Print(cb, cf, buf);
  va_end(stuff);
  Debug("User::Screen::Printf(...) End");
  return ret;
  }

int Screen::Print(long cb, long cf, const char *text)  {
  Debug("User::Screen::Print(...) Begin");
  if(font[' '] == NULL)  Exit(-1, "Must Screen.SetFont before Screen.Print!\n");
  unsigned char *ind = (unsigned char *)text;
//  printf("%s\n", text);
  for(;(*ind) != 0; ind++)  {
//    printf("%d,%d:%d: ", TXPos(), TYPos(), *ind); fflush(stdout);
    if((*ind) == (unsigned char)'\t')  {
      int tabstops = ysize / 10;
      tcx+=tabstops;
      tcx-=(tcx%tabstops);
      AlignCursor();
      }
    else if((*ind) == (unsigned char)KEY_LEFT)  {
      tcx-=font[' ']->xsize;
      AlignCursor();
      }
    else if((*ind) == (unsigned char)KEY_RIGHT)  {
      tcx+=font[' ']->xsize;
      AlignCursor();
      }
    else if((*ind) == (unsigned char)KEY_UP)  {
      tcy-=(font[' ']->ysize+2);
      AlignCursor();
      }
    else if((*ind) == (unsigned char)KEY_DOWN)  {
      tcy+=(font[' ']->ysize+2);
      AlignCursor();
      }
    else if((*ind) == (unsigned char)'\n')  {
      tcx=1;
      tcy+=(font[' ']->ysize+2);
      AlignCursor();
      }
    else if((*ind) == (unsigned char)'\r')  {
      tcx=1;
      AlignCursor();
      }
    else if(font[*ind] != NULL)  {
      if(((int)tcx + (int)font[*ind]->xsize + (int)font[*ind]->xcenter)
                >= xsize)  {
        tcx=1;
        tcy+=(font[' ']->ysize+2);
        AlignCursor();
        }
      int ctrx, ctry;
      Graphic let(*font[*ind]);
      for(ctrx=0; ctrx<(int)let.xsize; ctrx++)  {
        for(ctry=0; ctry<(int)let.ysize; ctry++)  {
          if(let.image[ctry][ctrx] == 0) let.image[ctry][ctrx] = cb;
          else let.image[ctry][ctrx] = cf;
          }
        }
      DrawGraphic(let, tcx-let.xcenter, tcy-let.ycenter);
      tcx+=font[*ind]->xsize+1;
      AlignCursor();
      }
//    printf("<\n");
    }
  if(TCursor != NULL)  TCursor->Move(tcx, tcy);
  Debug("User::Screen::Print(...) End");
  return tcx;
  }

int Screen::CompareSprites(Sprite *s1, Sprite *s2) {
  if(s1->priority != s2->priority)  return (s1->priority)-(s2->priority);
  else return (s1->snum)-(s2->snum);
  }

void Screen::TGotoXY(int x, int y) {
  tcx=x; tcy=y; AlignCursor();
  }
