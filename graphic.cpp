// *************************************************************************
// graphic.cpp
// Advanced Graphic class, Pre ALPHA non-distribution version
//
// -By Insomnia (Steaphan Greene)      (Copyright 1997-1999 Steaphan Greene)
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

//#define		M_PI 3.1415926535

#include        "config.h"
#include        "graphic.h"

#define cd2rad(a) ((double)((((double)(a))*((double)M_PI))/(double)(32768)))

Graphic::Graphic() {
  image = NULL;  image3d = NULL;
  xdef = 0;  ydef = 0;  zdef = 0;
  depth = 8; tcolor=0;
  }

Graphic::Graphic(int xsz, int ysz) {
  xdef = 0;  ydef = 0;  zdef = 0;
  depth = 8; tcolor=0;
  DefSize(xsz, ysz);
  }

Graphic::Graphic(int xsz, int ysz, int zsz) {
  xdef = 0;  ydef = 0;  zdef = 0;
  depth = 8; tcolor=0;
  DefSize(xsz, ysz, zsz);
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

Graphic Graphic::Hashed()  {
  Graphic ret;
  int X, Y;
  ret = *this;
  for(Y=0; Y<(long)ysize; Y++)  {
    for(X=(Y&1); X<(long)xsize; X+=2)  {
      if(depth == 8) ret.image[Y].uc[X] = 0;
      else if(depth == 32) ret.image[Y].ul[X] = 0;
      else Exit(-1, "Unknown depth error!\n");
      }
    }
  return ret;
  }

Graphic Graphic::OffHashed()  {
  Graphic ret;
  int X, Y;
  ret = *this;
  for(Y=0; Y<(long)ysize; Y++)  {
    for(X=(1-(Y&1)); X<(long)xsize; X+=2)  {
      if(depth == 8) ret.image[Y].uc[X] = 0;
      else if(depth == 32) ret.image[Y].ul[X] = 0;
      else Exit(-1, "Unknown depth error!\n");
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

Graphic Graphic::Rotated(double sc, int xa, int ya, int za)  {
  Graphic ret;
  ret.tcolor = tcolor;
  int sz, xsz, ysz;
  sz = (int)(sc*zsize*(1.75)+1.5);
  xsz = (int)(sc*xsize*(1.75)+1.5);
  ysz = (int)(sc*ysize*(1.75)+1.5);
  if(ysz > sz) sz = ysz;
  if(xsz > sz) sz = xsz;
  ret.depth = depth;
  ret.DefSize(sz, sz);
  ret.FindTrueCenter();
  sz++;  sz >>= 1;

  int ctr, ctr2;
  double ax = cd2rad(xa);
  double ay = cd2rad(ya);
  double az = cd2rad(za);
  double b1x, b1y, b1z;
  double b2x, b2y, b2z;
  double cx, cy, cz;
  double cosx = cos(ax);
  double sinx = sin(ax);
  double cosy = cos(ay);
  double siny = sin(ay);
  double cosz = cos(az);
  double sinz = sin(az);

//  double incx_x=cosz,	incy_x= -sinz,	incz_x= 0;
//  double incx_y=sinz,	incy_y= cosz,	incz_y= 0;
//  double incx_z=0,	incy_z= 0,	incz_z= 1;

//  double incx_x= cosz*cosy,	incy_x= -sinz*cosy,	incz_x= siny;
//  double incx_y=   sinz,	incy_y=    cosz,	incz_y= 0;
//  double incx_z= -siny*cosz,	incy_z=  sinz*siny,	incz_z= cosy;

  double incx_x=cosz*cosy, incy_x= cosz*siny*sinx-sinz*cosx, incz_x= cosz*siny*cosx+sinz*sinx;
  double incx_y=sinz*cosy, incy_y= sinz*siny*sinx+cosz*cosx, incz_y= sinz*siny*cosx-cosz*sinx;
  double incx_z=  -siny,   incy_z=         cosy*sinx,        incz_z=          cosy*cosx;

  incx_x /= sc;  incx_y /= sc;  incx_z /= sc;
  incy_x /= sc;  incy_y /= sc;  incy_z /= sc;
  incz_x /= sc;  incz_y /= sc;  incz_z /= sc;

  b1x = (double)0.5;
  b1x += -(incx_x * (double)sz);
  b1x += -(incy_x * (double)sz);
  b1x += -(incz_x * (double)sz);
  b1x += (double)xcenter;

  b1y = (double)0.5;
  b1y += -(incx_y * (double)sz);
  b1y += -(incy_y * (double)sz);
  b1y += -(incz_y * (double)sz);
  b1y += (double)ycenter;

  b1z = (double)0.5;
  b1z += -(incx_z * (double)sz);
  b1z += -(incy_z * (double)sz);
  b1z += -(incz_z * (double)sz);
  b1z += (double)zcenter;

  unsigned long point;
  mfmt curline;

  for(ctr=0; ctr<(int)ret.ysize; ctr++)  {
    if(depth == 8) memset(ret.image[ctr].uc, tcolor, ret.xsize);
    else if(depth == 32) memset(ret.image[ctr].uc, tcolor, ret.xsize<<2);
    }
  for(ctr=0; ctr<(int)ret.ysize; ctr++)  {
    curline.v = ret.image[ctr].v;
    b2x = b1x;  b2y = b1y;  b2z = b1z;
    for(ctr2=0; ctr2<(int)ret.xsize; ctr2++)  {
      cx = b2x;  cy = b2y;  cz = b2z;  point=tcolor;
      int ctr3 = -(int)(sz/sc);
      for(; point==tcolor && ctr3<=(sz/sc); ctr3++)  {
	if(cz>=0 && cz<zsize && cy>=0 && cy<ysize && cx>=0 && cx<xsize)  {
	  if(depth == 8) {
	    point = image3d[(int)cz][(int)cy].uc[(int)cx];
	    if(point != tcolor) curline.uc[ctr2] = point;
	    }
	  else if(depth == 32) {
	    point = image3d[(int)cz][(int)cy].ul[(int)cx];
	    if(point != tcolor) curline.ul[ctr2] = point;
	    }
	  else Exit(-1, "Unknown depth error!\n");
	  }
	cx += (incz_x*sc);  cy += (incz_y*sc);  cz += (incz_z*sc);
	}
      b2x += incx_x;  b2y += incx_y;  b2z += incx_z;
      }
    b1x += incy_x;  b1y += incy_y;  b1z += incy_z;
    }
  ret.Trim();

/*
  xa &= 255;	ya &= 255;	za &= 255;
  int fy = (xa >= 64 && xa < 192);
  if(fy)  { xa+=128; xa&=255; }
  int fx = (xa >= 64 && xa < 192);
  if(fx)  { xa+=128; xa&=255; }
  Graphic tmp1, tmp2, tmp3;
  double tmpa;
  tmpa = xa;
  tmpa *= M_PI;
  tmpa /= 128;
  ret = Scaled(xsize, 1+(unsigned)((double)cos(tmpa)*(double)(ysize-1)));
  if(fy)  ret.YFlip();
  if(fx)  ret.XFlip();
  ret = ret.Rotated(za);
*/
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
      else Exit(-1, "Unknown depth Error!\n");
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
      else Exit(-1, "Unknown depth Error!\n");
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
  tcolor=0; if(depth==8 && tcolor==c) ++tcolor;
  depth=d; DefSize(abs(xs)+1,abs(ys)+1);
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
      else Exit(-1, "Unknown depth error!\n");
      }
    else {
      if(depth==8)
	for(ctrx=0; ctrx<xsize; ++ctrx)
	  image[(ysize*ctrx)/xsize].uc[ctrx] = c;
      else if(depth==32)
	for(ctrx=0; ctrx<xsize; ++ctrx)
	  image[(ysize*ctrx)/xsize].ul[ctrx] = c;
      else Exit(-1, "Unknown depth error!\n");
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
      else Exit(-1, "Unknown depth error!\n");
      }
    else {
      if(depth==8)
	for(ctry=0; ctry<ysize; ++ctry)
	  image[ctry].uc[(xsize*ctry)/ysize] = c;
      else if(depth==32)
	for(ctry=0; ctry<ysize; ++ctry)
	  image[ctry].ul[(xsize*ctry)/ysize] = c;
      else Exit(-1, "Unknown depth error!\n");
      }
    }
  if(xs<0) xcenter=xsize-1;
  else xcenter=0;
  if(ys<0) ycenter=ysize-1;
  else ycenter=0;
  }

void Graphic::SetBox(int xs, int ys, int d, unsigned long c) {
  if(xs<1 || ys<1) return;
  tcolor=0; if(depth==8 && tcolor==c) ++tcolor;
  depth=d; DefSize(xs,ys);
  int ctry, ctrx;

  for(ctry=0; ctry<ysize; ++ctry)
    memset(image[ctry].uc, tcolor, xsize*(depth>>2));

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
  else Exit(-1, "Unknown depth error!\n");
  }

void Graphic::SetFillBox(int xs, int ys, int d, unsigned long c) {
  if(xs<1 || ys<1) return;
  tcolor=0; if(depth==8 && tcolor==c) ++tcolor;
  depth=d; DefSize(xs,ys);
  int ctry;
  if(depth==8) {
    for(ctry=0; ctry<ysize; ++ctry) {
      memset(image[ctry].uc, c, xsize);
      }
    }
  else if(depth==32) {
    for(ctry=0; ctry<ysize; ++ctry) {
      for(int ctrx=0; ctrx<xsize; ++ctrx) image[ctry].ul[ctrx] = c;
      }
    }
  else Exit(-1, "Unknown depth error!\n");
  }

void Graphic::SetRotated(Graphic &in, int angle) {
  tcolor = in.tcolor;
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
	else Exit(-1, "Unknow depth error!\n");
	}
      else  {
	if(depth == 8) *(curpt.uc) = tcolor;
	else if(depth == 32) *(curpt.ul) = tcolor;
	else Exit(-1, "Unknow depth error!\n");
	}
      if(depth == 8) curpt.uc++;
      else if(depth == 32) curpt.ul++;
      else Exit(-1, "Unknow depth error!\n");
      curx+=incxx;
      cury+=incyx;
      }
    basex+=incxy;
    basey+=incyy;
    }
  }

Graphic::Graphic(const Graphic &from) {
//  Debug("Graphic:Copy Constructor   Begin");
  int ctr, ctr2;
  xdef = 0;  ydef = 0;  zdef = 0;
//  DefSize(from.xsize, from.ysize, from.zsize);
  depth = from.depth;
  DefSize(from.xsize, from.ysize);
  for(ctr=0; ctr<(int)from.zsize; ctr++)  {
    for(ctr2=0; ctr2<(int)from.ysize; ctr2++)  {
      memcpy(image3d[ctr][ctr2].v, from.image3d[ctr][ctr2].v,
		from.xsize*depth>>3);
      }
    }
  xcenter = from.xcenter;
  ycenter = from.ycenter;
  zcenter = from.zcenter;
  tcolor = from.tcolor;
//  Debug("Graphic:Copy Constructor   End");
  }

void Graphic::operator =(const Graphic &from) {
  int ctr, ctr2;
  mfmt *cur;
  cur = image;
//  if(image != NULL)  {
//    for(ctr=0; ctr<(long)ysize; ctr++)  {
//      if(*cur != NULL)  delete(*cur);
//      cur++;
//      }
//    delete(image);
//    }

//  DefSize(from.xsize, from.ysize, from.zsize);
  depth = from.depth;
  DefSize(from.xsize, from.ysize);
  for(ctr=0; ctr<(int)from.zsize; ctr++)  {
    for(ctr2=0; ctr2<(int)from.ysize; ctr2++)  {
      memcpy(image3d[ctr][ctr2].v, from.image3d[ctr][ctr2].v,
		from.xsize*depth>>3);
      }
    }
  xcenter = from.xcenter;
  ycenter = from.ycenter;
  zcenter = from.zcenter;
  tcolor = from.tcolor;
  }

Graphic Graphic::operator +(const Graphic &from)  {
  Graphic ret;
  unsigned char tcol;
  tcol = 0;
  int xc=xcenter, xe=xsize-xcenter, yc=ycenter, ye=ysize-ycenter;
  if(from.xcenter > xc)  xc = from.xcenter;
  if(from.ycenter > yc)  yc = from.ycenter;
  if(((long)from.xsize-from.xcenter) > xe)  xe = from.xsize-from.xcenter;
  if(((long)from.ysize-from.ycenter) > ye)  ye = from.ysize-from.ycenter;
  ret.DefSize(xc+xe, yc+ye);
  ret.xcenter = xc;
  ret.ycenter = yc;
  int ctrx, ctry;
  for(ctry=-yc; ctry<ye; ctry++)  {
    for(ctrx=-xc; ctrx<xe; ctrx++)  {
      if(depth == 8) {
	if((ctrx+from.xcenter >= 0) && (ctry+from.ycenter >= 0) && 
		(ctrx+from.xcenter < (long)from.xsize) && 
		(ctry+from.ycenter < (long)from.ysize) &&
		(from.image[ctry+from.ycenter].uc[ctrx+from.xcenter] != tcol))
	  ret.image[ctry+yc].uc[ctrx+xc] =
		from.image[ctry+from.ycenter].uc[ctrx+from.xcenter];
	else if((ctrx+xcenter >= 0) && (ctry+ycenter >= 0) && 
		(ctrx+xcenter < (long)xsize) && (ctry+ycenter < (long)ysize) &&
		(image[ctry+ycenter].uc[ctrx+xcenter] != tcol))
	  ret.image[ctry+yc].uc[ctrx+xc] = image[ctry+ycenter].uc[ctrx+xcenter];
	else ret.image[ctry+yc].uc[ctrx+xc] = tcol;
	}
      else if(depth == 32) {
	if((ctrx+from.xcenter >= 0) && (ctry+from.ycenter >= 0) && 
		(ctrx+from.xcenter < (long)from.xsize) && 
		(ctry+from.ycenter < (long)from.ysize) &&
		(from.image[ctry+from.ycenter].ul[ctrx+from.xcenter] != tcol))
	  ret.image[ctry+yc].ul[ctrx+xc] =
		from.image[ctry+from.ycenter].ul[ctrx+from.xcenter];
	else if((ctrx+xcenter >= 0) && (ctry+ycenter >= 0) && 
		(ctrx+xcenter < (long)xsize) && (ctry+ycenter < (long)ysize) &&
		(image[ctry+ycenter].ul[ctrx+xcenter] != tcol))
	  ret.image[ctry+yc].ul[ctrx+xc] = image[ctry+ycenter].ul[ctrx+xcenter];
	else ret.image[ctry+yc].ul[ctrx+xc] = 0;
	}
      else Exit(-1, "Unknown depth error!\n");
      }
    }
  return ret;
  }

void Graphic::XFlip() {  //** 8-bit only
  int ctr, ctr2;
  mfmt tmp;
  for(ctr = 0; ctr < (long)ysize; ctr++)  {
    tmp.v = image[ctr].v;
    image[ctr].uc = new unsigned char[xsize];
    for(ctr2 = 0; ctr2 < (long)xsize; ctr2++)  {
      image[ctr].uc[ctr2] = tmp.uc[(xsize-1) - ctr2];
      }
    delete tmp.v;
    }
  xcenter = (xsize-1)-xcenter;
  }

void Graphic::YFlip() {
  int ctr;
  mfmt tmp;
  for(ctr = 0; ctr < ((long)ysize/2); ctr++)  {
    tmp.v = image[ctr].v;
    image[ctr].v = image[(ysize-1) - ctr].v;
    image[(ysize-1) - ctr].v = tmp.v;
    }
  ycenter = (ysize-1)-ycenter;
  }

void Graphic::Trim() {
  Debug("Graphic:Trim()  Begin");
  int yb = ysize, ye = 0, xb = xsize, xe = 0, ctrx, ctry, ctr;
  int yclear = 1;
  if((ysize == 0) || (xsize == 0) || zsize == 0)  {
    Debug("Graphic:Trim()  Pre-Delete 1");
    if(image != NULL)  {
      for(ctr=0; ctr<(long)ysize; ctr++)
	if(image[ctr].v != NULL)
	  delete(image[ctr].v);
      delete(image);
      }
    image3d = new mfmt*[1];
    image3d[0] = new mfmt[1];
    image3d[0][0].ul = new unsigned long;
    image3d[0][0].ul[0] = tcolor;
    image = image3d[0];
    xsize = 1;  ysize = 1;  zsize = 1;
    xdef = 1;  ydef = 1;  zdef = 1;
    xcenter = 0;  ycenter = 0;  zcenter = 0;
    Debug("Graphic:Trim()  Post-Delete 1 -- End");
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
    Debug("Graphic:Trim()  Do nothing -- End");
    return;
    }
  if((yb > ye) || (xb > xe))  {
    Debug("Graphic:Trim()  Pre-Delete 2");
    if(image != NULL)  {
      for(ctr=0; ctr<(long)ysize; ctr++)  if(image[ctr].v != NULL)
	delete(image[ctr].v);
      delete(image);
      }
    image3d = new mfmt*[1];
    image3d[0] = new mfmt[1];
    image3d[0][0].ul = new unsigned long;
    image3d[0][0].ul[0] = tcolor;
    image = image3d[0];
    xsize = 1;  ysize = 1;  zsize = 1;
    xdef = 1;  ydef = 1;  zdef = 1;
    xcenter = 0;  ycenter = 0;  zcenter = 0;
    Debug("Graphic:Trim()  Post-Delete 2 -- End");
    return;
    }
  Debug("Graphic:Trim()  Begin Alloc");
  mfmt *tmpimage = image;
  int tmpysz = ysize;
  int tmpxcn = xcenter - xb;
  int tmpycn = ycenter - yb;
  xdef = 0;  ydef = 0; zdef = 0;
  DefSize((xe-xb)+1, (ye-yb)+1);
  Debug("Graphic:Trim()  Begin Trim");
  for(ctry = yb; ctry < (ye+1); ctry++)  {
    if(depth == 8)
	memcpy(image[ctry-yb].uc, &tmpimage[ctry].uc[xb], xsize);
    else if(depth == 32)
	memcpy(image[ctry-yb].ul, &tmpimage[ctry].ul[xb], xsize*4);
    else Exit(-1, "Unknown depth error!\n");
    }
  Debug("Graphic:Trim()  Begin Cleanup");
  for(ctr=0; ctr<tmpysz; ctr++)  delete(tmpimage[ctr].v);
  free(tmpimage);
  xcenter = tmpxcn;
  ycenter = tmpycn;
  Debug("Graphic:Trim()  End");
  }

void Graphic::FindTrueCenter() {
  xcenter = xsize / 2;
  ycenter = ysize / 2;
  zcenter = zsize / 2;
  }

void Graphic::SetCenter(int xcr, int ycr) {
  xcenter = xcr;
  ycenter = ycr;
  }

void Graphic::DefSize(int xsz, int ysz) {
//  Debug("User::Graphic::DefSize Begin");
  int ctr, ctr2;
  xcenter = 0;		ycenter = 0;	linedef = 0;
  ysize = ysz;          xsize = xsz;	zsize = 1;
  if(xsz <= (int)xdef && ysz <= (int)ydef && 1 == zdef)  return;
  if(zdef > 0)  {
    if(ydef > 0)  {
      for(ctr=0; ctr<(int)zdef; ctr++)  {
	if(xdef > 0)  {
	  for(ctr2=0; ctr2<(int)ydef; ctr2++)  {
	    delete image3d[ctr][ctr2].v;
	    }
	  }
        delete image3d[ctr];
	}
      }
    delete image3d;
    }
//  Debug("User::Graphic::DefSize Middle");
  ysize = (ysz>ydef)?ysz:ydef; xsize = (xsz>xdef)?xsz:xdef;
  image = new mfmt[ysize];
  for(ctr=0; ctr<(long)ysize; ctr++)
    image[ctr].uc = new unsigned char[xsize*(depth>>3)];
  image3d = new mfmt*[1];
  image3d[0] = image;
  ydef = ysize;  xdef = xsize;  zdef = 1;
//  Debug("User::Graphic::DefSize End");
  }

void Graphic::DefSize(int xsz, int ysz, int zsz) {
  int ctr, ctr2;
  xcenter = 0;		ycenter = 0;	linedef = 0;
  ysize = ysz;          xsize = xsz;	zsize = zsz;
  if(xsz <= (int)xdef && ysz <= (int)ydef && zsz <= (int)zdef)  return;
  if(zdef > 0)  {
    if(ydef > 0)  {
      for(ctr=0; ctr<(int)zdef; ctr++)  {
	if(xdef > 0)  {
	  for(ctr2=0; ctr2<(int)ydef; ctr2++)  {
	    delete image3d[ctr][ctr2].v;
	    }
	  }
        delete image3d[ctr];
	}
      }
    delete image3d;
    }

  ysize = (ysz>ydef)?ysz:ydef;
  xsize = (xsz>xdef)?xsz:xdef;
  zsize = (zsz>zdef)?zsz:zdef;
  image3d = new mfmt*[zsize];
  for(ctr=0; ctr<(long)zsize; ctr++)  {
    image3d[ctr] = new mfmt[ysize];
    for(ctr2=0; ctr2<(long)ysize; ctr2++)
	image3d[ctr][ctr2].uc = new unsigned char[xsize*(depth>>3)];
    }
  image = image3d[0];
  ydef = ysize;  xdef = xsize;  zdef = zsize;
  }

Graphic::~Graphic() {
  int ctr;
  if(image != NULL)  {
    for(ctr=0; ctr<((ysize>ydef)?(long)ysize:(long)ydef); ctr++)
      if(image[ctr].v != NULL)  delete(image[ctr].v);
    delete(image);
    }
  }

void Graphic::DefLin(char *data)  {
  if(ysize == 0 || xsize == 0 || zsize == 0)  return;
  int plane = linedef/ysize, line = linedef%ysize;
  plane %= zsize;
  memcpy(image3d[plane][line].uc, data, xsize*(depth>>3));
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
  char buffer[16];
  FILE *bmp = fopen(fn, "rb");
  if(bmp == NULL)  {
    Exit(1, "\"%s\" Not Found!\n", fn);
    }
  read(fileno(bmp), buffer, 16);
  if((buffer[0] != 'B') || (buffer[1] != 'M'))  {
    Exit(1, "\"%s\" is Not A Bitmap file!\n", fn);
    }
  read(fileno(bmp), buffer, 16);
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
    Exit(1, "I only support 8 and 24 bit Bitmap files, \"%s\" is %d-bit!\n",
	fn, depth);
    }
  tcolor = image[0].uc[0]; //** 8-bit only
  }

Graphic::Graphic(char *fn)  {
  Init(fn);
  }

void Graphic::InitTGA32(char *fn)  {
  int ctry;
  unsigned char buf[256];
  FILE *tga=fopen(fn, "rb");
  if(tga != NULL) {
    int nr=read(fileno(tga), buf, 18);
    if(nr == 18 && buf[1] == 0 && buf[2] == 2) {
      depth = 32; tcolor=0; int rev=(buf[17]&0x20);
      if(buf[17]&0x10) Exit(1, "%s is a backward TGA file!\n", fn);
      if(buf[16] != 32)
	Exit(1, "Depth of \"%s\" is %d, not 32!\n", fn, buf[16]);
      DefSize((buf[13]<<8) + buf[12], (buf[15]<<8) + buf[14]);
      read(fileno(tga), buf, (int)buf[0]);
      if(!rev) for(ctry=0; ctry<ysize; ++ctry) {
	read(fileno(tga), image[ysize-(ctry+1)].ul, xsize*4);
	}
      else for(ctry=0; ctry<ysize; ++ctry) {
	read(fileno(tga), image[ctry].ul, xsize*4);
	}
      }
    else if(nr == 18 && buf[1] == 0 && buf[2] == 10) {
      depth = 32; tcolor=0; int rev=(buf[17]&0x20), ctr, x, y, yr, sz;
      if(buf[17]&0x10) Exit(1, "%s is a backward TGA file!\n", fn);
      if(buf[16] != 32)
	Exit(1, "Depth of \"%s\" is %d, not 32!\n", fn, buf[16]);
      DefSize((buf[13]<<8) + buf[12], (buf[15]<<8) + buf[14]);
      read(fileno(tga), buf, (int)buf[0]);

      x=0; y=0; yr=0; if(!rev) yr=ysize-1;
      while(y<ysize) {
	unsigned long tmpv;
	read(fileno(tga), buf, 1);
	sz=(buf[0]&0x7F)+1;
	if(buf[0]&0x80) {
	  printf("1, %d\n", sz);
	  read(fileno(tga), &tmpv, 4);
	  for(ctr=0; ctr<sz; ++ctr) {
	    image[y].ul[x++] = tmpv;
	    if(x==xsize) {
	      fprintf(stderr, "\"%s\" breaks TGA rules!!!\n", fn);
	      ++y; --yr; x=0;
	      }
	    }
	  }
	else {
	  printf("2, %d\n", sz);
	  while(x+sz>=xsize) {
	    int sz2 = xsize-x; sz-=sz2;
	    fprintf(stderr, "\"%s\" breaks TGA rules!!!\n", fn);
	    read(fileno(tga), &image[y].ul[x], sz2<<2);
	    ++y; --yr; x=0;
	    }
	  read(fileno(tga), &image[y].ul[x], sz<<2);
	  x+=sz;
	  }
	if(x==xsize) { ++y; --yr; x=0; }
	else if(x>xsize)
	  Exit(1, "RLE Overrun (%d>%d) in \"%s\"\n", x, xsize, fn);
	}
      }
    else {
      Exit(1, "\"%s\" isn't a 32-bit TGA file!\n", fn);
      }
    fclose(tga);
    }
  else {
    Exit(1, "Can't open \"%s\"!\n", fn);
    }
  }

void Graphic::Init(char *fn)  {
 Debug("Graphic::Init Begin");
 xdef = 0;  ydef = 0;  zdef = 0;
 if(!strcasecmp(".tga", &fn[strlen(fn)-4])) { InitTGA32(fn); return; }
 int bmp, colused;
 long size2, width, height, off = 0;
 unsigned char buffer[1280];
 long detect;
 {
#ifdef O_BINARY
  bmp = open(fn, O_RDONLY|O_BINARY);
#else
  bmp = open(fn, O_RDONLY);
#endif

  Debug("Graphic::Init First Read");
  read(bmp, buffer, 16);
  if((buffer[0] != 'B') || (buffer[1] != 'M'))  {
    Debug("Graphic::Init Using First Exit!");
    Exit(1, "\"%s\" Not Found, or Not A Bitmap file!\n", fn);
    }
  Debug("Graphic::Init After First Exit");
  size2 = buffer[14]+256*(buffer[15]);
  read(bmp, buffer, (size2 - 2));
  width = buffer[2]+256*(buffer[3]);
  height = buffer[6]+256*(buffer[7]);
  detect = buffer[10]+256*(buffer[11]);
  Debug("Graphic::Init Plane Read");
  if(detect != 1)  {
    Exit(1, "I only suport 1 plane Bitmap files, \"%s\" is %d-plane!\n",
        fn, detect);
    }
  depth = buffer[12]+256*(buffer[13]);
  Debug("Graphic::Init Depth Read");
  if(depth != 8 && depth != 24)  {
    Exit(1, "\"%s\" is %d-bit, Only 8 and 24-bit BMPs supported w/o palette!\n",
	fn, depth);
    }
  detect = buffer[14]+256*(buffer[15]);
  Debug("Graphic::Init Compression Read");
  if(detect != 0)  {
    Exit(1, "I do not support compressed Bitmap files, \"%s\" is compressed!\n",
        fn);
    }
  int bytes = (depth>>3);
  if((bytes*width) %4 != 0)  off = 4 - ((bytes*width) % 4);
  if(depth == 8)  {
    int ctr;
    colused = buffer[30]+256*(buffer[31]);
    if(colused == 0)  colused = 256;
    read(bmp, buffer, colused*4);
    DefSize(width, height);
    for(ctr = height; ctr > 0; ctr--)  {
      int tmp = read(bmp, image[ctr-1].uc, width*bytes);
      if(tmp != ((long)(width*bytes)))  {
	Exit(1, "Read error in 8-bit file \"%s\"\n", fn);
	}
      read(bmp, buffer, off);
      }
    }
  else if(depth == 24)  {
    int ctr, ctr2;
    depth = 32;
    DefSize(width, height);
    for(ctr = height; ctr > 0; --ctr)  {
      for(ctr2 = 0; ctr2 <width; ++ctr2)  {
	int tmp = read(bmp, &image[ctr-1].ul[ctr2], 3);
	if(tmp != 3)  {
	  Exit(1, "Read error in 24-bit file \"%s\"\n", fn);
	  }
	}
      read(bmp, buffer, off);
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
  else Exit(-1, "Unknown depth error!\n");
  Debug("Graphic::Init Close File");
  close(bmp);
  }
 Debug("Graphic::Init End");
 }

#define fpint(f, i) fprintf(f, "%c%c%c%c", (char)(i&255), (char)((i>>8)&255), (char)((i>>16)&255), (char)((i>>24)&255))

void Graphic::SaveBMP(char *fn, const Palette &pal) {
 Debug("User::Graphic::SaveBMP(2) Begin");
 FILE *bmp;
 int ctr, ctr2;
 {
  bmp = fopen(fn, "wb");

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
 Debug("User::Graphic::SaveBMP(2) End");
 }

void Graphic::SaveBMP(char *fn)  {
 Debug("User::Graphic::SaveBMP(1) Begin");
 if(depth != 32) Exit(1, "Depth = %d and no Palette given!\n", depth);
 FILE *bmp;
 int ctr, ctr2;
 {
  bmp = fopen(fn, "wb");

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
 Debug("User::Graphic::SaveBMP(1) End");
 }

void Graphic::Init24(char *fn, Palette &p)  {
  Debug("User::Graphic::Init24 Begin");
  xdef = 0;  ydef = 0;  zdef = 0;
  FILE *bmp;
  long size2, width, height;
  unsigned char buffer[4096];
  long detect;
  int ctr, ctr2;

  bmp = fopen(fn, "rb");

  read(fileno(bmp), buffer, 16);
  if((buffer[0] != 'B') || (buffer[1] != 'M'))
	Exit(1, "\"%s\" Not Found, or Not A Bitmap file!\n", fn);
  size2 = buffer[14]+256*(buffer[15]);

  read(fileno(bmp), buffer, (size2 - 2));
  width = buffer[2]+256*(buffer[3]);
  height = buffer[6]+256*(buffer[7]);
  detect = buffer[10]+256*(buffer[11]);
  if(detect != 1)  {
    Exit(1, "I only suport 1 plane Bitmap files, \"%s\" is %d-plane!\n",
        fn, detect);
    }
  depth = buffer[12]+256*(buffer[13]);
  if(depth != 24)  {
    Exit(1, "\"%s\" is a %d-bit file that detected as 24-bit!?!\n", depth);
    }
  detect = buffer[14]+256*(buffer[15]);
  if(detect != 0)  {
    Exit(1, "I do not support compressed Bitmap files, \"%s\" is compressed!\n",
        fn);
    }
  DefSize(width, height);

  int off = (4-((width*3) & 3)) & 3;
  for(ctr = height; ctr > 0; ctr--)  {
    int tmp = read(fileno(bmp), buffer, width*3+off);
    if(tmp != (long)width*3+off)  {
      Exit(1, "Read error in 24-bit file loading \"%s\"\n", fn);
      }
    for(ctr2 = 0; ctr2 < (int)width; ctr2++)  { //** 8-bit (maybe not an error)
      image[ctr-1].uc[ctr2] = p.GetClosestColor(buffer[ctr2*3+2],
		buffer[ctr2*3+1], buffer[ctr2*3]);
      }
    }

  linedef = height;
  if(depth == 8) tcolor = image[0].uc[0]; //** Same tcolor 8
  else if(depth == 32) tcolor = image[0].uc[3]; //** 32-bit kludge
  else Exit(-1, "Unknown depth error!\n");
  
  fclose(bmp);
  Debug("User::Graphic::Init24 End");
  }

void Graphic::PasteGraphic(Graphic &g, int x, int y) {
  PasteGraphic(&g, x, y);
  }

void Graphic::PasteGraphic(Graphic *g, int x, int y) {
  if(g==NULL) return;
  if(g->depth != depth)
    Exit(1, "Depth mismatch in %s\n", __PRETTY_FUNCTION__);
  int ctry;
  if(x<0 || y<0 || x+g->xsize > xsize || y+g->ysize > ysize)
    Exit(1, "Out of bounds error in %s\n", __PRETTY_FUNCTION__);
  if(depth == 8) {
    for(ctry=0; ctry<g->ysize; ++ctry) {
      memcpy(image[ctry+y].ul+x, g->image[ctry].ul, g->xsize);
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
  }

void Graphic::DepthConvert(int d, const Palette &p) {
  Debug("User::Graphic::DepthConvert Begin");
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
          image[ctry].uc[(ctrx<<2)+2] = 0;
          image[ctry].uc[(ctrx<<2)+1] = 0;
          image[ctry].uc[(ctrx<<2)] = 0;
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
  else Exit(-1, "Unimplemented Graphic depth convert from %d to %d\n", depth, d);
  Debug("User::Graphic::DepthConvert End");
  }
