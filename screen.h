#ifndef INSOMNIA_USER_SCREEN_H
#define INSOMNIA_USER_SCREEN_H

#include "config.h"
#include "mfmt.h"
#include "engine.h"

#define MAX_SPRITES	4096

#define VIDEO_NONE	0
#define VIDEO_XWINDOWS	1
#define VIDEO_XF86DGA	2
#define VIDEO_SVGALIB	3
#define VIDEO_FB	4
#define VIDEO_DOS	5
#define VIDEO_DIRECTX	6

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

class Screen  {
  public:
  Screen();
  Screen(int, int);
  ~Screen();
  void SetFrameRate(int);
  int SetSize(int, int);
  int XSize() { return xsize; };
  int YSize() { return ysize; };
  void Show();
  void Hide();
  void RefreshFast();
  void RefreshFull();
  void Refresh();
  void SetPaletteEntry(int, int, int, int);
  void GetPalette(const char *);
  void FadeIn();
  void FadeOut();
  void FadeIn(int);
  void FadeOut(int);
  void Clear();
  void RestoreRectangle(int, int, int, int);
  void FullScreenBMP(const char *);
  void FullScreenGraphic(Graphic &);
  void DrawTransparentGraphic(Graphic &, int, int);
  void DrawGraphic(Graphic &, int, int);
  void FullScreenGraphicFG(Graphic &);
  void DrawTransparentGraphicFG(Graphic &, int, int);
  void DrawGraphicFG(Graphic &, int, int);
  void MakeFriendly(Graphic *);
  void MakeFriendly(Sprite *);
  void SetPoint(int, int, int);
  void SetPoint(int, int, int, int, int);
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
  static int CompareSprites(Sprite *, Sprite *);
  int RegisterSprite(Sprite *);
  void RemoveSprite(int, Sprite *);
  friend class Sprite;
  int vtype;
  char updated;
  long framedelay, lasttime, ulasttime;
  Sprite *TCursor;
  Graphic *font[256];
  int tcx, tcy;
  };

#endif
