// *************************************************************************
// palette.cpp
// Basic Palette class, Pre ALPHA non-distribution version
//
// -By Insomnia (Steaphan Greene)      (Copyright 1997-1998 Steaphan Greene)
//                   (insomnia@core.binghamton.edu)
//      No waranty stated or implied, I am not responsible for any damage
// caused directly or indirectly by this software.
//      Permision granted for use/distribution/modification by anyone,
// provided this header remains intact, and modified versions are marked
// as so immediately below this header.
//      Products utilizing the code or methods within may be distributed
// freely along with this licence, but any sales for profit of such products
// must have the author's permission, and may be subject to a royaltee fee.
// *************************************************************************

#include "os_defs.h"
#include "palette.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#ifdef DOS
#include <io.h>
#endif

void Palette::GetPSPPalette(const char *fn)  {
  int r, g, b, ctr;
  FILE *palfl = fopen(fn, "r");
  if(palfl==NULL)  {
    printf("Palette File Not Found!\n");
    exit(1);
    }
  fscanf(palfl, "%*[^\n]\n%*[^\n]\n%*[^\n]\n");
  for(ctr=0; ctr<256; ctr++)  {
    fscanf(palfl, "%d %d %d\n", &r, &g, &b);
    SetPaletteEntry(ctr, r, g, b);
    }
  }

void Palette::GetBMPPalette(const char *fn)  {
 int bmp, colused;
 unsigned size2, width, height;
 unsigned char buffer[1280];
 int ctr;
 {
  bmp = open(fn, O_RDONLY);
  if(bmp == -1)  {
    printf("\"%s\" Not Found!\n", fn);
    exit(1);
    }
  read(bmp, buffer, 16);
  if((buffer[0] != 'B') || (buffer[1] != 'M'))  {
    printf("\"%s\" is Not A Bitmap file!\n", fn);
    exit(1);
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

Palette::Palette()  {
  coldec = 0;
  memset(colors, 0, 768);
  }

Palette::Palette(const char *fn)  {
  GetPSPPalette(fn);
  }

Palette::Palette(cval *in)  {
  coldec = 256;
  memcpy(colors, in, 768);
  }

void Palette::SetPalette(cval *in)  {
  coldec = 256;
  memcpy(colors, in, 768);
  }

Palette::Palette(const Palette &in)  {
  coldec = in.coldec;
  memcpy(colors, in.colors, 768);
  }

Palette Palette::operator = (const Palette &in)  {
  coldec = in.coldec;
  memcpy(colors, in.colors, 768);
  return *this;
  }

void Palette::SetPaletteEntry(cval el, cval r, cval g, cval b)  {
  colors[el*3] = r;
  colors[el*3+1] = g;
  colors[el*3+2] = b;
  if(coldec <= el) coldec = el+1;
  }

cval Palette::GetClosestColor(cval r, cval g, cval b)  {
  int ctr, lel, tdiff, ldiff, r1, g1, b1;
  r1=colors[0];  g1=colors[1];  b1=colors[2];
  ldiff = ((r1-r)*(r1-r))+((g1-g)*(g1-g))+((b1-b)*(b1-b));
  lel = 0;
  for(ctr=3; ctr<768; ctr+=3)  {
    r1=colors[ctr];  g1=colors[ctr+1]; b1=colors[ctr+2];
    tdiff = ((r1-r)*(r1-r))+((g1-g)*(g1-g))+((b1-b)*(b1-b));
    if(tdiff<ldiff)  {
      ldiff = tdiff;  lel = ctr/3;
      }
    }
  return lel;
  }

cval Palette::GetRedEntry(cval el)  {
  return colors[el*3];
  }

cval Palette::GetGreenEntry(cval el)  {
  return colors[el*3+1];
  }

cval Palette::GetBlueEntry(cval el)  {
  return colors[el*3+2];
  }

TrueColor Palette::GetTrueColor(cval el)  {
  return tcols[el];
  }

void Palette::SetTrueColor(cval el, TrueColor tc)  {
  tcols[el] = tc;
  }
