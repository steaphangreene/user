#ifndef INSOMNIA_USER_SPRITE_H
#define INSOMNIA_USER_SPRITE_H

#include "config.h"

#define SPRITE_ISCONTROL	1
#define SPRITE_NOCOLLISIONS	2
#define SPRITE_SOLID		4
#define SPRITE_RECTANGLE	8
#define SPRITE_LARGE		16
#define SPRITE_HUGE		32

class Screen;
class Graphic;
class IntList;
typedef int Panel;
typedef unsigned long color;

class Sprite  {
  private:
  Sprite *next, **prev;

  public:
  Sprite();
  Sprite(const Graphic &);
  void SetImage(const Graphic *);
  void SetImage(const Graphic &);
  void UseImage(const Graphic *);
  void UseImage(const Graphic &);
  Graphic *GetImage() { return trueimage; };
  void RedrawArea(int, int, int, int);
  IntList CMove(int, int, int);
  IntList CMove(int, int);
  IntList CDraw(int, int, int);
  IntList CDraw(int, int);
  IntList CDraw();
  void Move(int, int, int);
  void Move(int, int);
  void Position();
  void Position(int, int);
  void Draw(int, int, int);
  void Draw(int, int);
  void Draw();
  void Remove();
  void Erase();
  void DefLin(char*);
  void DefLinH(char*);
  void DefSize(int, int);
  void SetLine(int, int, int, color);
  int Number() { return snum; };
  int Priority() { return priority; };
  void SetPriority(int p) { priority = p; };
  ~Sprite();
  int XPos() { return xpos; };
  int YPos() { return ypos; };
  int XCenter();
  int YCenter();
  int IsControl() { return ((flags&SPRITE_ISCONTROL)>0); };
  void EnableCollisions() { flags &= (~SPRITE_NOCOLLISIONS); };
  void DisableCollisions() { flags |= SPRITE_NOCOLLISIONS; };
  void SetFlag(int i) { flags |= i; };
  void ClearFlag(int i) { flags &= (~i); };
  int Flag(int i) { return (flags&i); };
  void SetPanel(Panel p) { pan = p; };
  void SetColormap(unsigned long *);
  int Visible() { return drawn; };

  protected:
  int Hits(Sprite *);
  int Hits(int, int, int, int);
  Graphic *image, *trueimage;
  int inum, snum, priority, xpos, ypos, angle;
  char drawn, ownimage;
  unsigned long flags;
  unsigned long *remap;
  Panel pan;
  friend class Screen;
  };

#endif
