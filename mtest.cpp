#include <stdio.h>

#include "engine.h"
#include "screen.h"
#include "mouse.h"
#include "keyboard.h"
#include "graphic.h"

int main(int argc, char **argv)  {
  InitUserEngine(argc, argv);
  Debug("test p1000");
  Screen *screen = new Screen;
  Mouse *mouse = new Mouse;
  screen->GetPalette("cursor.bmp");
  Graphic *mc = new Graphic("cursor.bmp");
  screen->MakeFriendly(mc);
  Keyboard *key = new Keyboard;
  Debug("test p1001");
  screen->SetSize(320,200);
  mouse->SetCursor(*mc);
  mouse->ShowCursor();
  Debug("test p1002");
  screen->Refresh();
  Debug("test p1003");
  screen->Show();
  int quit = 0;
  while(!quit)  {
    screen->Refresh();
    if(key->GetAKey() != 0) quit=1;
    }
  Debug("test p1004");
  delete key;
  delete mouse;
  delete screen;
  }
