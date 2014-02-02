// *************************************************************************
// screen.h
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

#ifndef INSOMNIA_USER_SCREEN_H
#define INSOMNIA_USER_SCREEN_H

#include "config.h"
#include "mfmt.h"
#include "engine.h"
#ifdef DOS
#include "vesa.h"
#endif

#define VIDEO_NONE	0
#define VIDEO_XWINDOWS	1
#define VIDEO_XF86DGA	2
#define VIDEO_XDGA2	3
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

#ifdef XF86_DGA
#include <X11/extensions/xf86dga.h>
#include <X11/extensions/xf86vmode.h>
#ifdef X_XDGAQueryModes
#define X_DGA2
#endif
#endif

#undef Screen
#undef Window
#undef Cursor
#endif

class Graphic;
class Palette;
class Sprite;
class IntList;

typedef int Panel;
typedef unsigned int color;

#define DEFAULT_NAME "User Engine 2.0"

#define ConvertColor(c, ad, d) if(d!=ad) { \
  if(d==16 && ad==32) { \
    c=((c>>8)&0xF800)|((c>>5)&0x07E0)|((c>>3)&0x001F); \
    } \
  else if(d==32 && ad==16) { \
    c=((c&0xF800)<<8)|((c&0x07E0)<<5)|((c&0x001F)<<3); \
    } \
  else U2_Exit(1, "%s\nDon't know how to ConvertColor from %d to %d\n", \
	__PRETTY_FUNCTION__, ad, d); \
  }

class Screen  {
  public:
  Screen(const char *n = DEFAULT_NAME);
  Screen(int, int, const char *n = DEFAULT_NAME);
  Screen(int, int, int, const char *n = DEFAULT_NAME);
  ~Screen();
  void SetApparentDepth(int);
  int GetApparentDepth() { return appdepth; }
  int GetDepth() { return depth; }
  color GetColor(int, int, int);
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

  void Clear(color c=0);
  void InvalidateRectangle(int, int, int, int);
  void RestoreRectangle(int, int, int, int);
  void RestoreInvalidRectangles();
  void FullScreenBMP(const char *);

  void FullScreenGraphic(Graphic &);
  void FullScreenGraphicFG(Graphic &);

  void ScrollPanel(Panel, int, int);
  void ErasePanelSprites(Panel);

  void DrawTransparentGraphic(Graphic &, int, int, Panel p=0);
  void DrawGraphic(Graphic &, int, int, Panel p=0);
  void DrawTransparentGraphicFG(Graphic &, int, int, Panel p=0);
  void DrawGraphicFG(Graphic &, int, int, Panel p=0);

  void DrawPartialTransparentGraphic(Graphic &, int, int, int, int, int, int, Panel p=0);
  void DrawPartialGraphic(Graphic &, int, int, int, int, int, int, Panel p=0);
  void DrawPartialTransparentGraphicFG(Graphic &, int, int, int, int, int, int, Panel p=0);
  void DrawPartialGraphicFG(Graphic &, int, int, int, int, int, int, Panel p=0);

  void RCDrawPartialTransparentGraphic(Graphic &, mfmt, int, int, int, int, int, int, Panel p=0);
  void RCDrawPartialGraphic(Graphic &, mfmt, int, int, int, int, int, int, Panel p=0);
  void RCDrawPartialTransparentGraphicFG(Graphic &, mfmt, int, int, int, int, int, int, Panel p=0);
  void RCDrawPartialGraphicFG(Graphic &, mfmt, int, int, int, int, int, int, Panel p=0);

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
  void ClearArea(int, int, int, int, color c=0);
  int VideoType() { return vtype; };

  int SetFont(const char *);
  int SetFont(const char *, const char *);
  void GetStringSize(char *, int *, int *);
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
  int CGPrint(Graphic *g, int, int, color, color, const char *);
  int CGPrintf(Graphic *g, int, int, color, color, const char *, ...)
	__attribute__ ((format (printf, 7, 8)));
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
#ifdef XF86_DGA
  XF86VidModeModeInfo oldv;
#ifdef X_DGA2
  int oldm;
  int DGAFlags;
#endif
#endif
#endif

#ifdef DOS
#endif

  private:
  void Init();
  void WaitForNextFrame();
  void DetectVideoType();
  int xsize, ysize, rowlen, collen, depth, shown, appdepth;
  const char *name;
  mfmt video_buffer, *image;
  mfmt background_buffer, *backg;
  mfmt frame;
  Palette *pal;
  IntList CollideRectangle(int, int, int, int, int);
  Sprite *sprites[MAX_SPRITES];
  Sprite *huges, ***bins, ***lbins;
  int xbins, ybins, xlbins, ylbins;
  int nextsprite;
  int RegisterSprite(Sprite *);
  void RemoveSprite(int, Sprite *);
  friend class Sprite;
  int vtype;
  int framedelay, lasttime, ulasttime;
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
  int *vesax, *vesay, *vesad;
  int numvesamodes;
  void (*vbe2_bank)(char);
  VBE2_PM_Info *vbe2_info;
#endif

  };

#endif
