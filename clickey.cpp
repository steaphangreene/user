#include "graphic.h"
#include "sprite.h"
#include "control.h"

Clickey::Clickey() {
  iscontrol = 1;
  }

Clickey::~Clickey() {
  }

void Clickey::Click(int b) {
  printf("Click the Clickey!\n");
  }

void Clickey::UnClick(int b) {
  printf("UnClick the Clickey!\n");
  }

void Clickey::RemappedKeyPressed() {
  }

void Clickey::RemappedKeyReleased() {
  }

