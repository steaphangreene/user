#include <user/screen.h>
#include <user/graphic.h>

int main(int argc, char **argv)  {
  if(argc < 2)  Exit(1, "USAGE: vb <filename>\n");
  Palette p;
//  p.GetPalette("base.pal");
  Graphic img(argv[1], p);
//  Screen screen(img.xsize, img.ysize, img.depth);
  Screen screen(img.xsize, img.ysize);
  User user;
//  screen.GetPalette("base.pal");
  screen.GetPalette(argv[1]);
  screen.FullScreenGraphic(img);
  screen.RefreshFull();
  screen.ShowScreen();
  while((!user.IsPressed(SCAN_Q)) && (!user.IsPressed(SCAN_ESC)))
	screen.Refresh();
  }
