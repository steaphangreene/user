#ifndef INSOMNIA_USER_INPUT_H
#define INSOMNIA_USER_INPUT_H

#include "config.h"

union InputAction;

struct GeneralAction {
  InputAction *next;
  char type;
  };

struct MouseAction {
  InputAction *next;
  char type;
  };

struct KeyboardAction {
  InputAction *next;
  char type;
  };

union InputAction {
  InputAction *next;
  GeneralAction g;
  KeyboardAction k;
  MouseAction m;
  };

class InputQueue {
  public:
  InputQueue();
  ~InputQueue();
  void Update();
  };

#endif
