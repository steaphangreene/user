// *************************************************************************
// graphic.cpp
// Advanced Graphic class, Pre ALPHA non-distribution version
//
// -By Insomnia (Steaphan Greene)   (Copyright 1997-2001 Steaphan Greene)
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

#include        <stdio.h>
#include        <stdlib.h>
#include        <sys/types.h>
#include        <sys/stat.h>
#include        <fcntl.h>
#include        <math.h>
#include        <string.h>
#include	<stdarg.h>
#include	<unistd.h>

#include        "engine.h"

#ifdef DOS
#include	<io.h>
#endif

#ifndef		M_PI
#define		M_PI 3.1415926535
#endif

#include        "config.h"
#include        "graphic.h"

#define cd2rad(a) ((double)((((double)(a))*((double)M_PI))/(double)(32768)))
#define ConvertColor(c, ad, d) if(d!=ad) { \
  if(d==16 && ad==32) { \
    c=((c>>8)&0xF800)|((c>>5)&0x07E0)|((c>>3)&0x001F); \
    } \
  else if(d==32 && ad==16) { \
    c=((c&0xF800)<<8)|((c&0x07E0)<<5)|((c&0x001F)<<3); \
    } \
  else U2_Exit(1, "%s\nDon't know how to ConvertColor from %d to %d\n", \
	__PRETTY_FUNCTION__, ad, d); \
  }


Graphic::Graphic() {
  image = NULL;
  xdef = 0;  ydef = 0;
  xsize = 0;  ysize = 0;
  depth = 8; tcolor=0;
  }

Graphic::Graphic(int xsz, int ysz) {
  image = NULL;
  xdef = 0;  ydef = 0;
  xsize = 0;  ysize = 0;
  depth = 8; tcolor=0;
  DefSize(xsz, ysz);
  }

void Graphic::PaletteConvert(const Palette &p1, const Palette &p2)  {
  int ctr, ctr2, sse, csse, tmpa;
  cval remap[256];
  bzero(remap, 256*sizeof(cval));
  for(ctr=0; ctr<256; ctr++)  {
    tmpa = (p1.colors[ctr*3]-p2.colors[0]);
    tmpa *= tmpa;
    tmpa *= 3;
    sse = tmpa;
    tmpa = (p1.colors[ctr*3+1]-p2.colors[1]);
    tmpa *= tmpa;
    tmpa *= 6;
    sse += tmpa;
    tmpa = (p1.colors[ctr*3+2]-p2.colors[2]);
    tmpa *= tmpa;
    tmpa *= 1;
    sse += tmpa;

    for(ctr2=1; ctr2<256; ctr2++)  {
      tmpa = p1.colors[3*ctr]-p2.colors[3*ctr2];
      tmpa *= tmpa;
      tmpa *= 3;
      csse = tmpa;
      tmpa = p1.colors[3*ctr+1]-p2.colors[3*ctr2+1];
      tmpa *= tmpa;
      tmpa *= 6;
      csse += tmpa;
      tmpa = p1.colors[3*ctr+2]-p2.colors[3*ctr2+2];
      tmpa *= tmpa;
      tmpa *= 1;
      csse += tmpa;
      if(csse < sse) { remap[ctr] = ctr2; sse = csse; }
      }
//    if(remap[ctr] != ctr) printf("%d->%d\t", ctr, remap[ctr]);
    }
  for(ctr=0; ctr<(int)ysize; ++ctr)  {
    for(ctr2=0; ctr2<(int)xsize; ++ctr2)  {
      image[ctr].uc[ctr2] = remap[image[ctr].uc[ctr2]];
      }
    }
  tcolor = remap[tcolor];
  }

Graphic Graphic::DoubleX()  {
  Graphic ret;
  int Y;
  ret.depth = depth;
  ret.tcolor = tcolor;
  ret.xcenter = xcenter;
  ret.ycenter = ycenter;
  ret.DefSize(xsize*2, ysize);
  for(Y=0; Y<(long)ysize; Y++)  {
    memcpy(ret.image[Y].uc, image[Y].uc, xsize*(depth>>3));
    if(depth == 8)
      memcpy(ret.image[Y].uc+xsize, image[Y].uc, xsize);
    else if(depth == 32)
      memcpy(ret.image[Y].ul+xsize, image[Y].ul, xsize*4);
    else if(depth == 16)
      memcpy(ret.image[Y].us+xsize, image[Y].us, xsize*2);
    }
  return ret;
  }

Graphic Graphic::DoubleY()  {
  Graphic ret;
  int Y;
  ret.depth = depth;
  ret.tcolor = tcolor;
  ret.xcenter = xcenter;
  ret.ycenter = ycenter;
  ret.DefSize(xsize, ysize*2);
  for(Y=0; Y<(long)ysize; Y++)  {
    memcpy(ret.image[Y].uc, image[Y].uc, xsize*(depth>>3));
    memcpy(ret.image[Y+ysize].uc, image[Y].uc, xsize*(depth>>3));
    }
  return ret;
  }

Graphic Graphic::Hashed(color c)  {
  Graphic ret;
  int X, Y;
  ret = *this;
  for(Y=0; Y<(long)ysize; Y++)  {
    for(X=(Y&1); X<(long)xsize; X+=2)  {
      if(depth == 8) ret.image[Y].uc[X] = c;
      else if(depth == 32) ret.image[Y].ul[X] = c;
      else U2_Exit(-1, "Unknown depth error (%ld) in %s!\n", depth, __PRETTY_FUNCTION__);
      }
    }
  return ret;
  }

Graphic Graphic::OffHashed(color c)  {
  Graphic ret;
  int X, Y;
  ret = *this;
  for(Y=0; Y<(long)ysize; Y++)  {
    for(X=(1-(Y&1)); X<(long)xsize; X+=2)  {
      if(depth == 8) ret.image[Y].uc[X] = c;
      else if(depth == 32) ret.image[Y].ul[X] = c;
      else U2_Exit(-1, "Unknown depth error (%ld) in %s!\n", depth, __PRETTY_FUNCTION__);
      }
    }
  return ret;
  }

Graphic Graphic::Partial(int x1, int y1, int x2, int y2)  {
  Graphic ret;
  x1 += xcenter;	y1 += ycenter;
  x2 += xcenter;	y2 += ycenter;
  if(x1 < 0)  x1 = 0;
  if(y1 < 0)  y1 = 0;
  if(x1 > (int)xsize)  x1 = xsize;
  if(y1 > (int)ysize)  y1 = ysize;
  if(x2 < 0)  x2 = 0;
  if(y2 < 0)  y2 = 0;
  if(x2 > (int)xsize)  x2 = xsize;
  if(y2 > (int)ysize)  y2 = ysize;
  int xsz = x2-x1;
  int ysz = y2-y1;
  int ctr;
  ret.DefSize(xsz, ysz);
  for(ctr=y1; ctr<y2; ctr++)  {
    ret.DefLin((char *)&image[ctr].uc[x1]);  //*** 8-bit only!
    }
  return ret;
  }

Graphic Graphic::Scaled(double scale)  {
  Graphic ret;
  ret = Scaled((int)((double)scale*(double)xsize+(double)0.5),
	(int)((double)scale*(double)ysize+(double)0.5));
  return ret;
  }

Graphic Graphic::Scaled(int xsz, int ysz)  {
  Graphic ret;
  int xi, yi;
  double ox, oy;
  ret.depth = depth;
  ret.DefSize(xsz, ysz);
  for(yi=0; yi<(long)ret.ysize; yi++)  {
    for(xi=0; xi<(long)ret.xsize; xi++)  {
      ox = (xi * xsize);
      ox /= ret.xsize;
      oy = (yi * ysize);
      oy /= ret.ysize;
      if(depth == 8) ret.image[yi].uc[xi] = image[(int)oy].uc[(int)ox];
      else if(depth == 32) ret.image[yi].ul[xi] = image[(int)oy].ul[(int)ox];
      }
    }
  ret.xcenter = (xcenter * ret.xsize) / xsize;
  ret.ycenter = (ycenter * ret.ysize) / ysize;
  ret.tcolor = tcolor;
  return ret;
  }

Graphic Graphic::RotatedCounterClock()  {
  int ctr1, ctr2;
  Graphic ret(ysize, xsize);
  char tmpb[ret.xsize+2];
  for(ctr1=0; ctr1<(int)ret.ysize; ctr1++)  {
    ret.DefLin(tmpb);
    for(ctr2=0; ctr2<(int)ret.xsize; ctr2++)  {
      if(depth == 8)
	ret.image[ctr1].uc[ctr2] = image[ctr2].uc[(xsize-1)-ctr1];
      else if(depth == 32)
	ret.image[ctr1].ul[ctr2] = image[ctr2].ul[(xsize-1)-ctr1];
      else U2_Exit(-1, "Unknown depth error (%ld) in %s!\n", depth, __PRETTY_FUNCTION__);
      }
    }
  ret.tcolor = tcolor;
  return ret;
  }

Graphic Graphic::RotatedClock()  {
  int ctr1, ctr2;
  Graphic ret(ysize, xsize);
  char tmpb[ret.xsize+2];
  for(ctr1=0; ctr1<(int)ret.ysize; ctr1++)  {
    ret.DefLin(tmpb);
    for(ctr2=0; ctr2<(int)ret.xsize; ctr2++)  {
      if(depth == 8)
	ret.image[ctr1].uc[ctr2] = image[(ysize-1)-ctr2].uc[ctr1];
      else if(depth == 32)
	ret.image[ctr1].ul[ctr2] = image[(ysize-1)-ctr2].ul[ctr1];
      else U2_Exit(-1, "Unknown depth error (%ld) in %s!\n", depth, __PRETTY_FUNCTION__);
      }
    }
  ret.tcolor = tcolor;
  return ret;
  }

Graphic Graphic::Rotated(int angle)  {
  Graphic ret;
  ret.SetRotated(*this, angle);
  return ret;
  }

void Graphic::SetLine(int xs, int ys, int d, unsigned long c) {
  int ctrx, ctry;
  depth=d; DefSize(abs(xs)+1,abs(ys)+1);
  tcolor=0;
  if(depth==8 && tcolor==c) ++tcolor;
  else if(depth==16 && tcolor==c) tcolor+=0x0101;
  int rev=(abs(xs*ys)!=(xs*ys));

  for(ctry=0; ctry<ysize; ++ctry)
    memset(image[ctry].uc, tcolor, xsize*(depth>>3));

  if(abs(xs)>abs(ys)) {
    if(rev) {
      if(depth==8)
	for(ctrx=0; ctrx<xsize; ++ctrx)
	  image[(ysize-1)-((ysize*ctrx)/xsize)].uc[ctrx] = c;
      else if(depth==32)
	for(ctrx=0; ctrx<xsize; ++ctrx)
	  image[(ysize-1)-((ysize*ctrx)/xsize)].ul[ctrx] = c;
      else if(depth==16)
	for(ctrx=0; ctrx<xsize; ++ctrx)
	  image[(ysize-1)-((ysize*ctrx)/xsize)].us[ctrx] = c;
      else U2_Exit(-1, "Unknown depth error (%ld) in %s!\n", depth, __PRETTY_FUNCTION__);
      }
    else {
      if(depth==8)
	for(ctrx=0; ctrx<xsize; ++ctrx)
	  image[(ysize*ctrx)/xsize].uc[ctrx] = c;
      else if(depth==32)
	for(ctrx=0; ctrx<xsize; ++ctrx)
	  image[(ysize*ctrx)/xsize].ul[ctrx] = c;
      else if(depth==16)
	for(ctrx=0; ctrx<xsize; ++ctrx)
	  image[(ysize*ctrx)/xsize].us[ctrx] = c;
      else U2_Exit(-1, "Unknown depth error (%ld) in %s!\n", depth, __PRETTY_FUNCTION__);
      }
    }
  else {
    if(rev) {
      if(depth==8)
	for(ctry=0; ctry<ysize; ++ctry)
	  image[ctry].uc[(xsize-1)-((xsize*ctry)/ysize)] = c;
      else if(depth==32)
	for(ctry=0; ctry<ysize; ++ctry)
	  image[ctry].ul[(xsize-1)-((xsize*ctry)/ysize)] = c;
      else if(depth==16)
	for(ctry=0; ctry<ysize; ++ctry)
	  image[ctry].us[(xsize-1)-((xsize*ctry)/ysize)] = c;
      else U2_Exit(-1, "Unknown depth error (%ld) in %s!\n", depth, __PRETTY_FUNCTION__);
      }
    else {
      if(depth==8)
	for(ctry=0; ctry<ysize; ++ctry)
	  image[ctry].uc[(xsize*ctry)/ysize] = c;
      else if(depth==32)
	for(ctry=0; ctry<ysize; ++ctry)
	  image[ctry].ul[(xsize*ctry)/ysize] = c;
      else if(depth==16)
	for(ctry=0; ctry<ysize; ++ctry)
	  image[ctry].us[(xsize*ctry)/ysize] = c;
      else U2_Exit(-1, "Unknown depth error (%ld) in %s!\n", depth, __PRETTY_FUNCTION__);
      }
    }
  if(xs<0) xcenter=xsize-1;
  else xcenter=0;
  if(ys<0) ycenter=ysize-1;
  else ycenter=0;
  }

void Graphic::SetRect(int xs, int ys, int d, unsigned long c) {
  if(xs<1 || ys<1) return;
  tcolor=0; if(depth==8 && tcolor==c) ++tcolor;
  depth=d; DefSize(xs,ys);
  int ctry, ctrx;

  for(ctry=0; ctry<ysize; ++ctry)
    memset(image[ctry].uc, tcolor, xsize*(depth>>3));

  if(depth==8) {
    memset(image[0].uc, c, xsize);
    memset(image[ysize-1].uc, c, xsize);
    for(ctry=1; ctry<(ysize-1); ++ctry) {
      image[ctry].uc[0] = c;
      image[ctry].uc[xsize-1] = c;
      }
    }
  else if(depth==32) {
    for(ctrx=0; ctrx<xsize; ++ctrx) {
      image[0].ul[ctrx] = c;
      image[ysize-1].ul[ctrx] = c;
      }
    for(ctry=1; ctry<(ysize-1); ++ctry) {
      image[ctry].uc[0] = c;
      image[ctry].uc[xsize-1] = c;
      }
    }
  else U2_Exit(-1, "Unknown depth error (%ld) in %s!\n", depth, __PRETTY_FUNCTION__);
  }

void Graphic::SetFillRect(int xs, int ys, int d, unsigned long c) {
  if(xs<1 || ys<1) return;
  tcolor=0; if(depth==8 && tcolor==c) ++tcolor;
  depth=d; DefSize(xs,ys);
  int ctry;
  if(depth==8) {
    for(ctry=0; ctry<ysize; ++ctry) {
      memset(image[ctry].uc, c, xsize);
      }
    }
  else if(depth==16) {
    for(ctry=0; ctry<ysize; ++ctry) {
      for(int ctrx=0; ctrx<xsize; ++ctrx) image[ctry].us[ctrx] = c;
      }
    }
  else if(depth==32) {
    for(ctry=0; ctry<ysize; ++ctry) {
      for(int ctrx=0; ctrx<xsize; ++ctrx) image[ctry].ul[ctrx] = c;
      }
    }
  else U2_Exit(-1, "Unknown depth error (%ld) in %s!\n", depth, __PRETTY_FUNCTION__);
  }

void Graphic::Clear() {
  ClearArea(0, 0, xsize, ysize);
  }

void Graphic::ClearArea(int x, int y, int xs, int ys) {
  if(tcolor == 0) {
    int ctr;
    for(ctr=y; ctr<y+ys; ++ctr) {
      memset(image[ctr].uc+(x*(depth>>3)), 0, xs*(depth>>3));
      }
    }
  else {
    int ctrx, ctry;
    for(ctry=0; ctry<ysize; ++ctry) {
      for(ctrx=0; ctrx<xsize; ++ctrx) {
	if(depth == 8) image[ctry].uc[ctry] = tcolor;
	else if(depth == 16) image[ctry].us[ctry] = tcolor;
	else if(depth == 32) image[ctry].ul[ctry] = tcolor;
	}
      }
    }
  }

void Graphic::DrawPixel(int x, int y, int d, unsigned long c) {
  ConvertColor(c, d, (int)depth);
  if(depth == 8) {
    image[y].uc[x] = c;
    }
  else if (depth == 16) {
    image[y].us[x] = c;
    }
  else if (depth == 32) {
    image[y].ul[x] = c;
    }
  }

void Graphic::DrawLine(int x, int y, int xs, int ys, int d, unsigned long c) {
  Graphic *g = new Graphic;
  ConvertColor(c, d, (int)depth);
  g->SetLine(xs, ys, depth, c);
  PasteTransparentGraphic(g, x, y);
  delete g;
  }

void Graphic::DrawRect(int x, int y, int xs, int ys, int d, unsigned long c) {
  Graphic *g = new Graphic;
  ConvertColor(c, d, (int)depth);
  g->SetRect(xs, ys, depth, c);
  PasteTransparentGraphic(g, x, y);
  delete g;
  }

void Graphic::DrawFillRect(int x, int y, int xs, int ys, int d, unsigned long c) {
  Graphic *g = new Graphic;
  ConvertColor(c, d, (int)depth);
  g->SetFillRect(xs, ys, depth, c);
  PasteTransparentGraphic(g, x, y);
  delete g;
  }

void Graphic::SetRotated(Graphic &in, int angle) {
  tcolor = in.tcolor;
  depth = in.depth;
  int dx, dy, size;
  if((in.xcenter) > (((long)in.xsize - in.xcenter) - 1))  dx = in.xcenter;
  else  dx = (in.xsize - in.xcenter) - 1;
  if((in.ycenter) > (((long)in.ysize - in.ycenter) - 1))  dy = in.ycenter;
  else  dy = (in.ysize - in.ycenter) - 1;
  size = (int)sqrt(dx*dx + dy*dy);
  size+=2;
  
  double basex, basey, curcos, cursin, Tg,
	incxx, incxy, incyx, incyy, curx, cury;
  DefSize(2*(size)+1, 2*(size)+1);
  SetCenter(size, size);
  Tg = cd2rad(angle);
  curcos = cos(Tg);
  cursin = sin(Tg);

  basex = (curcos * (double)-xcenter);
  basex -= (cursin * (double)-ycenter);
  basex += (double)in.xcenter;
  basex += (double)0.5;

  basey = (cursin * (double)-xcenter);
  basey += (curcos * (double)-ycenter);
  basey += (double)in.ycenter;
  basey += (double)0.5;

  incxx = curcos;
  incxy = -cursin;
  incyx = cursin;
  incyy = curcos;

  int yi, xi;
  mfmt curpt;
  for(yi=0; yi<(long)ysize; yi++)  {
    curpt.v = image[yi].v;
    curx = basex;  cury = basey;
    for(xi=0; xi<(long)xsize; xi++)  {
      dx=(int)curx;	dy=(int)cury;
      if((dx>=0) && (dy>=0) && (dx < (long)in.xsize) && (dy < (long)in.ysize)) {
	if(depth == 8) *(curpt.uc) = in.image[dy].uc[dx];
	else if(depth == 32) *(curpt.ul) = in.image[dy].ul[dx];
        else U2_Exit(-1, "Unknown depth error (%ld) in %s!\n", depth, __PRETTY_FUNCTION__);
	}
      else  {
	if(depth == 8) *(curpt.uc) = tcolor;
	else if(depth == 32) *(curpt.ul) = tcolor;
        else U2_Exit(-1, "Unknown depth error (%ld) in %s!\n", depth, __PRETTY_FUNCTION__);
	}
      if(depth == 8) curpt.uc++;
      else if(depth == 32) curpt.ul++;
      else U2_Exit(-1, "Unknown depth error (%ld) in %s!\n", depth, __PRETTY_FUNCTION__);
      curx+=incxx;
      cury+=incyx;
      }
    basex+=incxy;
    basey+=incyy;
    }
  }

Graphic::Graphic(const Graphic &from) {
//  UserDebug("Graphic:Copy Constructor   Begin");
  int ctr;
  image = NULL;
  xdef = 0;  ydef = 0;
  depth = from.depth;
  DefSize(from.xsize, from.ysize);
  for(ctr=0; ctr<(int)from.ysize; ctr++)  {
    memcpy(image[ctr].v, from.image[ctr].v,
	from.xsize*(depth>>3));
    }
  xcenter = from.xcenter;
  ycenter = from.ycenter;
  tcolor = from.tcolor;
//  UserDebug("Graphic:Copy Constructor   End");
  }

void Graphic::operator =(const Graphic &from) {
  int ctr;
  mfmt *cur;
  cur = image;

  depth = from.depth;
  DefSize(from.xsize, from.ysize);
  for(ctr=0; ctr<(int)from.ysize; ctr++)  {
    memcpy(image[ctr].v, from.image[ctr].v, from.xsize*(depth>>3));
    }
  xcenter = from.xcenter;
  ycenter = from.ycenter;
  tcolor = from.tcolor;
  }

Graphic Graphic::operator +(const Graphic &from)  {
  Graphic ret;
  int xc=xcenter, xe=xsize-xcenter, yc=ycenter, ye=ysize-ycenter;
  if(from.xcenter > xc)  xc = from.xcenter;
  if(from.ycenter > yc)  yc = from.ycenter;
  if(((long)from.xsize-from.xcenter) > xe)  xe = from.xsize-from.xcenter;
  if(((long)from.ysize-from.ycenter) > ye)  ye = from.ysize-from.ycenter;
  ret.DefSize(xc+xe, yc+ye);
  ret.tcolor = tcolor;
  ret.xcenter = xc;
  ret.ycenter = yc;
  int ctrx, ctry;
  for(ctry=-yc; ctry<ye; ctry++)  {
    for(ctrx=-xc; ctrx<xe; ctrx++)  {
      if(depth == 8) {
	if((ctrx+from.xcenter >= 0) && (ctry+from.ycenter >= 0) && 
		(ctrx+from.xcenter < (long)from.xsize) && 
		(ctry+from.ycenter < (long)from.ysize) &&
		(from.image[ctry+from.ycenter].uc[ctrx+from.xcenter] != from.tcolor))
	  ret.image[ctry+yc].uc[ctrx+xc] =
		from.image[ctry+from.ycenter].uc[ctrx+from.xcenter];
	else if((ctrx+xcenter >= 0) && (ctry+ycenter >= 0) && 
		(ctrx+xcenter < (long)xsize) && (ctry+ycenter < (long)ysize) &&
		(image[ctry+ycenter].uc[ctrx+xcenter] != from.tcolor))
	  ret.image[ctry+yc].uc[ctrx+xc] = image[ctry+ycenter].uc[ctrx+xcenter];
	else ret.image[ctry+yc].uc[ctrx+xc] = from.tcolor;
	}
      else if(depth == 32) {
	if((ctrx+from.xcenter >= 0) && (ctry+from.ycenter >= 0) && 
		(ctrx+from.xcenter < (long)from.xsize) && 
		(ctry+from.ycenter < (long)from.ysize) &&
		(from.image[ctry+from.ycenter].ul[ctrx+from.xcenter] != from.tcolor))
	  ret.image[ctry+yc].ul[ctrx+xc] =
		from.image[ctry+from.ycenter].ul[ctrx+from.xcenter];
	else if((ctrx+xcenter >= 0) && (ctry+ycenter >= 0) && 
		(ctrx+xcenter < (long)xsize) && (ctry+ycenter < (long)ysize) &&
		(image[ctry+ycenter].ul[ctrx+xcenter] != from.tcolor))
	  ret.image[ctry+yc].ul[ctrx+xc] = image[ctry+ycenter].ul[ctrx+xcenter];
	else ret.image[ctry+yc].ul[ctrx+xc] = 0;
	}
      else U2_Exit(-1, "Unknown depth error (%ld) in %s!\n", depth, __PRETTY_FUNCTION__);
      }
    }
  return ret;
  }

void Graphic::XFlip() {
  int ctr, ctr2;
  if(depth == 8) {
    unsigned char tmp;
    for(ctr = 0; ctr < (long)ysize; ctr++)  {
      for(ctr2 = 0; ctr2 < (long)xsize/2; ctr2++)  {
 	tmp = image[ctr].uc[ctr2];
	image[ctr].uc[ctr2] = image[ctr].uc[(xsize-1) - ctr2];
	image[ctr].uc[(xsize-1) - ctr2] = tmp;
        }
      }
    }
  else if(depth == 16) {
    unsigned short tmp;
    for(ctr = 0; ctr < (long)ysize; ctr++)  {
      for(ctr2 = 0; ctr2 < (long)xsize/2; ctr2++)  {
 	tmp = image[ctr].us[ctr2];
	image[ctr].us[ctr2] = image[ctr].us[(xsize-1) - ctr2];
	image[ctr].us[(xsize-1) - ctr2] = tmp;
        }
      }
    }
  else if(depth == 32) {
    unsigned long tmp;
    for(ctr = 0; ctr < (long)ysize; ctr++)  {
      for(ctr2 = 0; ctr2 < (long)xsize/2; ctr2++)  {
 	tmp = image[ctr].ul[ctr2];
	image[ctr].ul[ctr2] = image[ctr].ul[(xsize-1) - ctr2];
	image[ctr].ul[(xsize-1) - ctr2] = tmp;
        }
      }
    }
  xcenter = (xsize-1)-xcenter;
  }

void Graphic::YFlip() {
  int ctr;
  unsigned char tmp[xsize*(depth>>3)];
  for(ctr = 0; ctr < ((long)ysize/2); ctr++)  {
    memcpy(tmp, image[ctr].uc, xsize*(depth>>3));
    memcpy(image[ctr].uc, image[(ysize-1) - ctr].uc, xsize*(depth>>3));
    memcpy(image[(ysize-1) - ctr].uc, tmp, xsize*(depth>>3));
    }
  ycenter = (ysize-1)-ycenter;
  }

void Graphic::Trim() {
  UserDebug("Graphic:Trim()  Begin");
  int yb = ysize, ye = 0, xb = xsize, xe = 0, ctrx, ctry;
  int yclear = 1;
  if((ysize == 0) || (xsize == 0))  {
    UserDebug("Graphic:Trim()  Pre-Delete 1");
    if(image != NULL)  { delete image;  image = NULL; }
    image = new mfmt[1];
    xsize = 1;  ysize = 1;
    xdef = 1;  ydef = 1;
    xcenter = 0;  ycenter = 0;
    UserDebug("Graphic:Trim()  Post-Delete 1 -- End");
    return;
    }
  for(ctry = 0; ctry < (long)ysize; ctry++)  {
    int xclear = 1;
    for(ctrx = 0; ctrx < (long)xsize; ctrx++)  {
      if((depth == 8 && image[ctry].uc [ctrx] != 0)
	  || (depth == 32 && image[ctry].ul[ctrx] != 0))  {
	if((yclear == 1) && (xclear == 1) && (yb > ctry))  yb = ctry;
        yclear = 0;
	if(ye < ctry)  ye = ctry;
	if((xclear == 1) && (xb > ctrx))  xb = ctrx;
	xclear = 0;
	if(xe < ctrx)  xe = ctrx;
	}
      }
    }
  if((yb == 0) && (xb == 0) && (ye == ((long)ysize -1))
	&& (xe == ((long)xsize -1)))  {
    UserDebug("Graphic:Trim()  Do nothing -- End");
    return;
    }
  if((yb > ye) || (xb > xe))  {
    UserDebug("Graphic:Trim()  Pre-Delete 2");
    if(image != NULL)  { delete image;  image = NULL; }
    image = new mfmt[1];
    xsize = 1;  ysize = 1;
    xdef = 1;  ydef = 1;
    xcenter = 0;  ycenter = 0;
    UserDebug("Graphic:Trim()  Post-Delete 2 -- End");
    return;
    }
  UserDebug("Graphic:Trim()  Begin Alloc");
  xcenter -= xb;
  ycenter -= yb;
  xsize -= (xsize - xe) - 1;
  ysize -= (ysize - ye) - 1;
  xsize -= xb;
  ysize -= yb;
  UserDebug("Graphic:Trim()  Begin Trim");
  for(ctry = yb; ctry < (ye+1); ctry++)  {
    if(depth == 8)
	memmove(image[ctry-yb].uc, &image[ctry].uc[xb], xsize);
    else if(depth == 16)
	memmove(image[ctry-yb].us, &image[ctry].us[xb], xsize*2);
    else if(depth == 32)
	memmove(image[ctry-yb].ul, &image[ctry].ul[xb], xsize*4);
    else U2_Exit(-1, "Unknown depth error (%ld) in %s!\n", depth, __PRETTY_FUNCTION__);
    }
  UserDebug("Graphic:Trim()  End");
  }

void Graphic::FindTrueCenter() {
  xcenter = xsize / 2;
  ycenter = ysize / 2;
  }

void Graphic::SetCenter(int xcr, int ycr) {
  xcenter = xcr;
  ycenter = ycr;
  }

void Graphic::DefSize(int xsz, int ysz) {
  UserDebug("User:Graphic:DefSize Begin");
  int ctr;
  xcenter = 0;		ycenter = 0;	linedef = 0;
  ysize = ysz;          xsize = xsz;
  if(xsz <= (int)xdef && ysz <= (int)ydef)  return;

  UserDebug("User:Graphic:DefSize Continuing");
  chunk.SetSize(xsz*ysz*(depth>>3));
  ydef = 1; xdef = chunk.Size()/(depth>>3);
  while(ydef < ysz || (xdef > (ydef<<1) && (xdef>>1) >= xsz)) {
    xdef >>= 1;  ydef <<= 1;
    }
  if(xdef < xsz || ydef < ysz) {
    ydef = ysz;
    xdef = chunk.Size()/(ydef*(depth>>3));
    }

  UserDebug("User:Graphic:DefSize Pre-Delete");
  if(image) { delete image; image = NULL; }
  UserDebug("User:Graphic:DefSize Pre-New");
  image = new mfmt[ydef];
  UserDebug("User:Graphic:DefSize After Memory");
  if(!image) U2_Exit(-1, "Out of memory (image)!\n");
  for(ctr=0; ctr<ydef; ++ctr) {
    image[ctr].c = chunk.Data().c + ctr*xdef*(depth>>3);
    }

  UserDebug("User:Graphic:DefSize End");
  }

Graphic::~Graphic() {
//  UserDebug("User:Graphic:~Graphic() Begin");
  if(image) { delete image; image = NULL; }
  ydef = 0;
  xdef = 0;
  ysize = 0;
  xsize = 0;
//  UserDebug("User:Graphic:~Graphic() End");
  }

void Graphic::DefLin(char *data)  {
  if(ysize == 0 || xsize == 0)  return;
  int line = linedef%ysize;
  memcpy(image[line].uc, data, xsize*(depth>>3));
  linedef++;
  }

void Graphic::DefLinH(char *data)  { //** 8-bit only
  int ctr;
  int tmp, tmp2;
  for(ctr=0; ctr<(long)xsize; ctr++)  {
    tmp2 = data[ctr*2]-'0';
    if((tmp2 < 0) || (tmp2 > 9))  tmp2 = data[ctr*2]+10-'a';
    if((tmp2 < 0) || (tmp2 > 15))  tmp2 = data[ctr*2]+10-'A';
    if((tmp2 < 0) || (tmp2 > 15))  tmp2 = 0;
    tmp = data[ctr*2+1]-'0';
    if((tmp < 0) || (tmp > 9))  tmp = data[ctr*2+1]+10-'a';
    if((tmp < 0) || (tmp > 15))  tmp = data[ctr*2+1]+10-'A';
    if((tmp < 0) || (tmp > 15))  tmp = 0;
    tmp = tmp2*16+tmp;
    image[linedef].uc[ctr] = tmp;
    }
  linedef++;
  if(linedef >= (long)ysize)  linedef = 0;
  }

Graphic::Graphic(char *fn, Palette &p)  {
  image = NULL;
  xdef = 0;  ydef = 0;
  xsize = 0;  ysize = 0;
  depth = 8; tcolor=0;
  char buffer[16];
  FILE *bmp = U2_FOpenRead(fn);
  if(bmp == NULL)  {
    U2_Exit(1, "\"%s\" Not Found!\n", fn);
    }
  fread(buffer, 1, 16, bmp);
  if((buffer[0] != 'B') || (buffer[1] != 'M'))  {
    U2_Exit(1, "\"%s\" is Not A Bitmap file!\n", fn);
    }
  fread(buffer, 1, 16, bmp);
  fclose(bmp);
  depth = buffer[12]+256*(buffer[13]);
  if(depth == 8)  {
    Init(fn);
    Palette po;
    po.Set(fn);
    PaletteConvert(po, p);
    }
  else if(depth == 24)  {
    Init24(fn, p);
    }
  else  {
    U2_Exit(1, "I only support 8 and 24 bit Bitmap files, \"%s\" is %ld-bit!\n",
	fn, depth);
    }
  tcolor = image[0].uc[0]; //** 8-bit only
  }

Graphic::Graphic(char *fn)  {
  image = NULL;
  xdef = 0;  ydef = 0;
  xsize = 0;  ysize = 0;
  depth = 8; tcolor=0;
  Init(fn);
  }

void Graphic::InitTGA32(char *fn)  {
  int ctry;
  unsigned char buf[256];
  FILE *tga = U2_FOpenRead(fn);
  if(tga != NULL) {
    int nr=fread(buf, 1, 18, tga);
    if(nr == 18 && buf[1] == 0 && buf[2] == 2) {
      depth = 32; tcolor=0; int rev=(buf[17]&0x20);
      if(buf[17]&0x10) U2_Exit(1, "%s is a backward TGA file!\n", fn);
      if(buf[16] != 32)
	U2_Exit(1, "Depth of \"%s\" is %d, not 32!\n", fn, buf[16]);
      DefSize((buf[13]<<8) + buf[12], (buf[15]<<8) + buf[14]);
      fread(buf, 1, (int)buf[0], tga);
      if(!rev) for(ctry=0; ctry<ysize; ++ctry) {
	fread(image[ysize-(ctry+1)].ul, 1, xsize*4, tga);
	}
      else for(ctry=0; ctry<ysize; ++ctry) {
	fread(image[ctry].ul, 1, xsize*4, tga);
	}
      }
    else if(nr == 18 && buf[1] == 0 && buf[2] == 10) {
      depth = 32; tcolor=0; int rev=(buf[17]&0x20), ctr, x, y, yr, sz;
      if(buf[17]&0x10) U2_Exit(1, "%s is a backward TGA file!\n", fn);
      if(buf[16] != 32)
	U2_Exit(1, "Depth of \"%s\" is %d, not 32!\n", fn, buf[16]);
      DefSize((buf[13]<<8) + buf[12], (buf[15]<<8) + buf[14]);
      fread(buf, 1, (int)buf[0], tga);

      x=0; y=0; yr=0; if(!rev) yr=ysize-1;
      while(y<ysize) {
	unsigned long tmpv;
	fread(buf, 1, 1, tga);
	sz=(buf[0]&0x7F)+1;
	if(buf[0]&0x80) {
	  fread(&tmpv, 1, 4, tga);
	  for(ctr=0; ctr<sz; ++ctr) {
	    image[y].ul[x++] = tmpv;
	    if(x==xsize) {
//	      fprintf(stderr, "\"%s\" breaks TGA rules!!!\n", fn);
	      ++y; --yr; x=0;
	      }
	    }
	  }
	else {
	  while(x+sz>=xsize) {
	    int sz2 = xsize-x; sz-=sz2;
//	    fprintf(stderr, "\"%s\" breaks TGA rules!!!\n", fn);
	    fread(&image[y].ul[x], 1, sz2<<2, tga);
	    ++y; --yr; x=0;
	    }
	  fread(&image[y].ul[x], 1, sz<<2, tga);
	  x+=sz;
	  }
	if(x==xsize) { ++y; --yr; x=0; }
	else if(x>xsize)
	  U2_Exit(1, "RLE Overrun (%d>%ld) in \"%s\"\n", x, xsize, fn);
	}
      }
    else {
      U2_Exit(1, "\"%s\" isn't a 32-bit TGA file!\n", fn);
      }
    fclose(tga);
    }
  else {
    U2_Exit(1, "Can't open \"%s\"!\n", fn);
    }
  }

void Graphic::Init(char *fn)  {
 UserDebug("Graphic::Init Begin");
 xdef = 0;  ydef = 0;
 if(!strcasecmp(".tga", &fn[strlen(fn)-4])) { InitTGA32(fn); return; }
 FILE *bmp;
 int colused;
 long size2, width, height, off = 0;
 unsigned char buffer[1280];
 long detect;
 {
  bmp = U2_FOpenRead(fn);
  UserDebug("Graphic::Init First Read");
  fread(buffer, 1, 16, bmp);
  if((buffer[0] != 'B') || (buffer[1] != 'M'))  {
    UserDebug("Graphic::Init Using First U2_Exit!");
    U2_Exit(1, "\"%s\" Is Not A Bitmap file!\n", fn);
    }
  UserDebug("Graphic::Init After First U2_Exit");
  size2 = buffer[14]+256*(buffer[15]);
  fread(buffer, 1, (size2 - 2), bmp);
  width = buffer[2]+256*(buffer[3]);
  height = buffer[6]+256*(buffer[7]);
  detect = buffer[10]+256*(buffer[11]);
  UserDebug("Graphic::Init Plane Read");
  if(detect != 1)  {
    U2_Exit(1, "I only suport 1 plane Bitmap files, \"%s\" is %ld-plane!\n",
        fn, detect);
    }
  depth = buffer[12]+256*(buffer[13]);
  UserDebug("Graphic::Init Depth Read");
  if(depth != 8 && depth != 24)  {
    U2_Exit(1, "\"%s\" is %ld-bit, Only 8 and 24-bit BMPs supported w/o palette!\n",
	fn, depth);
    }
  detect = buffer[14]+256*(buffer[15]);
  UserDebug("Graphic::Init Compression Read");
  if(detect != 0)  {
    U2_Exit(1, "I do not support compressed Bitmap files, \"%s\" is compressed!\n",
        fn);
    }
  int bytes = (depth>>3);
  if((bytes*width) %4 != 0)  off = 4 - ((bytes*width) % 4);
  if(depth == 8)  {
    int ctr;
    colused = buffer[30]+256*(buffer[31]);
    if(colused == 0)  colused = 256;
    fread(buffer, 1, colused*4, bmp);
    DefSize(width, height);
    for(ctr = height; ctr > 0; ctr--)  {
      int tmp = fread(image[ctr-1].uc, 1, width*bytes, bmp);
      if(tmp != ((long)(width*bytes)))  {
	U2_Exit(1, "Read error in 8-bit file \"%s\"\n", fn);
	}
      fread(buffer, 1, off, bmp);
      }
    }
  else if(depth == 24)  {
    int ctr, ctr2;
    depth = 32;
    DefSize(width, height);
    for(ctr = height; ctr > 0; --ctr)  {
      for(ctr2 = 0; ctr2 <width; ++ctr2)  {
	int tmp = fread(&image[ctr-1].ul[ctr2], 1, 3, bmp);
	if(tmp != 3)  {
	  U2_Exit(1, "Read error in 24-bit file \"%s\"\n", fn);
	  }
	}
      fread(buffer, 1, off, bmp);
      }
    for(ctr = 0; ctr < height; ++ctr) {
      for(ctr2 = 0; ctr2 <width; ++ctr2) {
	image[ctr].uc[(ctr2<<2)+3] = 0;
	if((image[0].ul[0] & 0x0FFFFFF) != (image[ctr].ul[ctr2] & 0x0FFFFFF))
	  image[ctr].uc[(ctr2<<2)+3] = 255;
	}
      }
    }
  linedef = height;
  if(depth == 8) tcolor = image[0].uc[0];
  else if(depth == 32) tcolor = image[0].uc[3]; //** 32-bit kludge
  else U2_Exit(-1, "Unknown depth error (%ld) in %s!\n", depth, __PRETTY_FUNCTION__);
  UserDebug("Graphic::Init Close File");
  fclose(bmp);
  }
 UserDebug("Graphic::Init End");
 }

#define fpint(f, i) fprintf(f, "%c%c%c%c", (char)(i&255), (char)((i>>8)&255), (char)((i>>16)&255), (char)((i>>24)&255))

void Graphic::SaveBMP(char *fn, const Palette &pal) {
 UserDebug("User::Graphic::SaveBMP(2) Begin");
 FILE *bmp;
 int ctr, ctr2;
 {
  bmp = U2_FOpenWrite(fn);

  int filesize = ((ysize * ((xsize+3) - ((xsize+3)&3))) + (pal.coldec*4) + 54);
	//BITMAPFILEHEADER
  fprintf(bmp, "BM");				// ID
  fpint(bmp, filesize);	//*FILE SIZE!!!!
  fprintf(bmp, "%c%c%c%c", 0, 0, 0, 0);		// Reserved
  fpint(bmp, (pal.coldec*4) + 54);	//*OFFSET OF DATA!!!!
  
	//BITMAPINFOHEADER
  fprintf(bmp, "%c%c%c%c", 40, 0, 0, 0);	// Header Size
  fpint(bmp, xsize);				// X Size
  fpint(bmp, ysize);				// Y Size
  fprintf(bmp, "%c%c", 1, 0);			// 1 Plane
  fprintf(bmp, "%c%c", 8, 0);			// 8 Bit
  fprintf(bmp, "%c%c%c%c", 0, 0, 0, 0);		// Not Compressed
  fpint(bmp, xsize*ysize);			// Size Default
  fprintf(bmp, "%c%c%c%c", 0, 0, 0, 0);		// Don't Care About 
  fprintf(bmp, "%c%c%c%c", 0, 0, 0, 0);		// - Pels Per Meter
  fpint(bmp, pal.coldec);			// Used this many colors
  fpint(bmp, pal.coldec);			// All are Important

	//Palette
  for(ctr=0; ctr<pal.coldec; ctr++)  {
    fprintf(bmp, "%c%c%c%c", ((Palette&)pal).GetBlueEntry(ctr),
      ((Palette&)pal).GetGreenEntry(ctr), ((Palette&)pal).GetRedEntry(ctr), 0);
    }
  for(ctr=ysize-1; ctr>=0; ctr--)  {
    for(ctr2=0; ctr2<(int)xsize; ctr2++)
      fprintf(bmp, "%c", image[ctr].uc[ctr2]); //** 8-bit only!
    for(; (ctr2 & 3) != 0; ctr2++)
      fprintf(bmp, "%c", 0);
    }
  fclose(bmp);
//  printf("Xsize = %d, Ysize = %d\n  Size ?= (%d)\n", xsize, ysize,
//	(ysize * ((xsize+3) - ((xsize+3)&3))) + (pal.coldec*4) + 54);
  }
 UserDebug("User::Graphic::SaveBMP(2) End");
 }

void Graphic::SaveBMP(char *fn)  {
 UserDebug("User::Graphic::SaveBMP(1) Begin");
 if(depth != 32) U2_Exit(1, "Depth = %ld and no Palette given!\n", depth);
 FILE *bmp;
 int ctr, ctr2;
 {
  bmp = U2_FOpenRead(fn);

  int filesize = ((ysize * ((xsize+3) - ((xsize+3)&3))) + 54);
	//BITMAPFILEHEADER
  fprintf(bmp, "BM");				// ID
  fpint(bmp, filesize);	//*FILE SIZE!!!!
  fprintf(bmp, "%c%c%c%c", 0, 0, 0, 0);		// Reserved
  fpint(bmp, 54);	//*OFFSET OF DATA!!!!
  
	//BITMAPINFOHEADER
  fprintf(bmp, "%c%c%c%c", 40, 0, 0, 0);	// Header Size
  fpint(bmp, xsize);				// X Size
  fpint(bmp, ysize);				// Y Size
  fprintf(bmp, "%c%c", 1, 0);			// 1 Plane
  fprintf(bmp, "%c%c", 24, 0);			// 24 Bit
  fprintf(bmp, "%c%c%c%c", 0, 0, 0, 0);		// Not Compressed
  fpint(bmp, xsize*ysize);			// Size Default
  fprintf(bmp, "%c%c%c%c", 0, 0, 0, 0);		// Don't Care About 
  fprintf(bmp, "%c%c%c%c", 0, 0, 0, 0);		// - Pels Per Meter
  fpint(bmp, 255);			// Used this many colors
  fpint(bmp, 255);			// All are Important

  for(ctr=ysize-1; ctr>=0; ctr--)  {
    for(ctr2=0; ctr2<(int)xsize*(depth>>3); ctr2++)
      if((ctr2&3) != 3) fprintf(bmp, "%c", image[ctr].uc[ctr2]);
//    for(ctr2=0; ctr2<(int)xsize*(depth>>3); ctr2++)
//      fprintf(bmp, "%c", image[ctr].uc[ctr2]);  //** Broken!!!!!!
//    for(; (ctr2 & 3) != 0; ctr2++)
//      fprintf(bmp, "%c", 0);
    }
  fclose(bmp);
//  printf("Xsize = %d, Ysize = %d\n  Size ?= (%d)\n", xsize, ysize,
//	(ysize * ((xsize+3) - ((xsize+3)&3))) + 54);
  }
 UserDebug("User::Graphic::SaveBMP(1) End");
 }

void Graphic::Init24(char *fn, Palette &p)  {
  UserDebug("User::Graphic::Init24 Begin");
  xdef = 0;  ydef = 0;
  FILE *bmp;
  long size2, width, height;
  unsigned char buffer[4096];
  long detect;
  int ctr, ctr2;

  bmp = U2_FOpenRead(fn);

  fread(buffer, 1, 16, bmp);
  if((buffer[0] != 'B') || (buffer[1] != 'M'))
	U2_Exit(1, "\"%s\" Is Not A Bitmap file!\n", fn);
  size2 = buffer[14]+256*(buffer[15]);

  fread(buffer, 1, (size2 - 2), bmp);
  width = buffer[2]+256*(buffer[3]);
  height = buffer[6]+256*(buffer[7]);
  detect = buffer[10]+256*(buffer[11]);
  if(detect != 1)  {
    U2_Exit(1, "I only suport 1 plane Bitmap files, \"%s\" is %ld-plane!\n",
        fn, detect);
    }
  depth = buffer[12]+256*(buffer[13]);
  if(depth != 24)  {
    U2_Exit(1, "\"%s\" is a %ld-bit file that detected as 24-bit!?!\n", fn, depth);
    }
  detect = buffer[14]+256*(buffer[15]);
  if(detect != 0)  {
    U2_Exit(1, "I do not support compressed Bitmap files, \"%s\" is compressed!\n",
        fn);
    }
  DefSize(width, height);

  int off = (4-((width*3) & 3)) & 3;
  for(ctr = height; ctr > 0; ctr--)  {
    int tmp = fread(buffer, 1, width*3+off, bmp);
    if(tmp != (long)width*3+off)  {
      U2_Exit(1, "Read error in 24-bit file loading \"%s\"\n", fn);
      }
    for(ctr2 = 0; ctr2 < (int)width; ctr2++)  { //** 8-bit (maybe not an error)
      image[ctr-1].uc[ctr2] = p.GetClosestColor(buffer[ctr2*3+2],
		buffer[ctr2*3+1], buffer[ctr2*3]);
      }
    }

  linedef = height;
  if(depth == 8) tcolor = image[0].uc[0]; //** Same tcolor 8
  else if(depth == 32) tcolor = image[0].uc[3]; //** 32-bit kludge
  else U2_Exit(-1, "Unknown depth error (%ld) in %s!\n", depth, __PRETTY_FUNCTION__);
  
  fclose(bmp);
  UserDebug("User::Graphic::Init24 End");
  }

void Graphic::PasteGraphic(Graphic &g, int x, int y) {
  PasteGraphic(&g, x, y);
  }

void Graphic::PasteGraphic(Graphic *g, int x, int y) {
  UserDebug("User::Graphic::PasteGraphic() Begin");
  if(g==NULL) return;
  if(g->depth != depth)
    U2_Exit(1, "Depth mismatch in %s\n", __PRETTY_FUNCTION__);
  int ctry;
  if(x<0 || y<0 || x+g->xsize > xsize || y+g->ysize > ysize) {
    fprintf(stderr, "Pasting %ldx%ld into %ldx%ld at (%d,%d)\n",
	g->xsize, g->ysize, xsize, ysize, x, y);
    U2_Exit(1, "Out of bounds error in %s\n", __PRETTY_FUNCTION__);
    }
  if(depth == 8) {
    for(ctry=0; ctry<g->ysize; ++ctry) {
      memcpy(image[ctry+y].uc+x, g->image[ctry].ul, g->xsize);
      }   
    }
  else if(depth == 32) {
    int ctrx;
    for(ctry=0; ctry<g->ysize; ++ctry) {
      memcpy(image[ctry+y].ul+x, g->image[ctry].ul, (g->xsize)<<2);
      for(ctrx=0; ctrx<g->xsize; ++ctrx)
        image[ctry+y].uc[((ctrx+x)<<2)+3]=0xFF;
      }   
    }
  else if(depth == 16) {
    for(ctry=0; ctry<g->ysize; ++ctry) {
      memcpy(image[ctry+y].us+x, g->image[ctry].us, (g->xsize)<<1);
      }   
    }
  UserDebug("User::Graphic::PasteGraphic() End");
  }

void Graphic::PasteTransparentGraphic(Graphic &g, int x, int y) {
  PasteTransparentGraphic(&g, x, y);
  }

void Graphic::PasteTransparentGraphic(Graphic *gr, int x, int y) {
  UserDebug("User::Graphic::PasteTransparentGraphic() Begin");
  if(gr==NULL) return;
  if(gr->depth != depth)
    U2_Exit(1, "Depth mismatch in %s\n", __PRETTY_FUNCTION__);
  int ctry, ctrx;
  if(x<0 || y<0 || x+gr->xsize > xsize || y+gr->ysize > ysize)
    U2_Exit(1, "Out of bounds error (%d->%ld, %d->%ld)->(%ldx%ld) in %s\n",
	x, x+gr->xsize, y, y+gr->ysize, xsize, ysize, __PRETTY_FUNCTION__);
  UserDebug("User::Graphic::PasteTransparentGraphic() Write");
  if(depth == 8) {
    UserDebug("User::Graphic::PasteTransparentGraphic() 8bpp Write");
    for(ctry=0; ctry<gr->ysize; ++ctry) {
      for(ctrx=0; ctrx<gr->xsize; ++ctrx) {
	if(gr->image[ctry].uc[ctrx] != tcolor)
	  image[ctry+y].uc[ctrx+x] = gr->image[ctry].uc[ctrx];
	}
      }   
    }
  else if(depth == 32) {
    UserDebug("User::Graphic::PasteTransparentGraphic() 32bpp Write");
    for(ctry=0; ctry<gr->ysize; ++ctry) {
      for(ctrx=0; ctrx<gr->xsize; ++ctrx) {
	unsigned long alpha=gr->image[ctry].uc[(ctrx<<2)+3];
	unsigned long cr = image[ctry+y].uc[((ctrx+x)<<2)]*(255-alpha);
	unsigned long cg = image[ctry+y].uc[((ctrx+x)<<2)+1]*(255-alpha);
	unsigned long cb = image[ctry+y].uc[((ctrx+x)<<2)+2]*(255-alpha);
	cr += gr->image[ctry].uc[(ctrx<<2)]*alpha;   cr/=255;
	cg += gr->image[ctry].uc[(ctrx<<2)+1]*alpha; cg/=255;
	cb += gr->image[ctry].uc[(ctrx<<2)+2]*alpha; cb/=255;
	image[ctry+y].uc[((ctrx+x)<<2)] = cr;
	image[ctry+y].uc[((ctrx+x)<<2)+1] = cg;
	image[ctry+y].uc[((ctrx+x)<<2)+2] = cb;
	image[ctry+y].uc[((ctrx+x)<<2)+3] = 255;
	}
      }   
    }
  else if(depth == 16) {
    UserDebug("User::Graphic::PasteTransparentGraphic() 16bpp Write");
    for(ctry=0; ctry<gr->ysize; ++ctry) {
      for(ctrx=0; ctrx<gr->xsize; ++ctrx) {
	if(gr->image[ctry].us[ctrx] != tcolor)
	  image[ctry+y].us[ctrx+x] = gr->image[ctry].us[ctrx];
	}
      }   
    }
  UserDebug("User::Graphic::PasteTransparentGraphic() End");
  }

void Graphic::DepthConvert(int d, const Palette &p) {
  UserDebug("User::Graphic::DepthConvert Begin");
  int ctrx, ctry;
  if(depth==8 && d==32)  {
    Graphic g(*this);
    DefSize(xsize<<2, ysize);
    xcenter=g.xcenter; ycenter=g.ycenter;
    depth = d; xsize>>=2;
    for(ctry=0; ctry<g.ysize; ctry++)  {
      for(ctrx=0; ctrx<g.xsize; ctrx++)  {
	if(g.image[ctry].uc[ctrx] == tcolor) {
          image[ctry].uc[(ctrx<<2)+3] = 0;
          image[ctry].uc[(ctrx<<2)+2] = p.GetRedEntry(g.image[ctry].uc[ctrx]);
          image[ctry].uc[(ctrx<<2)+1] = p.GetGreenEntry(g.image[ctry].uc[ctrx]);
          image[ctry].uc[(ctrx<<2)] = p.GetBlueEntry(g.image[ctry].uc[ctrx]);
	  }
	else {
          image[ctry].uc[(ctrx<<2)+3] = 0xFF;
          image[ctry].uc[(ctrx<<2)+2] = p.GetRedEntry(g.image[ctry].uc[ctrx]);
          image[ctry].uc[(ctrx<<2)+1] = p.GetGreenEntry(g.image[ctry].uc[ctrx]);
          image[ctry].uc[(ctrx<<2)] = p.GetBlueEntry(g.image[ctry].uc[ctrx]);
	  }
	}
      }
    }
/*
  else if(depth==8 && d==16)  {
    Graphic g(*this);
    DefSize(xsize<<2, ysize);
    xcenter=g.xcenter; ycenter=g.ycenter;
    depth = d; xsize>>=2;
    for(ctry=0; ctry<g.ysize; ctry++)  {
      for(ctrx=0; ctrx<g.xsize; ctrx++)  {
	if(g.image[ctry].uc[ctrx] == tcolor) {
          image[ctry].ul[ctrx] = 0;
	  }
	else {
          image[ctry].uc[(ctrx<<2)+3] = 0xFF;
          image[ctry].uc[(ctrx<<2)+2] = p.GetRedEntry(g.image[ctry].uc[ctrx]);
          image[ctry].uc[(ctrx<<2)+1] = p.GetGreenEntry(g.image[ctry].uc[ctrx]);
          image[ctry].uc[(ctrx<<2)] = p.GetBlueEntry(g.image[ctry].uc[ctrx]);
	  }
	}
      }
    }
*/
  else if(depth==32 && d==16)  {
    Graphic g(*this);
    depth = 16;
    DefSize(xsize, ysize);
    xcenter=g.xcenter; ycenter=g.ycenter;
    tcolor=0xFFFFFFFF;
    for(ctry=0; ctry<g.ysize; ctry++)  {
      for(ctrx=0; ctrx<g.xsize; ctrx++)  {
	if(g.image[ctry].uc[(ctrx<<2)+3] == 0) {
	  if(tcolor==0xFFFFFFFF) {
	    tcolor = (g.image[ctry].uc[(ctrx<<2)+2] >> 3);
	    tcolor <<= 6;
	    tcolor |= (g.image[ctry].uc[(ctrx<<2)+1] >> 2);
	    tcolor <<= 5;
	    tcolor |= (g.image[ctry].uc[ctrx<<2] >> 3);
	    }
          image[ctry].us[ctrx] = tcolor;
	  }
	else {
	  unsigned long col = (g.image[ctry].uc[(ctrx<<2)+2] >> 3);
	  col <<= 6;
	  col |= (g.image[ctry].uc[(ctrx<<2)+1] >> 2);
	  col <<= 5;
	  col |= (g.image[ctry].uc[ctrx<<2] >> 3);
	  image[ctry].us[ctrx] = col;
	  }
	}
      }
    }
  else U2_Exit(-1, "Unimplemented Graphic depth convert from %ld to %d\n", depth, d);
  UserDebug("User::Graphic::DepthConvert End");
  }

void Graphic::Undo3ds() {
  int ctrx, ctry;
  for(ctry=0; ctry<ysize; ++ctry) {
    for(ctrx=0; ctrx<xsize; ++ctrx) {
      unsigned long cr, cg, cb, a;
      a = image[ctry].uc[(ctrx<<2)+3];
      if(a) {
        cb = image[ctry].uc[(ctrx<<2)+2];
        cg = image[ctry].uc[(ctrx<<2)+1];
        cr = image[ctry].uc[(ctrx<<2)+0];
        cb*=255; cg*=255; cr*=255;
        cb/=a; cg/=a; cr/=a;
        image[ctry].uc[(ctrx<<2)+2] = cb;
        image[ctry].uc[(ctrx<<2)+1] = cg;
        image[ctry].uc[(ctrx<<2)+0] = cr;
        }
      }  
    }    
  }

color Graphic::GetPixel(int x, int y) {
  if(depth == 8) {
    return image[y].uc[x];
    }
  else if (depth == 16) {
    return image[y].us[x];
    }
  else if (depth == 32) {
    return image[y].ul[x];
    }
  return 0;
  }
