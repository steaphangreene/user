#ifndef INSOMNIA_USER_CONTROL_H
#define INSOMNIA_USER_CONTROL_H

#include "sprite.h"

class Sound;

class Control : public Sprite {
  public:
  Control();
  virtual ~Control();
  virtual void Click(int);
  virtual void UnClick(int);
  virtual void Drag(int, int, int);
  virtual void KeyPressed(int);
  virtual void KeyReleased(int);
  virtual void RemappedKeyPressed();
  virtual void RemappedKeyReleased();
  void Disable();
  void Enable();

  protected:
  char enabled;
  };

class Clickey : public Control {
  public:
  Clickey();
  Clickey(Graphic *, Graphic *);
  Clickey(Graphic, Graphic);
  Clickey(int, int, const char *, color, color, color, color);
  virtual ~Clickey();
  virtual void Click(int);
  virtual void UnClick(int);
  virtual void RemappedKeyPressed();
  virtual void RemappedKeyReleased();
  void SetImage(Graphic *, Graphic *);
  void SetImage(Graphic, Graphic);
  int State() { return state; };
  void SetState(int);
  void SetSound(Sound *);
  void SetSound(Sound *, Sound *);

  protected:
  void Create(int, int, const char *, color, color, color, color);
  char state, lastb;
  Sound *dsnd, *usnd;
  };

//class Stickey : public Control {
class Stickey : public Clickey {
  public:
  Stickey();
  Stickey(Graphic *, Graphic *);
  Stickey(Graphic, Graphic);
  Stickey(int, int, const char *, color, color, color, color);
  virtual ~Stickey();
  virtual void Click(int);
  virtual void UnClick(int);
  virtual void RemappedKeyPressed();
  virtual void RemappedKeyReleased();
  int State() { return state; };
  void SetSound(Sound *);
  void SetSound(Sound *, Sound *);
  void SetSound(Sound *, Sound *, Sound *, Sound *);

  protected:
  char state, lastb;
  Sound *dsnd, *dsnd2, *usnd, *usnd2;
  };

class Movable : public Control {
  public:
  Movable();
  virtual ~Movable();
  virtual void Drag(int, int, int);

  protected:
  };

class MovableClickey : public Clickey {
  public:
  MovableClickey();
  MovableClickey(Graphic *, Graphic *);
  MovableClickey(Graphic, Graphic);
  virtual ~MovableClickey();
  virtual void Drag(int, int, int);

  protected:
  };

class MovableStickey : public Stickey {
  public:
  MovableStickey();
  virtual ~MovableStickey();
  virtual void Drag(int, int, int);

  protected:
  };

#endif
