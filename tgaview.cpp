#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <user.h>

struct Image {
  int width;
  int height;
  unsigned long *data;
  };

int main(int argc, char **argv) {
  int ctr, num=0, ind=0, maxwidth=1, maxheight=1;
  unsigned char buf[256];
  FILE *tga;
  char **fn;
  struct Image *image;

  if(argc < 2) U2_Exit(1, "Usage: tgaview <filename>...\n");
  fn = (char **)malloc((argc-1)*sizeof(char*));
  memset(fn, 0, (argc-1)*sizeof(char*));
  for(ctr=1; ctr<argc; ++ctr) {
    tga=fopen(argv[ctr], "rb");
    if(tga != NULL) {
      if(read(fileno(tga), buf, 18) == 18 && buf[1] == 0 && buf[2] == 2) {
	fn[ind] = argv[ctr];
	++ind; ++num;
	}
      else {
	fprintf(stderr, "\"%s\" isn't a 32-bit TGA file!\n", argv[ctr]);
	}
      fclose(tga);
      }
    else {
      fprintf(stderr, "Can't open \"%s\"!\n", argv[ctr]);
      }
    }
  if(num<1) U2_Exit(1, "No valid 32-bit TGA files found!\n");
  image = (struct Image *)malloc(num*sizeof(struct Image));
  for(ctr=0; ctr<num; ++ctr) {
    tga=fopen(fn[ctr], "rb");
    read(fileno(tga), buf, 18);
    image[ctr].width = (buf[13]<<8) + buf[12];
    image[ctr].height = (buf[15]<<8) + buf[14];
    if(image[ctr].width > maxwidth) maxwidth=image[ctr].width;
    if(image[ctr].height > maxheight) maxheight=image[ctr].height;
    read(fileno(tga), buf, (int)buf[0]);
    image[ctr].data = (unsigned long *)
	malloc(image[ctr].height*image[ctr].width*sizeof(unsigned long *));
    for(ind=0; ind<image[ctr].height; ++ind) {
      read(fileno(tga),
	&image[ctr].data[(image[ctr].height-(ind+1))*image[ctr].width],
	image[ctr].width*sizeof(unsigned long));
      }
    }
  Screen *screen = new Screen(image[0].width, image[0].height);
  Keyboard *key = new Keyboard;
  Graphic g; g.depth=32; g.DefSize(image[0].width, image[0].height);
  for(ctr=0; ctr<32; ctr++) {
    g.image[ctr].ul = new unsigned long[32];
    memcpy(g.image[ctr].ul, &image[0].data[ctr*image[0].width],
	image[0].width*sizeof(unsigned long));
    }
  screen->Show();
  screen->FullScreenGraphic(g);
  screen->RefreshFull();
  while((!key->IsPressed(KEY_ESC)) && (!key->IsPressed(KEY_Q)))
    screen->Refresh();
  }
