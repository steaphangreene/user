#include <stdio.h>

#include "graphic.h"
#include "sprite.h"
#include "control.h"

Stickey::Stickey() {
  flags |= SPRITE_ISCONTROL;
  state = 0;
  }

Stickey::Stickey(int x, int y,
	const char *l, color ct, color cl, color cb, color cd) {
  flags |= SPRITE_ISCONTROL;
  state = 0;
  Create(x, y, l, ct, cl, cb, cd);
  }

Stickey::~Stickey() {
  }

void Stickey::Click(int b) {
  if(state) { lastb=b; Clickey::UnClick(b); }
  else Clickey::Click(b);
  }

void Stickey::UnClick(int b) {
  }

void Stickey::RemappedKeyPressed() {
  }

void Stickey::RemappedKeyReleased() {
  }

