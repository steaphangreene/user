#ifndef INSOMNIA_USER_SCREEN_H
#define INSOMNIA_USER_SCREEN_H

#include "config.h"
#include "mfmt.h"
#include "engine.h"
#ifdef DOS
#include "vesa.h"
#endif

#define MAX_SPRITES	4096
#define REDRAW_RECTS	8

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

class Screen  {
  public:
  Screen();
  Screen(int, int);
  ~Screen();
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

  IntList CollideRectangle(int, int, int, int);
  Sprite *GetSpriteByNumber(int);
  void MakeFriendly(Graphic *);
  void MakeFriendly(Sprite *);
  void SetPointFG(int, int, int);
  void SetPointFG(int, int, int, int, int);
  void SetPoint(int, int, int);
  void SetPoint(int, int, int, int, int);
  void DrawRectangle(int, int, int, int, int);
  void DrawRectangleFG(int, int, int, int, int);
  int VideoType() { return vtype; };
  int SetFont(const char *);
  int SetFont(const char *, const char *);
  void AlignCursor();
  void SetCursor(Graphic &);
  void TGotoXY(int, int);
  int TXPos() { return tcx; };
  int TYPos() { return tcy; };
  int Print(long, long, const char *);
  int Printf(long, long, const char *, ...)
	__attribute__ ((format (printf, 4, 5)));

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
  int xsize, ysize, rowlen, collen, depth, shown;
  char *name;
  mfmt video_buffer, *image;
  mfmt background_buffer, *backg;
  mfmt frame;
  Palette *pal;
  IntList CollideRectangle(int, int, int, int, int);
  Sprite *sprites[MAX_SPRITES], *spbuf[MAX_SPRITES];
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
