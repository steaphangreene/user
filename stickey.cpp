#include <stdio.h>

#include "graphic.h"
#include "sprite.h"
#include "control.h"

Stickey::Stickey() {
  flags |= SPRITE_ISCONTROL;
  state = 0;
  }

Stickey::~Stickey() {
  }

void Stickey::Click(int b) {
  printf("Click the Stickey!\n");
  }

void Stickey::UnClick(int b) {
  printf("UnClick the Stickey!\n");
  }

void Stickey::RemappedKeyPressed() {
  }

void Stickey::RemappedKeyReleased() {
  }

