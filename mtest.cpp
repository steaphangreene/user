#include <stdio.h>
#include <user.h>

int main(int argc, char **argv)  {
  InitUserEngine(argc, argv);
  Debug("test p1000");
  Screen *screen = new Screen;
  Mouse *mouse = new Mouse;
  screen->SetPalette("cursor.bmp");
  screen->SetPaletteEntry(255, 255, 255, 255);
  screen->SetPaletteEntry(0, 0, 0, 0);
  screen->SetFont("basic10.sgf");
  Graphic *mc = new Graphic("cursor.bmp");
  mc->FindTrueCenter();
  mc->Trim();
  screen->MakeFriendly(mc);
  Keyboard *key = new Keyboard;
  Debug("test p1001");
  screen->SetSize(640,480);
  mouse->SetCursor(*mc);
  mouse->ShowCursor();
  Debug("test p1002");
  screen->Refresh();
  Debug("test p1003");
  screen->Show();
  int hit=0;

//  Control b;
//  Movable b;
//  Clickey b;
//  MovableClickey b;
  MovableStickey b;
  b.SetImage("test.bmp");
  b.Move(100, 100);

  while(hit != KEY_Q)  {
    screen->Refresh();
    hit = key->GetAKey();
    if(hit != 0)  {
      screen->DrawRectangle(0, 0, 640, 12, 0);
      screen->TGotoXY(1, 1);
      screen->Printf(0, 255, "%d (0x%X)", hit, hit);
      }
    }
  Debug("test p1004");
  delete key;
  delete mouse;
  delete screen;
  }
