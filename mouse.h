#ifndef INSOMNIA_USER_MOUSE_H
#define INSOMNIA_USER_MOUSE_H

#include "config.h"

class Graphic;
class Sprite;
class Control;

typedef int Panel;

#define MAX_MBUTTONS	8

#define MOUSE_NONE      0
#define MOUSE_XWINDOWS  1
#define MOUSE_XF86DGA   2
#define MOUSE_SVGALIB   3
#define MOUSE_FB        4
#define MOUSE_DOS       5
#define MOUSE_DIRECTX   6

#define MB_IGNORE	0
#define MB_CLICK	1
#define MB_BOX		2
#define MB_DRAG		3
#define MB_LINE		4
#define MB_DRAW		5

class Mouse {
  public:
  Mouse();
  ~Mouse();
  void Update();
  int IsPressed(int);
  int XPos();
  int YPos();
  void SetCursor(Graphic *);
  void SetCursor(Graphic &);
  void ShowCursor();
  void HideCursor();
  void SetRange(int, int, int, int);
  void SetBehavior(Panel, int, int);

  private:
  void Pressed(int, int, int);
  void Released(int, int, int);
  void Moved(int, int);
  char butt_stat[MAX_MBUTTONS];
  int mtype, xpos, ypos;
  char crit, cvis;
  char num_butt;
  Sprite *cursor;
  Control *curcont;
  int contx, conty, contb;
  int rngxs, rngys, rngxe, rngye;
  int pb[MAX_PANELS][MAX_MBUTTONS];
  };

#endif
