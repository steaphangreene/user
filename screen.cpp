// *************************************************************************
// screen.cpp
// Advanced Screen class, Pre ALPHA non-distribution version
//
// -By Insomnia (Steaphan Greene)   (Copyright 1997-2001 Steaphan Greene)
//                   (insomnia@core.binghamton.edu)
//      No waranty stated or implied, I am not responsible for any damage
// caused directly or indirectly by this software.
//      Permision granted for use/distribution/modification by anyone,
// provided this header remains intact, and modified versions are marked
// as so immediately below this header.
//      Products utilizing the code or methods within may be distributed
// freely along with this licence, but any sales for profit of such products
// must have the author's permission, and may be subject to a royaltee fee.
// *************************************************************************

#include "config.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <stdarg.h>
#include <algorithm>
using std::min;
using std::max;

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
//  UserDebug("User::Screen::~Screen() Begin");
  if(__Da_Screen != this) { __Da_Screen = NULL; U2_Exit(1, "Phantom Screen!\n"); }
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
    #ifdef X_DGA2
    case(VIDEO_XDGA2): {
      XDGACloseFramebuffer(_Xdisplay, _Xscreen);
      }break;
    #endif
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
//  UserDebug("User::Screen::~Screen() End");
  }

Screen::Screen(const char *n)  {
  name=n;
  Init();
  }

Screen::Screen(int x, int y, const char *n)  {
  name=n;
  Init();
  SetSize(x, y);
  }

Screen::Screen(int x, int y, int d, const char *n)  {
  name=n;
  Init();
  SetApparentDepth(d);
  SetSize(x, y);
  }

void Screen::Init()  {
  UserDebug("User::Screen::Init() 1000");
  if(__Da_Screen != NULL)  U2_Exit(1, "One screen at a time!\n");
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
  xbins = 0;
  ybins = 0;
  xlbins = 0;
  ylbins = 0;
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

  UserDebug("User::Screen::Init() 1001");
  switch(vtype)  {
    #ifdef DOS
    case(VIDEO_DOS): {
      depth = 8;
      }
    #endif

    #ifdef X_WINDOWS
    case(VIDEO_XWINDOWS): {
      rowlen = xsize;
      UserDebug("User::Screen::Init() 1005");
      _Xscreen = DefaultScreen(_Xdisplay);
      depth = DefaultDepth(_Xdisplay, _Xscreen);
//      printf("Colordepth = %d\n", depth);
      if(depth == 24) depth = 32;

      UserDebug("User::Screen::Init() 1100");
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

      UserDebug("User::Screen::Init() 1200");
      _Xwmhints = XAllocWMHints();
      _Xwmhints->flags = InputHint;
      _Xwmhints->input = True;
      }break;
    #ifdef XF86_DGA
    #ifdef X_DGA2
    case(VIDEO_XDGA2): {
      UserDebug("User::Screen::Init() 1250");
      _Xscreen = DefaultScreen(_Xdisplay);
      depth = DefaultDepth(_Xdisplay, _Xscreen);
      if(depth == 24) depth = 32;
      }break;
    #endif
    case(VIDEO_XF86DGA): {
      UserDebug("User::Screen::Init() 1300");
      _Xscreen = DefaultScreen(_Xdisplay);
      depth = DefaultDepth(_Xdisplay, _Xscreen);
      if(depth == 24) depth = 32;
      }break;
    #endif
    #endif
    }
  appdepth = depth;
  pal->depth = depth;
  UserDebug("User::Screen::Init() 1400");
  }


int Screen::SetSize(int x, int y)  {
  UserDebug("User::Screen::SetSize() Begin"); 
  int ctr, ctr2;

  if(bins!=NULL) {
    for(ctr=0; ctr<xbins; ++ctr) {
      delete bins[ctr];
      }
    delete bins;
    }
  if(lbins!=NULL) {
    for(ctr=0; ctr<xlbins; ++ctr) {
      delete lbins[ctr];
      }
    delete lbins;
    }
  xsize = x; ysize = y;
  xbins = (xsize+BIN_SIZE-1)>>BIN_FACTOR;
  ybins = (ysize+BIN_SIZE-1)>>BIN_FACTOR;
  xlbins = (xsize+LARGE_BIN_SIZE-1)>>LARGE_BIN_FACTOR;
  ylbins = (ysize+LARGE_BIN_SIZE-1)>>LARGE_BIN_FACTOR;

  pxs[0] = 0; pys[0] = 0;
  pxe[0] = x; pye[0] = y;
  bins = new Sprite**[xbins];
  if(!bins) U2_Exit(1, "Insufficient memory!\n");
  for(ctr=0; ctr<xbins; ++ctr) {
    bins[ctr] = new Sprite*[ybins];
    if(!bins[ctr]) U2_Exit(1, "Insufficient memory!\n");
    for(ctr2=0; ctr2<ybins; ++ctr2) {
      bins[ctr][ctr2] = NULL;
      }
    }
  lbins = new Sprite**[xlbins];
  if(!lbins) U2_Exit(1, "Insufficient memory!\n");
  for(ctr=0; ctr<xlbins; ++ctr) {
    lbins[ctr] = new Sprite*[ylbins];
    if(!lbins[ctr]) U2_Exit(1, "Insufficient memory!\n");
    for(ctr2=0; ctr2<ylbins; ++ctr2) {
      lbins[ctr][ctr2] = NULL;
      }
    }

  if(depth==8)  {
    video_buffer.u8 = new unsigned char[ysize*xsize];
    background_buffer.u8 = new unsigned char[ysize*xsize];
    image = new mfmt[ysize];
    backg = new mfmt[ysize];
    for(ctr=0; ctr<ysize; ctr++)  {
      image[ctr].u8 = &video_buffer.u8[xsize*ctr];
      backg[ctr].u8 = &background_buffer.u8[xsize*ctr];
      for(ctr2=0; ctr2<xsize; ctr2++)  {
//	image[ctr].u8[ctr2] = BlackPixel(_Xdisplay, 0);
//	backg[ctr].u8[ctr2] = BlackPixel(_Xdisplay, 0);
	image[ctr].u8[ctr2] = 0;
	backg[ctr].u8[ctr2] = 0;
	}
      }
    }
  else if(depth==32)  {
    video_buffer.u32 = new unsigned int[ysize*xsize];
    background_buffer.u32 = new unsigned int[ysize*xsize];
    image = new mfmt[ysize];
    backg = new mfmt[ysize];
    for(ctr=0; ctr<ysize; ctr++)  {
      image[ctr].u32 = &video_buffer.u32[xsize*ctr];
      backg[ctr].u32 = &background_buffer.u32[xsize*ctr];
      for(ctr2=0; ctr2<xsize; ctr2++)  {
//	image[ctr].u32[ctr2] = BlackPixel(_Xdisplay, 0);
//	backg[ctr].u32[ctr2] = BlackPixel(_Xdisplay, 0);
	image[ctr].u32[ctr2] = 0;
	backg[ctr].u32[ctr2] = 0;
	}
      }
    }
  else if(depth==16)  {
    video_buffer.u16 = new unsigned short[ysize*xsize];
    background_buffer.u16 = new unsigned short[ysize*xsize];
    image = new mfmt[ysize];
    backg = new mfmt[ysize];
    for(ctr=0; ctr<ysize; ctr++)  {
      image[ctr].u16 = &video_buffer.u16[xsize*ctr];
      backg[ctr].u16 = &background_buffer.u16[xsize*ctr];
      for(ctr2=0; ctr2<xsize; ctr2++)  {
//	image[ctr].u16[ctr2] = BlackPixel(_Xdisplay, 0);
//	backg[ctr].u16[ctr2] = BlackPixel(_Xdisplay, 0);
	image[ctr].u16[ctr2] = 0;
	backg[ctr].u16[ctr2] = 0;
	}
      }
    }
  else U2_Exit(-1, "Unknown depth error (%d) in %s\n", depth, __PRETTY_FUNCTION__);

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
	  if(regs.x.ax != 0x004F) U2_Exit(-1, "VESA init failed!\n");
	  dosmemget(__tb, sizeof(VESAInfo), &vinfo);
	  if(strncmp(vinfo.VESASignature,"VESA",4))
		U2_Exit(-1, "No VESA detected!\n");
	  short vmode = 0;
	  int ctr;
	  UserDebug("User::Screen::SetSize() Before Get Modes"); 
	  unsigned long vlist=(((unsigned long)vinfo.VideoModePtr)>>12) & 0xF0000;
	  vlist |= ((unsigned long)vinfo.VideoModePtr & 0xFFFF);
	  for(numvesamodes=0; vmode != -1; numvesamodes++) {
	    dosmemget((numvesamodes<<1)+vlist, 2, &vmode);
	    }
	  --numvesamodes;
	  UserDebug("User::Screen::SetSize() After Get Modes"); 
	  vesamode = new short[numvesamodes];
	  vesax = new long[numvesamodes];
	  vesay = new long[numvesamodes];
	  vesad = new long[numvesamodes];
	  dosmemget(vlist, 2*numvesamodes, vesamode);
	  UserDebug("User::Screen::SetSize() After Get Mode List"); 
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
	  UserDebug("User::Screen::SetSize() After Get Mode Info"); 
	  }
	int ctr;
	int mode = -1;
	for(ctr=0; ctr<numvesamodes; ctr++)  {
	  if(vesad[ctr] == depth && vesax[ctr] >= xsize && vesay[ctr] >= ysize
		&& (mode==-1
		|| (vesax[ctr] <= vesax[mode] && vesay[ctr] <= vesay[mode])))
	    mode=ctr;
	  }
	if(mode==-1) U2_Exit(-1, "No VESA %d-bit mode compatible with %dx%d!\n",
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
		U2_Exit(-1, "VESA Mode 0x%X Failed!\n", vesamode[mode]);

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
	  if(regs.x.ax!=0x004F) U2_Exit(-1,"VBE2: Error getting PM interface!\n");
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
		U2_Exit(-1, "VESA Mode 0x%X Failed!\n", vesamode[mode]);

	    __dpmi_meminfo mi;
	    mi.size = (unsigned long)(vminfo.XResolution * vminfo.YResolution);
	    mi.address = vminfo.PhysBasePtr;
	    __dpmi_physical_address_mapping(&mi);
	    frame.UL = __dpmi_allocate_ldt_descriptors(1);
	    __dpmi_set_segment_base_address(frame.UL, mi.address);
	    __dpmi_set_segment_limit(frame.UL, rowlen*ysize);
//	    U2_Exit(0, "Linear frame buffer found at %X\n", vminfo.PhysBasePtr);
	    }
//	  else U2_Exit(-1, "Linear frame buffer not found\n");
//	  U2_Exit(0, "VESA 2.0!\n");
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
		(char*)(video_buffer.u8), xsize, ysize, 8, 0);
	}
      else if(depth==16)  {
	_Ximage = XCreateImage(_Xdisplay, None,
		DefaultDepth(_Xdisplay, _Xscreen), ZPixmap, 0,
		(char*)(video_buffer.u8), xsize, ysize, 16, 0);
	}
      else if(depth==32)  {
	_Ximage = XCreateImage(_Xdisplay, None,
		DefaultDepth(_Xdisplay, _Xscreen), ZPixmap, 0,
		(char*)(video_buffer.u8), xsize, ysize, 32, 0);
	}
      else U2_Exit(-1, "Unknown depth error (%d) in %s\n", depth, __PRETTY_FUNCTION__);

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
    #ifdef X_DGA2
    case(VIDEO_XDGA2): {
      XDGADevice *DGAdev; XDGAMode *modes;  int ctr, numm, mode=-1;
      modes = XDGAQueryModes(_Xdisplay, _Xscreen, &numm);
      int targetdepth = appdepth;
      if(appdepth == 32) targetdepth = 24;
      for(ctr=0; ctr<numm; ctr++)  {
        if(modes[ctr].depth == targetdepth
		&& modes[ctr].viewportWidth >= xsize
		&& modes[ctr].viewportHeight >= ysize) {
          mode = ctr;
	  break;
	  }
        }
      if(mode == -1) U2_Exit(0, "No DGA Mode appropriate!\n");
      for(; ctr<numm; ctr++)  {
        if(modes[ctr].depth == targetdepth
		&& modes[ctr].viewportWidth >= xsize
		&& modes[ctr].viewportHeight >= ysize
		&& modes[ctr].viewportWidth <= modes[mode].viewportWidth
		&& modes[ctr].viewportHeight <= modes[mode].viewportHeight) {
          mode = ctr;
	  }
        }
      rowlen = modes[mode].bytesPerScanline * 8 / modes[mode].bitsPerPixel;
// *************************************
//	Radeon HACK!!!!  REMOVE THIS!!!!
      rowlen = 1600;
// *************************************
      DGAFlags = modes[mode].flags;
      collen = modes[mode].viewportHeight;
      DGAdev = XDGASetMode(_Xdisplay, _Xscreen, modes[mode].num);
      frame.u8 = DGAdev->data;
      depth = appdepth;
      pal->depth = appdepth;
      XDGASetViewport(_Xdisplay, _Xscreen, 0, 0, 0);
      if(XGrabKeyboard(_Xdisplay, DefaultRootWindow(_Xdisplay), True,
		GrabModeAsync, GrabModeAsync,  CurrentTime) != GrabSuccess) {
	fprintf(stderr, "Failed to grab keyboard!\n");
	}
      if(XGrabPointer(_Xdisplay, DefaultRootWindow(_Xdisplay), True,
		PointerMotionMask | ButtonPressMask | ButtonReleaseMask,
		GrabModeAsync, GrabModeAsync, None,  None, CurrentTime)
		!= GrabSuccess) {
	fprintf(stderr, "Failed to grab mouse!\n");
	}
      if(depth == 8)  {
	_Xmap = XDGACreateColormap(_Xdisplay, _Xscreen, DGAdev, AllocAll);
	}
      for(ctr=0; ctr<modes[mode].viewportHeight; ctr++) {
	memset(frame.u8+((rowlen * modes[mode].bitsPerPixel / 8)*ctr), 0, 
		modes[mode].viewportWidth * modes[mode].bitsPerPixel / 8);
	}
      frame.u8 += (modes[mode].viewportWidth - xsize) * modes[mode].bitsPerPixel / 16;
      frame.u8 += (modes[mode].viewportHeight - ysize) * modes[mode].bytesPerScanline / 2;
      delete(modes);
      }break;
    #endif
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
	  memset(frame.u8+rowlen*ctr, 0, xsize);
	}
      else if(depth == 32)  {
	for(ctr=0; ctr<collen; ctr++)
	  memset(frame.u32+rowlen*ctr, 0, xsize<<2);
	}
      else if(depth == 16)  {
	for(ctr=0; ctr<collen; ctr++)
	  memset(frame.u8+rowlen*ctr, 0, xsize<<1);
	}
      else U2_Exit(-1, "Unknown depth error (%d) in %s\n", depth, __PRETTY_FUNCTION__);
      }break;
    #endif
    #endif
    }
  if(__Da_Mouse != NULL)  __Da_Mouse->SetRange(0, 0, xsize, ysize);
  UserDebug("User::Screen::SetSize() End"); 
  return 1;
  }

void Screen::Hide()  {
  }

void Screen::Show()  {
  if(shown) return;
  shown = 1;
  UserDebug("User::Screen::Show() Entry");

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
    #ifdef X_DGA2
    case(VIDEO_XDGA2): {
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
	XDGAInstallColormap(_Xdisplay, _Xscreen, _Xmap);
	}
      RefreshFull();
      }break;
    #endif
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
  UserDebug("User::Screen::Show() End");
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
  UserDebug("User::Screen::RefreshFast Begin");
  if(__Da_Speaker != NULL) __Da_Speaker->Update();
  if(__Da_InputQueue != NULL)  __Da_InputQueue->Update();
  else {
    if(__Da_Mouse != NULL) __Da_Mouse->Update();
    }
  if(!shown) return;
  int ctrb;
//  memset(frame.u8, 0, rowlen * ysize);
  for(ctrb=0; ctrb<REDRAW_RECTS; ctrb++)  {
    if(rxs[ctrb] != -1) {
      switch(vtype)  {
	#ifdef X_WINDOWS
	case(VIDEO_XWINDOWS): {
	  XPutImage(_Xdisplay, _Xwindow, _Xgc, _Ximage, rxs[ctrb], rys[ctrb],
	      rxs[ctrb], rys[ctrb], rxe[ctrb]-rxs[ctrb], rye[ctrb]-rys[ctrb]);
	  }break;
	#ifdef XF86_DGA
	#ifdef X_DGA2
	case(VIDEO_XDGA2):
	#endif
	case(VIDEO_XF86DGA): {
	  int ctry;
	  if(depth == 8) {
	    for(ctry=rys[ctrb]; ctry<rye[ctrb]; ctry++) {
	      memcpy(frame.u8 + ctry*rowlen + rxs[ctrb],
		image[ctry].u8 + rxs[ctrb],
		rxe[ctrb]-rxs[ctrb]);
	      }
	    }
	  else if(depth == 16) {
	    for(ctry=rys[ctrb]; ctry<rye[ctrb]; ctry++) {
	      memcpy(frame.u16 + ctry*rowlen + rxs[ctrb],
		image[ctry].u16 + rxs[ctrb],
		(rxe[ctrb]-rxs[ctrb])<<1);
	      }
	    }
	  else if(depth == 32) {
	    for(ctry=rys[ctrb]; ctry<rye[ctrb]; ctry++) {
	      memcpy(frame.u32 + ctry*rowlen + rxs[ctrb],
		image[ctry].u32 + rxs[ctrb],
		(rxe[ctrb]-rxs[ctrb])<<2);
	      }
	    }
	  else U2_Exit(-1, "Unknown depth error (%d) in %s\n", depth, __PRETTY_FUNCTION__);
	  }break;
	#endif
	#endif

	#ifdef DOS
	case(VIDEO_VBE2):
	case(VIDEO_VESA): {
	  U2_Exit(-1, "VESA w/o liner buffer and quick refresh not implemented!\n");
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
	  else U2_Exit(-1, "Unknown depth error (%d) in %s\n", depth, __PRETTY_FUNCTION__);
	  }break;
	case(VIDEO_DOS): {
	  int ctry;
	  if(depth == 8) {
	    for(ctry=rys[ctrb]; ctry<rye[ctrb]; ctry++) {
	      dosmemput(image[ctry].u8+rxs[ctrb], rxe[ctrb]-rxs[ctrb],
		frame.UL+ctry*rowlen+rxs[ctrb]);
	      }
	    }
	  else U2_Exit(-1, "Unknown depth error (%d) in %s\n", depth, __PRETTY_FUNCTION__);
	  }break;
	#endif
	}
      rxs[ctrb] = -1;
      rxe[ctrb] = -1;
      rys[ctrb] = -1;
      rye[ctrb] = -1;
      }
    }
  UserDebug("User::Screen::RefreshFast End");
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
      UserDebug("User::Screen::RefreshFull() 1000");
      XPutImage(_Xdisplay, _Xwindow, _Xgc, _Ximage, 0, 0, 0, 0, xsize, ysize);
      }break;
    #ifdef XF86_DGA
    #ifdef X_DGA2
    case(VIDEO_XDGA2):
    #endif
    case(VIDEO_XF86DGA): {
      int ctry;
      if(depth == 8) {
	for(ctry=0; ctry<ysize; ctry++) {
	  memcpy(frame.u8 + ctry*rowlen, image[ctry].u8, xsize);
	  }
	}
      else if(depth == 32) {
	for(ctry=0; ctry<ysize; ctry++) {
	  memcpy(frame.u32 + ctry*rowlen, image[ctry].u32, xsize<<2);
	  }
	}
      else if(depth == 16) {
	for(ctry=0; ctry<ysize; ctry++) {
	  memcpy(frame.u16 + ctry*rowlen, image[ctry].u16, xsize<<1);
	  }
	}
      else U2_Exit(-1, "Unknown depth error (%d) in %s\n", depth, __PRETTY_FUNCTION__);
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
	  dosmemput(&video_buffer.u8[bank<<16], 65536, frame.UL);
	  }
	if((rowlen*ysize)&65535) {
	  SetBank(bank);
	  dosmemput(&video_buffer.u8[bank<<16], (rowlen*ysize)&65535, frame.UL);
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
	    dosmemput(image[ctry].u8, xsize, frame.UL+ctry*rowlen);
	    }
	  }
	else  {
	  dosmemput(video_buffer.u8, xsize*ysize, frame.UL);
	  }
	}
      }break;
    #endif
    }
  WaitForNextFrame();
  UserDebug("User::Screen::RefreshFull() 1100");
  }

void Screen::Clear(color c)  {
  int ctr, ctr2;
  if(depth==8) {
    for(ctr=0; ctr<ysize; ctr++)  {
      memset(image[ctr].u8, c, xsize);
      memset(backg[ctr].u8, c, xsize);
      }
    }
  else if(depth==16)  {
    for(ctr=0; ctr<ysize; ctr++)  {
      for(ctr2=0; ctr2<xsize; ctr2++)  {
	image[ctr].u16[ctr2] = c;
	backg[ctr].u16[ctr2] = c;
	}
      }
    }
  else if(depth==32)  {
    for(ctr=0; ctr<ysize; ctr++)  {
      for(ctr2=0; ctr2<xsize; ctr2++)  {
	image[ctr].u32[ctr2] = c;
	backg[ctr].u32[ctr2] = c;
	}
      }
    }
  InvalidateRectangle(0, 0, xsize, ysize);
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

void Screen::ClearArea(int x, int y, int xs, int ys, color c)  {
  int ctr, ctr2;
  if(depth==8) {
    for(ctr=y; ctr<(y+ys); ctr++)  {
      memset(&(image[ctr].u8[x]), c, xs);
      memset(&(backg[ctr].u8[x]), c, xs);
      }
    }
  else if(depth==16)  {
    for(ctr=y; ctr<y+ys; ctr++)  {
      for(ctr2=x; ctr2<x+xs; ctr2++)  {
	image[ctr].u16[ctr2] = c;
	backg[ctr].u16[ctr2] = c;
	}
      }
    }
  else if(depth==32)  {
    for(ctr=y; ctr<y+ys; ctr++)  {
      for(ctr2=x; ctr2<x+xs; ctr2++)  {
	image[ctr].u32[ctr2] = c;
	backg[ctr].u32[ctr2] = c;
	}
      }
    }
  else U2_Exit(-1, "Unknown depth error (%d) in %s\n", depth, __PRETTY_FUNCTION__);
  InvalidateRectangle(x, y, xs, ys);
  }

void Screen::DrawRectangle(int x, int y, int xs, int ys, color c)  {
  ConvertColor(c, appdepth, depth);
  InvalidateRectangle(x, y, xs, ys);
  int ctrx, ctry;
  if(depth==8) {
    for(ctry=y; ctry<(y+ys); ctry++)  {
      memset(&(image[ctry].u8[x]), c, xs);
      memset(&(backg[ctry].u8[x]), c, xs);
      }
    }
  else if(depth==32) {
    for(ctry=y; ctry<(y+ys); ctry++)  {
      for(ctrx=x; ctrx<(x+xs); ctrx++)  {
	image[ctry].u32[ctrx] = c;
	backg[ctry].u32[ctrx] = c;
	}
      }
    }
  else if(depth==16) {
    for(ctry=y; ctry<(y+ys); ctry++)  {
      for(ctrx=x; ctrx<(x+xs); ctrx++)  {
	image[ctry].u16[ctrx] = c;
	backg[ctry].u16[ctrx] = c;
	}
      }
    }
  else U2_Exit(-1, "Unknown depth error (%d) in %s\n", depth, __PRETTY_FUNCTION__);
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
  if(pal && appdepth == 8 && depth != 8) {
    c = GetColor(pal->GetRedEntry(c), pal->GetGreenEntry(c), pal->GetBlueEntry(c));
    }
  else {
    ConvertColor(c, appdepth, depth);
    }
  InvalidateRectangle(x, y, 1, 1);
  if(depth==8)  {
    image[y].u8[x] = c;
    backg[y].u8[x] = c;
    }
  else if(depth==32)  {
    image[y].u32[x] = c;
    backg[y].u32[x] = c;
    }
  else if(depth==16)  {
    image[y].u16[x] = c;
    backg[y].u16[x] = c;
    }
  }

void Screen::SetPointFG(int x, int y, color c)  {
  if(pal && appdepth == 8 && depth != 8) {
    c = GetColor(pal->GetRedEntry(c), pal->GetGreenEntry(c), pal->GetBlueEntry(c));
    }
  else {
    ConvertColor(c, appdepth, depth);
    }
  InvalidateRectangle(x, y, 1, 1);
  if(depth==8)  {
    image[y].u8[x] = c;
    }
  else if(depth==32)  {
    image[y].u32[x] = c;
    }
  else if(depth==16)  {
    image[y].u16[x] = c;
    }
  }

void Screen::SetPoint(int x, int y, int r, int g, int b)  {
  InvalidateRectangle(x, y, 1, 1);
  }

void Screen::SetPointFG(int x, int y, int r, int g, int b)  {
  InvalidateRectangle(x, y, 1, 1);
  }

void Screen::FullScreenBMP(const char *fn) {
  pal->Set((char*)fn);
  Graphic g((char*)fn);
  FullScreenGraphic(g);
  }

void Screen::DrawPartialTransparentGraphicFG(Graphic &g, int x, int y,
	int xb, int yb, int xs, int ys, Panel p)  {
//  printf("At (%d,%d), using (%d,%d)->(%d,%d)\n", x, y, xb, yb, xb+xs, yb+ys);
  if(x+g.xsize <= pxs[p] || y+g.ysize <= pys[p] || x >= pxe[p] || y >= pye[p])
	return;
  if(g.depth != depth)  {
    Graphic *g2 = new Graphic(g);
    g2->DepthConvert(depth, *pal);
    DrawPartialTransparentGraphicFG(*g2, x, y, xb, yb, xs, ys, p);
    delete g2;
    return;
    }
//  UserDebug("User:Screen:DrawPartialTransparentGraphicFG Middle");
  int ctrx, ctry;
  if(depth == 8)  {
//    UserDebug("User:Screen:DrawPartialTransparentGraphicFG Depth 8");
    for(ctry=max(yb, pys[p]-y); ctry<min(pye[p]-y, ys+yb); ctry++)  {
      for(ctrx=max(xb, pxs[p]-x); ctrx<min(pxe[p]-x, xs+xb); ctrx++)  {
	if(g.image[ctry].u8[ctrx] != g.tcolor)  {
	  image[ctry+y].u8[ctrx+x] = g.image[ctry].u8[ctrx];
	  }
	}
      }
    }
  else if(depth == 32)  {
//    UserDebug("User:Screen:DrawPartialTransparentGraphicFG Depth 32");
    for(ctry=max(yb, pys[p]-y); ctry<min(pye[p]-y, ys+yb); ctry++)  {
      for(ctrx=max(xb, pxs[p]-x); ctrx<min(pxe[p]-x, xs+xb); ctrx++)  {
	if(g.image[ctry].u8[(ctrx<<2)+3] == 0xFF)  {
	  image[ctry+y].u32[ctrx+x] = g.image[ctry].u32[ctrx];
	  }
	else if(g.image[ctry].u8[(ctrx<<2)+3])  {
	  int alpha = g.image[ctry].u8[(ctrx<<2)+3];
	  unsigned long r1 = image[ctry+y].u8[((ctrx+x)<<2)];
	  unsigned long g1 = image[ctry+y].u8[((ctrx+x)<<2)+1];
	  unsigned long b1 = image[ctry+y].u8[((ctrx+x)<<2)+2];
	  unsigned long r2 = g.image[ctry].u8[(ctrx<<2)];
	  unsigned long g2 = g.image[ctry].u8[(ctrx<<2)+1];
	  unsigned long b2 = g.image[ctry].u8[(ctrx<<2)+2];
          r1 *= (0xFF-alpha);  r2 *= alpha; r1 += r2; r1 /= 255;
          g1 *= (0xFF-alpha);  g2 *= alpha; g1 += g2; g1 /= 255;
          b1 *= (0xFF-alpha);  b2 *= alpha; b1 += b2; b1 /= 255;
	  image[ctry+y].u8[((ctrx+x)<<2)] = r1;
	  image[ctry+y].u8[((ctrx+x)<<2)+1] = g1;
	  image[ctry+y].u8[((ctrx+x)<<2)+2] = b1;
	  }
	}
      }
    }
  else if(depth == 16)  {
//    UserDebug("User:Screen:DrawPartialTransparentGraphicFG Depth 16");
    for(ctry=max(yb, pys[p]-y); ctry<min(pye[p]-y, ys+yb); ctry++)  {
      for(ctrx=max(xb, pxs[p]-x); ctrx<min(pxe[p]-x, xs+xb); ctrx++)  {
	if(g.image[ctry].u16[ctrx] != g.tcolor)  {
	  image[ctry+y].u16[ctrx+x] = g.image[ctry].u16[ctrx];
	  }
	}
      }
    }
  else U2_Exit(-1, "Unknown depth error (%d) in %s\n", depth, __PRETTY_FUNCTION__);
//  UserDebug("User:Screen:DrawPartialTransparentGraphicFG End");
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
//  UserDebug("User:Screen:DrawPartialGraphicFG Middle");
  int ctry;
  int ix = max(xb, pxs[p]-x), ex = min(pxe[p]-x, xs+xb);
  int sx = ex-ix;
  if(depth == 8)  {
//    UserDebug("User:Screen:DrawPartialGraphicFG Depth 8");
    for(ctry=max(yb, pys[p]-y); ctry<min(pye[p]-y, ys+yb); ctry++)  {
      memcpy(image[ctry+y].u8+x+ix, g.image[ctry].u8+ix, sx);
      }
    }
  else if(depth == 16)  {
//    UserDebug("User:Screen:DrawPartialGraphicFG Depth 16");
    for(ctry=max(yb, pys[p]-y); ctry<min(pye[p]-y, ys+yb); ctry++)  {
      memcpy(image[ctry+y].u16+x+ix, g.image[ctry].u16+ix, sx<<1);
      }
    }
  else if(depth == 32)  {
//    UserDebug("User:Screen:DrawPartialGraphicFG Depth 32");
    for(ctry=max(yb, pys[p]-y); ctry<min(pye[p]-y, ys+yb); ctry++)  {
      memcpy(image[ctry+y].u32+x+ix, g.image[ctry].u32+ix, sx<<2);
      }
    }
  else U2_Exit(-1, "Unknown depth error (%d) in %s\n", depth, __PRETTY_FUNCTION__);
//  UserDebug("User:Screen:DrawPartialGraphicFG End");
  }

void Screen::RCDrawPartialTransparentGraphicFG(Graphic &g, mfmt remap, 
	int x, int y, int xb, int yb, int xs, int ys, Panel p)  {
  if(x+g.xsize <= pxs[p] || y+g.ysize <= pys[p] || x >= pxe[p] || y >= pye[p])
	return;
  if(g.depth != depth)  {
    Graphic *g2 = new Graphic(g);
    g2->DepthConvert(depth, *pal);
    DrawPartialTransparentGraphicFG(*g2, x, y, xb, yb, xs, ys, p);
    delete g2;
    return;
    }
//  UserDebug("User:Screen:RCDrawPartialTransparentGraphicFG Middle");
  int ctrx, ctry;
  if(depth == 8)  {
//    UserDebug("User:Screen:RCDrawPartialTransparentGraphicFG Depth 8");
    for(ctry=max(yb, pys[p]-y); ctry<min(pye[p]-y, ys+yb); ctry++)  {
      for(ctrx=max(xb, pxs[p]-x); ctrx<min(pxe[p]-x, xs+xb); ctrx++)  {
	if(g.image[ctry].u8[ctrx] != g.tcolor)  {
	  image[ctry+y].u8[ctrx+x] = remap.u8[g.image[ctry].u8[ctrx]];
	  }
	}
      }
    }
  else if(depth == 32)  {
//    UserDebug("User:Screen:RCDrawPartialTransparentGraphicFG Depth 32");
    for(ctry=max(yb, pys[p]-y); ctry<min(pye[p]-y, ys+yb); ctry++)  {
      for(ctrx=max(xb, pxs[p]-x); ctrx<min(pxe[p]-x, xs+xb); ctrx++)  {
	if(g.image[ctry].u8[(ctrx<<2)+3] == 0xFF)  {
	  image[ctry+y].u32[ctrx+x] = g.image[ctry].u32[ctrx];
	  }
	else if(g.image[ctry].u8[(ctrx<<2)+3])  {
	  int alpha = g.image[ctry].u8[(ctrx<<2)+3];
	  unsigned long r1 = image[ctry+y].u8[((ctrx+x)<<2)];
	  unsigned long g1 = image[ctry+y].u8[((ctrx+x)<<2)+1];
	  unsigned long b1 = image[ctry+y].u8[((ctrx+x)<<2)+2];
	  unsigned long r2 = g.image[ctry].u8[(ctrx<<2)];
	  unsigned long g2 = g.image[ctry].u8[(ctrx<<2)+1];
	  unsigned long b2 = g.image[ctry].u8[(ctrx<<2)+2];
          r1 *= (0xFF-alpha);  r2 *= alpha; r1 += r2; r1 /= 255;
          g1 *= (0xFF-alpha);  g2 *= alpha; g1 += g2; g1 /= 255;
          b1 *= (0xFF-alpha);  b2 *= alpha; b1 += b2; b1 /= 255;
	  image[ctry+y].u8[((ctrx+x)<<2)] = r1;
	  image[ctry+y].u8[((ctrx+x)<<2)+1] = g1;
	  image[ctry+y].u8[((ctrx+x)<<2)+2] = b1;
	  }
	}
      }
    }
  else if(depth == 16)  {
//    UserDebug("User:Screen:RCDrawPartialTransparentGraphicFG Depth 16");
    for(ctry=max(yb, pys[p]-y); ctry<min(pye[p]-y, ys+yb); ctry++)  {
      for(ctrx=max(xb, pxs[p]-x); ctrx<min(pxe[p]-x, xs+xb); ctrx++)  {
	if(g.image[ctry].u16[ctrx] != g.tcolor)  {
	  image[ctry+y].u16[ctrx+x] = g.image[ctry].u16[ctrx];
	  }
	}
      }
    }
  else U2_Exit(-1, "Unknown depth error (%d) in %s\n", depth, __PRETTY_FUNCTION__);
//  UserDebug("User:Screen:RCDrawPartialTransparentGraphicFG End");
  }

void Screen::RCDrawPartialGraphicFG(Graphic &g, mfmt remap, int x, int y,
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
//  UserDebug("User:Screen:RCDrawPartialGraphicFG Middle");
  int ctry;
  int ix = max(xb, pxs[p]-x), ex = min(pxe[p]-x, xs+xb);
  int sx = ex-ix;
  if(depth == 8)  {
//    UserDebug("User:Screen:RCDrawPartialGraphicFG Depth 8");
    for(ctry=max(yb, pys[p]-y); ctry<min(pye[p]-y, ys+yb); ctry++)  {
      memcpy(image[ctry+y].u8+x+ix, g.image[ctry].u8+ix, sx);
      }
    }
  else if(depth == 16)  {
//    UserDebug("User:Screen:RCDrawPartialGraphicFG Depth 16");
    for(ctry=max(yb, pys[p]-y); ctry<min(pye[p]-y, ys+yb); ctry++)  {
      memcpy(image[ctry+y].u16+x+ix, g.image[ctry].u16+ix, sx<<1);
      }
    }
  else if(depth == 32)  {
//    UserDebug("User:Screen:RCDrawPartialGraphicFG Depth 32");
    for(ctry=max(yb, pys[p]-y); ctry<min(pye[p]-y, ys+yb); ctry++)  {
      memcpy(image[ctry+y].u32+x+ix, g.image[ctry].u32+ix, sx<<2);
      }
    }
  else U2_Exit(-1, "Unknown depth error (%d) in %s\n", depth, __PRETTY_FUNCTION__);
//  UserDebug("User:Screen:RCDrawPartialGraphicFG End");
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
  UserDebug("User:Screen:DrawTransparentGraphicFG Middle");
  int ctrx, ctry;
  if(depth == 8)  {
    UserDebug("User:Screen:DrawTransparentGraphicFG Depth 8");
    for(ctry=max(0, pys[p]-y); ctry<min(pye[p]-y, g.ysize); ctry++)  {
      for(ctrx=max(0, pxs[p]-x); ctrx<min(pxe[p]-x, g.xsize); ctrx++)  {
	if(g.image[ctry].u8[ctrx] != g.tcolor)  {
	  image[ctry+y].u8[ctrx+x] = g.image[ctry].u8[ctrx];
	  }
	}
      }
    }
  else if(depth == 32)  {
    UserDebug("User:Screen:DrawTransparentGraphicFG Depth 32");
    for(ctry=max(0, pys[p]-y); ctry<min(pye[p]-y, g.ysize); ctry++)  {
      for(ctrx=max(0, pxs[p]-x); ctrx<min(pxe[p]-x, g.xsize); ctrx++)  {
	if(g.image[ctry].u8[(ctrx<<2)+3] == 0xFF)  {
	  image[ctry+y].u32[ctrx+x] = g.image[ctry].u32[ctrx];
	  }
	else if(g.image[ctry].u8[(ctrx<<2)+3])  {
	  int alpha = g.image[ctry].u8[(ctrx<<2)+3];
	  unsigned long r1 = image[ctry+y].u8[((ctrx+x)<<2)];
	  unsigned long g1 = image[ctry+y].u8[((ctrx+x)<<2)+1];
	  unsigned long b1 = image[ctry+y].u8[((ctrx+x)<<2)+2];
	  unsigned long r2 = g.image[ctry].u8[(ctrx<<2)];
	  unsigned long g2 = g.image[ctry].u8[(ctrx<<2)+1];
	  unsigned long b2 = g.image[ctry].u8[(ctrx<<2)+2];
          r1 *= (255-alpha);  r2 *= alpha; r1 += r2; r1 /= 255;
          g1 *= (255-alpha);  g2 *= alpha; g1 += g2; g1 /= 255;
          b1 *= (255-alpha);  b2 *= alpha; b1 += b2; b1 /= 255;
	  image[ctry+y].u8[((ctrx+x)<<2)] = r1;
	  image[ctry+y].u8[((ctrx+x)<<2)+1] = g1;
	  image[ctry+y].u8[((ctrx+x)<<2)+2] = b1;
	  }
	}
      }
    }
  else if(depth == 16)  {
    UserDebug("User:Screen:DrawTransparentGraphicFG Depth 16");
    for(ctry=max(0, pys[p]-y); ctry<min(pye[p]-y, g.ysize); ctry++)  {
      for(ctrx=max(0, pxs[p]-x); ctrx<min(pxe[p]-x, g.xsize); ctrx++)  {
	if(g.image[ctry].u16[ctrx] != g.tcolor)  {
	  image[ctry+y].u16[ctrx+x] = g.image[ctry].u16[ctrx];
	  }
	}
      }
    }
  else U2_Exit(-1, "Unknown depth error (%d) in %s\n", depth, __PRETTY_FUNCTION__);
  UserDebug("User:Screen:DrawTransparentGraphicFG End");
  }

void Screen::DrawGraphicFG(Graphic &g, int x, int y, Panel p)  {
  if(x+g.xsize <= pxs[p] || y+g.ysize <= pys[p] || x >= pxe[p] || y >= pye[p])
	return;
  if(g.depth != depth)  {
    Graphic *g2 = new Graphic(g);
    g2->DepthConvert(depth, *pal);
    DrawGraphic(*g2, x, y);
    delete g2;
    return;
    }
  int ctry, xbc=0, xrlen=0, ybc=0, yrlen=0;
  if(x < pxs[p]) xbc = pxs[p] - x;
  if(x+g.xsize > pxe[p]) xrlen = xbc + (x+g.xsize) - pxe[p];
  if(y < pys[p]) ybc = pys[p] - y;
  if(y+g.ysize > pye[p]) yrlen = ybc + (y+g.ysize) - pye[p];
  InvalidateRectangle(x+xbc, y+ybc, g.xsize-xrlen, g.ysize-yrlen);
  if(depth == 8)  {
    for(ctry=0+ybc; ctry < g.ysize-yrlen; ctry++)  {
      memcpy(image[ctry+y].u8+x+xbc, g.image[ctry].u8+xbc, g.xsize-xrlen);
      }
    }
  else if(depth == 16)  {
    for(ctry=0+ybc; ctry < g.ysize-yrlen; ctry++)  {
      memcpy(image[ctry+y].u32+x+xbc, g.image[ctry].u32+xbc, g.xsize-xrlen);
      }
    }
  else if(depth == 32)  {
    for(ctry=0+ybc; ctry < g.ysize-yrlen; ctry++)  {
      memcpy(image[ctry+y].u16+x+xbc, g.image[ctry].u16+xbc, g.xsize-xrlen);
      }
    }
  else U2_Exit(-1, "Unknown depth error (%d) in %s\n", depth, __PRETTY_FUNCTION__);
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
    for(ctry=0; ctry<min(ysize, g.ysize); ctry++)  {
      for(ctrx=0; ctrx<min(xsize, g.xsize); ctrx++)  {
	image[ctry].u8[ctrx] = g.image[ctry].u8[ctrx];
	}
      }
    }
  else if(depth == 32)  {
    for(ctry=0; ctry<min(ysize, g.ysize); ctry++)  {
      for(ctrx=0; ctrx<min(xsize, g.xsize); ctrx++)  {
	image[ctry].u32[ctrx] = g.image[ctry].u32[ctrx];
	}
      }
    }
  else U2_Exit(-1, "Unknown depth error (%d) in %s\n", depth, __PRETTY_FUNCTION__);
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
    for(ctry=0; ctry<min(ysize-y, g.ysize); ctry++)  {
      for(ctrx=0; ctrx<min(xsize-x, g.xsize); ctrx++)  {
	if(g.image[ctry].u8[ctrx] != g.tcolor)  {
	  image[ctry+y].u8[ctrx+x] = g.image[ctry].u8[ctrx];
	  backg[ctry+y].u8[ctrx+x] = g.image[ctry].u8[ctrx];
	  }
	}
      }
    }
  else if(depth == 32)  {
    UserDebug("User:Screen:DrawTransparentGraphic Depth 32");
    for(ctry=max(0, pys[p]-y); ctry<min(pye[p]-y, g.ysize); ctry++)  {
      for(ctrx=max(0, pxs[p]-x); ctrx<min(pxe[p]-x, g.xsize); ctrx++)  {
	if(g.image[ctry].u8[(ctrx<<2)+3] == 255)  {
	  image[ctry+y].u32[ctrx+x] = g.image[ctry].u32[ctrx];
	  backg[ctry+y].u32[ctrx+x] = g.image[ctry].u32[ctrx];
	  }
	else if(g.image[ctry].u8[(ctrx<<2)+3])  {
	  unsigned long alpha = g.image[ctry].u8[(ctrx<<2)+3];
	  unsigned long r1 = backg[ctry+y].u8[((ctrx+x)<<2)];
	  unsigned long g1 = backg[ctry+y].u8[((ctrx+x)<<2)+1];
	  unsigned long b1 = backg[ctry+y].u8[((ctrx+x)<<2)+2];
	  unsigned long r2 = g.image[ctry].u8[(ctrx<<2)];
	  unsigned long g2 = g.image[ctry].u8[(ctrx<<2)+1];
	  unsigned long b2 = g.image[ctry].u8[(ctrx<<2)+2];
	  r2 *= alpha; r1 *= (255-alpha); r1 += r2; r1 /= 255;
	  g2 *= alpha; g1 *= (255-alpha); g1 += g2; g1 /= 255;
	  b2 *= alpha; b1 *= (255-alpha); b1 += b2; b1 /= 255;
	  image[ctry+y].u8[((ctrx+x)<<2)] = r1;
	  image[ctry+y].u8[((ctrx+x)<<2)+1] = g1;
	  image[ctry+y].u8[((ctrx+x)<<2)+2] = b1;
	  backg[ctry+y].u8[((ctrx+x)<<2)] = r1;
	  backg[ctry+y].u8[((ctrx+x)<<2)+1] = g1;
	  backg[ctry+y].u8[((ctrx+x)<<2)+2] = b1;
	  }
	}
      }
    }
  else if(depth == 16)  {
    for(ctry=0; ctry<min(ysize-y, g.ysize); ctry++)  {
      for(ctrx=0; ctrx<min(xsize-x, g.xsize); ctrx++)  {
	if(g.image[ctry].u16[ctrx] != g.tcolor)  {
	  image[ctry+y].u16[ctrx+x] = g.image[ctry].u16[ctrx];
	  backg[ctry+y].u16[ctrx+x] = g.image[ctry].u16[ctrx];
	  }
	}
      }
    }
  else U2_Exit(-1, "Unknown depth error (%d) in %s\n", depth, __PRETTY_FUNCTION__);
  }

void Screen::DrawPartialGraphic(Graphic &g, int x, int y, 
	int xb, int yb, int xs, int ys, Panel p)  {
  if(x+xs <= pxs[p] || y+ys <= pys[p] || x >= pxe[p] || y >= pye[p])
	return;
  if(g.depth != depth)  {
    Graphic *g2 = new Graphic(g);
    g2->DepthConvert(depth, *pal);
    DrawGraphic(*g2, x, y);
    delete g2;
    return;
    }
  int ctry, xbc=0, xrlen=0, ybc=0, yrlen=0;
  if(x < pxs[p]) xbc = pxs[p] - x;
  if(x+xs > pxe[p]) xrlen = xbc + (x+xs) - pxe[p];
  if(y < pys[p]) ybc = pys[p] - y;
  if(y+ys > pye[p]) yrlen = ybc + (y+ys) - pye[p];
  InvalidateRectangle(x+xbc, y+ybc, xs-xrlen, ys-yrlen);
  if(depth == 8)  {
    for(ctry=0+ybc; ctry < ys-yrlen; ctry++)  {
      memcpy(image[ctry+y].u8+x+xbc, g.image[ctry+yb].u8+xb+xbc, xs-xrlen);
      memcpy(backg[ctry+y].u8+x+xbc, g.image[ctry+yb].u8+xb+xbc, xs-xrlen);
      }
    }
  else if(depth == 16)  {
    for(ctry=0+ybc; ctry < ys-yrlen; ctry++)  {
      memcpy(image[ctry+y].u32+x+xbc, g.image[ctry+yb].u32+xb+xbc, xs-xrlen);
      memcpy(backg[ctry+y].u32+x+xbc, g.image[ctry+yb].u32+xb+xbc, xs-xrlen);
      }
    }
  else if(depth == 32)  {
    for(ctry=0+ybc; ctry < ys-yrlen; ctry++)  {
      memcpy(image[ctry+y].u16+x+xbc, g.image[ctry+yb].u16+xb+xbc, xs-xrlen);
      memcpy(backg[ctry+y].u16+x+xbc, g.image[ctry+yb].u16+xb+xbc, xs-xrlen);
      }
    }
  else U2_Exit(-1, "Unknown depth error (%d) in %s\n", depth, __PRETTY_FUNCTION__);
  }

void Screen::DrawGraphic(Graphic &g, int x, int y, Panel p)  {
  if(x+g.xsize <= pxs[p] || y+g.ysize <= pys[p] || x >= pxe[p] || y >= pye[p])
	return;
  if(g.depth != depth)  {
    Graphic *g2 = new Graphic(g);
    g2->DepthConvert(depth, *pal);
    DrawGraphic(*g2, x, y);
    delete g2;
    return;
    }
  int ctry, xbc=0, xrlen=0, ybc=0, yrlen=0;
  if(x < pxs[p]) xbc = pxs[p] - x;
  if(x+g.xsize > pxe[p]) xrlen = xbc + (x+g.xsize) - pxe[p];
  if(y < pys[p]) ybc = pys[p] - y;
  if(y+g.ysize > pye[p]) yrlen = ybc + (y+g.ysize) - pye[p];
  InvalidateRectangle(x+xbc, y+ybc, g.xsize-xrlen, g.ysize-yrlen);
  if(depth == 8)  {
    for(ctry=0+ybc; ctry < g.ysize-yrlen; ctry++)  {
      memcpy(image[ctry+y].u8+x+xbc, g.image[ctry].u8+xbc, g.xsize-xrlen);
      memcpy(backg[ctry+y].u8+x+xbc, g.image[ctry].u8+xbc, g.xsize-xrlen);
      }
    }
  else if(depth == 16)  {
    for(ctry=0+ybc; ctry < g.ysize-yrlen; ctry++)  {
      memcpy(image[ctry+y].u32+x+xbc, g.image[ctry].u32+xbc, g.xsize-xrlen);
      memcpy(backg[ctry+y].u32+x+xbc, g.image[ctry].u32+xbc, g.xsize-xrlen);
      }
    }
  else if(depth == 32)  {
    for(ctry=0+ybc; ctry < g.ysize-yrlen; ctry++)  {
      memcpy(image[ctry+y].u16+x+xbc, g.image[ctry].u16+xbc, g.xsize-xrlen);
      memcpy(backg[ctry+y].u16+x+xbc, g.image[ctry].u16+xbc, g.xsize-xrlen);
      }
    }
  else U2_Exit(-1, "Unknown depth error (%d) in %s\n", depth, __PRETTY_FUNCTION__);
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
    for(ctry=0; ctry<min(ysize, g.ysize); ctry++)  {
      for(ctrx=0; ctrx<min(xsize, g.xsize); ctrx++)  {
	image[ctry].u8[ctrx] = g.image[ctry].u8[ctrx];
	backg[ctry].u8[ctrx] = g.image[ctry].u8[ctrx];
	}
      }
    }
  else if(depth == 32)  {
    for(ctry=0; ctry<min(ysize, g.ysize); ctry++)  {
      for(ctrx=0; ctrx<min(xsize, g.xsize); ctrx++)  {
	image[ctry].u32[ctrx] = g.image[ctry].u32[ctrx];
	backg[ctry].u32[ctrx] = g.image[ctry].u32[ctrx];
	}
      }
    }
  else if(depth == 16)  {
    for(ctry=0; ctry<min(ysize, g.ysize); ctry++)  {
      for(ctrx=0; ctrx<min(xsize, g.xsize); ctrx++)  {
	image[ctry].u16[ctrx] = g.image[ctry].u16[ctrx];
	backg[ctry].u16[ctrx] = g.image[ctry].u16[ctrx];
	}
      }
    }
  else U2_Exit(-1, "Unknown depth error (%d) in %s\n", depth, __PRETTY_FUNCTION__);
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
      #ifdef X_DGA2
      case(VIDEO_XDGA2): {
	if(depth == 8)  {
	  _Xpal[c].pixel = c;
	  _Xpal[c].red = r<<8;
	  _Xpal[c].green = g<<8;
	  _Xpal[c].blue = b<<8;
	  _Xpal[c].flags = DoRed|DoGreen|DoBlue;
	  XStoreColor(_Xdisplay, _Xmap, &_Xpal[c]);
	  XDGAInstallColormap(_Xdisplay, _Xscreen, _Xmap);
	  }
	else {
//	  RefreshFull();
	  }
	}break;
      #endif
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
  if(appdepth != p.depth) U2_Exit(1, "Palette and Screen depth mismatch\n");
  (*pal) = p;
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
      #ifdef X_DGA2
      case(VIDEO_XDGA2): {
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
	  XDGAInstallColormap(_Xdisplay, _Xscreen, _Xmap);
	  }
        }break;
      #endif
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
  pal->depth = appdepth;
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
      #ifdef X_DGA2
      case(VIDEO_XDGA2): {
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
	  XDGAInstallColormap(_Xdisplay, _Xscreen, _Xmap);
	  }
        }break;
      #endif
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
  if(sprites[n] == NULL) U2_Exit(-1, "Tried to remove non-existant sprite!\n");
  if(sprites[n] != s) U2_Exit(-1, "Tried to remove other sprite!\n");
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
    if ((((rxs[ctr] >= x && rxs[ctr] < xe)
	  || (rxe[ctr] >= x && rxe[ctr] < xe))
	 && ((rys[ctr] >= y && rys[ctr] < ye)
	  || (rye[ctr] >= y && rye[ctr] < ye)))
	|| (((x >= rxs[ctr] && x < rxe[ctr])
	  || (xe >= rxs[ctr] && xe < rxe[ctr]))
	 && ((y >= rys[ctr] && y < rye[ctr])
	  || (ye >= rys[ctr] && ye < rye[ctr])))) {
      rxs[ctr] = min(rxs[ctr], x);
      rxe[ctr] = max(rxe[ctr], xe);
      rys[ctr] = min(rys[ctr], y);
      rye[ctr] = max(rye[ctr], ye);
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
  loss = (max(rxe[0], xe)-min(rxs[0], x))*(max(rye[0], ye)-min(rys[0], y));
  loss -= (rxe[0]-rxs[0])*(rye[0]-rys[0]);
  loss -= (xe-x)*(ye-y);
  for(ctr=1; ctr<REDRAW_RECTS; ctr++) {
    tl = (max(rxe[ctr], xe)-min(rxs[ctr], x))*(max(rye[ctr], ye)-min(rys[ctr], y));
    tl -= (rxe[ctr]-rxs[ctr])*(rye[ctr]-rys[ctr]);
    tl -= (xe-x)*(ye-y);
    if(tl<loss) {
      num = ctr;
      loss = tl;
      }
    }
  rxs[num] = min(rxs[num], x);
  rxe[num] = max(rxe[num], xe);
  rys[num] = min(rys[num], y);
  rye[num] = max(rye[num], ye);
  }

void Screen::RestoreRectangle(int x, int y, int xs, int ys)  {
  UserDebug("Screen:RestoreRectangle() Begin");
  static Sprite *spbin1[256], *spbin2[256];
  if(x>xsize || y>ysize || x+xs<=0 || y+ys<=0) return;
  if(x<0) { xs += x; x=0; }
  if(y<0) { ys += y; y=0; }
  if(x+xs > xsize) xs = xsize-x;
  if(y+ys > ysize) ys = ysize-y;
  if(xs < 0 || ys < 0) return;
//  printf("Restoring (%d,%d)->(%d,%d)\n", x, y, x+xs, y+ys);
  InvalidateRectangle(x, y, xs, ys);
  int ctrx, ctry;
  UserDebug("Screen:RestoreRectangle() Before Write");
  if(depth == 8)  {
    for(ctry=y; ctry<y+ys; ctry++)  {
      memcpy(image[ctry].u8+x, backg[ctry].u8+x, xs);
      }
    }
  else if(depth == 16)  {
    for(ctry=y; ctry<y+ys; ctry++)  {
      memcpy(image[ctry].u16+x, backg[ctry].u16+x, xs<<1);
      }
    }
  else if(depth == 32)  {
    for(ctry=y; ctry<y+ys; ctry++)  {
      memcpy(image[ctry].u32+x, backg[ctry].u32+x, xs<<2);
      }
    }
  else U2_Exit(-1, "Unknown depth error (%d) in %s\n", depth, __PRETTY_FUNCTION__);

  UserDebug("Screen:RestoreRectangle() Before Identification of Sprites");

  memset(spbin1, 0, sizeof(spbin1));
#define snag(bxy) \
    for(tmps=bxy; tmps!=NULL;) {					\
      if(tmps->drawn && tmps->image != NULL				\
	  && (y+ys) > tmps->ypos && (x+xs) > tmps->xpos			\
	  && y < (tmps->ypos + tmps->image->ysize)			\
	  && x < (tmps->xpos + tmps->image->xsize))  {			\
	cur = tmps;							\
	tmps = tmps->next;						\
	cur->next2 = spbin1[cur->priority & 255];			\
	spbin1[cur->priority & 255] = cur;				\
	}								\
      else tmps = tmps->next;						\
      }

  int xb=(x>>BIN_FACTOR)-1, yb=(y>>BIN_FACTOR)-1;
  int xe=(x+xs+BIN_SIZE-1)>>BIN_FACTOR, ye=(y+ys+BIN_SIZE-1)>>BIN_FACTOR;
  Sprite *ind, *cur, *tmps;
  if(xb<0) xb=0;
  if(yb<0) yb=0;
  if(xe>xbins) xe=xbins;
  if(ye>ybins) ye=ybins;
  for(ctrx=xb; ctrx<xe; ++ctrx) {
    for(ctry=yb; ctry<ye; ++ctry) {
      snag(bins[ctrx][ctry]);
      }
    }

  xb=(x>>LARGE_BIN_FACTOR)-1; yb=(y>>LARGE_BIN_FACTOR)-1;
  xe=(x+xs+LARGE_BIN_SIZE-1)>>LARGE_BIN_FACTOR;
  ye=(y+ys+LARGE_BIN_SIZE-1)>>LARGE_BIN_FACTOR;
  if(xb<0) xb=0;
  if(yb<0) yb=0;
  if(xe>xlbins) xe=xlbins;
  if(ye>ylbins) ye=ylbins;
  for(ctrx=xb; ctrx<xe; ++ctrx) {
    for(ctry=yb; ctry<ye; ++ctry) {
      snag(lbins[ctrx][ctry]);
      }
    }

  snag(huges);

#undef snag

  UserDebug("Screen:RestoreRectangle() Before Radix Sort");
  memset(spbin2, 0, sizeof(spbin2));
  for(ctrx=255; ctrx>=0; --ctrx) {
    ind = spbin1[ctrx];
    while(ind != NULL) {
      cur = ind;
      ind = ind->next2;
      cur->next2 = spbin2[(cur->priority >> 8) & 255];
      spbin2[(cur->priority >> 8) & 255] = cur;
      }
    }
  memset(spbin1, 0, sizeof(spbin1));
  for(ctrx=0; ctrx<256; ++ctrx) {
    ind = spbin2[ctrx];
    while(ind != NULL) {
      cur = ind;
      ind = ind->next2;
      cur->next2 = spbin1[(cur->priority >> 16) & 255];
      spbin1[(cur->priority >> 16) & 255] = cur;
      }
    }
  memset(spbin2, 0, sizeof(spbin2));
  for(ctrx=255; ctrx>=0; --ctrx) {
    ind = spbin1[ctrx];
    while(ind != NULL) {
      cur = ind;
      ind = ind->next2;
      cur->next2 = spbin2[((cur->priority >> 24) ^ 128) & 255];
      spbin2[((cur->priority >> 24) ^ 128) & 255] = cur;
      }
    }
  ind = NULL;
  for(ctrx=0; ctrx<256; ++ctrx) {
    cur = spbin2[ctrx];
    while(cur != NULL) {
      tmps = cur;
      cur = cur->next2;
      tmps->next2 = ind;
      ind = tmps;
      }
    }
  while(ind != NULL) {
    ind->RedrawArea(x, y, xs, ys);
    ind = ind->next2;
    }
  UserDebug("Screen:RestoreRectangle() End");
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
    if(geteuid() == 0 || (!access("/dev/mem", W_OK)))  {
      int M, m, Q, q;
      #ifdef X_DGA2
      if(!XDGAQueryVersion(_Xdisplay, &M, &m)) M = 0;
      if(M >= 2) {
	XDGAQueryExtension(_Xdisplay, &Q, &q);
	XDGAOpenFramebuffer(_Xdisplay, _Xscreen);

	int uid, euid;
	uid = getuid() ; euid = geteuid() ;
	if (euid != uid && setuid(uid)) U2_Exit(-1, "SETUID Failed!\n");

	vtype = VIDEO_XDGA2;
	}
      else if(M == 1) {
      #else
      if(XF86DGAQueryVersion(_Xdisplay, &M, &m)) {
      #endif
//	fprintf(stderr, "Got DGA version %d.%d\n", M, m);
	int Xbank, Xmem, Flags;
	XF86DGAQueryDirectVideo(_Xdisplay, _Xscreen, &Flags);
	if(!(Flags & XF86DGADirectPresent)) U2_Exit(0, "Failed DGA Query\n");
	XF86DGAGetVideo(_Xdisplay, _Xscreen,
		(char **)&(frame.u8), &rowlen, &Xbank, &Xmem);
//	fprintf(stderr, "Width %d, Bank %d, Mem %d\n",  rowlen, Xbank, Xmem);

	if(XF86VidModeQueryVersion(_Xdisplay, &M, &m))  {
//	  fprintf(stderr, "Got VidMode version %d.%d\n", M, m);
	  }
	else { fprintf(stderr, "No VidMode support.\n"); return; }
	vtype = VIDEO_XF86DGA;

	int uid, euid;
	uid = getuid() ; euid = geteuid() ;
	if (euid != uid && setuid(uid)) U2_Exit(-1, "SETUID Failed!\n");
	}
      else { fprintf(stderr, "No DGA support.\n"); return; }
      }
#endif
    }
#endif
//  printf("Video type = %d\n", vtype);
  if(vtype != VIDEO_NONE) return;
  U2_Exit(-1, "No video capability detected!\n");
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
  for(ctr=0; ctr<256; ctr++) {
    if(font[ctr] != NULL) delete font[ctr];
    font[ctr] = NULL;
    font[ctr] = frf.GetGraphic();
    }
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
  UserDebug("Screen::AlignCursor() Begin");
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
  UserDebug("Screen::AlignCursor() End");
  }

void Screen::SetCursor(Graphic &g)  {
  TCursor = new Sprite(g);
  TCursor->Move(tcx, tcy);
  }

int Screen::GPrintf(Graphic *g, int x, int y, color cb, color cf,
	const char *text, ...) {
  UserDebug("User::Screen::GPrintf(...) Begin");
  if(g==NULL) return 0;
  int ret;
  va_list stuff;
  va_start(stuff, text);
  char buf[4096];
  bzero(buf, 4096);
  vsprintf(buf, text, stuff);
  ret = GPrint(g, x, y, cb, cf, buf);
  va_end(stuff);
  UserDebug("User::Screen::GPrintf(...) End");
  return ret;
  }

int Screen::CGPrintf(Graphic *g, int x, int y, color cb, color cf,
	const char *text, ...) {
  UserDebug("User::Screen::CGPrintf(...) Begin");
  if(g==NULL) return 0;
  int ret;
  va_list stuff;
  va_start(stuff, text);
  char buf[4096];
  bzero(buf, 4096);
  vsprintf(buf, text, stuff);
  ret = CGPrint(g, x, y, cb, cf, buf);
  va_end(stuff);
  UserDebug("User::Screen::CGPrintf(...) End");
  return ret;
  }

int Screen::Printf(int x, int y, color cb, color cf, const char *text, ...) {
  UserDebug("User::Screen::Printf(x, y, ...) Begin");
  TGotoXY(x, y);
  int ret;
  va_list stuff;
  va_start(stuff, text);
  char buf[4096];
  bzero(buf, 4096);
  vsprintf(buf, text, stuff);
  ret = Print(cb, cf, buf);
  va_end(stuff);
  UserDebug("User::Screen::Printf(x, y, ...) End");
  return ret;
  }

int Screen::Printf(color cb, color cf, const char *text, ...)  {
  UserDebug("User::Screen::Printf(...) Begin");
  int ret;
  va_list stuff;
  va_start(stuff, text);
  char buf[4096];
  bzero(buf, 4096);
  vsprintf(buf, text, stuff);
  ret = Print(cb, cf, buf);
  va_end(stuff);
  UserDebug("User::Screen::Printf(...) End");
  return ret;
  }

void Screen::GetStringSize(char *text, int *xsz, int *ysz) {
  unsigned char *ind = (unsigned char *)text;
  int xpos = 1, ypos = font[' ']->ysize;
  *ysz = font[' ']->ysize;
  *xsz = 1;
  for(;(*ind) != 0; ind++)  {
    if((*ind) == (unsigned char)'\t') (*ind)=' ';
    if((*ind) == (unsigned char)CHAR_LEFT)  {
      xpos -= font[' ']->xsize+1;
      }
    else if((*ind) == (unsigned char)CHAR_RIGHT)  {
      xpos += font[' ']->xsize+1;
      if(xpos > *xsz) *xsz = xpos;
      }
    else if((*ind) == (unsigned char)CHAR_UP)  {
      ypos -= font[' ']->ysize+2;
      }
    else if((*ind) == (unsigned char)CHAR_DOWN)  {
      ypos += font[' ']->ysize+2;
      if(ypos > *ysz) *ysz = ypos;
      }
    else if((*ind) == (unsigned char)'\n')  {
      xpos = 1;
      ypos += font[' ']->ysize+2;
      if(ypos > *ysz) *ysz = ypos;
      }
    else if((*ind) == (unsigned char)'\r')  {
      xpos = 1;
      }
    else if(font[*ind] != NULL)  {
      xpos += font[*ind]->xsize+1;
      if(xpos > *xsz) *xsz = xpos;
      }
    }
  }

int Screen::GPrint(Graphic *g, int x, int y, color cb, color cf,
	const char *text)  {
  if(g==NULL) return 0;
  UserDebug("User::Screen::GPrint(...) Begin");
  ConvertColor(cb, appdepth, (int)g->depth);
  ConvertColor(cf, appdepth, (int)g->depth);
  if(font[' '] == NULL)  U2_Exit(-1, "Must Screen.SetFont before Screen.Print!\n");
  unsigned char *ind = (unsigned char *)text;
//  printf("%s\n", text);
  for(;(*ind) != 0; ind++)  {
//    printf("%d,%d:%d: ", TXPos(), TYPos(), *ind); fflush(stdout);

    if((*ind) == (unsigned char)'\t') (*ind)=' ';
    if((*ind) == (unsigned char)CHAR_LEFT)  {
      x-=font[' ']->xsize+1;
      AlignCursor();
      }
    else if((*ind) == (unsigned char)CHAR_RIGHT)  {
      x+=font[' ']->xsize+1;
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
      UserDebug("User::Screen::GPrint(...) Printing character");
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
      UserDebug("User::Screen::GPrint(...) Outputting character");
      if(depth==8) {
	for(ctrx=0; ctrx<(int)let.xsize; ctrx++)  {
	  for(ctry=0; ctry<(int)let.ysize; ctry++)  {
	     if(let.image[ctry].u8[ctrx] == 0) res.image[ctry].u8[ctrx] = cb;
	     else res.image[ctry].u8[ctrx] = cf;
	     }
	  }
        UserDebug("User::Screen::GPrint(...) Pasting character graphic (8bpp)");
	g->PasteTransparentGraphic(res, x-let.xcenter, y-let.ycenter);
	}
      else if(depth==32) {
	unsigned long alpha;
	for(ctrx=0; ctrx<(int)let.xsize; ctrx++)  {
	  for(ctry=0; ctry<(int)let.ysize; ctry++)  {
	    alpha=let.image[ctry].u8[ctrx];
	    if(alpha) {
	      res.image[ctry].u32[ctrx] = cf;
	      alpha*=((unsigned char*)&cf)[3]; alpha/=255;
	      res.image[ctry].u8[(ctrx<<2)+3] = alpha;
	      }
	    else res.image[ctry].u32[ctrx]=0;
	    }
	  }
        UserDebug("User::Screen::GPrint(...) Pasting character graphic (32bpp)");
	g->PasteTransparentGraphic(res, x-let.xcenter, y-let.ycenter);
	}
      else if(depth==16) {
	for(ctrx=0; ctrx<(int)let.xsize; ctrx++)  {
	  for(ctry=0; ctry<(int)let.ysize; ctry++)  {
	    color cl, clf = cf, clb = cb;
	    unsigned long alpha = let.image[ctry].u8[ctrx];
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
	    res.image[ctry].u16[ctrx] = cl;
	    }
	  }
        UserDebug("User::Screen::GPrint(...) Pasting character graphic (16bpp)");
	g->PasteTransparentGraphic(res, x-let.xcenter, y-let.ycenter);
	}
      else U2_Exit(-1, "Unknown depth error (%d) in %s\n", depth, __PRETTY_FUNCTION__);

      UserDebug("User::Screen::GPrint(...) Finishing character");
      x+=font[*ind]->xsize+1;
      AlignCursor();
      }
    }
  UserDebug("User::Screen::GPrint(...) End");
  return x;
  }

int Screen::CGPrint(Graphic *g, int x, int y, color cb, color cf,
	const char *text)  {
  if(g==NULL) return 0;
  UserDebug("User::Screen::CGPrint(...) Begin");
  ConvertColor(cb, appdepth, (int)g->depth);
  ConvertColor(cf, appdepth, (int)g->depth);
  if(font[' '] == NULL)  U2_Exit(-1, "Must Screen.SetFont before Screen.Print!\n");
  { int xsz, ysz;
    GetStringSize((char *)text, &xsz, &ysz);
    g->DefSize(xsz, ysz);
    }
  unsigned char *ind = (unsigned char *)text;
//  printf("%s\n", text);
  g->Clear();
  for(;(*ind) != 0; ind++)  {
//    printf("%d,%d:%d: ", TXPos(), TYPos(), *ind); fflush(stdout);

    if((*ind) == (unsigned char)'\t') (*ind)=' ';
    if((*ind) == (unsigned char)CHAR_LEFT)  {
      x-=font[' ']->xsize+1;
      AlignCursor();
      }
    else if((*ind) == (unsigned char)CHAR_RIGHT)  {
      x+=font[' ']->xsize+1;
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
      UserDebug("User::Screen::CGPrint(...) Printing character");
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
      UserDebug("User::Screen::CGPrint(...) Outputting character");
      if(depth==8) {
	for(ctrx=0; ctrx<(int)let.xsize; ctrx++)  {
	  for(ctry=0; ctry<(int)let.ysize; ctry++)  {
	     if(let.image[ctry].u8[ctrx] == 0) res.image[ctry].u8[ctrx] = cb;
	     else res.image[ctry].u8[ctrx] = cf;
	     }
	  }
        UserDebug("User::Screen::CGPrint(...) Pasting character graphic (8bpp)");
	g->PasteGraphic(res, x-let.xcenter, y-let.ycenter);
	}
      else if(depth==32) {
	unsigned long alpha;
	for(ctrx=0; ctrx<(int)let.xsize; ctrx++)  {
	  for(ctry=0; ctry<(int)let.ysize; ctry++)  {
	    alpha=let.image[ctry].u8[ctrx];
	    if(alpha) {
	      res.image[ctry].u32[ctrx] = cf;
	      alpha*=((unsigned char*)&cf)[3]; alpha/=255;
	      res.image[ctry].u8[(ctrx<<2)+3] = alpha;
	      }
	    else res.image[ctry].u32[ctrx]=0;
	    }
	  }
        UserDebug("User::Screen::CGPrint(...) Pasting character graphic (32bpp)");
	g->PasteGraphic(res, x-let.xcenter, y-let.ycenter);
	}
      else if(depth==16) {
	for(ctrx=0; ctrx<(int)let.xsize; ctrx++)  {
	  for(ctry=0; ctry<(int)let.ysize; ctry++)  {
	    color cl, clf = cf, clb = cb;
	    unsigned long alpha = let.image[ctry].u8[ctrx];
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
	    res.image[ctry].u16[ctrx] = cl;
	    }
	  }
        UserDebug("User::Screen::CGPrint(...) Pasting character graphic (16bpp)");
	g->PasteGraphic(res, x-let.xcenter, y-let.ycenter);
	}
      else U2_Exit(-1, "Unknown depth error (%d) in %s\n", depth, __PRETTY_FUNCTION__);

      UserDebug("User::Screen::CGPrint(...) Finishing character");
      x+=font[*ind]->xsize+1;
      AlignCursor();
      }
    }
  UserDebug("User::Screen::CGPrint(...) End");
  return x;
  }

int Screen::Print(int x, int y, color cb, color cf, const char *text)  {
  TGotoXY(x, y);
  return(Print(cb, cf, text));
  }

int Screen::Print(color cb, color cf, const char *text)  {
  if(pal && appdepth == 8 && depth != 8) {
    cb = GetColor(pal->GetRedEntry(cb), pal->GetGreenEntry(cb), pal->GetBlueEntry(cb));
    cf = GetColor(pal->GetRedEntry(cf), pal->GetGreenEntry(cf), pal->GetBlueEntry(cf));
    }
  else {
    ConvertColor(cb, appdepth, depth);
    ConvertColor(cf, appdepth, depth);
    }
  UserDebug("User::Screen::Print(...) Begin");
  if(font[' '] == NULL)  U2_Exit(-1, "Must Screen.SetFont before Screen.Print!\n");
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
	     if(let.image[ctry].u8[ctrx] == 0) res.image[ctry].u8[ctrx] = cb;
	     else res.image[ctry].u8[ctrx] = cf;
	     }
	  }
	DrawGraphic(res, tcx-let.xcenter, tcy-let.ycenter);
	}
      else if(depth==32) {
	unsigned long alpha;
	for(ctrx=0; ctrx<(int)let.xsize; ctrx++)  {
	  for(ctry=0; ctry<(int)let.ysize; ctry++)  {
	    alpha=let.image[ctry].u8[ctrx];
	    if(alpha) {
	      res.image[ctry].u32[ctrx] = cf;
	      alpha*=((unsigned char*)&cf)[3]; alpha/=255;
	      res.image[ctry].u8[(ctrx<<2)+3] = alpha;
	      }
	    else res.image[ctry].u32[ctrx]=0;
	    }
	  }
	DrawTransparentGraphic(res, tcx-let.xcenter, tcy-let.ycenter);
	}
      else if(depth==16) {
	for(ctrx=0; ctrx<(int)let.xsize; ctrx++)  {
	  for(ctry=0; ctry<(int)let.ysize; ctry++)  {
	    color cl, clf = cf, clb = cb;
	    unsigned long alpha = let.image[ctry].u8[ctrx];
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
	    res.image[ctry].u16[ctrx] = cl;
	    }
	  }
	DrawTransparentGraphic(res, tcx-let.xcenter, tcy-let.ycenter);
	}
      else U2_Exit(-1, "Unknown depth error (%d) in %s\n", depth, __PRETTY_FUNCTION__);

      tcx+=font[*ind]->xsize+1;
      AlignCursor();
      }
    }
  if(TCursor != NULL)  TCursor->Move(tcx, tcy);
  UserDebug("User::Screen::Print(...) End");
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
  if(ctr >= MAX_PANELS) U2_Exit(-1, "Out of Panels!\n");
  pxs[ctr] = x1; pxe[ctr] = x2;
  pys[ctr] = y1; pye[ctr] = y2;
  return ctr;
  }

void Screen::RemovePanel(Panel p) {
  if(pxs[0] == -1) U2_Exit(-1, "Deleting non-existant panel!\n");
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

void Screen::ErasePanelSprites(Panel p) {
  int ctr;
  for(ctr=0; ctr<nextsprite; ctr++)  {
    if(sprites[ctr] != NULL && sprites[ctr]->pan == p) {
      if(sprites[ctr]->drawn) sprites[ctr]->Erase();
      }
    }
  }

void Screen::ScrollPanel(Panel p, int x, int y) {
  if(x==0 && y==0) return;
  int xs, xd, xsz, ys, yd, ysz, ctr, ydiff, xdiff;
  if(x>=0) { xs = pxs[p]+x; xd = pxs[p]+0; xsz = (pxe[p]-pxs[p])-x; }
  else     { xs = pxs[p]+0; xd = pxs[p]-x; xsz = (pxe[p]-pxs[p])+x; }
  if(y>=0) { ys = pys[p]+y; yd = pys[p]+0; ysz = (pye[p]-pys[p])-y; }
  else     { ys = pys[p]+0; yd = pys[p]-y; ysz = (pye[p]-pys[p])+y; }
  ydiff = yd - ys;
  xdiff = xd - xs;

  for(ctr=0; ctr<nextsprite; ctr++)  {
    if(sprites[ctr] != NULL && sprites[ctr]->pan == p) {
      if(sprites[ctr]->drawn) LiftSprite(sprites[ctr]);
      sprites[ctr]->xpos += xdiff;
      sprites[ctr]->ypos += ydiff;
      if(sprites[ctr]->drawn) DropSprite(sprites[ctr]);
      }
    }

  if(yd <= ys) {
    for(ctr=ys; ctr<ys+ysz; ++ctr) {
      if(depth == 8) {
	memmove(image[ctr+ydiff].u8 + xd, image[ctr].u8 + xs, xsz);
	memmove(backg[ctr+ydiff].u8 + xd, backg[ctr].u8 + xs, xsz);
	}
      else if(depth == 32) {
	memmove(image[ctr+ydiff].u32 + xd, image[ctr].u8 + xs, xsz);
	memmove(backg[ctr+ydiff].u32 + xd, backg[ctr].u8 + xs, xsz);
	}
      else if(depth == 16) {
	memmove(image[ctr+ydiff].u16 + xd, image[ctr].u8 + xs, xsz);
	memmove(backg[ctr+ydiff].u16 + xd, backg[ctr].u8 + xs, xsz);
	}
      }
    }
  else {
    for(ctr=ys+ysz-1; ctr>=ys; --ctr) {
      if(depth == 8) {
	memmove(image[ctr+ydiff].u8 + xd, image[ctr].u8 + xs, xsz);
	memmove(backg[ctr+ydiff].u8 + xd, backg[ctr].u8 + xs, xsz);
	}
      else if(depth == 32) {
	memmove(image[ctr+ydiff].u32 + xd, image[ctr].u8 + xs, xsz);
	memmove(backg[ctr+ydiff].u32 + xd, backg[ctr].u8 + xs, xsz);
	}
      else if(depth == 16) {
	memmove(image[ctr+ydiff].u16 + xd, image[ctr].u8 + xs, xsz);
	memmove(backg[ctr+ydiff].u16 + xd, backg[ctr].u8 + xs, xsz);
	}
      }
    }

  switch(vtype)  {  
    #ifdef X_WINDOWS
    #ifdef XF86_DGA
    #ifdef X_DGA2
    case(VIDEO_XDGA2): {
      if(DGAFlags & XDGABlitRect) {
	XDGACopyArea(_Xdisplay, _Xscreen, xs, ys, xsz, ysz, xd, yd);
	if(!(DGAFlags & XDGAConcurrentAccess)) XDGASync(_Xdisplay, _Xscreen);
	break;
	}
      }	// No "break;" - passes through to "default:"
    #endif
    #endif
    #endif
    default: {
      InvalidateRectangle(pxs[p], pys[p], pxe[p]-pxs[p], pye[p]-pys[p]);
      }break;
    }
  }

Palette &Screen::GetPalette() {
  return *pal;
  }

#ifdef DOS
void Screen::SetBank(int bank) {
  if(curbank == bank)  return;
  curbank = bank;
  switch(vtype) {
    case(VIDEO_VBE2L): { U2_Exit(-1, "Banking with LFB???\n"); } break;
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
  UserDebug("Screen::DropSprite(s) Begin");
  if((!bins) || (!lbins)) U2_Exit(1, "No bins in Dropsprite!\n");
  Sprite *sp;
  if(s->Flag(SPRITE_HUGE)) {
    UserDebug("Screen::DropSprite(s) In large");
    if(huges==NULL) { huges=s; s->prev = &huges; }
    else {
      for(sp=huges; sp->next != NULL; sp=sp->next);
      sp->next = s; s->prev = &(sp->next);
      }
    }
  else if(s->Flag(SPRITE_LARGE)) {
    UserDebug("Screen::DropSprite(s) In lbin");
    int x=(s->xpos)>>LARGE_BIN_FACTOR;
    int y=(s->ypos)>>LARGE_BIN_FACTOR;
    if(x<0) x=0; if(y<0) y=0;
    if(x>=xlbins) x=xlbins-1;
    if(y>=ylbins) y=ylbins-1;
    if(lbins[x][y]==NULL) { lbins[x][y]=s; s->prev = &lbins[x][y]; }
    else {
      for(sp=(lbins[x][y]); sp->next != NULL; sp=sp->next);
      sp->next = s; s->prev = &(sp->next);
      }
    }
  else {
    UserDebug("Screen::DropSprite(s) In bin");
    int x=(s->xpos)>>BIN_FACTOR;
    int y=(s->ypos)>>BIN_FACTOR;
    if(x<0) x=0; if(y<0) y=0;
    if(x>=xbins) x=xbins-1;
    if(y>=ybins) y=ybins-1;
    if(bins[x][y]==NULL) { bins[x][y]=s; s->prev = &bins[x][y]; }
    else {
      for(sp=(bins[x][y]); sp->next != NULL; sp=sp->next);
      sp->next = s; s->prev = &(sp->next);
      }
    }
  UserDebug("Screen::DropSprite(s) End");
  }

void Screen::LiftSprite(Sprite *s) {
  UserDebug("Screen::LiftSprite(s) Begin");
  if(!s->prev) U2_Exit(1, "Lifting non-placed Sprite (%d)!!\n", s->drawn);
  if(s->next) s->next->prev = s->prev;
  *(s->prev) = s->next; s->next = NULL; s->prev=NULL;
  UserDebug("Screen::LiftSprite(s) End");
  }

void Screen::SetApparentDepth(int ad) {
  appdepth = ad;
  pal->depth = ad;
  }

color Screen::GetColor(int r, int g, int b) {
  color ret = 0;
  if(depth == 32) {
    ret = 0xFF000000;
    ret |= (r << 16);
    ret |= (g << 8);
    ret |= b;
    }
  return ret;
  }
