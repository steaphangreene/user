#ifndef INSOMNIA_USER_INPUT_H
#define INSOMNIA_USER_INPUT_H

#include "config.h"
#include "control.h"

#define INPUTACTION_NONE		0
#define INPUTACTION_KEYDOWN		1
#define INPUTACTION_KEYUP		2
#define INPUTACTION_MOUSEDOWN		3
#define INPUTACTION_MOUSEUP		4
#define INPUTACTION_CONTROLDOWN		5
#define INPUTACTION_CONTROLUP		6
#define INPUTACTION_CONTROLCHANGE	7
#define INPUTACTION_SYSTEM_QUIT		8

struct GeneralAction {
  int type;
  int modkeys;
  };

struct KeyboardAction {
  int type;
  int modkeys;
  int key;
  int chr;
  };

struct MouseAction {
  int type;
  int modkeys;
  int button;
  int x, y;
  int xs, ys;
  int panel;
  };

struct ControlAction {
  int type;
  int modkeys;
  int control;
  };

union InputAction {
  GeneralAction g;
  KeyboardAction k;
  MouseAction m;
  ControlAction c;
  };

struct RemapedKey {
  int k;
  Control *c;
  RemapedKey *next;
  };

class InputQueue {
  public:
  InputQueue();
  ~InputQueue();
  void Update();
  void ActionOccurs(InputAction *);
  InputAction *NextAction();
  InputAction *PeekNextAction();
  InputAction *WaitForNextAction();

  void MapKeyToControl(int, Control &);
  void MapKeyToControl(int, Control *);
  void MapKeyToControl(int, int);
  void UnmapKey(int);
  void UnmapControl(Control &);
  void UnmapControl(Control *);

  private:
  InputAction queue[1024];
  int head, tail;
  RemapedKey *keymap[256];
  };

#endif
