#ifndef INSOMNIA_USER_KEYBOARD_H
#define INSOMNIA_USER_KEYBOARD_H

#include "config.h"
#include "key_defs.h"

#ifdef DOS
#include <dpmi.h>
#endif

class Sprite;
class Control;

class Keyboard {
  public:
  Keyboard();
  ~Keyboard();
  void Update();
  int IsPressed(int);
  char WaitForChar();
  int WaitForKey();
  char GetAChar(int);
  int GetAKey(int);
  char GetAChar();
  int GetAKey();
  char KeyToChar(int);
  void DisableQueue();
  void EnableQueue();
  int ModKeys();
  int ModKey(int);
  void AddModKey(int);
  void RemoveModKey(int);
  void MapKeyToControl(int, Control &);
  void MapKeyToControl(int, Control *);
  void MapKeyToControl(int, int);

  private:
  int crit;
  Control *KeyRemap[128];
  char KeyStats[128];
  void Down(int);
  void Up(int);

#ifdef DOS
  __dpmi_meminfo handler_area;
  __dpmi_paddr old_handler;
  __dpmi_paddr handler_seginfo;

  static volatile void keyboard_handler();
  static volatile unsigned short ModKey[10];
#endif
  static volatile int modkey[32];
  static volatile char key_stat[KEY_MAX];
  static volatile char queue_keys;
#ifdef DOS
  static volatile unsigned short keyboard_buf[KB_BUF_SIZE];
  static volatile unsigned short modkey_buf[KB_BUF_SIZE];
  static volatile unsigned long buf_ind;
  static volatile char in_ext;
  static volatile void dummy_marker_function();
#endif
  };

#endif
