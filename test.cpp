#include "keyboard.h"
#include "screen.h"

#define ROTATE_MAX 256

int main(int argc, char **argv)  {
  unsigned long ang = 0;
  Screen screen(MODE_1024x768);
  User user;
  screen.ClearScreen();
  screen.FullScreenBMP("test.bmp");
  screen.GetPalette("test.bmp");
  screen.RefreshFull();
  Graphic guy("test.bmp");
  guy.Trim();
  guy.FindTrueCenter();
  Graphic guyp(guy.Rotated(0));
  guyp.FindTrueCenter();
  Mouse mouse;
  mouse.ShowCursor();
  screen.Refresh();
  screen.FadeIn(4);
  int mod = 1;
  Sprite guy2;
  guyp.FindTrueCenter();
  guy2.SetImage(guyp /*.Scaled(75, 75)*/);
  guy2.Move((screen.GetXSize() / 2), (screen.GetYSize() / 2));
  while((!user.IsPressed(SCAN_Q)) && (!user.IsPressed(SCAN_A))
	&& (ang < (guyp.xsize-75)))  {
    while(user.IsPressed(SCAN_S));
    guy2.SetImage(guyp.Scaled(ang+75, ang+75));
    guy2.Move((screen.GetXSize() / 2), (screen.GetYSize() / 2));
    screen.Refresh();
    ang+=mod;
    }
  for(ang = 0; ang < 50 && (!user.IsPressed(SCAN_Q)); ang++)  {
	screen.Scroll(1, 1);
	screen.RefreshFull();
	}
  for(ang = 0; ang < 50 && (!user.IsPressed(SCAN_Q)); ang++)  {
	screen.Scroll(1, -1);
	screen.RefreshFull();
	}
  for(ang = 0; ang < 50 && (!user.IsPressed(SCAN_Q)); ang++)  {
	screen.Scroll(-1, 1);
	screen.RefreshFull();
	}
  for(ang = 0; ang < 50 && (!user.IsPressed(SCAN_Q)); ang++)  {
	screen.Scroll(-1, -1);
	screen.RefreshFull();
	}
  ang = 0;

  printf("Done Scroll!\n");
  Graphic guyt(guy);
  printf("Made new g\n");
  while((!user.IsPressed(SCAN_Q)) && (!user.IsPressed(SCAN_B)))  {
    while(user.IsPressed(SCAN_S));
    guy2.SetImage(guyt.Rotated(ang));
    guy2.Move((screen.GetXSize() / 2), (screen.GetYSize() / 2));
    screen.Refresh();
    ang+=mod<<8;
    ang = (ang % 65536);
    }
  guy2.SetImage(guy.Scaled(screen.GetXSize(), screen.GetYSize()));
  guy2.Move((screen.GetXSize() / 2), (screen.GetYSize() / 2));
  while(!user.IsPressed(SCAN_Q))  {
    if(user.IsPressed(SCAN_R))  {
      guy2.SetImage(guy.Scaled(screen.GetXSize(), screen.GetYSize()));
      guy2.Move((screen.GetXSize() / 2), (screen.GetYSize() / 2));
      }
    if(user.IsPressed(SCAN_M))  {
      guy2.Move((screen.GetXSize() / 2), (screen.GetYSize() / 2));
      }
    if(user.IsPressed(SCAN_L))  screen.RefreshFull();
    else  screen.Refresh();
    }
  screen.FadeOut(8);
  return 0;
  }
