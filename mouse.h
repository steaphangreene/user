#ifndef INSOMNIA_USER_MOUSE_H
#define INSOMNIA_USER_MOUSE_H

#include "config.h"

class Graphic;
class Sprite;

#define MOUSE_NONE      0
#define MOUSE_XWINDOWS  1
#define MOUSE_XF86DGA   2
#define MOUSE_SVGALIB   3
#define MOUSE_FB        4
#define MOUSE_DOS       5
#define MOUSE_DIRECTX   6

class Mouse {
  public:
  Mouse();
  ~Mouse();
  void Update();
  int IsPressed(int);
  int XPos();
  int YPos();
  void SetCursor(Graphic &);
  void ShowCursor();
  void HideCursor();

  private:
  char butt_stat[8];
  int mtype, xpos, ypos;
  char crit, cvis;
  Sprite *cursor;
  };

#endif
