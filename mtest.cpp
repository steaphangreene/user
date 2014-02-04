#include "user.h"

#include <stdio.h>

int main(int argc, char **argv)  {
  U2_Init(argc, argv);
  UserDebug("test p1000");
  Screen *screen = new Screen;
  Mouse *mouse = new Mouse;
  screen->SetPalette("cursor.bmp");
  screen->SetFont("basic10.sgf");
  Graphic *mc = new Graphic("cursor.bmp");
  mc->FindTrueCenter();
  mc->Trim();
  screen->MakeFriendly(mc);
  Keyboard *key = new Keyboard;
  InputQueue input;
  UserDebug("test p1001");
  screen->SetSize(640,480);
  mouse->SetCursor(*mc);
  mouse->ShowCursor();
  UserDebug("test p1002");
  screen->Refresh();
  UserDebug("test p1003");
  screen->Show();
  Panel pan = screen->NewPanel(50, 50, 590, 430);
  color textc = screen->GetColor(255, 255, 255);

  MovableClickey b("test1.bmp", "test2.bmp");
  b.Move(100, 100);
  b.SetPanel(pan);

  InputAction *hit=NULL;

  while(hit == NULL || hit->k.key != KEY_Q)  {
    screen->Refresh();
    hit = input.WaitForNextAction();
    if(hit->g.type == INPUTACTION_KEYDOWN)  {
      screen->DrawRectangle(0, 0, 640, 12, 0);
      screen->TGotoXY(1, 1);
      screen->Printf(0, textc, "%d (0x%X) \"%c\"",
		hit->k.key, hit->k.key, hit->k.chr);
      }
    else if(hit->g.type == INPUTACTION_CONTROLUP
			&& hit->c.control == b.Number())  {
      screen->DrawRectangle(0, 12, 640, 12, 0);
      }
    else if(hit->g.type == INPUTACTION_CONTROLDOWN
			&& hit->c.control == b.Number())  {
      screen->DrawRectangle(0, 12, 640, 12, 0);
      screen->TGotoXY(1, 13);
      if(hit->k.modkeys & key->ModKey(KEY_LSHIFT))
	screen->Printf(0, textc, "Pressed w/ LSHIFT!");
      else if(hit->k.modkeys & key->ModKey(KEY_RSHIFT))
	screen->Printf(0, textc, "Pressed w/ RSHIFT!");
      else
	screen->Printf(0, textc, "Pressed!");
      }
    }
  UserDebug("test p1004");
  delete key;
  delete mouse;
  delete screen;
  }
