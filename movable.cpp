#include "graphic.h"
#include "sprite.h"
#include "control.h"

Movable::Movable() {
  flags |= SPRITE_ISCONTROL;
  }

Movable::~Movable() {
  }

void Movable::Drag(int b, int dx, int dy) {
  Move(xpos+image->xcenter + dx, ypos+image->ycenter + dy);
  }

MovableClickey::MovableClickey() {
  flags |= SPRITE_ISCONTROL;
  }

MovableClickey::MovableClickey(Graphic g1, Graphic g2) {
  flags |= SPRITE_ISCONTROL;
  SetImage(&g1, &g2);
  }

MovableClickey::MovableClickey(Graphic *g1, Graphic *g2) {
  flags |= SPRITE_ISCONTROL;
  SetImage(g1, g2);
  }

MovableClickey::~MovableClickey() {
  }

void MovableClickey::Drag(int b, int dx, int dy) {
  Move(xpos+image->xcenter + dx, ypos+image->ycenter + dy);
  }

MovableStickey::MovableStickey() {
  flags |= SPRITE_ISCONTROL;
  }

MovableStickey::~MovableStickey() {
  }

void MovableStickey::Drag(int b, int dx, int dy) {
  Move(xpos+image->xcenter + dx, ypos+image->ycenter + dy);
  }
