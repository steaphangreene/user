#ifndef INSOMNIA_USER_JOYSTICK_H
#define INSOMNIA_USER_JOYSTICK_H

#include "config.h"

class Sprite;
class Control;

#define JS_AXIS		0
#define JS_AXIS_MAX	16
#define JS_BUTTON	256
#define JS_BUTTON_MAX	272

class Joystick {
  public:
  Joystick();
  Joystick(const char *devfl);
  void Create(const char *devfl);
  ~Joystick();
  void Update();
  int IsPressed(int);
  int WaitForAction();
  int GetAction(int);
  int GetAction();
  void DisableQueue();
  void EnableQueue();
  void MapActionToControl(int, Control &);
  void MapActionToControl(int, Control *);
  void MapActionToControl(int, int);
  unsigned long Buttons();

  private:
  int crit, jnum, jdev;
  Control *AxisRemap[16];
  Control *ButtonRemap[16];
  int AxisStats[16];
  char ButtonStats[16];
  char queue_actions;
  };

#endif
