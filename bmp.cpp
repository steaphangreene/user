#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <io.h>
#include <fcntl.h>
#include <sys\stat.h>

#include "screen.h"

extern Screen *__Da_Screen;

void Screen::FullScreenBMP(char *fn)  {
  Graphic tmpg(fn);
  FullScreenGraphic(tmpg);
  }

void Screen::GetPSPPalette(char *fn)  {
  int r, g, b, ctr;
  FILE *palfl = fopen(fn, "r");
  if(palfl==NULL)  {
    Exit(1, "Palette File Not Found!\n");
    }
  fscanf(palfl, "%*[^\n]\n%*[^\n]\n%*[^\n]\n");
  for(ctr=0; ctr<256; ctr++)  {
    fscanf(palfl, "%d %d %d\n", &r, &g, &b);
    SetPaletteEntry(ctr, r, g, b);
    }
  }

void Screen::GetBMPPalette(char *fn)  {
 int bmp, colused;
 unsigned size2, width, height;
 unsigned char buffer[1280];
 int ctr;
 {
  bmp = _open(fn, O_RDONLY);
  if(bmp == -1)  {
    Exit(1, "\"%s\" Not Found!\n", fn);
    }
  read(bmp, buffer, 16);
  if((buffer[0] != 'B') || (buffer[1] != 'M'))  {
    Exit(1, "\"%s\" is Not A Bitmap file!\n", fn);
    }
  size2 = buffer[14]+(buffer[15]<<8);
  read(bmp, buffer, (size2 - 2));
  width = buffer[2]+(buffer[3]<<8);
  height = buffer[6]+(buffer[7]<<8);
  colused = buffer[30]+(buffer[31]<<8);
  if(colused == 0)  colused = 256;
  read(bmp, buffer, colused<<2);
  for(ctr = 0; ctr < (colused<<2); ctr+=4)  {
    SetPaletteEntry(ctr>>2, buffer[ctr+2], buffer[ctr+1], buffer[ctr]);
    }
  close(bmp);
  }
 }

void Screen::PasteBMP(char *fn, int X, int Y)  {
 int bmp, colused;
 unsigned size2, width, height;
 unsigned char buffer[1280];
 unsigned char buffer32[32];
 int ctr, ctr2, ctr3;
 {
  bmp = _open(fn, O_RDONLY);
  if(bmp == -1)  {
    Exit(1, "\"%s\" Not Found!\n", fn);
    }
  read(bmp, buffer, 16);
  if((buffer[0] != 'B') || (buffer[1] != 'M'))  {
    Exit(1, "\"%s\" is Not A Bitmap file!\n", fn);
    }
  size2 = buffer[14]+256*(buffer[15]);
  read(bmp, buffer, (size2 - 2));
  width = buffer[2]+256*(buffer[3]);
  height = buffer[6]+256*(buffer[7]);
  colused = buffer[30]+256*(buffer[31]);
  if(colused == 0)  colused = 256;
  read(bmp, buffer, colused*4);
  for(ctr = height; ctr > 0; ctr--)  {
    for(ctr2 = 0; ctr2 < ((long)width>>5); ctr2++)  {
      read(bmp, buffer32, 32);
        SetBlock32(ctr2+X, ctr-1+Y, buffer32);
      }
    if((width%32) != 0)  {
      for(ctr3 = 0; ctr3 < 32; ctr3++)
	buffer32[ctr3] = 0;
      read(bmp, buffer32, (width%32));
	SetBlock32(ctr2+X, ctr-1+Y, buffer32);
      }
    }
  close(bmp);
  Refresh();
  }
 }
