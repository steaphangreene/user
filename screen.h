#ifndef INSOMNIA_USER_SCREEN_H
#define INSOMNIA_USER_SCREEN_H

#include "config.h"
#include "mfmt.h"
#include "engine.h"
#ifdef DOS
#include "vesa.h"
#endif

#define MAX_SPRITES	16384
#define MAX_LG_SPRITES	1024
#define MAX_BIN_SPRITES	1024
#define REDRAW_RECTS	8
//#define BIN_SIZE	32  // Must Agree
//#define BIN_FACTOR	5   // (eg: 2^BIN_FACTOR = BIN_SIZE)
#define BIN_SIZE	8  // Must Agree
#define BIN_FACTOR	3   // (eg: 2^BIN_FACTOR = BIN_SIZE)

#define VIDEO_NONE	0
#define VIDEO_XWINDOWS	1
#define VIDEO_XF86DGA	2
#define VIDEO_SVGALIB	3
#define VIDEO_FB	4
#define VIDEO_DOS	5
#define VIDEO_VESA	6
#define VIDEO_VBE2	7
#define VIDEO_VBE2L	8
#define VIDEO_DIRECTX	9

#ifdef X_WINDOWS
#define Screen __XScreen
#define Window __XWindow
#define Cursor __XCursor

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/xf86dga.h>
#include <X11/extensions/xf86vmode.h>

#undef Screen
#undef Window
#undef Cursor
#endif

class Graphic;
class Palette;
class Sprite;
class IntList;

typedef int Panel;
typedef unsigned long color;

#define DEFAULT_NAME "User Engine 2.0"

#define ConvertColor(c, ad, d) if(d!=ad) { \
  if(d==16 && ad==32) { \
    c=((c>>8)&0xF800)|((c>>5)&0x07E0)|((c>>3)&0x001F); \
    } \
  else if(d==32 && ad==16) { \
    c=((c&0xF800)<<8)|((c&0x07E0)<<5)|((c&0x001F)<<3); \
    } \
  else Exit(1, "Don't know how to ConvertColor from %d to %d\n", ad, d); \
  }

class Screen  {
  public:
  Screen(char *n = DEFAULT_NAME);
  Screen(int, int, char *n = DEFAULT_NAME);
  ~Screen();
  void SetApparentDepth(int d) { appdepth=d; }
  void SetFrameRate(int);
  int SetSize(int, int);
  int XSize() { return xsize; };
  int YSize() { return ysize; };
  int DefaultXSize();
  int DefaultYSize();
  void Show();
  void Hide();
  void RefreshFast();
  void RefreshFull();
  void Refresh();
  void FullScreenModeOn();
  void FullScreenModeOff();
  void FullScreenModeToggle();

  Palette &GetPalette();
  void SetPaletteEntry(int, int, int, int);
  void SetPalette(const char *);
  void SetPalette(Palette &);
  void FadeIn();
  void FadeOut();
  void FadeIn(int);
  void FadeOut(int);

  void Clear();
  void InvalidateRectangle(int, int, int, int);
  void RestoreRectangle(int, int, int, int);
  void RestoreInvalidRectangles();
  void FullScreenBMP(const char *);

  void FullScreenGraphic(Graphic &);
  void FullScreenGraphicFG(Graphic &);

  void DrawTransparentGraphic(Graphic &, int, int, Panel p=0);
  void DrawGraphic(Graphic &, int, int, Panel p=0);
  void DrawTransparentGraphicFG(Graphic &, int, int, Panel p=0);
  void DrawGraphicFG(Graphic &, int, int, Panel p=0);

  void DrawPartialTransparentGraphic(Graphic &, int, int, int, int, int, int, Panel p=0);
  void DrawPartialGraphic(Graphic &, int, int, int, int, int, int, Panel p=0);
  void DrawPartialTransparentGraphicFG(Graphic &, int, int, int, int, int, int, Panel p=0);
  void DrawPartialGraphicFG(Graphic &, int, int, int, int, int, int, Panel p=0);

  void DropSprite(Sprite *sp);
  void LiftSprite(Sprite *sp);
  IntList CollideRectangle(int, int, int, int);
  Sprite *GetSpriteByNumber(int);
  void MakeFriendly(Graphic *);
  void MakeFriendly(Sprite *);
  void SetPointFG(int, int, color);
  void SetPointFG(int, int, int, int, int);
  void SetPoint(int, int, color);
  void SetPoint(int, int, int, int, int);
  void SetLineFG(int, int, int, int, color);
  void SetLineFG(int, int, int, int, int, int, int);
  void SetLine(int, int, int, int, color);
  void SetLine(int, int, int, int, int, int, int);
  void DrawRectangle(int, int, int, int, color);
  void DrawRectangleFG(int, int, int, int, color);
  int VideoType() { return vtype; };
  int SetFont(const char *);
  int SetFont(const char *, const char *);
  void AlignCursor();
  void SetCursor(Graphic &);
  void TGotoXY(int, int);
  int TXPos() { return tcx; };
  int TYPos() { return tcy; };
  int Print(color, color, const char *);
  int Printf(color, color, const char *, ...)
	__attribute__ ((format (printf, 4, 5)));
  int Print(int, int, color, color, const char *);
  int Printf(int, int, color, color, const char *, ...)
	__attribute__ ((format (printf, 6, 7)));
  int GPrint(Graphic *g, int, int, color, color, const char *);
  int GPrintf(Graphic *g, int, int, color, color, const char *, ...)
	__attribute__ ((format (printf, 7, 8)));

  Panel NewPanel(int, int, int, int);
  void RemovePanel(Panel);
  int PanelXStart(Panel p) { return pxs[p]; }
  int PanelYStart(Panel p) { return pys[p]; }
  int PanelXEnd(Panel p) { return pxe[p]; }
  int PanelYEnd(Panel p) { return pye[p]; }
  Panel WhichPanel(int, int);

#ifdef X_WINDOWS
  Display *_Xdisplay;
  int _Xscreen;
  __XWindow _Xwindow;
  XSizeHints *_Xshints;
  XWMHints *_Xwmhints;
  GC _Xgc;
  Colormap _Xmap;
  XColor _Xpal[256];
  XImage *_Ximage;
  XEvent _Xevent;
  XF86VidModeModeInfo oldv;
#endif

#ifdef DOS
#endif

  private:
  void Init();
  void WaitForNextFrame();
  void DetectVideoType();
  int xsize, ysize, rowlen, collen, depth, shown, appdepth;
  char *name;
  mfmt video_buffer, *image;
  mfmt background_buffer, *backg;
  mfmt frame;
  Palette *pal;
  IntList CollideRectangle(int, int, int, int, int);
  Sprite *sprites[MAX_SPRITES], *spbuf[MAX_SPRITES];
  Sprite *larges, ***bins;
  int xlong, ylong;
  int nextsprite;
  int RegisterSprite(Sprite *);
  void RemoveSprite(int, Sprite *);
  friend class Sprite;
  int vtype;
  long framedelay, lasttime, ulasttime;
  Sprite *TCursor;
  Graphic *font[256];
  int tcx, tcy;
  int fullscreen;
  int rxs[REDRAW_RECTS], rys[REDRAW_RECTS];
  int rxe[REDRAW_RECTS], rye[REDRAW_RECTS];
  int pxs[MAX_PANELS], pys[MAX_PANELS];
  int pxe[MAX_PANELS], pye[MAX_PANELS];

#ifdef DOS
  void SetBank(int);
  int curbank;
  VESAInfo vinfo;
  VESAModeInfo vminfo;
  short *vesamode;
  long *vesax, *vesay, *vesad;
  long numvesamodes;
  void (*vbe2_bank)(char);
  VBE2_PM_Info *vbe2_info;
#endif

  };

#endif
