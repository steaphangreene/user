#include "config.h"

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
#include "input.h"

#ifdef X_DGA
#include <X11/extensions/xf86dga.h>
#include <X11/extensions/xf86vmode.h>
#endif

#ifdef DOS
#include <dos.h>
#include <go32.h>
#include "vesa.h"
#endif

extern Screen *__Da_Screen;
extern Speaker *__Da_Speaker;
extern Mouse *__Da_Mouse;
extern InputQueue *__Da_InputQueue;

Screen::~Screen()  {
//  Debug("User::Screen::~Screen() Begin");
  if(__Da_Screen != this) { __Da_Screen = NULL; Exit(1, "Phantom Screen!\n"); }
  __Da_Screen = NULL;

  switch(vtype)  {
    #ifdef DOS
    case(VIDEO_VESA):
    case(VIDEO_VBE2):
    case(VIDEO_VBE2L):
    case(VIDEO_DOS): {
      union REGS inregs;
      union REGS outregs;
      inregs.w.ax = 0x0003;
      int386(0x10, &inregs, &outregs);
      }break;
    #endif

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
//  Debug("User::Screen::~Screen() End");
  }

Screen::Screen(char *n)  {
  name=n;
  Init();
  }

Screen::Screen(int x, int y, char *n)  {
  name=n;
  Init();
  SetSize(x, y);
  }

void Screen::Init()  {
  Debug("User::Screen::Init() 1000");
  if(__Da_Screen != NULL)  Exit(1, "One screen at a time!\n");
  __Da_Screen = this;

  int ctr;
  memset(sprites, 0, sizeof(Sprite*)*MAX_SPRITES);
  memset(rxs, -1, sizeof(rxs));
  memset(rys, -1, sizeof(rys));
  memset(rxe, -1, sizeof(rxe));
  memset(rye, -1, sizeof(rye));
  memset(pxs, -1, sizeof(pxs));
  memset(pys, -1, sizeof(pys));
  memset(pxe, -1, sizeof(pxe));
  memset(pye, -1, sizeof(pye));
  nextsprite = 0;
  huges = NULL;
  bins = NULL;
  lbins = NULL;
  frame.v = NULL;
  shown = 0;
  fullscreen = 0;
  xlong = 0;
  ylong = 0;
#ifdef DOS
  vbe2_info = NULL;
  vbe2_bank = NULL;
  vesamode = NULL;
  vesax = NULL;
  vesay = NULL;
  vesad = NULL;
  curbank = -1;
#endif
  pal = new Palette;
  for(ctr=0; ctr<256; ctr++) font[ctr] = NULL;
  TCursor = NULL; tcx = 1; tcy = 1; AlignCursor();

  DetectVideoType();

  Debug("User::Screen::Init() 1001");
  switch(vtype)  {
    #ifdef DOS
    case(VIDEO_DOS): {
      depth = 8;
      }
    #endif

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
  appdepth = depth;
  Debug("User::Screen::Init() 1400");
  }


int Screen::SetSize(int x, int y)  {
  Debug("User::Screen::SetSize() Begin"); 
  int ctr, ctr2;
  if(bins!=NULL) {
    for(ctr=0; ctr<xsize>>BIN_FACTOR; ++ctr) {
      delete bins[ctr];
      }
    delete bins;
    }
  if(lbins!=NULL) {
    for(ctr=0; ctr<xsize>>LARGE_BIN_FACTOR; ++ctr) {
      delete lbins[ctr];
      }
    delete lbins;
    }
  xsize = x; ysize = y;
  pxs[0] = 0; pys[0] = 0;
  pxe[0] = x; pye[0] = y;
  bins = new (Sprite**)[xsize>>BIN_FACTOR];
  if(!bins) Exit(1, "Insufficient memory!\n");
  for(ctr=0; ctr<xsize>>BIN_FACTOR; ++ctr) {
    bins[ctr] = new (Sprite*)[ysize>>BIN_FACTOR];
    if(!bins[ctr]) Exit(1, "Insufficient memory!\n");
    for(ctr2=0; ctr2<ysize>>BIN_FACTOR; ++ctr2) {
      bins[ctr][ctr2] = NULL;
      }
    }
  lbins = new (Sprite**)[xsize>>LARGE_BIN_FACTOR];
  if(!lbins) Exit(1, "Insufficient memory!\n");
  for(ctr=0; ctr<xsize>>LARGE_BIN_FACTOR; ++ctr) {
    lbins[ctr] = new (Sprite*)[ysize>>LARGE_BIN_FACTOR];
    if(!lbins[ctr]) Exit(1, "Insufficient memory!\n");
    for(ctr2=0; ctr2<ysize>>LARGE_BIN_FACTOR; ++ctr2) {
      lbins[ctr][ctr2] = NULL;
      }
    }

  if(depth==8)  {
    video_buffer.uc = new unsigned char[ysize*xsize];
    background_buffer.uc = new unsigned char[ysize*xsize];
    image = new mfmt[ysize];
    backg = new mfmt[ysize];
    for(ctr=0; ctr<ysize; ctr++)  {
      image[ctr].uc = &video_buffer.uc[xsize*ctr];
      backg[ctr].uc = &background_buffer.uc[xsize*ctr];
      for(ctr2=0; ctr2<xsize; ctr2++)  {
//	image[ctr].uc[ctr2] = BlackPixel(_Xdisplay, 0);
//	backg[ctr].uc[ctr2] = BlackPixel(_Xdisplay, 0);
	image[ctr].uc[ctr2] = 0;
	backg[ctr].uc[ctr2] = 0;
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
//	image[ctr].ul[ctr2] = BlackPixel(_Xdisplay, 0);
//	backg[ctr].ul[ctr2] = BlackPixel(_Xdisplay, 0);
	image[ctr].ul[ctr2] = 0;
	backg[ctr].ul[ctr2] = 0;
	}
      }
    }
  else if(depth==16)  {
    video_buffer.us = new unsigned short[ysize*xsize];
    background_buffer.us = new unsigned short[ysize*xsize];
    image = new mfmt[ysize];
    backg = new mfmt[ysize];
    for(ctr=0; ctr<ysize; ctr++)  {
      image[ctr].us = &video_buffer.us[xsize*ctr];
      backg[ctr].us = &background_buffer.us[xsize*ctr];
      for(ctr2=0; ctr2<xsize; ctr2++)  {
//	image[ctr].us[ctr2] = BlackPixel(_Xdisplay, 0);
//	backg[ctr].us[ctr2] = BlackPixel(_Xdisplay, 0);
	image[ctr].us[ctr2] = 0;
	backg[ctr].us[ctr2] = 0;
	}
      }
    }
  else Exit(-1, "Unknown depth error (%d) in %s\n", depth, __PRETTY_FUNCTION__);

  switch(vtype)  {
    #ifdef DOS
    case(VIDEO_DOS): {
      if(xsize <= 320 && ysize <= 200)  {
	union REGS inregs;
	union REGS outregs;
	inregs.w.ax = 0x0013;
	int386(0x10, &inregs, &outregs);
	frame.L = 0xA0000;
	rowlen = 320;
	}
      else  {
	__dpmi_regs regs;
	char tmps[65536], *sptr=tmps;
	if(vesamode == NULL)  {
	  regs.x.ax = 0x4F00;
	  regs.x.di = __tb & 0x0F;
	  regs.x.es = (__tb >> 4) &0xFFFF;
	  __dpmi_int(0x10, &regs);
	  if(regs.x.ax != 0x004F) Exit(-1, "VESA init failed!\n");
	  dosmemget(__tb, sizeof(VESAInfo), &vinfo);
	  if(strncmp(vinfo.VESASignature,"VESA",4))
		Exit(-1, "No VESA detected!\n");
	  short vmode = 0;
	  int ctr;
	  Debug("User::Screen::SetSize() Before Get Modes"); 
	  unsigned long vlist=(((unsigned long)vinfo.VideoModePtr)>>12) & 0xF0000;
	  vlist |= ((unsigned long)vinfo.VideoModePtr & 0xFFFF);
	  for(numvesamodes=0; vmode != -1; numvesamodes++) {
	    dosmemget((numvesamodes<<1)+vlist, 2, &vmode);
	    }
	  --numvesamodes;
	  Debug("User::Screen::SetSize() After Get Modes"); 
	  vesamode = new short[numvesamodes];
	  vesax = new long[numvesamodes];
	  vesay = new long[numvesamodes];
	  vesad = new long[numvesamodes];
	  dosmemget(vlist, 2*numvesamodes, vesamode);
	  Debug("User::Screen::SetSize() After Get Mode List"); 
	  for(ctr=0; ctr<numvesamodes; ctr++) {
	    regs.x.ax = 0x4F01;
	    regs.x.cx = vesamode[ctr];
	    regs.x.di = __tb & 0x0F;
	    regs.x.es = (__tb >> 4) & 0xFFFF;
	    __dpmi_int(0x10, &regs);
	    dosmemget(__tb, sizeof(VESAModeInfo), &vminfo);

	    vesax[ctr] = vminfo.XResolution;
	    vesay[ctr] = vminfo.YResolution;
	    vesad[ctr] = vminfo.BitsPerPixel;

	    sptr += sprintf(sptr, "%X:%ldx%ldx%ld\t", vesamode[ctr],
		vesax[ctr], vesay[ctr], vesad[ctr]);
	    }
	  sptr += sprintf(sptr, "\n");
	  Debug("User::Screen::SetSize() After Get Mode Info"); 
	  }
	int ctr;
	int mode = -1;
	for(ctr=0; ctr<numvesamodes; ctr++)  {
	  if(vesad[ctr] == depth && vesax[ctr] >= xsize && vesay[ctr] >= ysize
		&& (mode==-1
		|| (vesax[ctr] <= vesax[mode] && vesay[ctr] <= vesay[mode])))
	    mode=ctr;
	  }
	if(mode==-1) Exit(-1, "No VESA %d-bit mode compatible with %dx%d!\n",
		depth, xsize, ysize);
	regs.x.ax = 0x4F01;
	regs.x.cx = vesamode[mode];
	regs.x.di = __tb & 0x0F;
	regs.x.es = (__tb >> 4) & 0xFFFF;
	__dpmi_int(0x10, &regs);
	dosmemget(__tb, sizeof(VESAModeInfo), &vminfo);

	regs.x.ax = 0x4F02;
	regs.x.bx = vesamode[mode];
	__dpmi_int(0x10, &regs);
	if(regs.x.ax != 0x4F)
		Exit(-1, "VESA Mode 0x%X Failed!\n", vesamode[mode]);

	regs.x.ax = 0x4F06;
	regs.x.bx = 0x0001;
	__dpmi_int(0x10, &regs);
	frame.L = 0xA0000;
	rowlen = regs.x.cx;
	vtype=VIDEO_VESA;

	if(vinfo.VESAVersion >= 0x200)  {
	  vtype=VIDEO_VBE2;
	  regs.x.ax = 0x4F0A;
	  regs.x.bx = 0x0000;
	  __dpmi_int(0x10, &regs);
	  if(regs.x.ax!=0x004F) Exit(-1,"VBE2: Error getting PM interface!\n");
	  vbe2_info = (VBE2_PM_Info *) new unsigned char[regs.x.cx];

	  _go32_dpmi_lock_data(vbe2_info, regs.x.cx);
	  dosmemget((regs.x.es*16)+regs.x.di, regs.x.cx, vbe2_info);
	  (void*&)vbe2_bank = (void*)((char *)vbe2_info + vbe2_info->setWindow);

	  if(vminfo.ModeAttributes & 0x08) {
	    vtype=VIDEO_VBE2L;
	    regs.x.ax = 0x4F02;
	    regs.x.bx = 0x4000 | vesamode[mode];
	    __dpmi_int(0x10, &regs);
	    if(regs.x.ax != 0x4F)
		Exit(-1, "VESA Mode 0x%X Failed!\n", vesamode[mode]);

	    __dpmi_meminfo mi;
	    mi.size = (unsigned long)(vminfo.XResolution * vminfo.YResolution);
	    mi.address = vminfo.PhysBasePtr;
	    __dpmi_physical_address_mapping(&mi);
	    frame.UL = __dpmi_allocate_ldt_descriptors(1);
	    __dpmi_set_segment_base_address(frame.UL, mi.address);
	    __dpmi_set_segment_limit(frame.UL, rowlen*ysize);
//	    Exit(0, "Linear frame buffer found at %X\n", vminfo.PhysBasePtr);
	    }
//	  else Exit(-1, "Linear frame buffer not found\n");
//	  Exit(0, "VESA 2.0!\n");
	  }
	}
      }break;
    #endif

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
      else if(depth==16)  {
	_Ximage = XCreateImage(_Xdisplay, None,
		DefaultDepth(_Xdisplay, _Xscreen), ZPixmap, 0,
		video_buffer.c, xsize, ysize, 16, 0);
	}
      else if(depth==32)  {
	_Ximage = XCreateImage(_Xdisplay, None,
		DefaultDepth(_Xdisplay, _Xscreen), ZPixmap, 0,
		video_buffer.c, xsize, ysize, 32, 0);
	}
      else Exit(-1, "Unknown depth error (%d) in %s\n", depth, __PRETTY_FUNCTION__);

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
      if(depth == 8)  {
	for(ctr=0; ctr<collen; ctr++)
	  memset(frame.c+rowlen*ctr, 0, xsize);
	}
      else if(depth == 32)  {
	for(ctr=0; ctr<collen; ctr++)
	  memset(frame.ul+rowlen*ctr, 0, xsize<<2);
	}
      else if(depth == 16)  {
	for(ctr=0; ctr<collen; ctr++)
	  memset(frame.uc+rowlen*ctr, 0, xsize<<1);
	}
      else Exit(-1, "Unknown depth error (%d) in %s\n", depth, __PRETTY_FUNCTION__);
      }break;
    #endif
    #endif
    }
  if(__Da_Mouse != NULL)  __Da_Mouse->SetRange(0, 0, xsize, ysize);
  Debug("User::Screen::SetSize() End"); 
  return 1;
  }

void Screen::Hide()  {
  }

void Screen::Show()  {
  if(shown) return;
  shown = 1;
  Debug("User::Screen::Show() Entry");

  switch(vtype)  {
    #ifdef DOS
    case(VIDEO_VESA):
    case(VIDEO_VBE2):
    case(VIDEO_VBE2L):
    case(VIDEO_DOS): {
      int ctr;
      outportb(0x3C8, 0);
      for(ctr = 0; ctr < 256; ctr++)  {
	outportb(0x3C9, pal->GetRedEntry(ctr)>>2);
	outportb(0x3C9, pal->GetGreenEntry(ctr)>>2);
	outportb(0x3C9, pal->GetBlueEntry(ctr)>>2);
	}
      }break;
    #endif

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

void Screen::FullScreenModeOn() {
  if(fullscreen == 1) return;
  }

void Screen::FullScreenModeOff() {
  if(fullscreen == 0) return;
  }

void Screen::FullScreenModeToggle() {
  if(fullscreen == 0) FullScreenModeOn();
  else FullScreenModeOff();
  }

void Screen::Refresh()  {
  if(!shown) return;
  RefreshFast();
  WaitForNextFrame();
  }

void Screen::RefreshFast()  {
  Debug("User::Screen::RefreshFast Begin");
  if(__Da_Speaker != NULL) __Da_Speaker->Update();
  if(__Da_InputQueue != NULL)  __Da_InputQueue->Update();
  else {
    if(__Da_Mouse != NULL) __Da_Mouse->Update();
    }
  if(!shown) return;
  int ctrb;
  for(ctrb=0; ctrb<REDRAW_RECTS; ctrb++)  {
    if(rxs[ctrb] != -1) {
      switch(vtype)  {
	#ifdef X_WINDOWS
	case(VIDEO_XWINDOWS): {
	  XPutImage(_Xdisplay, _Xwindow, _Xgc, _Ximage, rxs[ctrb], rys[ctrb],
	      rxs[ctrb], rys[ctrb], rxe[ctrb]-rxs[ctrb], rye[ctrb]-rys[ctrb]);
	  }break;
	#ifdef XF86_DGA
	case(VIDEO_XF86DGA): {
	  int ctry;
	  if(depth == 8) {
	    for(ctry=rys[ctrb]; ctry<rye[ctrb]; ctry++) {
	      memcpy(frame.uc + ctry*rowlen + rxs[ctrb],
		image[ctry].uc + rxs[ctrb],
		rxe[ctrb]-rxs[ctrb]);
	      }
	    }
	  else if(depth == 16) {
	    for(ctry=rys[ctrb]; ctry<rye[ctrb]; ctry++) {
	      memcpy(frame.us + ctry*rowlen + rxs[ctrb],
		image[ctry].us + rxs[ctrb],
		(rxe[ctrb]-rxs[ctrb])<<1);
	      }
	    }
	  else if(depth == 32) {
	    for(ctry=rys[ctrb]; ctry<rye[ctrb]; ctry++) {
	      memcpy(frame.ul + ctry*rowlen + rxs[ctrb],
		image[ctry].ul + rxs[ctrb],
		(rxe[ctrb]-rxs[ctrb])<<2);
	      }
	    }
	  else Exit(-1, "Unknown depth error (%d) in %s\n", depth, __PRETTY_FUNCTION__);
	  }break;
	#endif
	#endif

	#ifdef DOS
	case(VIDEO_VBE2):
	case(VIDEO_VESA): {
	  Exit(-1, "VESA w/o liner buffer and quick refresh not implemented!\n");
	  }break;
	case(VIDEO_VBE2L): {
	  int ctry;
	  if(depth == 8) {
	    for(ctry=rys[ctrb]; ctry<rye[ctrb]; ctry++) {
	      movedata(_my_ds(), video_buffer.UL + ctry*rowlen + rxs[ctrb],
		frame.UL, ctry*rowlen + rxs[ctrb], rxe[ctrb]-rxs[ctrb]);
	      }
	    }
	  else if(depth == 32) {
	    for(ctry=rys[ctrb]; ctry<rye[ctrb]; ctry++) {
	      movedata(_my_ds(), video_buffer.UL + ((ctry*rowlen+rxs[ctrb])<<2),
		frame.UL, (ctry*rowlen + rxs[ctrb]) << 2,
		(rxe[ctrb]-rxs[ctrb])<<2);
	      }
	    }
	  else Exit(-1, "Unknown depth error (%d) in %s\n", depth, __PRETTY_FUNCTION__);
	  }break;
	case(VIDEO_DOS): {
	  int ctry;
	  if(depth == 8) {
	    for(ctry=rys[ctrb]; ctry<rye[ctrb]; ctry++) {
	      dosmemput(image[ctry].uc+rxs[ctrb], rxe[ctrb]-rxs[ctrb],
		frame.UL+ctry*rowlen+rxs[ctrb]);
	      }
	    }
	  else Exit(-1, "Unknown depth error (%d) in %s\n", depth, __PRETTY_FUNCTION__);
	  }break;
	#endif
	}
      rxs[ctrb] = -1;
      rxe[ctrb] = -1;
      rys[ctrb] = -1;
      rye[ctrb] = -1;
      }
    }
  Debug("User::Screen::RefreshFast End");
  }

void Screen::RefreshFull()  {
  if(__Da_Speaker != NULL) __Da_Speaker->Update();
  if(__Da_InputQueue != NULL)  __Da_InputQueue->Update();
  else {
    if(__Da_Mouse != NULL) __Da_Mouse->Update();
    }
  if(!shown) return;
  int ctrb;
  for(ctrb=0; ctrb<REDRAW_RECTS; ctrb++)  {
    rxs[ctrb] = -1;
    rxe[ctrb] = -1;
    rys[ctrb] = -1;
    rye[ctrb] = -1;
    }
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
	  memcpy(frame.ul + ctry*rowlen, image[ctry].ul, xsize<<2);
	  }
	}
      else if(depth == 16) {
	for(ctry=0; ctry<ysize; ctry++) {
	  memcpy(frame.us + ctry*rowlen, image[ctry].us, xsize<<1);
	  }
	}
      else Exit(-1, "Unknown depth error (%d) in %s\n", depth, __PRETTY_FUNCTION__);
      }break;
    #endif
    #endif

    #ifdef DOS
    case(VIDEO_VBE2):
    case(VIDEO_VESA): {
      if(depth == 8) {
	int bank;
	for(bank=0; bank<((rowlen*ysize)>>16); bank++) {
	  SetBank(bank);
	  dosmemput(&video_buffer.uc[bank<<16], 65536, frame.UL);
	  }
	if((rowlen*ysize)&65535) {
	  SetBank(bank);
	  dosmemput(&video_buffer.uc[bank<<16], (rowlen*ysize)&65535, frame.UL);
	  }
	}
      }break;
    case(VIDEO_VBE2L): {
      movedata(_my_ds(), video_buffer.UL, frame.UL, 0, rowlen*ysize);
      }break;
    case(VIDEO_DOS): {
      int ctry;
      if(depth == 8) {
	if(rowlen != xsize)  {
	  for(ctry=0; ctry<ysize; ctry++) {
	    dosmemput(image[ctry].uc, xsize, frame.UL+ctry*rowlen);
	    }
	  }
	else  {
	  dosmemput(video_buffer.uc, xsize*ysize, frame.UL);
	  }
	}
      }break;
    #endif
    }
  WaitForNextFrame();
  Debug("User::Screen::RefreshFull() 1100");
  }

void Screen::Clear()  {
  int ctr, ctr2;
  if(depth==8)  {
    for(ctr=0; ctr<ysize; ctr++)  {
      for(ctr2=0; ctr2<xsize; ctr2++)  {
//	image[ctr].uc[ctr2] = BlackPixel(_Xdisplay, 0);
//	backg[ctr].uc[ctr2] = BlackPixel(_Xdisplay, 0);
	image[ctr].uc[ctr2] = 0;
	backg[ctr].uc[ctr2] = 0;
	}
      }
    }
  else if(depth==32)  {
    for(ctr=0; ctr<ysize; ctr++)  {
      for(ctr2=0; ctr2<xsize; ctr2++)  {
//	image[ctr].ul[ctr2] = BlackPixel(_Xdisplay, 0);
//	backg[ctr].ul[ctr2] = BlackPixel(_Xdisplay, 0);
	image[ctr].ul[ctr2] = 0;
	backg[ctr].ul[ctr2] = 0;
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

void Screen::ClearArea(int x, int y, int xs, int ys)  {
  int ctry;
  if(depth==8) {
    for(ctry=y; ctry<(y+ys); ctry++)  {
      memset(&(image[ctry].uc[x]), 0, xs);
      memset(&(backg[ctry].uc[x]), 0, xs);
      }
    }
  else if(depth==16) {
    for(ctry=y; ctry<(y+ys); ctry++)  {
      memset(&(image[ctry].us[x]), 0, xs<<1);
      memset(&(backg[ctry].us[x]), 0, xs<<1);
      }
    }
  else if(depth==32) {
    for(ctry=y; ctry<(y+ys); ctry++)  {
      memset(&(image[ctry].ul[x]), 0, xs<<2);
      memset(&(backg[ctry].ul[x]), 0, xs<<2);
      }
    }
  else Exit(-1, "Unknown depth error (%d) in %s\n", depth, __PRETTY_FUNCTION__);
  }

void Screen::DrawRectangle(int x, int y, int xs, int ys, color c)  {
  ConvertColor(c, appdepth, depth);
  InvalidateRectangle(x, y, xs, ys);
  int ctrx, ctry;
  if(depth==8) {
    for(ctry=y; ctry<(y+ys); ctry++)  {
      memset(&(image[ctry].uc[x]), c, xs);
      memset(&(backg[ctry].uc[x]), c, xs);
      }
    }
  else if(depth==32) {
    for(ctry=y; ctry<(y+ys); ctry++)  {
      for(ctrx=x; ctrx<(x+xs); ctrx++)  {
	image[ctry].ul[ctrx] = c;
	backg[ctry].ul[ctrx] = c;
	}
      }
    }
  else if(depth==16) {
    for(ctry=y; ctry<(y+ys); ctry++)  {
      for(ctrx=x; ctrx<(x+xs); ctrx++)  {
	image[ctry].us[ctrx] = c;
	backg[ctry].us[ctrx] = c;
	}
      }
    }
  else Exit(-1, "Unknown depth error (%d) in %s\n", depth, __PRETTY_FUNCTION__);
/*
  for(ctrx=x; ctrx<(x+xs); ctrx++)  {
    for(ctry=y; ctry<(y+ys); ctry++)  {
      SetPoint(ctrx, ctry, c);
      }
    }
*/
  }

void Screen::DrawRectangleFG(int x, int y, int xs, int ys, color c)  {
  InvalidateRectangle(x, y, xs, ys);
  int ctrx, ctry;
  for(ctrx=x; ctrx<(x+xs); ctrx++)  {
    for(ctry=y; ctry<(y+ys); ctry++)  {
      SetPointFG(ctrx, ctry, c);
      }
    }
  }

void Screen::SetLine(int xs, int ys, int xe, int ye, color c)  {
  Graphic tmpg;
  ConvertColor(c, appdepth, depth);
  tmpg.SetLine(xe-xs, ye-ys, depth, c);
  DrawTransparentGraphic(tmpg, xs-tmpg.xcenter, ys-tmpg.ycenter);
  }

void Screen::SetLineFG(int xs, int ys, int xe, int ye, color c)  {
  Graphic tmpg;
  ConvertColor(c, appdepth, depth);
  tmpg.SetLine(xe-xs, ye-ys, depth, c);
  DrawTransparentGraphicFG(tmpg, xs, ys);
  }

void Screen::SetLine(int xs, int ys, int xe, int ye, int r, int g, int b)  {
  InvalidateRectangle(xs, ys, xe-xs, ye-ys);
  }

void Screen::SetLineFG(int xs, int ys, int xe, int ye, int r, int g, int b)  {
  InvalidateRectangle(xs, ys, xe-xs, ye-ys);
  }

void Screen::SetPoint(int x, int y, color c)  {
  ConvertColor(c, appdepth, depth);
  InvalidateRectangle(x, y, 1, 1);
  if(depth==8)  {
    image[y].uc[x] = c;
    backg[y].uc[x] = c;
    }
  else if(depth==32)  {
    image[y].ul[x] = c;
    backg[y].ul[x] = c;
    }
  else if(depth==16)  {
    image[y].us[x] = c;
    backg[y].us[x] = c;
    }
  }

void Screen::SetPointFG(int x, int y, color c)  {
  ConvertColor(c, appdepth, depth);
  InvalidateRectangle(x, y, 1, 1);
  if(depth==8)  {
    image[y].uc[x] = c;
    }
  else if(depth==32)  {
    image[y].ul[x] = c;
    }
  else if(depth==16)  {
    image[y].us[x] = c;
    }
  }

void Screen::SetPoint(int x, int y, int r, int g, int b)  {
  InvalidateRectangle(x, y, 1, 1);
  }

void Screen::SetPointFG(int x, int y, int r, int g, int b)  {
  InvalidateRectangle(x, y, 1, 1);
  }

void Screen::FullScreenBMP(const char *fn) {
  Graphic g((char*)fn);
  FullScreenGraphic(g);
  }

void Screen::DrawPartialTransparentGraphicFG(Graphic &g, int x, int y,
	int xb, int yb, int xs, int ys, Panel p)  {
  if(x+g.xsize <= pxs[p] || y+g.ysize <= pys[p] || x >= pxe[p] || y >= pye[p])
	return;
  if(g.depth != depth)  {
    Graphic *g2 = new Graphic(g);
    g2->DepthConvert(depth, *pal);
    DrawPartialTransparentGraphicFG(*g2, x, y, xb, yb, xs, ys, p);
    delete g2;
    return;
    }
  Debug("User:Screen:DrawPartialTransparentGraphicFG Middle");
  int ctrx, ctry;
  if(depth == 8)  {
    Debug("User:Screen:DrawPartialTransparentGraphicFG Depth 8");
    for(ctry=(yb>?(pys[p]-y)); ctry<((pye[p]-y)<?(ys+yb)); ctry++)  {
      for(ctrx=(xb>?(pxs[p]-x)); ctrx<((pxe[p]-x)<?(xs+xb)); ctrx++)  {
	if(g.image[ctry].uc[ctrx] != g.tcolor)  {
	  image[ctry+y].uc[ctrx+x] = g.image[ctry].uc[ctrx];
	  }
	}
      }
    }
  else if(depth == 32)  {
    Debug("User:Screen:DrawPartialTransparentGraphicFG Depth 32");
    for(ctry=(yb>?(pys[p]-y)); ctry<((pye[p]-y)<?(ys+yb)); ctry++)  {
      for(ctrx=(xb>?(pxs[p]-x)); ctrx<((pxe[p]-x)<?(xs+xb)); ctrx++)  {
	if(g.image[ctry].uc[(ctrx<<2)+3] == 0xFF)  {
	  image[ctry+y].ul[ctrx+x] = g.image[ctry].ul[ctrx];
	  }
	else if(g.image[ctry].uc[(ctrx<<2)+3])  {
	  int alpha = g.image[ctry].uc[(ctrx<<2)+3];
	  unsigned long r1 = image[ctry+y].uc[((ctrx+x)<<2)];
	  unsigned long g1 = image[ctry+y].uc[((ctrx+x)<<2)+1];
	  unsigned long b1 = image[ctry+y].uc[((ctrx+x)<<2)+2];
	  unsigned long r2 = g.image[ctry].uc[(ctrx<<2)];
	  unsigned long g2 = g.image[ctry].uc[(ctrx<<2)+1];
	  unsigned long b2 = g.image[ctry].uc[(ctrx<<2)+2];
          r1 *= (0xFF-alpha);  r2 *= alpha; r1 += r2; r1 /= 255;
          g1 *= (0xFF-alpha);  g2 *= alpha; g1 += g2; g1 /= 255;
          b1 *= (0xFF-alpha);  b2 *= alpha; b1 += b2; b1 /= 255;
	  image[ctry+y].uc[((ctrx+x)<<2)] = r1;
	  image[ctry+y].uc[((ctrx+x)<<2)+1] = g1;
	  image[ctry+y].uc[((ctrx+x)<<2)+2] = b1;
	  }
	}
      }
    }
  else if(depth == 16)  {
    Debug("User:Screen:DrawPartialTransparentGraphicFG Depth 16");
    for(ctry=(yb>?(pys[p]-y)); ctry<((pye[p]-y)<?(ys+yb)); ctry++)  {
      for(ctrx=(xb>?(pxs[p]-x)); ctrx<((pxe[p]-x)<?(xs+xb)); ctrx++)  {
	if(g.image[ctry].us[ctrx] != g.tcolor)  {
	  image[ctry+y].us[ctrx+x] = g.image[ctry].us[ctrx];
	  }
	}
      }
    }
  else Exit(-1, "Unknown depth error (%d) in %s\n", depth, __PRETTY_FUNCTION__);
  Debug("User:Screen:DrawPartialTransparentGraphicFG End");
  }

void Screen::DrawPartialGraphicFG(Graphic &g, int x, int y,
	int xb, int yb, int xs, int ys, Panel p)  {
  if(x+g.xsize <= pxs[p] || y+g.ysize <= pys[p] || x >= pxe[p] || y >= pye[p])
	return;
  if(g.depth != depth)  {
    Graphic *g2 = new Graphic(g);
    g2->DepthConvert(depth, *pal);
    DrawPartialTransparentGraphicFG(*g2, x, y, xb, yb, xs, ys, p);
    delete g2;
    return;
    }
  Debug("User:Screen:DrawPartialTransparentGraphicFG Middle");
  int ctry;
  int ix = (xb>?(pxs[p]-x)), ex = (pxe[p]-x)<?(xs+xb);
  int sx = ex-ix;
  if(depth == 8)  {
    Debug("User:Screen:DrawPartialTransparentGraphicFG Depth 8");
    for(ctry=(yb>?(pys[p]-y)); ctry<((pye[p]-y)<?(ys+yb)); ctry++)  {
      memcpy(image[ctry+y].uc+x+ix, g.image[ctry].uc+ix, sx);
      }
    }
  else if(depth == 16)  {
    Debug("User:Screen:DrawPartialTransparentGraphicFG Depth 16");
    for(ctry=(yb>?(pys[p]-y)); ctry<((pye[p]-y)<?(ys+yb)); ctry++)  {
      memcpy(image[ctry+y].us+x+ix, g.image[ctry].us+ix, sx<<1);
      }
    }
  else if(depth == 32)  {
    Debug("User:Screen:DrawPartialTransparentGraphicFG Depth 32");
    for(ctry=(yb>?(pys[p]-y)); ctry<((pye[p]-y)<?(ys+yb)); ctry++)  {
      memcpy(image[ctry+y].ul+x+ix, g.image[ctry].ul+ix, sx<<2);
      }
    }
  else Exit(-1, "Unknown depth error (%d) in %s\n", depth, __PRETTY_FUNCTION__);
  Debug("User:Screen:DrawPartialTransparentGraphicFG End");
  }

void Screen::DrawTransparentGraphicFG(Graphic &g, int x, int y, Panel p)  {
  if(x+g.xsize <= pxs[p] || y+g.ysize <= pys[p] || x >= pxe[p] || y >= pye[p])
	return;
  if(g.depth != depth)  {
    Graphic *g2 = new Graphic(g);
    g2->DepthConvert(depth, *pal);
    DrawTransparentGraphicFG(*g2, x, y, p);
    delete g2;
    return;
    }
  InvalidateRectangle(x, y, g.xsize, g.ysize);
  Debug("User:Screen:DrawTransparentGraphicFG Middle");
  int ctrx, ctry;
  if(depth == 8)  {
    Debug("User:Screen:DrawTransparentGraphicFG Depth 8");
    for(ctry=(0>?(pys[p]-y)); ctry<((pye[p]-y)<?g.ysize); ctry++)  {
      for(ctrx=(0>?(pxs[p]-x)); ctrx<((pxe[p]-x)<?g.xsize); ctrx++)  {
	if(g.image[ctry].uc[ctrx] != g.tcolor)  {
	  image[ctry+y].uc[ctrx+x] = g.image[ctry].uc[ctrx];
	  }
	}
      }
    }
  else if(depth == 32)  {
    Debug("User:Screen:DrawTransparentGraphicFG Depth 32");
    for(ctry=(0>?(pys[p]-y)); ctry<((pye[p]-y)<?g.ysize); ctry++)  {
      for(ctrx=(0>?(pxs[p]-x)); ctrx<((pxe[p]-x)<?g.xsize); ctrx++)  {
	if(g.image[ctry].uc[(ctrx<<2)+3] == 0xFF)  {
	  image[ctry+y].ul[ctrx+x] = g.image[ctry].ul[ctrx];
	  }
	else if(g.image[ctry].uc[(ctrx<<2)+3])  {
	  int alpha = g.image[ctry].uc[(ctrx<<2)+3];
	  unsigned long r1 = image[ctry+y].uc[((ctrx+x)<<2)];
	  unsigned long g1 = image[ctry+y].uc[((ctrx+x)<<2)+1];
	  unsigned long b1 = image[ctry+y].uc[((ctrx+x)<<2)+2];
	  unsigned long r2 = g.image[ctry].uc[(ctrx<<2)];
	  unsigned long g2 = g.image[ctry].uc[(ctrx<<2)+1];
	  unsigned long b2 = g.image[ctry].uc[(ctrx<<2)+2];
          r1 *= (255-alpha);  r2 *= alpha; r1 += r2; r1 /= 255;
          g1 *= (255-alpha);  g2 *= alpha; g1 += g2; g1 /= 255;
          b1 *= (255-alpha);  b2 *= alpha; b1 += b2; b1 /= 255;
	  image[ctry+y].uc[((ctrx+x)<<2)] = r1;
	  image[ctry+y].uc[((ctrx+x)<<2)+1] = g1;
	  image[ctry+y].uc[((ctrx+x)<<2)+2] = b1;
	  }
	}
      }
    }
  else if(depth == 16)  {
    Debug("User:Screen:DrawTransparentGraphicFG Depth 16");
    for(ctry=(0>?(pys[p]-y)); ctry<((pye[p]-y)<?g.ysize); ctry++)  {
      for(ctrx=(0>?(pxs[p]-x)); ctrx<((pxe[p]-x)<?g.xsize); ctrx++)  {
	if(g.image[ctry].us[ctrx] != g.tcolor)  {
	  image[ctry+y].us[ctrx+x] = g.image[ctry].us[ctrx];
	  }
	}
      }
    }
  else Exit(-1, "Unknown depth error (%d) in %s\n", depth, __PRETTY_FUNCTION__);
  Debug("User:Screen:DrawTransparentGraphicFG End");
  }

void Screen::DrawGraphicFG(Graphic &g, int x, int y, Panel p)  {
  if(x+g.xsize <= pxs[p] || y+g.ysize <= pys[p] || x >= pxe[p] || y >= pye[p])
	return;
  InvalidateRectangle(x, y, g.xsize, g.ysize);
  if(g.depth != depth)  {
    Graphic *g2 = new Graphic(g);
    g2->DepthConvert(depth, *pal);
    DrawGraphicFG(*g2, x, y);
    delete g2;
    return;
    }
  int ctry;
  if(depth == 8)  {
    for(ctry=0; ctry<((ysize-y)<?g.ysize); ctry++)  {
      memcpy(image[ctry+y].uc+x, g.image[ctry].uc, ((xsize-x)<?g.xsize));
      }
    }
  else if(depth == 16)  {
    for(ctry=0; ctry<((ysize-y)<?g.ysize); ctry++)  {
      memcpy(image[ctry+y].us+x, g.image[ctry].us, (((xsize-x)<?g.xsize)<<1));
      }
    }
  else if(depth == 32)  {
    for(ctry=0; ctry<((ysize-y)<?g.ysize); ctry++)  {
      memcpy(image[ctry+y].ul+x, g.image[ctry].ul, (((xsize-x)<?g.xsize)<<2));
      }
    }
  else Exit(-1, "Unknown depth error (%d) in %s\n", depth, __PRETTY_FUNCTION__);
  }

void Screen::FullScreenGraphicFG(Graphic &g) {
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
	image[ctry].uc[ctrx] = g.image[ctry].uc[ctrx];
	}
      }
    }
  else if(depth == 32)  {
    for(ctry=0; ctry<(ysize<?g.ysize); ctry++)  {
      for(ctrx=0; ctrx<(xsize<?(g.xsize)); ctrx++)  {
	image[ctry].ul[ctrx] = g.image[ctry].ul[ctrx];
	}
      }
    }
  else Exit(-1, "Unknown depth error (%d) in %s\n", depth, __PRETTY_FUNCTION__);
  }

void Screen::DrawTransparentGraphic(Graphic &g, int x, int y, Panel p)  {
  if(x+g.xsize <= pxs[p] || y+g.ysize <= pys[p] || x >= pxe[p] || y >= pye[p])
	return;
  InvalidateRectangle(x, y, g.xsize, g.ysize);
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
	if(g.image[ctry].uc[ctrx] != g.tcolor)  {
	  image[ctry+y].uc[ctrx+x] = g.image[ctry].uc[ctrx];
	  backg[ctry+y].uc[ctrx+x] = g.image[ctry].uc[ctrx];
	  }
	}
      }
    }
  else if(depth == 32)  {
    Debug("User:Screen:DrawTransparentGraphic Depth 32");
    for(ctry=(0>?(pys[p]-y)); ctry<((pye[p]-y)<?g.ysize); ctry++)  {
      for(ctrx=(0>?(pxs[p]-x)); ctrx<((pxe[p]-x)<?g.xsize); ctrx++)  {
	if(g.image[ctry].uc[(ctrx<<2)+3] == 255)  {
	  image[ctry+y].ul[ctrx+x] = g.image[ctry].ul[ctrx];
	  backg[ctry+y].ul[ctrx+x] = g.image[ctry].ul[ctrx];
	  }
	else if(g.image[ctry].uc[(ctrx<<2)+3])  {
	  unsigned long alpha = g.image[ctry].uc[(ctrx<<2)+3];
	  unsigned long r1 = backg[ctry+y].uc[((ctrx+x)<<2)];
	  unsigned long g1 = backg[ctry+y].uc[((ctrx+x)<<2)+1];
	  unsigned long b1 = backg[ctry+y].uc[((ctrx+x)<<2)+2];
	  unsigned long r2 = g.image[ctry].uc[(ctrx<<2)];
	  unsigned long g2 = g.image[ctry].uc[(ctrx<<2)+1];
	  unsigned long b2 = g.image[ctry].uc[(ctrx<<2)+2];
	  r2 *= alpha; r1 *= (255-alpha); r1 += r2; r1 /= 255;
	  g2 *= alpha; g1 *= (255-alpha); g1 += g2; g1 /= 255;
	  b2 *= alpha; b1 *= (255-alpha); b1 += b2; b1 /= 255;
	  image[ctry+y].uc[((ctrx+x)<<2)] = r1;
	  image[ctry+y].uc[((ctrx+x)<<2)+1] = g1;
	  image[ctry+y].uc[((ctrx+x)<<2)+2] = b1;
	  backg[ctry+y].uc[((ctrx+x)<<2)] = r1;
	  backg[ctry+y].uc[((ctrx+x)<<2)+1] = g1;
	  backg[ctry+y].uc[((ctrx+x)<<2)+2] = b1;
	  }
	}
      }
    }
  else if(depth == 16)  {
    for(ctry=0; ctry<((ysize-y)<?g.ysize); ctry++)  {
      for(ctrx=0; ctrx<((xsize-x)<?g.xsize); ctrx++)  {
	if(g.image[ctry].us[ctrx] != g.tcolor)  {
	  image[ctry+y].us[ctrx+x] = g.image[ctry].us[ctrx];
	  backg[ctry+y].us[ctrx+x] = g.image[ctry].us[ctrx];
	  }
	}
      }
    }
  else Exit(-1, "Unknown depth error (%d) in %s\n", depth, __PRETTY_FUNCTION__);
  }

void Screen::DrawGraphic(Graphic &g, int x, int y, Panel p)  {
  if(x+g.xsize <= pxs[p] || y+g.ysize <= pys[p] || x >= pxe[p] || y >= pye[p])
	return;
  InvalidateRectangle(x, y, g.xsize, g.ysize);
  if(g.depth != depth)  {
    Graphic *g2 = new Graphic(g);
    g2->DepthConvert(depth, *pal);
    DrawGraphic(*g2, x, y);
    delete g2;
    return;
    }
  int ctry;
  if(depth == 8)  {
    for(ctry=0; ctry<((ysize-y)<?g.ysize); ctry++)  {
      memcpy(image[ctry+y].uc+x, g.image[ctry].uc, ((xsize-x)<?g.xsize));
      memcpy(backg[ctry+y].uc+x, g.image[ctry].uc, ((xsize-x)<?g.xsize));
      }
    }
  else if(depth == 16)  {
    for(ctry=0; ctry<((ysize-y)<?g.ysize); ctry++)  {
      memcpy(image[ctry+y].us+x, g.image[ctry].us, (((xsize-x)<?g.xsize)<<1));
      memcpy(backg[ctry+y].us+x, g.image[ctry].us, (((xsize-x)<?g.xsize)<<1));
      }
    }
  else if(depth == 32)  {
    for(ctry=0; ctry<((ysize-y)<?g.ysize); ctry++)  {
      memcpy(image[ctry+y].ul+x, g.image[ctry].ul, (((xsize-x)<?g.xsize)<<2));
      memcpy(backg[ctry+y].ul+x, g.image[ctry].ul, (((xsize-x)<?g.xsize)<<2));
      }
    }
  else Exit(-1, "Unknown depth error (%d) in %s\n", depth, __PRETTY_FUNCTION__);
  }

void Screen::FullScreenGraphic(Graphic &g) {
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
	image[ctry].uc[ctrx] = g.image[ctry].uc[ctrx];
	backg[ctry].uc[ctrx] = g.image[ctry].uc[ctrx];
	}
      }
    }
  else if(depth == 32)  {
    for(ctry=0; ctry<(ysize<?g.ysize); ctry++)  {
      for(ctrx=0; ctrx<(xsize<?(g.xsize)); ctrx++)  {
	image[ctry].ul[ctrx] = g.image[ctry].ul[ctrx];
	backg[ctry].ul[ctrx] = g.image[ctry].ul[ctrx];
	}
      }
    }
  else if(depth == 16)  {
    for(ctry=0; ctry<(ysize<?g.ysize); ctry++)  {
      for(ctrx=0; ctrx<(xsize<?(g.xsize)); ctrx++)  {
	image[ctry].us[ctrx] = g.image[ctry].us[ctrx];
	backg[ctry].us[ctrx] = g.image[ctry].us[ctrx];
	}
      }
    }
  else Exit(-1, "Unknown depth error (%d) in %s\n", depth, __PRETTY_FUNCTION__);
  }

void Screen::SetPaletteEntry(int c, int r, int g, int b) {
  pal->SetPaletteEntry(c, r, g, b);
  if(shown)  {
    switch(vtype)  {
      #ifdef DOS
      case(VIDEO_VESA):
      case(VIDEO_VBE2):
      case(VIDEO_VBE2L):
      case(VIDEO_DOS): {
	outportb(0x3C8, c);
	outportb(0x3C9, r>>2);
	outportb(0x3C9, g>>2);
	outportb(0x3C9, b>>2);
	}break;
      #endif

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

void Screen::SetPalette(Palette &p) {
  pal = new Palette(p);
  if(shown)  {
    switch(vtype)  {
      #ifdef DOS
      case(VIDEO_VESA):
      case(VIDEO_VBE2):
      case(VIDEO_VBE2L):
      case(VIDEO_DOS): {
	int ctr;
	outportb(0x3C8, 0);
	for(ctr = 0; ctr < 256; ctr++)  {
	  outportb(0x3C9, pal->GetRedEntry(ctr)>>2);
	  outportb(0x3C9, pal->GetGreenEntry(ctr)>>2);
	  outportb(0x3C9, pal->GetBlueEntry(ctr)>>2);
	  }
	}break;
      #endif

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
        }break;
      #endif
      #endif
      }
    }
  }

void Screen::SetPalette(const char *fn) {
  pal->Set(fn);
  if(shown)  {
    switch(vtype)  {
      #ifdef DOS
      case(VIDEO_VESA):
      case(VIDEO_VBE2):
      case(VIDEO_VBE2L):
      case(VIDEO_DOS): {
	int ctr;
	outportb(0x3C8, 0);
	for(ctr = 0; ctr < 256; ctr++)  {
	  outportb(0x3C9, pal->GetRedEntry(ctr)>>2);
	  outportb(0x3C9, pal->GetGreenEntry(ctr)>>2);
	  outportb(0x3C9, pal->GetBlueEntry(ctr)>>2);
	  }
	}break;
      #endif

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
        }break;
      #endif
      #endif
      }
    }
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

IntList Screen::CollideRectangle(int x, int y, int xs, int ys)  {
  int ctr; IntList ret;
  for(ctr=0; ctr<nextsprite; ctr++)  {
    if(sprites[ctr] != NULL && (!(sprites[ctr]->flags&SPRITE_NOCOLLISIONS))
	&& sprites[ctr]->drawn && sprites[ctr]->image != NULL)  {
      if(x < (sprites[ctr]->xpos + sprites[ctr]->image->xsize)
	    && y < (sprites[ctr]->ypos + sprites[ctr]->image->ysize)
	    && (y+ys) > sprites[ctr]->ypos
	    && (x+xs) > sprites[ctr]->xpos)  {
	if(sprites[ctr]->Hits(x, y, xs, ys)) ret += ctr;
	}      
      }
    }
  return ret;
  }

IntList Screen::CollideRectangle(int s, int x, int y, int xs, int ys)  {
  int ctr; IntList ret;
  for(ctr=0; ctr<nextsprite; ctr++)  {
    if(ctr!=s && sprites[ctr] != NULL
	&& (!(sprites[ctr]->flags&SPRITE_NOCOLLISIONS))
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

void Screen::RestoreInvalidRectangles()  {
  int ctr;
  for(ctr=0; ctr<REDRAW_RECTS; ctr++) {
    RestoreRectangle(rxs[ctr], rxe[ctr]-rxs[ctr], rys[ctr], rye[ctr]-rys[ctr]);
    }
  }

void Screen::InvalidateRectangle(int x, int y, int xs, int ys)  {
  if(x<0) { xs+=x; x=0; }
  if(y<0) { ys+=y; y=0; }
  if(x+xs > xsize) xs = xsize-x;
  if(y+ys > ysize) ys = ysize-y;
  if(xs<1 || ys<1) return;
  int ctr, found=REDRAW_RECTS;
  int xe=xs+x, ye=ys+y;

  for(ctr=0; ctr<REDRAW_RECTS && found>=REDRAW_RECTS; ctr++) {
    if(((rxs[ctr] >= x && rxs[ctr] < xe)
	|| (rxe[ctr] >= x && rxe[ctr] < xe))
	&& ((rys[ctr] >= y && rys[ctr] < ye)
	|| (rye[ctr] >= y && rye[ctr] < ye))
    || ((x >= rxs[ctr] && x < rxe[ctr])
	|| (xe >= rxs[ctr] && xe < rxe[ctr]))
	&& ((y >= rys[ctr] && y < rye[ctr])
	|| (ye >= rys[ctr] && ye < rye[ctr]))) {
      rxs[ctr] = (rxs[ctr] <? x);
      rxe[ctr] = (rxe[ctr] >? xe);
      rys[ctr] = (rys[ctr] <? y);
      rye[ctr] = (rye[ctr] >? ye);
      return;
      }
    }

  for(ctr=0; ctr<REDRAW_RECTS; ctr++) {
    if(rxs[ctr] == -1) {
      rxs[ctr] = x;
      rxe[ctr] = xe;
      rys[ctr] = y;
      rye[ctr] = ye;
      return;
      }
    }

  int num=0, loss, tl;
  loss = ((rxe[0] >? xe)-(rxs[0] <? x))*((rye[0] >? ye)-(rys[0] <? y));
  loss -= (rxe[0]-rxs[0])*(rye[0]-rys[0]);
  loss -= (xe-x)*(ye-y);
  for(ctr=1; ctr<REDRAW_RECTS; ctr++) {
    tl = ((rxe[ctr] >? xe)-(rxs[ctr] <? x))*((rye[ctr] >? ye)-(rys[ctr] <? y));
    tl -= (rxe[ctr]-rxs[ctr])*(rye[ctr]-rys[ctr]);
    tl -= (xe-x)*(ye-y);
    if(tl<loss) {
      num = ctr;
      loss = tl;
      }
    }
  rxs[num] = (rxs[num] <? x);
  rxe[num] = (rxe[num] >? xe);
  rys[num] = (rys[num] <? y);
  rye[num] = (rye[num] >? ye);
  }

void Screen::RestoreRectangle(int x, int y, int xs, int ys)  {
  Debug("Screen:RestoreRectangle() Begin");
  if(x>xsize || y>ysize || x+xs<0 || y+ys<0) return;
  if(x<0) { xs += x; x=0; }
  if(y<0) { ys += y; y=0; }
  if(x+xs > xsize) xs = xsize-x;
  if(y+ys > ysize) ys = ysize-y;
  InvalidateRectangle(x, y, xs, ys);
  int ctrx, ctry;
  Debug("Screen:RestoreRectangle() Before Write");
  if(depth == 8)  {
    for(ctry=y; ctry<y+ys; ctry++)  {
      memcpy(image[ctry].uc+x, backg[ctry].uc+x, xs);
      }
    }
  else if(depth == 16)  {
    for(ctry=y; ctry<y+ys; ctry++)  {
      memcpy(image[ctry].us+x, backg[ctry].us+x, xs<<1);
      }
    }
  else if(depth == 32)  {
    for(ctry=y; ctry<y+ys; ctry++)  {
      memcpy(image[ctry].ul+x, backg[ctry].ul+x, xs<<2);
      }
    }
  else Exit(-1, "Unknown depth error (%d) in %s\n", depth, __PRETTY_FUNCTION__);

  Debug("Screen:RestoreRectangle() Before Selection");
  Sprite **spp = spbuf;
  Debug("Screen:RestoreRectangle() Small 1");

#define snag(bxy) \
    for(tmps=bxy; tmps!=NULL; tmps=tmps->next) {			\
      if(tmps->drawn && (y+ys) > tmps->ypos && (x+xs) > tmps->xpos	\
	  && tmps->image != NULL					\
	  && x < (tmps->xpos + tmps->image->xsize)			\
	  && y < (tmps->ypos + tmps->image->ysize))  {			\
	*spp = tmps; ++spp;						\
	}								\
      }

  int xb=(x>>BIN_FACTOR)-1, yb=(y>>BIN_FACTOR)-1;
  int xe=(x+xs+BIN_SIZE-1)>>BIN_FACTOR, ye=(y+ys+BIN_SIZE-1)>>BIN_FACTOR;
  register Sprite *tmps;
  if(xb<0) xb=0;
  if(yb<0) yb=0;
  if(xe>=(xsize>>BIN_FACTOR)) xe=(xsize>>BIN_FACTOR)-1;
  if(ye>=(ysize>>BIN_FACTOR)) ye=(ysize>>BIN_FACTOR)-1;
  for(ctrx=xb; ctrx<xe; ++ctrx) {
    for(ctry=yb; ctry<ye; ++ctry) {
      snag(bins[ctrx][ctry]);
      }
    }
  snag(huges);

  xb=(x>>LARGE_BIN_FACTOR)-1; yb=(y>>LARGE_BIN_FACTOR)-1;
  xe=(x+xs+LARGE_BIN_SIZE-1)>>LARGE_BIN_FACTOR;
  ye=(y+ys+LARGE_BIN_SIZE-1)>>LARGE_BIN_FACTOR;
  if(xb<0) xb=0;
  if(yb<0) yb=0;
  if(xe>=(xsize>>LARGE_BIN_FACTOR)) xe=(xsize>>LARGE_BIN_FACTOR)-1;
  if(ye>=(ysize>>LARGE_BIN_FACTOR)) ye=(ysize>>LARGE_BIN_FACTOR)-1;
  for(ctrx=xb; ctrx<xe; ++ctrx) {
    for(ctry=yb; ctry<ye; ++ctry) {
      snag(lbins[ctrx][ctry]);
      }
    }
  snag(huges);
#undef snag(bxy)

  Debug("Screen:RestoreRectangle() Before Sort");
  *spp = NULL;

  Sprite **ind, **best, **begin, *tmp;
  for(begin=spbuf; *begin!=NULL; ++begin) {
    for(best=begin,ind=begin; ind!=spp; ++ind) {
      if((*ind)->priority > (*best)->priority) best=ind;
      else if((*ind)->priority == (*best)->priority
	&& (*ind)->snum > (*best)->snum) best=ind;
      }
    tmp = *best;
    *best=*begin;
    *begin=tmp;
    }
  Debug("Screen:RestoreRectangle() Before Redraw");
  for(spp = spbuf; *spp != NULL; spp++)  {
    (*spp)->RedrawArea(x, y, xs, ys);
    }
  Debug("Screen:RestoreRectangle() End");
  }

void Screen::DetectVideoType()  {
#ifdef DOS
  vtype = VIDEO_DOS;
#endif
#ifdef X_WINDOWS
  _Xdisplay = XOpenDisplay("");
  if(_Xdisplay != NULL)  {
    vtype = VIDEO_XWINDOWS;
#ifdef XF86_DGA
    if(!access("/dev/mem", W_OK))  {
      int M, m;
      if(XF86DGAQueryVersion(_Xdisplay, &M, &m))  {
//	fprintf(stderr, "Got DGA version %d.%d\n", M, m);
	int Xbank, Xmem, Flags;
	XF86DGAQueryDirectVideo(_Xdisplay, _Xscreen, &Flags);
	if(!(Flags & XF86DGADirectPresent)) Exit(0, "Failed DGA Query\n");
	XF86DGAGetVideo(_Xdisplay, _Xscreen,
		&(frame.c), &rowlen, &Xbank, &Xmem);
//	fprintf(stderr, "Width %d, Bank %d, Mem %d\n",  rowlen, Xbank, Xmem);
	}
      else { fprintf(stderr, "No DGA support.\n"); return; }
      if(XF86VidModeQueryVersion(_Xdisplay, &M, &m))  {
//	fprintf(stderr, "Got VidMode version %d.%d\n", M, m);
	}
      else { fprintf(stderr, "No VidMode support.\n"); return; }
      vtype = VIDEO_XF86DGA;
      }
#endif
    }
#endif
//  printf("Video trye = %d\n", vtype);
  if(vtype != VIDEO_NONE) return;
  Exit(-1, "No video capability detected!\n");
  }

void Screen::SetFrameRate(int rt)  {
  if(rt <= 0) { framedelay = 0; }
  else  {
#ifdef DOS
    framedelay = UCLOCKS_PER_SEC / rt;
    uclock_t dest=uclock();
    ulasttime = dest;
//    dest >>= 32;
//    lasttime = dest;
#else
    framedelay = 1000000 / rt;
    timeval tv;
    gettimeofday(&tv, NULL);
    lasttime = tv.tv_sec;
    ulasttime = tv.tv_usec;
#endif
    }
  }

void Screen::WaitForNextFrame()  {
  if(framedelay <= 0) return;
#ifdef DOS
  uclock_t dest, cur=uclock();
//  dest = lasttime;
//  dest <<= 32;
//  dest |= ulasttime;
  dest = ulasttime;
  dest += framedelay;
  while(cur < dest) {
    __Da_Screen->RefreshFast(); cur=uclock();
    }
  ulasttime = dest;
//  dest >>= 32;
//  lasttime = dest;
#else
  time_t dest; long udest;
  timeval tv;
  dest = lasttime; udest = ulasttime;
  udest += framedelay;
  if(udest > 1000000) { udest -= 1000000; dest += 1; }
  gettimeofday(&tv, NULL);
  while(tv.tv_sec<dest || (tv.tv_sec==dest && tv.tv_usec<udest)) {
    __Da_Screen->RefreshFast(); gettimeofday(&tv, NULL);
    }
//  lasttime = dest;
//  ulasttime = udest;
  lasttime = tv.tv_sec;
  ulasttime = tv.tv_usec;
#endif
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
  Debug("Screen::AlignCursor() Begin");
  if(tcx < 1) tcx = 1;
  else if(tcx >= xsize) tcx = xsize-1;
  if(tcy < 1) tcy = 1;
  else if(tcy >= ysize) tcy = ysize-1;
  if(font[' '] != NULL)  {
    if(tcy >= ysize-(font[' ']->ysize+2)) tcy = ysize-(font[' ']->ysize+3);
/*
    tcy += ((font[' ']->ysize+2)/2);
    tcy /= (font[' ']->ysize+2);
    tcy *= (font[' ']->ysize+2);
    tcy ++;
*/
    }
  Debug("Screen::AlignCursor() End");
  }

void Screen::SetCursor(Graphic &g)  {
  TCursor = new Sprite(g);
  TCursor->Move(tcx, tcy);
  }

int Screen::GPrintf(Graphic *g, int x, int y, color cb, color cf,
	const char *text, ...) {
  Debug("User::Screen::Printf(g, x, y, ...) Begin");
  if(g==NULL) return 0;
  int ret;
  va_list stuff;
  va_start(stuff, text);
  char buf[4096];
  bzero(buf, 4096);
  vsprintf(buf, text, stuff);
  ret = GPrint(g, x, y, cb, cf, buf);
  va_end(stuff);
  Debug("User::Screen::Printf(g, x, y, ...) End");
  return ret;
  }

int Screen::Printf(int x, int y, color cb, color cf, const char *text, ...) {
  Debug("User::Screen::Printf(x, y, ...) Begin");
  TGotoXY(x, y);
  int ret;
  va_list stuff;
  va_start(stuff, text);
  char buf[4096];
  bzero(buf, 4096);
  vsprintf(buf, text, stuff);
  ret = Print(cb, cf, buf);
  va_end(stuff);
  Debug("User::Screen::Printf(x, y, ...) End");
  return ret;
  }

int Screen::Printf(color cb, color cf, const char *text, ...)  {
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

int Screen::GPrint(Graphic *g, int x, int y, color cb, color cf,
	const char *text)  {
  if(g==NULL) return 0;
  ConvertColor(cb, appdepth, (int)g->depth);
  ConvertColor(cf, appdepth, (int)g->depth);
  Debug("User::Screen::Print(...) Begin");
  if(font[' '] == NULL)  Exit(-1, "Must Screen.SetFont before Screen.Print!\n");
  unsigned char *ind = (unsigned char *)text;
//  printf("%s\n", text);
  for(;(*ind) != 0; ind++)  {
//    printf("%d,%d:%d: ", TXPos(), TYPos(), *ind); fflush(stdout);

    if((*ind) == (unsigned char)'\t') (*ind)=' ';
    if((*ind) == (unsigned char)CHAR_LEFT)  {
      x-=font[' ']->xsize;
      AlignCursor();
      }
    else if((*ind) == (unsigned char)CHAR_RIGHT)  {
      x+=font[' ']->xsize;
      AlignCursor();
      }
    else if((*ind) == (unsigned char)CHAR_UP)  {
      y-=(font[' ']->ysize+2);
      AlignCursor();
      }
    else if((*ind) == (unsigned char)CHAR_DOWN)  {
      y+=(font[' ']->ysize+2);
      AlignCursor();
      }
    else if((*ind) == (unsigned char)'\n')  {
      x=1;
      y+=(font[' ']->ysize+2);
      AlignCursor();
      }
    else if((*ind) == (unsigned char)'\r')  {
      x=1;
      AlignCursor();
      }
    else if(font[*ind] != NULL)  {
      if(((int)x + (int)font[*ind]->xsize + (int)font[*ind]->xcenter)
                >= xsize)  {
        x=1;
        y+=(font[' ']->ysize+2);
        AlignCursor();
        }
      int ctrx, ctry;
      Graphic let(*font[*ind]);
      Graphic res;
      res.depth=depth;
      res.DefSize(let.xsize, let.ysize);
      if(depth==8) {
	for(ctrx=0; ctrx<(int)let.xsize; ctrx++)  {
	  for(ctry=0; ctry<(int)let.ysize; ctry++)  {
	     if(let.image[ctry].uc[ctrx] == 0) res.image[ctry].uc[ctrx] = cb;
	     else res.image[ctry].uc[ctrx] = cf;
	     }
	  }
	g->PasteTransparentGraphic(res, x-let.xcenter, y-let.ycenter);
	}
      else if(depth==32) {
	unsigned long alpha;
	for(ctrx=0; ctrx<(int)let.xsize; ctrx++)  {
	  for(ctry=0; ctry<(int)let.ysize; ctry++)  {
	    alpha=let.image[ctry].uc[ctrx];
	    if(alpha) {
	      res.image[ctry].ul[ctrx] = cf;
	      alpha*=((unsigned char*)&cf)[3]; alpha/=255;
	      res.image[ctry].uc[(ctrx<<2)+3] = alpha;
	      }
	    else res.image[ctry].ul[ctrx]=0;
	    }
	  }
	g->PasteTransparentGraphic(res, x-let.xcenter, y-let.ycenter);
	}
      else if(depth==16) {
	for(ctrx=0; ctrx<(int)let.xsize; ctrx++)  {
	  for(ctry=0; ctry<(int)let.ysize; ctry++)  {
	    color cl, clf = cf, clb = cb;
	    unsigned long alpha = let.image[ctry].uc[ctrx];
	    ConvertColor(clf, 16, 32);
	    ConvertColor(clb, 16, 32);
	    ((unsigned char*)&cl)[0] =
	      (((unsigned char*)&clf)[0]*alpha 
		+ ((unsigned char*)&clb)[0]*(255-alpha))/255;
	    ((unsigned char*)&cl)[1] =
	      (((unsigned char*)&clf)[1]*alpha 
		+ ((unsigned char*)&clb)[1]*(255-alpha))/255;
	    ((unsigned char*)&cl)[2] =
	      (((unsigned char*)&clf)[2]*alpha 
		+ ((unsigned char*)&clb)[2]*(255-alpha))/255;
	    ((unsigned char*)&cl)[3] = 255;
	    ConvertColor(cl, 32, 16);
	    res.image[ctry].us[ctrx] = cl;
	    }
	  }
	g->PasteGraphic(res, x-let.xcenter, y-let.ycenter);
//	DrawTransparentGraphic(res, x-let.xcenter, y-let.ycenter);
	}
      else Exit(-1, "Unknown depth error (%d) in %s\n", depth, __PRETTY_FUNCTION__);

      x+=font[*ind]->xsize+1;
      AlignCursor();
      }
    }
  Debug("User::Screen::Print(...) End");
  return x;
  }

int Screen::Print(int x, int y, color cb, color cf, const char *text)  {
  TGotoXY(x, y);
  return(Print(cb, cf, text));
  }

int Screen::Print(color cb, color cf, const char *text)  {
  ConvertColor(cb, appdepth, depth);
  ConvertColor(cf, appdepth, depth);
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
    else if((*ind) == (unsigned char)CHAR_LEFT)  {
      tcx-=font[' ']->xsize;
      AlignCursor();
      }
    else if((*ind) == (unsigned char)CHAR_RIGHT)  {
      tcx+=font[' ']->xsize;
      AlignCursor();
      }
    else if((*ind) == (unsigned char)CHAR_UP)  {
      tcy-=(font[' ']->ysize+2);
      AlignCursor();
      }
    else if((*ind) == (unsigned char)CHAR_DOWN)  {
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
      Graphic res;
      res.depth=depth;
      res.DefSize(let.xsize, let.ysize);
      if(depth==8) {
	for(ctrx=0; ctrx<(int)let.xsize; ctrx++)  {
	  for(ctry=0; ctry<(int)let.ysize; ctry++)  {
	     if(let.image[ctry].uc[ctrx] == 0) res.image[ctry].uc[ctrx] = cb;
	     else res.image[ctry].uc[ctrx] = cf;
	     }
	  }
	DrawGraphic(res, tcx-let.xcenter, tcy-let.ycenter);
	}
      else if(depth==32) {
	unsigned long alpha;
	for(ctrx=0; ctrx<(int)let.xsize; ctrx++)  {
	  for(ctry=0; ctry<(int)let.ysize; ctry++)  {
	    alpha=let.image[ctry].uc[ctrx];
	    if(alpha) {
	      res.image[ctry].ul[ctrx] = cf;
	      alpha*=((unsigned char*)&cf)[3]; alpha/=255;
	      res.image[ctry].uc[(ctrx<<2)+3] = alpha;
	      }
	    else res.image[ctry].ul[ctrx]=0;
	    }
	  }
	DrawTransparentGraphic(res, tcx-let.xcenter, tcy-let.ycenter);
	}
      else if(depth==16) {
	for(ctrx=0; ctrx<(int)let.xsize; ctrx++)  {
	  for(ctry=0; ctry<(int)let.ysize; ctry++)  {
	    color cl, clf = cf, clb = cb;
	    unsigned long alpha = let.image[ctry].uc[ctrx];
	    ConvertColor(clf, 16, 32);
	    ConvertColor(clb, 16, 32);
	    ((unsigned char*)&cl)[0] =
	      (((unsigned char*)&clf)[0]*alpha 
		+ ((unsigned char*)&clb)[0]*(255-alpha))/255;
	    ((unsigned char*)&cl)[1] =
	      (((unsigned char*)&clf)[1]*alpha 
		+ ((unsigned char*)&clb)[1]*(255-alpha))/255;
	    ((unsigned char*)&cl)[2] =
	      (((unsigned char*)&clf)[2]*alpha 
		+ ((unsigned char*)&clb)[2]*(255-alpha))/255;
	    ((unsigned char*)&cl)[3] = 255;
	    ConvertColor(cl, 32, 16);
	    res.image[ctry].us[ctrx] = cl;
	    }
	  }
	DrawTransparentGraphic(res, tcx-let.xcenter, tcy-let.ycenter);
	}
      else Exit(-1, "Unknown depth error (%d) in %s\n", depth, __PRETTY_FUNCTION__);

      tcx+=font[*ind]->xsize+1;
      AlignCursor();
      }
    }
  if(TCursor != NULL)  TCursor->Move(tcx, tcy);
  Debug("User::Screen::Print(...) End");
  return tcx;
  }

void Screen::TGotoXY(int x, int y) {
  tcx=x; tcy=y; AlignCursor();
  }

Sprite *Screen::GetSpriteByNumber(int n) {
  return sprites[n];
  }

Panel Screen::NewPanel(int x1, int y1, int x2, int y2) {
  int ctr;
  for(ctr=1; ctr<MAX_PANELS && pxs[ctr] != -1; ctr++);
  if(ctr >= MAX_PANELS) Exit(-1, "Out of Panels!\n");
  pxs[ctr] = x1; pxe[ctr] = x2;
  pys[ctr] = y1; pye[ctr] = y2;
  return ctr;
  }

void Screen::RemovePanel(Panel p) {
  if(pxs[0] == -1) Exit(-1, "Deleting non-existant panel!\n");
  pxs[p] = -1; pxe[p] = -1;
  pys[p] = -1; pye[p] = -1;
  }

Panel Screen::WhichPanel(int x, int y) {
  int ctr, ret=0;
  for(ctr=1; ctr<MAX_PANELS; ctr++)  {
    if(x>=pxs[ctr] && y>=pys[ctr] && x<pxe[ctr] && y<pye[ctr]) ret=ctr;
    }
  return ret;
  }

Palette &Screen::GetPalette() {
  return *pal;
  }

#ifdef DOS
void Screen::SetBank(int bank) {
  if(curbank == bank)  return;
  curbank = bank;
  switch(vtype) {
    case(VIDEO_VBE2L): { Exit(-1, "Banking with LFB???\n"); } break;
    case(VIDEO_VBE2):
//    {
//      vbe2_bank(bank);
//      }break;
    case(VIDEO_DOS): {
      __dpmi_regs regs;
      regs.x.ax = 0x4F05;
      regs.x.bx = 0x0000;
      regs.x.dx = curbank * (64 / vminfo.WinGranularity);
      __dpmi_int(0x10, &regs);
      }break;
    }
  }
#endif

int Screen::DefaultXSize() {
#ifdef X_WINDOWS
  if(vtype == VIDEO_XF86DGA)  {
    int x, y;
    XF86DGAGetViewPortSize(_Xdisplay, _Xscreen, &x, &y);
    return x;
    }
  else {
#endif
    return 0;
#ifdef X_WINDOWS
    }
#endif
 }

int Screen::DefaultYSize() {
#ifdef X_WINDOWS
  if(vtype == VIDEO_XF86DGA)  {
    int x, y;
    XF86DGAGetViewPortSize(_Xdisplay, _Xscreen, &x, &y);
    return y;
    }
  else {
#endif
    return 0;
#ifdef X_WINDOWS
    }
#endif
 }

void Screen::DropSprite(Sprite *s) {
  Debug("Screen::DropSprite(s) Begin");
  if((!bins) || (!lbins)) Exit(1, "No bins in Dropsprite!\n");
  Sprite *sp;
  if(s->Flag(SPRITE_HUGE)) {
    Debug("Screen::DropSprite(s) In large");
    if(huges==NULL) { huges=s; s->prev = &huges; }
    else {
      for(sp=huges; sp->next != NULL; sp=sp->next);
      sp->next = s; s->prev = &(sp->next);
      }
    }
  else if(s->Flag(SPRITE_LARGE)) {
    Debug("Screen::DropSprite(s) In lbin");
    int x=(s->xpos)>>LARGE_BIN_FACTOR, y=(s->ypos)>>LARGE_BIN_FACTOR;
    if(x<0) x=0; if(y<0) y=0;
    if(x>=(xsize>>LARGE_BIN_FACTOR)) x=(xsize>>LARGE_BIN_FACTOR)-1;
    if(y>=(ysize>>LARGE_BIN_FACTOR)) y=(ysize>>LARGE_BIN_FACTOR)-1;
    if(lbins[x][y]==NULL) { lbins[x][y]=s; s->prev = &lbins[x][y]; }
    else {
      for(sp=(lbins[x][y]); sp->next != NULL; sp=sp->next);
      sp->next = s; s->prev = &(sp->next);
      }
    }
  else {
    Debug("Screen::DropSprite(s) In bin");
    int x=(s->xpos)>>BIN_FACTOR, y=(s->ypos)>>BIN_FACTOR;
    if(x<0) x=0; if(y<0) y=0;
    if(x>=(xsize>>BIN_FACTOR)) x=(xsize>>BIN_FACTOR)-1;
    if(y>=(ysize>>BIN_FACTOR)) y=(ysize>>BIN_FACTOR)-1;
    if(bins[x][y]==NULL) { bins[x][y]=s; s->prev = &bins[x][y]; }
    else {
      for(sp=(bins[x][y]); sp->next != NULL; sp=sp->next);
      sp->next = s; s->prev = &(sp->next);
      }
    }
  Debug("Screen::DropSprite(s) End");
  }

void Screen::LiftSprite(Sprite *s) {
  Debug("Screen::LiftSprite(s) Begin");
  if(!s->prev) Exit(1, "Lifting non-placed Sprite (%d)!!\n", s->drawn);
  if(s->next) s->next->prev = s->prev;
  *(s->prev) = s->next; s->next = NULL; s->prev=NULL;
  Debug("Screen::LiftSprite(s) End");
  }

