#include "control.h"

Control::Control() {
  flags |= SPRITE_ISCONTROL;
  enabled = 1;
  }

Control::~Control() {
  }

void Control::Click(int b) {
  }

void Control::UnClick(int b) {
  }

void Control::Drag(int b, int dx, int dy) {
  }

void Control::KeyPressed(int k) {
  }

void Control::KeyReleased(int k) {
  }

void Control::RemappedKeyPressed() {
  }

void Control::RemappedKeyReleased() {
  }

void Control::Disable() {
  enabled = 0;
  }

void Control::Enable() {
  enabled = 1;
  }

