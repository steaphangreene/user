#ifndef INSOMNIA_USER_CONTROL_H
#define INSOMNIA_USER_CONTROL_H

#include "sprite.h"

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

  protected:
  };

class Clickey : public Control {
  public:
  Clickey();
  virtual ~Clickey();
  virtual void Click(int);
  virtual void UnClick(int);
  virtual void RemappedKeyPressed();
  virtual void RemappedKeyReleased();

  protected:
  };

class Stickey : public Control {
  public:
  Stickey();
  virtual ~Stickey();
  virtual void Click(int);
  virtual void UnClick(int);
  virtual void RemappedKeyPressed();
  virtual void RemappedKeyReleased();

  protected:
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
