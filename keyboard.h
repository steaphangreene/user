#ifndef INSOMNIA_USER_KEYBOARD_H
#define INSOMNIA_USER_KEYBOARD_H

#include "config.h"
#include "key_defs.h"

class Keyboard {
  public:
  Keyboard();
  void Update();
  int IsPressed(int);
  char WaitForChar();
  int WaitForKey();
  char GetAChar(int);
  int GetAKey(int);
  char GetAChar();
  int GetAKey();
  char KeyToChar(int);

  private:
  char key_stat[65536];
  int crit;
  };

#endif
