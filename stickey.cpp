#include "graphic.h"
#include "sprite.h"
#include "control.h"

Stickey::Stickey() {
  iscontrol = 1;
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

