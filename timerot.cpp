#include <user/keyboard.h>
#include <user/screen.h>
#include <time.h>

int main()  {
  int ctr;
  uclock_t begin, end;
  Screen screen;
  screen.SetSize(640, 480);
  User user;
  screen.GetBMPPalette("test.bmp");
  Graphic testg("test.bmp");
  testg.FindTrueCenter();
  Sprite test;
  test.SetImage(testg);
  test.Move(200, 200);
  screen.ShowScreen();
  screen.Refresh();
  begin=uclock();
  for(ctr=0; ctr<256 && (!user.IsPressed(SCAN_Q)); ctr++)  {
    test.Move(200, 200, ctr);
    screen.Refresh();
    }
  end=uclock();
  Exit(0, "Took %Ld. as opposed to 1636/2854/3205/4911/5834/5996\n", ((end-begin)/10000));
  }
