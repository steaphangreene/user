#ifndef INSOMNIA_USER_MOUSE_H
#define INSOMNIA_USER_MOUSE_H

#include "config.h"
#include "screen.h"

class Graphic;
class Sprite;
class Control;

typedef int Panel;

#define MAX_MBUTTONS	8

#define MOUSE_NONE      0
#define MOUSE_XWINDOWS  1
#define MOUSE_XF86DGA   2
#define MOUSE_XDGA2     3
#define MOUSE_DOS       10
#define MOUSE_DIRECTX   11

#define MB_IGNORE	0
#define MB_CLICK	1
#define MB_BOX		2
#define MB_LINE		3
#define MB_DRAW		4
#define MB_CLICKDRAW	5

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
  void SetSelColor(color);
  int DrawingSelector();

  private:
  void Pressed(int, int, int);
  void Released(int, int, int);
  void Moved(int, int);
  char butt_stat[MAX_MBUTTONS];
  int bdown, pin, startx, starty;
  int mtype, xpos, ypos;
  char crit, cvis;
  char num_butt;
  Sprite *cursor, *selector;
  Graphic *selg;
  color selcolor;
  Control *curcont;
  int contx, conty, contb;
  int rngxs, rngys, rngxe, rngye;
  int pb[MAX_PANELS][MAX_MBUTTONS];
  };

#endif
