#ifndef INSOMNIA_USER_SPRITE_H
#define INSOMNIA_USER_SPRITE_H

#include "config.h"

class Screen;
class Graphic;
class IntList;

class Sprite  {
  public:
  Sprite();
  Sprite(const Graphic &);
  void SetImage(const Graphic *);
  void SetImage(const Graphic &);
  void RedrawArea(int, int, int, int);
  IntList CMove(int, int);
  IntList CDraw(int, int);
  IntList CDraw();
  void Move(int, int);
  void Draw(int, int);
  void Draw();
  void Erase();
  void DefLin(char*);
  void DefLinH(char*);
  void DefSize(int, int);
  int Number() { return snum; };
  int Priority() { return priority; };
  void SetPriority(int p) { priority = p; };
  ~Sprite();
  int XPos() { return xpos; };
  int YPos() { return ypos; };
  int IsControl() { return iscontrol; };
  void EnableCollisions() { collisions = 1; };
  void DisableCollisions() { collisions = 0; };

  protected:
  int Hits(Sprite *);
  int Hits(int, int, int, int);
  Graphic *image, *trueimage;
  int snum, priority, xpos, ypos;
  char drawn, collisions, iscontrol;
  friend class Screen;
  };

#endif
