#ifndef INSOMNIA_MOUSE_H
#define INSOMNIA_MOUSE_H

#define MOUSE_LEFT      0
#define MOUSE_RIGHT     1 
#define MOUSE_MIDDLE    2
#define MOUSE_UNDEFINED -1
#define MOUSE_IGNORE    0
#define MOUSE_BOX       1   
#define MOUSE_CLICK     2
#define MOUSE_DRAG      3
#define MOUSE_LINE      4
#define MOUSE_DRAW      5

#include <user/graphic.h>
#include <user/screen.h>

class Mouse {
  public:
  Mouse();
  ~Mouse();
  Mouse(int, int);
  Mouse(const Graphic *);
  Mouse(const Graphic *, int, int);
  Mouse(const Graphic &);
  Mouse(const Graphic &, int, int);
  void SetBehavior(int v1, int v2, int v3)
        { SetPanelBehavior(0, v1, v2, v3); };
  void SetPanelBehavior(Panel, int, int, int);
  void SetCursor(const Graphic *);
  void SetCursor(const Graphic &);
  void SetPosition(int, int);
  void ShowCursor();
  void ShowCursor(int, int);
  void HideCursor();
  void SetXRange(int, int);
  void SetYRange(int, int);
  int DrawingBox();
  int XPos()  {  UpdatePos(); return xpos; };
  int YPos()  {  UpdatePos(); return ypos; };
  int LeftButton()  {  return(lbutton && (!rbutton)); };
  int RightButton()  {  return(rbutton && (!lbutton)); };
  int AButton()  {  return(lbutton || rbutton); };
  int MiddleButton()  {  return(lbutton && rbutton); };

  private:
  Sprite *curbutt;
  int behavior[MAX_PANEL][3];
  int shown; 
  int xpos;
  int ypos;
  int bstart;
  int xstart;
  int ystart;
  int xend;
  int yend;
  Panel pstart;
  int numbuttons;
  int lbutton;
  int rbutton;
  Graphic *Dot;
  Sprite Box[4];
  Sprite *Cursor;
  void Update();
  void UpdatePos(); 
  void InitMouse();
  friend class Screen;
  friend class User;
  };

#endif
