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
  GeneralAction ga;
  KeyboardAction ka;
  MouseAction ma;
  };

class InputQueue {
  public:
  InputQueue();
  void Update();
  };

#endif
