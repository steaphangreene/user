#include "graphic.h"
#include "sprite.h"
#include "control.h"

Movable::Movable() {
  iscontrol = 1;
  }

Movable::~Movable() {
  }

void Movable::Drag(int b, int dx, int dy) {
  Move(xpos+image->xcenter + dx, ypos+image->ycenter + dy);
  }

MovableClickey::MovableClickey() {
  iscontrol = 1;
  }

MovableClickey::MovableClickey(Graphic g1, Graphic g2) {
  iscontrol = 1;
  SetImage(&g1, &g2);
  }

MovableClickey::MovableClickey(Graphic *g1, Graphic *g2) {
  iscontrol = 1;
  SetImage(g1, g2);
  }

MovableClickey::~MovableClickey() {
  }

void MovableClickey::Drag(int b, int dx, int dy) {
  Move(xpos+image->xcenter + dx, ypos+image->ycenter + dy);
  }

MovableStickey::MovableStickey() {
  iscontrol = 1;
  }

MovableStickey::~MovableStickey() {
  }

void MovableStickey::Drag(int b, int dx, int dy) {
  Move(xpos+image->xcenter + dx, ypos+image->ycenter + dy);
  }
