#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <user/os_defs.h>
#include <user/screen.h>

unsigned char *Frame = NULL;

#ifdef X_WINDOWS
#define Screen X_Screen
#define Window X_Window
#define Cursor X_Cursor

#ifdef SVGALIB
#include <vga.h>
#endif

#include <sys/time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#ifdef X_DGA
#include <X11/extensions/xf86dga.h>
#include <X11/extensions/xf86vmode.h>
XF86VidModeModeInfo oldv;
#endif

#undef Screen
#undef Window
#undef Cursor

int IN_X = 1;

int ___myscreen;
Display *___mydisplay;
X_Window ___mywindow;
X_Window ___mywid;
XSizeHints ___myhint;
XWMHints ___mywmhint;
GC ___mygc;
XEvent ___myevent;       
Colormap ___mymap;
XColor ___mypal[256];
XImage *___mypic;
#endif

#ifdef DOS
#include <go32.h>
#include <io.h>
#include <pc.h>
#include <dos.h>
#include <dpmi.h>
#endif

#define Xsize pxend[0]
#define Ysize pyend[0]

extern User *__Da_User;
extern Screen *__Da_Screen;
extern Mouse *__Da_Mouse;

int ColorDepth = 8;

Screen::Screen(const char *nm)  {
  Name = new char[strlen(nm)+2];
  strcpy(Name, (char *)nm);
  InitScreen();
  }

Screen::Screen()  {
  Name = "USER";
  InitScreen();
  }

Screen::Screen(const char *nm, int xs, int ys, int bpp)  {
  Name = new char[strlen(nm)+2];
  strcpy(Name, (char *)nm);
  InitScreen();
  SetSize(xs, ys, bpp);
  }

Screen::Screen(int xs, int ys, int bpp)  {
  Name = "USER";
  InitScreen();
  SetSize(xs, ys, bpp);
  }

Screen::Screen(int md)  {
  Name = "USER";
  InitScreen();
  switch(md)  {

#ifdef DOS
    case(MODE_320x200):
    case(MODE_640x400):
    case(MODE_640x480):
    case(MODE_800x600):
    case(MODE_1024x768):
    case(MODE_1280x1024):  SetMode(md);  break;
    default:	Exit(0, "Unknown Mode %d", md);
#endif

#ifdef X_WINDOWS
    case(MODE_320x200):  SetSize(320, 200, 8);  break;
    case(MODE_640x400):  SetSize(640, 400, 8);  break;
    case(MODE_640x480):  SetSize(640, 480, 8);  break;
    case(MODE_800x600):  SetSize(800, 600, 8);  break;
    case(MODE_1024x768):  SetSize(1024, 768, 8);  break;
    case(MODE_1280x1024):  SetSize(1280, 1024, 8);  break;
    default:	Exit(0, "Unknown Mode %d", md);
#endif
    }
  }

Screen::Screen(const char *nm, int md)  {
  Name = new char[strlen(nm)+2];
  strcpy(Name, (char *)nm);
  InitScreen();
  switch(md)  {

#ifdef DOS
    case(MODE_320x200):
    case(MODE_640x400):
    case(MODE_640x480):
    case(MODE_800x600):
    case(MODE_1024x768):
    case(MODE_1280x1024):  SetMode(md);  break;
    default:	Exit(0, "Unknown Mode %d", md);
#endif

#ifdef X_WINDOWS
    case(MODE_320x200):  SetSize(320, 200);  break;
    case(MODE_640x400):  SetSize(640, 400);  break;
    case(MODE_640x480):  SetSize(640, 480);  break;
    case(MODE_800x600):  SetSize(800, 600);  break;
    case(MODE_1024x768):  SetSize(1024, 768);  break;
    case(MODE_1280x1024):  SetSize(1280, 1024);  break;
    default:	Exit(0, "Unknown Mode %d", md);
#endif
    }
  }

void Screen::InitScreen()  {
  if(__Da_Screen != NULL)  {
    Exit(1, "Two Screens At Once!?  You must be rich.\n");
    }

  Cursor = NULL;
  framedelay = 0;
#ifdef DOS
  lasttime = uclock();
#endif
#ifdef X_WINDOWS
  lasttime = clock();
#endif
  textp = 0;
  curx = 0;
  cury = 0;
  font = NULL;
  mode = 0;
  vmode = NULL;
  VESABlock = NULL;
  numvmodes = 0;
  VidBuf = NULL;
  int ctr;
  graph = 1;
  nextpanel = 1;
  for(ctr=0; ctr<MAX_PANEL; ctr++)  {
    pxstart[ctr] = 0;
    pystart[ctr] = 0;
    pxend[ctr] = 0;
    pyend[ctr] = 0;
    }

#ifdef DOS
  __dpmi_regs dregs;
  int block_seg, block_handle;
  unsigned long vmodes;
  short tmpvmode;

  block_seg = __dpmi_allocate_dos_memory(16, &block_handle);
  if(block_seg == -1)  {
    Exit(1, "Screen init failed.  Not enough DOS memory.\n");
    }
  dregs.x.ax = 0x4F00;
  dregs.x.es = block_seg;
  dregs.x.di = 0x0000;
  __dpmi_int(0x10, &dregs);
  if(dregs.x.ax == 0x004F)  {
    VESABlock = new unsigned char[40];
    dosmemget((block_seg * 16), 40, (void *)VESABlock);

    vmodes = VESABlock[17];
    vmodes <<= 4;
    vmodes += VESABlock[15];
    vmodes <<= 4;
    vmodes += VESABlock[16];
    vmodes <<= 4;
    vmodes += VESABlock[14];
    tmpvmode = 0;
    for(ctr=0; tmpvmode != -1; ctr+=2)  {
      dosmemget(vmodes+ctr, 2, (void *)(&tmpvmode));
      if(tmpvmode != -1)  numvmodes++;
      }
    if(numvmodes > 0)  {
      vmode = new short[numvmodes];
      dosmemget(vmodes, (numvmodes<<1), (void *)vmode);
      }
    }
  __dpmi_free_dos_memory(block_handle);
#endif

#ifdef X_WINDOWS
  ___mydisplay = XOpenDisplay("");
  if(___mydisplay == NULL)  {
#ifdef SVGALIB
    vga_setmousesupport(1);
    if(vga_init() != 0)  {
      Exit(1, "Failed to open display.  Are you in X-Windows?\r\n\
	If so, make sure your DISPLAY variable is set properly.\r\n\
	Also tried to use vgalib, but failed as well.\r\n");
      }
    IN_X = 0;
#else
    Exit(1, "Failed to open display.  Are you in X-Windows?\r\n\
	If so, make sure your DISPLAY variable is set properly.\r\n");
#endif
    }
  else  {
    ___myscreen = DefaultScreen(___mydisplay);
    ColorDepth = DefaultDepth(___mydisplay, ___myscreen);
//    fprintf(stdout, "ColorDepth = %d\n", ColorDepth);
//    if(depth != 8)  {
//      Exit(0, "Sorry, I am one of those loser-apps that can only run in a color depth of 8!\n");
//      }
#ifdef X_DGA
    if(geteuid() == 0)  {
      int M, m;
      if(XF86DGAQueryVersion(___mydisplay, &M, &m))  {
//	fprintf(stderr, "Got DGA version %d.%d\n", M, m);
 	int Xbank, Xmem, Flags;
	XF86DGAQueryDirectVideo(___mydisplay, ___myscreen, &Flags);
	if(!(Flags & XF86DGADirectPresent)) Exit(0, "Failed Query\n");
	XF86DGAGetVideo(___mydisplay, ___myscreen,
	  (char **)&Frame, &Xlen, &Xbank, &Xmem);
//	fprintf(stderr, "Width %d, Bank %d, Mem %d\n",  Xlen, Xbank, Xmem);
	IN_X = 0;
	}
      else fprintf(stderr, "No DGA support.\n");
      if(XF86VidModeQueryVersion(___mydisplay, &M, &m))  {
//	fprintf(stderr, "Got VidMode version %d.%d\n", M, m);
	}
      else fprintf(stderr, "No VidMode support.\n");
      }
#endif
    }
#endif

//  for(ctr = 0; ctr < 768; ctr++)  Palette[ctr] = 0;
  for(ctr=0; ctr < MAX_SPRITES; ctr++)  SpriteList[ctr] = NULL;
  __Da_Screen = this;
  }

int Screen::ModeSupported(int md)  {

#ifdef DOS
  if(md == MODE_320x200)  return (1==1);
  int ctr;
  for(ctr=0; ctr<numvmodes && vmode[ctr] != md; ctr++);
  if(ctr == numvmodes)  return (1==2);
  else	return (1==1);
#endif

#ifdef X_WINDOWS
  XWindowAttributes tmpatt;
  XGetWindowAttributes(___mydisplay, DefaultRootWindow(___mydisplay), &tmpatt);
  switch(md)  {
    case(MODE_320x200):
      if(tmpatt.width < 420 || tmpatt.height < 300)  return (1==2);
      return (1==1);
      break;
    case(MODE_640x400):
      if(tmpatt.width < 740 || tmpatt.height < 500)  return (1==2);
      return (1==1);
      break;
    case(MODE_640x480):
      if(tmpatt.width < 740 || tmpatt.height < 580)  return (1==2);
      return (1==1);
      break;
    case(MODE_800x600):
      if(tmpatt.width < 900 || tmpatt.height < 700)  return (1==2);
      return (1==1);
      break;
    case(MODE_1024x768):
      if(tmpatt.width < 1124 || tmpatt.height < 868)  return (1==2);
      return (1==1);
      break;
    case(MODE_1280x1024):
      if(tmpatt.width < 1380 || tmpatt.height < 1124)  return (1==2);
      return (1==1);
      break;
    default:
      return (1==2);
      break;
    }
#endif

  }

void Screen::SetSize(int xs, int ys, int bpp)  {
#ifdef DOS
  if(xs>1280 || ys>1024)  {
    Exit(0, "Size %dx%d out of range!\r\n", xs, ys);
    }
  else if(xs>1024 || ys>768)  {
    SetMode(MODE_1280x1024);
    }
  else if(xs>800 || ys>600)  {
    SetMode(MODE_1024x768);
    }
  else if(xs>640 || ys>480)  {
    if(bpp == 8)  {
      SetMode(MODE_800x600);
      }
    else  {
      SetMode(MODE_800x600x24);
      }
    }
  else if(xs>640 || ys>400)  {
    SetMode(MODE_640x480);
    }
  else if(xs>320 || ys>200)  {
    SetMode(MODE_640x480);
    }
  else {
    SetMode(MODE_320x200);
    }
#endif

#ifdef X_WINDOWS
  Xsize=xs;
  Ysize=ys;
  if(Frame == NULL) Xlen=xs;
  if((Xlen & 31) != 0)  {
    Xlen += 32;
    Xlen -= (Xlen&31);
    }
  XYsize = Xlen*Ysize;
  if(mode != 0)  {
    Exit(1, "On-the-fly mode switching not yet implemented.\n");
    }
  mode = 1;
  graph = 1;
  nextpanel = 1;

  spritehere = new Sprite **[Xsize / 32];
  backg = new unsigned char **[Xsize / 32];
#endif

  int ctr, ctr2;

#ifdef DOS
  update = new char *[Xsize / 32];
  for(ctr = 0; ctr < (Xsize / 32); ctr++)  {
    spritehere[ctr] = new Sprite *[Ysize];
    backg[ctr] = new unsigned char *[Ysize];
    update[ctr] = new char[Ysize];
    for(ctr2 = 0; ctr2 < Ysize; ctr2++)  {
      spritehere[ctr][ctr2] = NULL;
      backg[ctr][ctr2] = NULL;
      update[ctr][ctr2] = 1;
      }
    }
  BlankScreen();
#endif

#ifdef X_WINDOWS
  upx1 = 0;  upy1 = 0;  upx2 = Xsize-1;  upy2 = Ysize-1;
  for(ctr = 0; ctr < (Xsize / 32); ctr++)  {
    spritehere[ctr] = new Sprite *[Ysize];
    backg[ctr] = new unsigned char *[Ysize];
    for(ctr2 = 0; ctr2 < Ysize; ctr2++)  {
      spritehere[ctr][ctr2] = NULL;
      backg[ctr][ctr2] = NULL;
      }
    }
#endif

  VidBuf = new unsigned char[XYsize];
  bzero(VidBuf, XYsize);

#ifdef X_WINDOWS
  if(IN_X)  {
    ___myhint.x = 0; ___myhint.y = 0;
    ___myhint.width = Xsize; ___myhint.height = Ysize;
    ___myhint.flags = PSize;
    ___myhint.flags |= USSize;
    ___myhint.flags |= PBaseSize;
    ___myhint.flags |= PMinSize;
    ___myhint.flags |= PMaxSize;
//    ___myhint.flags |= PPosition;
//    ___myhint.flags |= USPosition;
//    ___myhint.flags |= PWinGravity;
//    ___myhint.flags |= PResizeInc;
    ___myhint.width = xs;
    ___myhint.height = ys;
    ___myhint.base_height = ___myhint.height;
    ___myhint.base_width = ___myhint.width;
    ___myhint.min_height = ___myhint.height;
    ___myhint.min_width = ___myhint.width;
    ___myhint.max_height = ___myhint.height;
    ___myhint.max_width = ___myhint.width;
  
    ___mywmhint.flags = InputHint;
    ___mywmhint.input = True;

    XSetWindowAttributes wina;

    wina.backing_store = WhenMapped;

    X_Window win1, win2, *win3;  int n;
    ___mywindow = XCreateWindow(___mydisplay,
	DefaultRootWindow(___mydisplay), ___myhint.x, ___myhint.y,
	___myhint.width, ___myhint.height, 5,
	DefaultDepth(___mydisplay, ___mywindow), InputOutput,
	DefaultVisual(___mydisplay, ___mywindow),
	CWBackingStore, &wina);
    if(ColorDepth == 8)  {
      ___mymap = XCreateColormap(___mydisplay, ___mywindow,
        DefaultVisual(___mydisplay, ___myscreen), AllocAll);
      }
    else  {
      ___mymap = XCreateColormap(___mydisplay, ___mywindow,
        DefaultVisual(___mydisplay, ___myscreen), AllocNone);
      }
    ___mypic = XCreateImage(___mydisplay, None, 8, ZPixmap, 0, (char*)VidBuf,
	Xlen, Ysize, 8, 0);
    XSetStandardProperties (___mydisplay, ___mywindow, Name, Name,
      None, NULL, 0, &___myhint);
//    XSetWMNormalHints (___mydisplay, ___mywindow, &___myhint);
    XSetWMHints (___mydisplay, ___mywindow, &___mywmhint);
    ___mygc = XCreateGC(___mydisplay, ___mywindow, 0, 0);
    XSelectInput (___mydisplay, ___mywindow, KeyPressMask | KeyReleaseMask
	| ExposureMask
//	| SubstructureNotifyMask
	| ButtonPressMask
	| ButtonReleaseMask 
	);

    for(ctr = 0; ctr < 256; ctr++)  ___mypal[ctr].pixel = ctr;
    if(ColorDepth == 8)  {
      XQueryColors(___mydisplay, DefaultColormap(___mydisplay, ___myscreen),
	___mypal, 256);
      for(ctr = 0; ctr < 256; ctr++)  {
        palette.SetPaletteEntry(ctr, ___mypal[ctr].red >> 8, 
	___mypal[ctr].green >> 8, ___mypal[ctr].blue >> 8);
        }
      XStoreColors(___mydisplay, ___mymap, ___mypal, 256);
      }
    XSetWindowColormap(___mydisplay, ___mywindow, ___mymap);
    XMapRaised(___mydisplay, ___mywindow);

//    if(ColorDepth == 8)  {
//      XPutImage(___mydisplay, ___mywindow, ___mygc, ___mypic, 0, 0, 0, 0,
//	Xsize, Ysize);
//      }
    if(XQueryTree(___mydisplay, ___mywindow,
      &win1, &win2, (X_Window **)&win3, (unsigned int *)&n))  {
      ___mywid = win1;
//      fprintf(stdout, "::%p::\n", ___mywid);
//      for(;n>=0; n--)  fprintf(stdout, " %p", win3[n]);
//      fprintf(stdout, "\n");
      }
    }
  else  {
    if(Frame != NULL)  {
/*
      ___myhint.x = 0; ___myhint.y = 0;
      ___myhint.width = Xsize; ___myhint.height = Ysize;
      ___myhint.flags = PSize;
      ___myhint.flags |= USSize;
      ___myhint.flags |= PBaseSize;
      ___myhint.flags |= PMinSize;
      ___myhint.flags |= PMaxSize;
      ___myhint.width = xs;
      ___myhint.height = ys;
      ___myhint.base_height = ___myhint.height;
      ___myhint.base_width = ___myhint.width;
      ___myhint.min_height = ___myhint.height;
      ___myhint.min_width = ___myhint.width;
      ___myhint.max_height = ___myhint.height;
      ___myhint.max_width = ___myhint.width;

      XSetWindowAttributes wina;
      wina.backing_store = WhenMapped;
      ___mywindow = XCreateWindow(___mydisplay,
	DefaultRootWindow(___mydisplay), ___myhint.x, ___myhint.y,
	___myhint.width, ___myhint.height, 5,
	DefaultDepth(___mydisplay, ___mywindow), InputOutput,
	DefaultVisual(___mydisplay, ___mywindow),
	CWBackingStore, &wina);
*/
      ___mywindow = DefaultRootWindow(___mydisplay);
      ___mymap = XCreateColormap(___mydisplay, ___mywindow,
        DefaultVisual(___mydisplay, ___myscreen), AllocAll);
      for(ctr = 0; ctr < 256; ctr++)  ___mypal[ctr].pixel = ctr;
      XQueryColors(___mydisplay, DefaultColormap(___mydisplay, ___myscreen),
	___mypal, 256);
      for(ctr = 0; ctr < 256; ctr++)  {
        palette.SetPaletteEntry(ctr, ___mypal[ctr].red >> 8, 
	___mypal[ctr].green >> 8, ___mypal[ctr].blue >> 8);
        }
      XStoreColors(___mydisplay, ___mymap, ___mypal, 256);
//      XSelectInput (___mydisplay, ___mywindow, KeyPressMask | KeyReleaseMask
//	| ExposureMask | ButtonPressMask | ButtonReleaseMask);
//	| SubstructureNotifyMask
//      XMapRaised(___mydisplay, ___mywindow);
//      XSetWindowColormap(___mydisplay, ___mywindow, ___mymap);

      XF86DGADirectVideo(___mydisplay, ___myscreen,
      XF86DGADirectGraphics | XF86DGADirectMouse); // | XF86DGADirectKeyb);
      {
	int ctr, n;
	XF86VidModeModeInfo curv, **allv;
	XF86VidModeGetAllModeLines(___mydisplay, ___myscreen, &n, &allv);
	oldv = *(allv[0]);
	curv = oldv;
	for(ctr=0; ctr<n; ctr++)  {
	  if(allv[ctr]->hdisplay == xs && allv[ctr]->vdisplay == ys)
		curv = *(allv[ctr]);
	  }
	delete(allv);
	XF86VidModeSwitchToMode(___mydisplay, ___myscreen, &curv);
	}
      XF86DGASetViewPort(___mydisplay, ___myscreen, 0, 0);
//      XF86DGADirectVideo(___mydisplay, ___myscreen, 0);
      memset(Frame, 0, 1280*1024);
      XF86DGAInstallColormap(___mydisplay, ___myscreen, ___mymap);
      }
#ifdef SVGALIB
    else  {
      if(xs != 320 || ys != 200)  Exit(0, "Only 320x200 supported in VGALib!\r\n");
      vga_setmode(G320x200x256);
      }
#endif
    }
#endif
  }

#ifdef DOS
void Screen::SetMode(int md)  {
  if(mode != 0)  {
    Exit(1, "On-the-fly mode switching not yet implemented.\n");
    }
  mode = md;
  int ctr, ctr2;

  union REGS inregs;
  union REGS outregs;

  graph = 1;
  nextpanel = 1;
  if(mode == MODE_320x200)  {

    inregs.w.ax = 0x0013;
    int386(0x10, &inregs, &outregs);
    VidMem = 0xA0000;

    Xlen = 320;
    Xsize = 320;
    Ysize = 200;
    XYsize = 320*200;
    curpage = 0;
    }
  else  {

    __dpmi_regs dregs;
    unsigned char block[40];
    int block_seg, block_handle;

    if(!ModeSupported(mode))  {
      Exit(1, "Screen init failed.  VESA mode %X not supported.\n", mode);
      }
    block_seg = __dpmi_allocate_dos_memory(16, &block_handle);
    if(block_seg == -1)  {
      Exit(1, "Screen init failed.  Not enough DOS memory.\n");
      }
    dregs.x.ax = 0x4F01;
    dregs.x.cx = mode;
    dregs.x.es = block_seg;
    dregs.x.di = 0x0000;
    __dpmi_int(0x10, &dregs);
    if(dregs.x.ax != 0x004F)  {
      Exit(1, "Screen init failed.  VESA mode %X failure.\n", mode);
      }
    dosmemget((block_seg * 16), 40, (void *)block);
    __dpmi_free_dos_memory(block_handle);

    unsigned tmpgran;
    tmpgran = block[5];
    tmpgran *= 256;
    tmpgran += block[4];
    granularity = 64 / tmpgran;

    VidMem = block[9];
    VidMem *= 256;
    VidMem += block[8];
    VidMem *= 16;

    inregs.w.ax = 0x4F02;
    inregs.w.bx = mode;
    int386(0x10, &inregs, &outregs);

    switch(mode)  {
      case(MODE_640x400):	Xsize = 640;	Ysize = 400;	break;
      case(MODE_640x480):	Xsize = 640;	Ysize = 480;	break;
      case(MODE_800x600):	Xsize = 800;	Ysize = 600;	break;
      case(MODE_1024x768):	Xsize = 1024;	Ysize = 768;	break;
      case(MODE_1280x1024):	Xsize = 1280;	Ysize = 1024;	break;
      }
    curpage = -1;

    SetPage(0);
    GetLogicalLineLength();
    if(Xlen != Xsize)  {
      if((!(SetLogicalLineLength(Xsize))) && (Xlen < Xsize))  {
	Exit(1, "Screen init failed.  VESA unable to reset line length, and.\nlogical line length < line length.\n%d < %d\n", Xlen, Xsize);
        }
      }
    Xlen = Xsize;

    XYsize = Xlen*Ysize;
    }
  spritehere = new Sprite **[Xsize / 32];
  backg = new unsigned char **[Xsize / 32];
  update = new char *[Xsize / 32];
  for(ctr = 0; ctr < (Xsize / 32); ctr++)  {
    spritehere[ctr] = new Sprite *[Ysize];
    backg[ctr] = new unsigned char *[Ysize];
    update[ctr] = new char[Ysize];
    for(ctr2 = 0; ctr2 < Ysize; ctr2++)  {
      spritehere[ctr][ctr2] = NULL;
      backg[ctr][ctr2] = NULL;
      update[ctr][ctr2] = 1;
      }
    }
  VidBuf = new unsigned char[XYsize];
  bzero(VidBuf, XYsize);

  BlankScreen();
  }
#endif

Screen::~Screen()  {
  __Da_Screen = NULL;

#ifdef DOS
  if(graph == 1)  __asm__ __volatile__("
    movw $0x0003, %%ax\n
    int $0x10"
    :
    :
    : "ax", "memory"
    );
  if(vmode != NULL)  delete vmode;
  if(VESABlock != NULL)  delete VESABlock;
#endif
#ifdef X_WINDOWS
  if(!IN_X)  {
#ifdef X_DGA
    if(Frame != NULL)  {
      XF86VidModeSwitchToMode(___mydisplay, ___myscreen, &oldv);
      XF86DGADirectVideo(___mydisplay, ___myscreen, 0);
      }
#endif
#ifdef SVGALIB
    else  vga_setmode(TEXT);
#endif
    }
#endif
  if(VidBuf != NULL)  delete VidBuf;
  }

#ifdef DOS
void Screen::SetPage(int pg)  {
  if(curpage == pg)  return;
  curpage = pg;
  union REGS inregs;
  union REGS outregs;
  inregs.w.ax = 0x4F05;
  inregs.w.bx = 0x0000;
  inregs.w.dx = (curpage * granularity);
  int386(0x10, &inregs, &outregs);
  }

int Screen::SetLogicalLineLength(int len)  {
  union REGS inregs;
  union REGS outregs;
  inregs.w.ax = 0x4F06;
  inregs.w.bx = 0x0000;
  inregs.w.cx = len;
  int386(0x10, &inregs, &outregs);
  Xlen = ((int)(short)outregs.x.cx);
  return(outregs.x.ax == 0x004F);
  }

int Screen::GetLogicalLineLength()  {
  union REGS inregs;
  union REGS outregs;
  inregs.w.ax = 0x4F06;
  inregs.w.bx = 0x0001;
  int386(0x10, &inregs, &outregs);
  Xlen = ((int)(short)outregs.x.cx);
  return(Xlen);
  }
#endif

Color Screen::GetPoint(int X, int Y)  {
  return VidBuf[X+(Y*Xlen)];
  }

void Screen::FillRectangle(int x1, int y1, int x2, int y2, Color c)  {
  if(x1>x2)  return;
  if(y1>y2)  return;
  int xs = (x2-x1)+1;
  int ctry;
  for(ctry=y1; ctry<=y2; ctry++)  {
    memset(&VidBuf[x1+(ctry*Xlen)], c, xs);
    }
  InvalidateRectangle(x1, y1, x2, y2);
  }

void Screen::BorderRectangle(int x1, int y1, int x2, int y2, Color c)  {
  if(x1>x2)  return;
  if(y1>y2)  return;
  int xs = (x2-x1)+1;
  int ys = (y2-y1)+1;
  int ctry;
  memset(&VidBuf[x1+(y1*Xlen)], c, xs);
  if(ys<2)  return;
  memset(&VidBuf[x1+(y2*Xlen)], c, xs);
  if(ys<3)  return;
  for(ctry=y1+1; ctry<y2; ctry++)  {
    VidBuf[x1+(ctry*Xlen)] = c;
    VidBuf[x2+(ctry*Xlen)] = c;
    }
  InvalidateRectangle(x1, y1, x2, y2);
  }

void Screen::SetPoint(int X, int Y, Color C)  {
  VidBuf[X+(Y*Xlen)] = C;
  InvalidateRectangle(X, Y, X, Y);
  }

void Screen::InvalidateRectangle(int x1, int y1, int x2, int y2)  {
#ifdef DOS
  int ctrx, ctry;
  for(ctrx = x1/32; ctrx <= x2/32; ctrx++)  {
    for(ctry = y1; ctry <= y2; ctry++)  {
      update[ctrx][ctry] = 1;
      }
    }
#endif
#ifdef X_WINDOWS
  int tmp=0;
  if(upx1 > x1)  { upx1 = x1;  tmp=1; }
  if(upy1 > y1)  { upy1 = y1;  tmp=1; }
  if(upx2 < x2)  { upx2 = x2;  tmp=1; }
  if(upy2 < y2)  { upy2 = y2;  tmp=1; }

#endif
  }

void Screen::BSetPoint(int X, int Y, Color C)  {
  VidBuf[X+(Y*Xlen)] = C;
  if(backg[(X/32)][Y] != NULL)  backg[(X/32)][Y][(X%32)] = C;
  InvalidateRectangle(X, Y, X, Y);
  }

void Screen::GetFriendlyColor(Color *pe, unsigned char r, unsigned char g,
        unsigned char b)  {
#ifdef X_WINDOWS
  if(ColorDepth == 8)  {
    XColor col;
    col.red = (r<<8);
    col.green = (g<<8);
    col.blue = (b<<8);
    col.flags = DoRed | DoBlue | DoGreen;
    if(!(XAllocColor(___mydisplay, DefaultColormap(___mydisplay,___myscreen),
	&col)))
      Exit(0, "Failed to allocate friendly color!\r\n");
    *pe = col.pixel;
    }
  else  *pe = palette.coldec;
  SetPaletteEntry(*pe, r, g, b);
#endif
  } 


void Screen::SetPaletteEntry(Color ent, cval r, cval g, cval b)  {
debug_position = 300;

#ifdef X_WINDOWS
  unsigned long pix = ent;
  if(ColorDepth != 8)  {
    XColor col;
    col.red = (r<<8);
    col.green = (g<<8);
    col.blue = (b<<8);
    col.flags = DoRed | DoBlue | DoGreen;
debug_position = 305;
//    fprintf(stdout, "Here once, for %d -> (%d, %d, %d)\n", ent, r, g, b);
    if(!(XAllocColor(___mydisplay, DefaultColormap(___mydisplay,___myscreen),
	&col)))  {
debug_position = 306;
      Exit(0, "Failed to locate color!\r\n");
      }
debug_position = 307;
    pix = col.pixel;
//    fprintf(stdout, "Got %.6X\n", pix);
debug_position = 308;
    palette.SetTrueColor(ent, pix);
    }
#endif

debug_position = 309;
  palette.SetPaletteEntry(ent, r, g, b);

debug_position = 310;

//  fprintf(stdou[Bt, "Set %d to (%d, %d, %d), as 0x%X\n", ent, r, g, b, pix);
//  fprintf(stdout, "Palette defined: %d entries\n", palette.coldec);

  if(blank == 0)  {

#ifdef DOS
    outportb(0x3C8, ent);
    outportb(0x3C9, r);
    outportb(0x3C9, g);
    outportb(0x3C9, b);
#endif
debug_position = 360;
#ifdef X_WINDOWS
    if(IN_X || (Frame != NULL))  {
      ___mypal[ent].pixel = pix;
      ___mypal[ent].red = r<<8;
      ___mypal[ent].green = g<<8;
      ___mypal[ent].blue = b<<8;
      ___mypal[ent].flags = DoRed | DoGreen | DoBlue;
      if(Frame != NULL || ColorDepth == 8)  {
	XStoreColors(___mydisplay, ___mymap, &___mypal[ent], 1);
        if(Frame != NULL)
	  XF86DGAInstallColormap(___mydisplay, ___myscreen, ___mymap);
	}
      }
#ifdef SVGALIB
    else  {
      vga_setpalette(ent, r>>2, g>>2, b>>2);
      }
#endif
#endif
    }
debug_position = 350;
  }

void Screen::BlankScreen()  {

#ifdef DOS
  int ctr;
  outportb(0x3C8, 0);
  for(ctr = 0; ctr < 768; ctr++)  {
    outportb(0x3C9, 0);
    }
#endif

#ifdef X_WINDOWS
  char tmppal[768];
  bzero(tmppal, 768);
  SetPalette((unsigned char *)tmppal);
#endif

  blank = 1;
  }

void Screen::ShowScreen()  {
  SetPalette(palette.colors);
  blank = 0;
  }

void Screen::FadeIn()  {
  FadeIn(4);
  }

void Screen::FadeOut()  {
  FadeOut(4);
  }

void Screen::FadeIn(int rate) {
  int ctr, ctr2;
  char ptmp[768];
  for(ctr2 = 1; ctr2 < (256/rate); ctr2++)  {
    for(ctr = 0; ctr < 768; ctr++)  ptmp[ctr] = (palette.colors[ctr] * ctr2) /
	((256/rate) - 1);

#ifdef DOS
    while((inportb(0x03DA) & 0x08) != 0);
    while((inportb(0x03DA) & 0x08) == 0);
    outportb(0x3C8, 0);
    for(ctr = 0; ctr < 768; ctr++)  {
//      if((ctr & 63) == 0)  while((inportb(0x03DA) & 0x08) == 0);
      outportb(0x3C9, ptmp[ctr]>>2);
      }
#endif

#ifdef X_WINDOWS
    SetPalette((unsigned char *)ptmp);
#endif

    }
  blank = 0;
  }

void Screen::FadeOut(int rate)  {
  int ctr, ctr2;
  char ptmp[768];
  for(ctr2 = ((256/rate) -1); ctr2 >= 0; ctr2--)  {
    for(ctr = 0; ctr < 768; ctr++)  ptmp[ctr] = (palette.colors[ctr] * ctr2) /
	((256/rate) - 1);

#ifdef DOS
    while((inportb(0x03DA) & 0x08) != 0);
    while((inportb(0x03DA) & 0x08) == 0);
    outportb(0x3C8, 0);
    for(ctr = 0; ctr < 768; ctr++)  {
//      if((ctr & 63) == 0)  while((inportb(0x03DA) & 0x08) == 0);
      outportb(0x3C9, ptmp[ctr]>>2);
      }
#endif

#ifdef X_WINDOWS
  SetPalette((unsigned char *)ptmp);
#endif

    }
  blank = 1;
  }

void Screen::ClearScreen()  {
  bzero(VidBuf, XYsize);
  }

void Screen::RefreshFull()  {
  if(__Da_Mouse != NULL)  __Da_Mouse->Update();
  if(__Da_User != NULL)  __Da_User->Update();

#ifdef DOS
  int pg = 0, ctr, ctr2;
  SetPage(0);
  for(;pg < (XYsize / 65536); pg++)  {
    dosmemput((void *)&VidBuf[pg*65536], 65536, VidMem);
    SetPage(pg+1);
    }
  dosmemput((void *)&VidBuf[pg*65536], (XYsize%65536), VidMem);
  for(ctr = 0; ctr < (Xsize/32); ctr++)
    for(ctr2 = 0; ctr2 < Ysize; ctr2++)
      update[ctr][ctr2] = 0;
#endif

#ifdef X_WINDOWS
  if(IN_X)  {
    XFlush(___mydisplay);
    while(XCheckMaskEvent(___mydisplay, ExposureMask, &___myevent));
    if(ColorDepth == 8)  {
      XPutImage(___mydisplay, ___mywindow, ___mygc, ___mypic, 0, 0, 0, 0,
	Xsize, Ysize);
      }
    else  {
      int ctrx, ctry;
      for(ctry=0; ctry<Ysize; ctry++)  {
	for(ctrx=0; ctrx<Xsize; ctrx++)  {
	  XSetForeground(___mydisplay, ___mygc,
		palette.tcols[VidBuf[(ctry*Xlen)+ctrx]]);
	  XDrawPoint(___mydisplay, ___mywindow, ___mygc, ctrx, ctry);
	  }
	}
      }
    upx1 = Xsize;  upy1 = Ysize;  upx2 = -1;  upy2 = -1;
    }
  else  {
    if(Frame != NULL)  {
      int(ctr);
      for(ctr=0; ctr<Ysize; ctr++)
	memcpy(&Frame[Xlen*ctr], &VidBuf[Xlen*ctr], Xsize);
      upx1 = Xsize;  upy1 = Ysize;  upx2 = -1;  upy2 = -1;
      }
#ifdef SVGALIB
    else  {
      memcpy(graph_mem, VidBuf, 320*200);
      upx1 = Xsize;  upy1 = Ysize;  upx2 = -1;  upy2 = -1;
      }
#endif
    }
#endif
  WaitForNextFrame();
  }

void Screen::Refresh()  {
  RefreshFast();
  WaitForNextFrame();
  }

void Screen::RefreshFast()  {
  if(__Da_Mouse != NULL)  __Da_Mouse->Update();
  if(__Da_User != NULL)  __Da_User->Update();

#ifdef DOS
  if(mode == MODE_320x200)  {
    int ctr, ctr2;
    for(ctr = 0; ctr < (Xsize/32); ctr++)  {
      for(ctr2 = 0; ctr2 < Ysize; ctr2++)  {
	if(update[ctr][ctr2] == 1)
	  dosmemput((void *)&VidBuf[(ctr2*Xlen) + (ctr*32)], 32,
		(VidMem + (ctr2*Xlen) + (ctr*32)));
	update[ctr][ctr2] = 0;
	}
      }
    }
  else  {
    int ctr, ctr2, rawloc;
    SetPage(0);
    for(ctr2 = 0; ctr2 < Ysize; ctr2++)  {
      for(ctr = 0; ctr < (Xlen/32); ctr++)  {
	rawloc = ((ctr2*Xlen) + (ctr*32)) % 65536;
	if(ctr<(Xsize/32))  {
	  if(update[ctr][ctr2] == 1)
	    dosmemput((void *)&VidBuf[(ctr2*Xlen) + (ctr*32)],
		32,(VidMem + rawloc));
	  update[ctr][ctr2] = 0;
	  }
	if(rawloc >= (65536 - 32))  SetPage(curpage+1);
	}
      }
    }
#endif

#ifdef X_WINDOWS
  if(IN_X)  {
    while(XCheckMaskEvent(___mydisplay, ExposureMask, &___myevent))  {
      if(___myevent.xexpose.window == ___mywindow)
        InvalidateRectangle(___myevent.xexpose.x, ___myevent.xexpose.y,
	  ___myevent.xexpose.x+___myevent.xexpose.width-1,
	  ___myevent.xexpose.y+___myevent.xexpose.height-1);
      }
    if(upx1 <= upx2 && upy1 <= upy2)  {
//      printf("Restoring (%d, %d) -> (%d, %d)\r\n", upx1, upy1, upx2, upy2);
      if(ColorDepth == 8)  {
	XPutImage(___mydisplay, ___mywindow, ___mygc, ___mypic,
		upx1, upy1, upx1, upy1, (upx2-upx1)+1, (upy2-upy1)+1);
	}
      else  {
	int ctrx, ctry;

// ****************************************************************************
// ***************  To Test Redrawing, uncomment the following  ***************
// ****************************************************************************
//	XSetForeground(___mydisplay, ___mygc,
//		palette.tcols[0]);
//	XFillRectangle(___mydisplay, ___mywindow, ___mygc, 0, 0, Xsize, Ysize);
// ****************************************************************************

	for(ctry=upy1; ctry<=upy2; ctry++)  {
	  for(ctrx=upx1; ctrx<=upx2; ctrx++)  {
	    XSetForeground(___mydisplay, ___mygc,
		palette.tcols[VidBuf[(ctry*Xlen)+ctrx]]);
	    XDrawPoint(___mydisplay, ___mywindow, ___mygc, ctrx, ctry);
	    }
	  }
	}
      }
    upx1 = Xsize;  upy1 = Ysize;  upx2 = -1;  upy2 = -1;
    }
  else  {
    if(Frame != NULL)  {
      int ctr;
      if(upx1<=upx2)  {
	for(ctr=upy1; ctr<=upy2; ctr++)  {
	  memcpy(Frame+Xlen*ctr+upx1, VidBuf+Xlen*ctr+upx1, (upx2-upx1)+1);
	  }
	}
      upx1 = Xsize;  upy1 = Ysize;  upx2 = -1;  upy2 = -1;
      }
#ifdef SVGALIB
    else  {
      int ctr;
      if(upx1<=upx2)  {
        for(ctr=upy1; ctr<=upy2; ctr++)  {
	  memcpy(graph_mem+Xlen*ctr+upx1, VidBuf+Xlen*ctr+upx1, (upx2-upx1)+1);
	  }
        }
      upx1 = Xsize;  upy1 = Ysize;  upx2 = -1;  upy2 = -1;
      }
#endif
    }
#endif
  }

void Screen::PastePartialGraphic(const Graphic &source, int basex, int basey,
	int xtrans, int ytrans, int X, int Y)  {
  if(basex < 0)  { xtrans += basex;  X -= basex;  basex = 0;  }
  if(basey < 0)  { ytrans += basey;  Y -= basey;  basey = 0;  }
  if(basex + xtrans > (long)source.xsize)  xtrans = source.xsize - basex;
  if(basey + ytrans > (long)source.ysize)  ytrans = source.ysize - basey;
  if((xtrans + X) > Xsize)  xtrans = Xsize - X;
  if((ytrans + Y) > Ysize)  ytrans = Ysize - Y;
  if(X < 0)  xtrans += X;
  if(Y < 0)  ytrans += Y;
  if((xtrans < 1) || (ytrans < 1))  return;

  int ctr, xbeg = 0, ybeg = 0, xloss = 0;
  if(Y > 0)  ybeg = Y;
  if(X > 0)  xbeg = X;
  if(X < 0)  xloss = -X;
  for(ctr = ybeg; ctr < (ybeg+ytrans); ctr++)  {
    memcpy((void *)&VidBuf[(ctr*Xlen)+xbeg],
	(void *)&source.image[ctr-Y+basey][xloss+basex], xtrans);
    }
  }

void Screen::PasteGraphic(const Graphic &source, int X, int Y)  {
  int xtrans, ytrans;
  xtrans = Xsize - X;
  ytrans = Ysize - Y;
  if((int)source.xsize < xtrans)  xtrans = (int)source.xsize;
  if((int)source.ysize < ytrans)  ytrans = (int)source.ysize;
  if(((int)source.xsize + X)< xtrans)  xtrans = (int)source.xsize + X;
  if(((int)source.ysize + Y)< ytrans)  ytrans = (int)source.ysize + Y;
  if((xtrans < 1) || (ytrans < 1))  return;

  int ctr, ctr2, xbeg = 0, ybeg = 0, xloss = 0;
  if(Y > 0)  ybeg = Y;
  if(X > 0)  xbeg = X;
  if(X < 0)  xloss = -X;
  IntList moved;
  InvalidateRectangle(xbeg, ybeg, xbeg+xtrans-1, ybeg+ytrans-1);
  for(ctr = ybeg; ctr < (ybeg+ytrans); ctr++)  {
    for(ctr2=(xbeg/32); ctr2<=((xbeg+xtrans)/32); ctr2++)  {
      moved += EraseSpritesHere(ctr2, ctr);
      if(backg[ctr2][ctr] != NULL)  {
	delete backg[ctr2][ctr];
	backg[ctr2][ctr] = NULL;
	}
      }
    memcpy((void *)&VidBuf[(ctr*Xlen)+xbeg],
	(void *)&source.image[ctr-Y][xloss], xtrans);
    }
  RedrawSprites(moved);
  }

void Screen::FullScreenGraphic(const Graphic &source)  {
  IntList moved = ErasePanelSprites(0);
  ErasePanelBackground(0);
  int xtrans, ytrans;
  xtrans = Xsize;
  ytrans = Ysize;
  if((long)source.xsize < xtrans)  xtrans = source.xsize;
  if((long)source.ysize < ytrans)  ytrans = source.ysize;
  int ctr;
  for(ctr = 0; ctr < ytrans; ctr++)  {
    memcpy((void *)&VidBuf[(ctr*Xlen)], (void *)source.image[ctr], xtrans);
    }
  RedrawSprites(moved);
  }

void Screen::Scroll(int X, int Y)  {
  if((X == 0) && (Y == 0))  return;
  Scroll1(X, Y);
  Scroll2(X, Y);
  }

void Screen::Scroll1(int X, int Y)  {
  if((X == 0) && (Y == 0))  return;
  int ctr, ctr2;
  for(ctr=0; ctr<MAX_SPRITES; ctr++)  {
    if(SpriteList[ctr] != NULL)  {
      if(SpriteList[ctr]->visible == 1)  {
	SpriteList[ctr]->Erase();
	SpriteList[ctr]->visible = 1;
	}
      if(SpriteList[ctr]->scrolls == 1)  {
	SpriteList[ctr]->xpos -= X;
	SpriteList[ctr]->ypos -= Y;
	}
      }
    }
  for(ctr = 0; ctr < (Xsize / 32); ctr++)  {
    for(ctr2 = 0; ctr2 < Ysize; ctr2++)  {
      if(backg[ctr][ctr2] != NULL)  delete backg[ctr][ctr2];
      backg[ctr][ctr2] = NULL;
      }
    }
  unsigned char *tmpbuf;
  tmpbuf = new unsigned char[Xlen*Ysize];
  if((X < Xlen) && ((-X) < Xlen) && (Y < Ysize) && ((-Y) < Ysize)) {
    if((X >= 0) && (Y >= 0))  {
      memcpy((void *)tmpbuf, (void *)&VidBuf[(Y*Xlen)+X],
		(Ysize-Y)*Xlen-X);
      if(X>0)
	for(ctr=0; ctr<Ysize; ctr++)
	  memset((void *)&tmpbuf[(ctr+1)*Xlen-X], 0, X);
      if(Y>0)
	for(ctr=Ysize-Y; ctr<Ysize; ctr++)
	  memset((void *)&tmpbuf[ctr*Xlen], 0, Xlen);
      }
    else if((X < 0) && (Y >= 0))  {
      memcpy((void *)&tmpbuf[-X], (void *)&VidBuf[(Y*Xlen)],
		(Ysize-Y)*Xlen+X);
      for(ctr=0; ctr<Ysize; ctr++)
	memset((void *)&tmpbuf[ctr*Xlen], 0, -X);
      if(Y>0)
	for(ctr=Ysize-Y; ctr<Ysize; ctr++)
	  memset((void *)&tmpbuf[ctr*Xlen], 0, Xlen);
      }
    else if((X >= 0) && (Y < 0))  {
      memcpy((void *)&tmpbuf[-(Y*Xlen)], (void *)&VidBuf[X],
		(Ysize+Y)*Xlen-X);
      if(X>0)
	for(ctr=0; ctr<Ysize; ctr++)
	  memset((void *)&tmpbuf[(ctr+1)*Xlen-X], 0, X);
      for(ctr=0; ctr<(-Y); ctr++)
	memset((void *)&tmpbuf[ctr*Xlen], 0, Xlen);
      }
    else if((X < 0) && (Y < 0))  {
      memcpy((void *)&tmpbuf[-(Y*Xlen)-X], (void *)VidBuf,
		(Ysize+Y)*Xlen+X);
      for(ctr=0; ctr<Ysize; ctr++)
	memset((void *)&tmpbuf[ctr*Xlen], 0, -X);
      for(ctr=0; ctr<(-Y); ctr++)
	memset((void *)&tmpbuf[ctr*Xlen], 0, Xlen);
      }
    }
  delete VidBuf;
  VidBuf = tmpbuf;
  }

void Screen::Scroll2(int X, int Y)  {
  if((X == 0) && (Y == 0))  return;
  int ctr;
  for(ctr=0; ctr<MAX_SPRITES; ctr++)  {
    if(SpriteList[ctr] != NULL)  {
      if(SpriteList[ctr]->visible == 1)  {
	SpriteList[ctr]->visible = 0;
	SpriteList[ctr]->Draw();
	}
      }
    }
  InvalidateRectangle(0, 0, Xsize-1, Ysize-1);
  }


void Screen::SetBlock16(int X, int Y, unsigned char *data)  {
  X *= 16;
  InvalidateRectangle(X, Y, X+15, Y);
  bcopy(data, &VidBuf[X+(Y*Xlen)], 16);
  }

void Screen::SetBlock32(int X, int Y, unsigned char *data)  {
  X *= 32;
  if((X < 0) || (Y < 0) || (X >= Xsize) || (Y >= Ysize))  return;
  InvalidateRectangle(X, Y, X+31, Y);
  bcopy(data, &VidBuf[X+(Y*Xlen)], 32);
  }

void Screen::SSetBlock(int tcol, int X, int Y, unsigned char *data)  {
  int ctr;
  X *= 32;
  InvalidateRectangle(X, Y, X+31, Y);
  for(ctr = 0; ctr < 32; ctr++)
    if(data[ctr] != tcol)  VidBuf[X+(Y*Xlen)+ctr] = data[ctr];
  }

void Screen::SetBlock64(int X, int Y, unsigned char *data)  {
  X *= 64;
  InvalidateRectangle(X, Y, X+63, Y);
  bcopy(data, &VidBuf[X+(Y*Xlen)], 64);
  }

unsigned char *Screen::GetBlock16(int X, int Y)  {
  return &VidBuf[(X*16)+(Y*Xlen)];
  }

unsigned char *Screen::GetBlock32(int X, int Y)  {
  return &VidBuf[(X*32)+(Y*Xlen)];
  }

unsigned char *Screen::GetBlock64(int X, int Y)  {
  return &VidBuf[(X*64)+(Y*Xlen)];
  }

void Screen::SetPalette(unsigned char *pal)  {
  int ctr;

//  palette.SetPalette(pal);

#ifdef DOS
  outportb(0x3C8, 0);
  for(ctr = 0; ctr < 768; ctr++)
    outportb(0x3C9, pal[ctr]>>2);
#endif

#ifdef X_WINDOWS
//  ***************
  if(IN_X || Frame != NULL)  {
    for(ctr = 0; ctr < 256; ctr++)  {
      ___mypal[ctr].pixel = ctr;
      ___mypal[ctr].red = pal[ctr*3]<<8;
      ___mypal[ctr].green = pal[ctr*3+1]<<8;
      ___mypal[ctr].blue = pal[ctr*3+2]<<8;
      ___mypal[ctr].flags = DoRed | DoGreen | DoBlue;
      }
    if(Frame != NULL || ColorDepth == 8)  {
      XStoreColors(___mydisplay, ___mymap, ___mypal, 256);
      if(Frame != NULL)
	XF86DGAInstallColormap(___mydisplay, ___myscreen, ___mymap);
      }
    }
#ifdef SVGALIB
  else  {
    int buf2[768];
    for(ctr=0; ctr<768; ctr++)  buf2[ctr] = pal[ctr]>>2;
    vga_setpalvec(0, 256, buf2);
    }
#endif
#endif
  }

void Screen::DeletePanel(Panel w)  {
  pxstart[nextpanel] = -1;
  pystart[nextpanel] = -1;
  pxend[nextpanel] = -1;
  pyend[nextpanel] = -1;
  nextpanel--;
  if(__Da_Mouse != NULL)  {
    __Da_Mouse->SetPanelBehavior(w, -1, -1, -1);
    }
  }

Panel Screen::NewPanel(int xb, int yb, int xe, int ye)  {
  if(nextpanel > MAX_PANEL)  {
    Exit(1, "Too many panels.\n");
    }
  pxstart[nextpanel] = xb;
  pxend[nextpanel] = xe;
  pystart[nextpanel] = yb;
  pyend[nextpanel] = ye;
  nextpanel++;
  return (nextpanel-1);
  }

void Screen::RestorePanelBackground(Panel w)  {
  int ctr, ctr2;
  for(ctr = pxstart[w]/32; ctr < pxend[w]/32; ctr++)  {
    for(ctr2 = pystart[w]; ctr2 < pyend[w]; ctr2++)  {
      RestoreBack(ctr, ctr2);
      }
    }
  }

void Screen::ErasePanelBackground(Panel w)  {
  int ctr, ctr2;
  for(ctr = pxstart[w]/32; ctr < pxend[w]/32; ctr++)  {
    for(ctr2 = pystart[w]; ctr2 < pyend[w]; ctr2++)  {
      if(backg[ctr][ctr2] != NULL)  delete backg[ctr][ctr2];
      backg[ctr][ctr2] = NULL;
      }
    }
  }

IntList Screen::ErasePanelSprites(Panel w)  {
  IntList ret;
  int ctr, ctr2;
  for(ctr = pxstart[w]/32; ctr < pxend[w]/32; ctr++)  {
    for(ctr2 = pystart[w]; ctr2 < pyend[w]; ctr2++)  {
      ret += EraseSpritesHere(ctr, ctr2);
      }
    }
  return ret;
  }

unsigned char *Screen::BackupPanel(Panel p)  {
  int ctr;
  int xsize = (pxend[p]-pxstart[p]);
  int ysize = (pyend[p]-pystart[p]);
  unsigned char *ret = new unsigned char[ysize*xsize];
  unsigned char *ptr;
  for(ctr=pystart[p]; ctr<pyend[p]; ctr++)  {
    ptr = &ret[xsize*(ctr-pystart[p])];
    memcpy(ptr, &VidBuf[(ctr*Xlen)+pxstart[p]], xsize);
    }
  return ret;
  }

void Screen::RestorePanel(Panel p, unsigned char *buf)  {
  int ctr;
  int xsize = (pxend[p]-pxstart[p]);
  unsigned char *ptr;
  for(ctr=pystart[p]; ctr<pyend[p]; ctr++)  {
    ptr = &buf[xsize*(ctr-pystart[p])];
    memcpy(&VidBuf[(ctr*Xlen)+pxstart[p]], ptr, xsize);
    }
  delete buf;
  InvalidateRectangle(pxstart[p], pystart[p], pxend[p]-1, pyend[p]-1);
  }

void Screen::ScrollPanel32(Panel p, int X, int Y)  {
  int ctr, xs, xe, ys, ye, xt;
  ys=pystart[p];	ye=pyend[p];
  if(Y>0)  ye-=Y;
  else  ys-=Y;
  xs=pxstart[p]/32;	xe=pxend[p]/32;
  if(X>0)  { xe-=X; xt=(pxend[p]-pxstart[p])-(X*32); }
  else  { xs-=X; xt=(pxend[p]-pxstart[p])+(X*32); }

  IntList moved = ErasePanelSprites(p);
  ErasePanelBackground(p);

  for(ctr=0; ctr<moved.Size(); ctr++)  {
    SpriteList[moved[ctr]]->xpos-=(X*32);
    SpriteList[moved[ctr]]->ypos-=Y;
    }

  if(Y>0)  {
    for(ctr=ys; ctr<ye; ctr++)  {
      memmove(&VidBuf[ctr*Xlen+xs*32], &VidBuf[(ctr+Y)*Xlen+(xs+X)*32], xt);

//      if(X>0)  {
//	for(ctr2=xs; ctr2<(xe-1); ctr2++)  {
//	  backg[ctr2][ctr] = backg[ctr2+X][ctr+Y];
//	  spritehere[ctr2][ctr] = spritehere[ctr2+X][ctr+Y];
//	  }
//	backg[ctr2][ctr] = NULL;
//	}
//      else  {
//	for(ctr2=xe-1; ctr2>xs; ctr2--)  {
//	  backg[ctr2][ctr] = backg[ctr2-X][ctr-Y];
//	  spritehere[ctr2][ctr] = spritehere[ctr2+X][ctr+Y];
//	  }
//	backg[ctr2][ctr] = NULL;
//	}

      }
    }
  else  {
    for(ctr=ye-1; ctr>=ys; ctr--)  {
      memmove(&VidBuf[ctr*Xlen+xs*32], &VidBuf[(ctr+Y)*Xlen+(xs+X)*32], xt);

//      if(X>0)  {
//	for(ctr2=xs; ctr2<(xe-1); ctr2++)  {
//	  backg[ctr2][ctr] = backg[ctr2+X][ctr+Y];
//	  spritehere[ctr2][ctr] = spritehere[ctr2+X][ctr+Y];
//	  }
//	backg[ctr2][ctr] = NULL;
//	}
//      else  {
//	for(ctr2=xe-1; ctr2>xs; ctr2--)  {
//	  backg[ctr2][ctr] = backg[ctr2-X][ctr-X];
//	  spritehere[ctr2][ctr] = spritehere[ctr2+X][ctr+Y];
//	  }
//	backg[ctr2][ctr] = NULL;
//	}

      }
    }
#ifdef X_WINDOWS
  if(IN_X)  {
    if(X == 0)  {
      if(Y > 0)  {
	XCopyArea(___mydisplay, ___mywindow, ___mywindow, ___mygc,
	  pxstart[p], pystart[p]+Y, pxend[p]-pxstart[p], (pyend[p]-pystart[p])-Y,
	  pxstart[p], pystart[p]);
	}
      else if(Y < 0)  {
	XCopyArea(___mydisplay, ___mywindow, ___mywindow, ___mygc,
	  pxstart[p], pystart[p], pxend[p]-pxstart[p], (pyend[p]-pystart[p])+Y,
	  pxstart[p], pystart[p]-Y);
	}
      }
    }
#endif
  RedrawSprites(moved);
  }

int Screen::print(Color cb, Color cf, char *text)  {
  if(font == NULL)  return -1;
  unsigned char *ind = (unsigned char *)text;
//  printf("%s\r\n", text);
  for(;(*ind) != 0; ind++)  {
    if((*ind) == (unsigned char)'\t')  {
      int tabstops = __Da_Screen->GetYSize() / 10;
      curx+=tabstops;
      curx-=(curx%tabstops);
      AlignCursor();
      }
    else if((*ind) == (unsigned char)KEY_LEFT)  {
      curx-=font[' ']->xsize;
      AlignCursor();
      }
    else if((*ind) == (unsigned char)KEY_RIGHT)  {
      curx+=font[' ']->xsize;
      AlignCursor();
      }
    else if((*ind) == (unsigned char)KEY_UP)  {
      cury-=(font[' ']->ysize+2);
      AlignCursor();
      }
    else if((*ind) == (unsigned char)KEY_DOWN)  {
      cury+=(font[' ']->ysize+2);
      AlignCursor();
      }
    else if((*ind) == (unsigned char)'\n')  {
      curx=1;
      cury+=(font[' ']->ysize+2);
      AlignCursor();
      }
    else if((*ind) == (unsigned char)'\r')  {
      curx=1;
      AlignCursor();
      }
    else if(font[*ind] != NULL)  {
      if(((int)curx + (int)font[*ind]->xsize + (int)font[*ind]->xcenter)
		>= (int)pxend[textp])  {
        curx=1;
        cury+=(font[' ']->ysize+2);
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
      PasteGraphic(let, curx-let.xcenter, cury-let.ycenter);
      curx+=font[*ind]->xsize+1;
      AlignCursor();
      }
    }
  if(Cursor != NULL)  Cursor->Move(curx, cury);
  return curx;
  }

int Screen::printf(Color cb, Color cf, const char *text, ...)  {
  int ret;
  va_list stuff;
  va_start(stuff, text);
  char buf[4096];
  bzero(buf, 4096);
  vsprintf(buf, text, stuff);
  ret = print(cb, cf, buf);
  va_end(stuff);
  return ret;
  }

Graphic Screen::gprintf(Color cb, Color cf, const char *text, ...)  {
  Graphic ret;
  va_list stuff;
  va_start(stuff, text);
  char buf[4096];
  bzero(buf, 4096);
  vsprintf(buf, text, stuff);
  ret = String2Graphic(cb, cf, buf);
  va_end(stuff);
  return ret;
  }

/*
Graphic Screen::String2Graphic(Color cb, Color cf, const char *text)  {
  if(font == NULL)  Exit(0, "Must SetFont before String2Graphic!\r\n");
  Graphic g1, g2, ret;
  char *ind = (char *)text;
  while(font[*ind] == NULL)  {
    if(*ind == 0)  {
      Graphic ret2(1, 1);
      ret2.DefLin((char *)&cb);
      return ret2;
      }
    ind++;
    }
  g1 = *(font[*(ind++)]);
  g1.xcenter = g1.xsize+1;
  for(;*ind != 0; ind++)  {
    if(font[*(ind)] != NULL)  {
      g2 = *(font[*(ind)]);
      g1 = g1 + g2;
      g1.xcenter += g2.xsize+1;
      }
    else if(*ind == '\n')  {
      g1.ycenter += font[' ']->ysize+2;
      g1.xcenter = 0;
      }
    }
  int ctrx, ctry;
  for(ctry=0; ctry<(int)g1.ysize; ctry++)  {
    for(ctrx=0; ctrx<(int)g1.xsize; ctrx++)  {
      switch(g1.image[ctry][ctrx])  {
	case(0): g1.image[ctry][ctrx] = cb; break;
	case(1): g1.image[ctry][ctrx] = cf; break;
	}
      }
    }
  g1.xcenter = 0;
  g1.ycenter = 0;
  return g1;
  }
*/

Graphic Screen::String2Graphic(Color cb, Color cf,
	const char *text)  return ret; {
  if(font == NULL)  Exit(0, "Must SetFont before String2Graphic!\r\n");
  int ctrx, ctry;
  int hsize=0, hsz=0, fsz=font[' ']->ysize, vsize=fsz;
  char *ind;
  for(ind = (char *)text; *ind != 0; ind++)  {
    if(*ind == '\n')  {
      vsize += (fsz+2);
      if(hsz>hsize) hsize = hsz;
      hsz = 0;
      }
    else if(font[*ind] != NULL)  {
      hsz += font[*ind]->xsize+1;
      }
    }
  if(hsz>hsize) hsize = hsz;
  if(hsize == 0)  {
    ret.DefSize(1, 1);
    ret.image[0][0] = 0;
    }
  else  {
    int xp=0, yp=0, fp, fpos;
    ret.DefSize(hsize, vsize);
    for(ctry=0; ctry<vsize; ctry++)  bzero(ret.image[ctry], hsize);
    for(ind = (char *)text; *ind != 0; ind++)  {
      if(*ind == '\n')  {
	xp=0; yp+=(fsz+2);
	}
      else if(font[*ind] != NULL)  {
	for(fp=0; fp<fsz; fp++)  {
	  fpos = fp+(font[*ind]->ycenter);
	  if(fpos >= 0 && fpos < (int)(font[*ind]->ysize))  {
	    memcpy(&ret.image[yp+fp][xp], font[*ind]->image[fpos],
		font[*ind]->xsize);
	    }
	  }
	xp += (font[*ind]->xsize + 1);
	}
      }
    }
  for(ctry=0; ctry<(int)ret.ysize; ctry++)  {
    for(ctrx=0; ctrx<(int)ret.xsize; ctrx++)  {
      switch(ret.image[ctry][ctrx])  {
	case(0): ret.image[ctry][ctrx] = cb; break;
	case(1): ret.image[ctry][ctrx] = cf; break;
	}
      }
    }
  }

void Screen::SetCursor(const Graphic &curpic)  {
  if(Cursor == NULL)  Cursor = new Sprite();
  Cursor->SetImage((Graphic &)curpic);
//  Cursor->Erase();
  Cursor->SetPanel(textp);
  AlignCursor();
  Cursor->Move(curx, cury);
  }

void Screen::AlignCursor()  {
  if(curx <= pxstart[textp]) curx = pxstart[textp]+1;
  if(cury <= pystart[textp]) cury = pystart[textp]+1;
  if(curx >= pxend[textp]) curx = pxend[textp];
  if(cury >= pyend[textp]) cury = pxend[textp];
  if(font != NULL)  {
    cury -= pystart[textp];
    cury += ((font[' ']->ysize+2)/2);
    cury /= (font[' ']->ysize+2);
    cury *= (font[' ']->ysize+2);
    cury += pystart[textp];
    cury ++;
    }
  }

void Screen::SetTextPanel(Panel w)  {
  textp = w;
  AlignCursor();
  if(Cursor != NULL)  {
    Cursor->Erase();
    Cursor->SetPanel(textp);
    Cursor->Move(curx, cury);
    }
  }

void Screen::PositionCursor(int x, int y)  {
  curx = x;
  cury = y;
  AlignCursor();
  if(Cursor != NULL)  Cursor->Move(curx, cury);
  }

int Screen::XPos()  {
  return curx;
  }

int Screen::YPos()  {
  return cury;
  }

void Screen::HideCursor()  {
  if(Cursor != NULL)  { Cursor->Erase(); delete Cursor; }
  Cursor = NULL;
  }

unsigned char *Screen::DoubleBuffer()  {
  return VidBuf;
  }

unsigned char *Screen::DoubleBuffer(int Y)  {
  return &VidBuf[Y*Xlen];
  }

unsigned char *Screen::DoubleBuffer(int X, int Y)  {
  return &VidBuf[X+(Y*Xlen)];
  }

void Screen::TakeBlock(unsigned char *start, unsigned char *end)  {
  end--;
  int X, Xs, Xe, Y;
  Y = ((long)start-(long)VidBuf) / Xlen;
  Xs = ((long)start-(long)VidBuf) % Xlen;
  Xe = ((long)end-(long)VidBuf) % Xlen;
  Xs /=32;
  Xe /=32;
  for(X=Xs; X<=Xe; X++)  {
    if(backg[X][Y] != NULL)  {
      RestoreBack(X, Y);
      delete backg[X][Y];
      backg[X][Y] = NULL;
      }
    }
  }

void Screen::GiveBlock(unsigned char *start, unsigned char *end)  {
  end--;
  int X, Xs, Xe, Y;
  Y = ((long)start-(long)VidBuf) / Xlen;
  Xs = ((long)start-(long)VidBuf) % Xlen;
  Xe = ((long)end-(long)VidBuf) % Xlen;
  Xs /=32;
  Xe /=32;
  InvalidateRectangle(Xs<<5, Y, ((Xe+1)<<5)-1, Y);;
  for(X=Xs; X<=Xe; X++)  {
    if(spritehere[X][Y] != NULL)  {
      SaveBack(X, Y);
      spritehere[X][Y]->DrawBlock(X, Y);
      }
    }
  }

void Screen::FullScreenBMP(char *fn)  {
  Graphic tmpg(fn);
  FullScreenGraphic(tmpg);
  }

void Screen::GetPalette(char *fn)  {
  palette.GetPalette(fn);
  }

void Screen::PasteBMP(char *fn, int X, int Y)  {
 int bmp, colused;
 unsigned size2, width, height;
 unsigned char buffer[1280];
 unsigned char buffer32[32];
 int ctr, ctr2, ctr3;
 {

#ifdef DOS
  bmp = _open(fn, O_RDONLY);
#endif

#ifdef X_WINDOWS
  bmp = open(fn, O_RDONLY);
#endif

  if(bmp == -1)  {
    Exit(1, "\"%s\" Not Found!\n", fn);
    }
  read(bmp, buffer, 16);
  if((buffer[0] != 'B') || (buffer[1] != 'M'))  {
    Exit(1, "\"%s\" is Not A Bitmap file!\n", fn);
    }
  size2 = buffer[14]+256*(buffer[15]);
  read(bmp, buffer, (size2 - 2));
  width = buffer[2]+256*(buffer[3]);
  height = buffer[6]+256*(buffer[7]);
  colused = buffer[30]+256*(buffer[31]);
  if(colused == 0)  colused = 256;
  read(bmp, buffer, colused*4);
  for(ctr = height; ctr > 0; ctr--)  {
    for(ctr2 = 0; ctr2 < ((long)width>>5); ctr2++)  {
      read(bmp, buffer32, 32);
        SetBlock32(ctr2+X, ctr-1+Y, buffer32);
      }
    if((width%32) != 0)  {
      for(ctr3 = 0; ctr3 < 32; ctr3++)
	buffer32[ctr3] = 0;
      read(bmp, buffer32, (width%32));
	SetBlock32(ctr2+X, ctr-1+Y, buffer32);
      }
    }
  close(bmp);
  Refresh();
  }
 }

void Screen::SetFrameRate(int fr)  {
  if(fr <= 0)  { framedelay = 0; return; }

#ifdef DOS
  framedelay = (UCLOCKS_PER_SEC / fr);
#endif

#ifdef X_WINDOWS
  framedelay = (CLOCKS_PER_SEC / fr);
#endif
  }

void Screen::WaitForNextFrame()  {
#ifdef DOS
  while((uclock() - lasttime) < framedelay)  RefreshFast();
  lasttime = uclock();
#endif

#ifdef X_WINDOWS
  while((clock() - lasttime) < framedelay)  {
    RefreshFast();
//    fprintf(stdout, "Waiting from %d to %d for %d!\r\n", clock(), lasttime,
//	framedelay);
    }
  lasttime = clock();
#endif
  }

void Screen::ClipToPanel(int &x, int &y, Panel w)  {
  if(x >= pxend[w]) x = pxend[w]-1;
  if(y >= pyend[w]) y = pyend[w];
  if(x < pxstart[w]) x = pxstart[w]-1;
  if(y < pystart[w]) y = pystart[w];
  }

int Screen::DefaultXSize()  {
#ifdef X_WINDOWS
  if(Frame != NULL)  {
    int x, y;
    XF86DGAGetViewPortSize(___mydisplay, ___myscreen, &x, &y);
    return x;
    }
  else  {
#endif
    return 0;
#ifdef X_WINDOWS
    }
#endif
  }

int Screen::DefaultYSize()  {
#ifdef X_WINDOWS
  if(Frame != NULL)  {
    int x, y;
    XF86DGAGetViewPortSize(___mydisplay, ___myscreen, &x, &y);
    return y;
    }
  else  {
#endif
    return 0;
#ifdef X_WINDOWS
    }
#endif
  }

int Screen::MaxXSize()  {
  return 1280;
  }

int Screen::MaxYSize()  {
  return 1024;
  }
