// *************************************************************************
// graphic.cpp
// Advanced Graphic class, Pre ALPHA non-distribution version
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

#include        <user/engine.h>
#include        <stdlib.h>
#include        <sys/types.h>
#include        <sys/stat.h>
#include        <fcntl.h>
#include        <math.h>
#include        <string.h>
#include	<stdarg.h>
#include	<unistd.h>

#ifdef DOS
#include	<io.h>
#endif

//#define		M_PI 3.1415926535

#include        "graphic.h"

#define cd2rad(a) ((double)((((double)(a))*((double)M_PI))/(double)(32768)))

Graphic::Graphic() {
  image = NULL;  image3d = NULL;
  xdef = 0;  ydef = 0;  zdef = 0;
  }

Graphic::Graphic(int xsz, int ysz) {
  xdef = 0;  ydef = 0;  zdef = 0;
  DefSize(xsz, ysz);
  }

Graphic::Graphic(int xsz, int ysz, int zsz) {
  xdef = 0;  ydef = 0;  zdef = 0;
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
  for(ctr=0; ctr<(int)ysize; ctr++)  {
    for(ctr2=0; ctr2<(int)xsize; ctr2++)  {
      image[ctr][ctr2] = remap[image[ctr][ctr2]];
      }
    }
  }

Graphic Graphic::Hashed()  {
  Graphic ret;
  int X, Y;
  ret = *this;
  for(Y=0; Y<(long)ysize; Y++)  {
    for(X=(Y&1); X<(long)xsize; X+=2)  {
      ret.image[Y][X] = 0;
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
      ret.image[Y][X] = 0;
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
    ret.DefLin((char *)&image[ctr][x1]);
    }
  return ret;
  }

Graphic Graphic::Scaled(double scale)  {
  Graphic ret;
  ret = Scaled((int)((double)scale*(double)xsize+(double)0.5),
	(int)((double)scale*(double)ysize+(double)0.5));
  return ret;
  }

Graphic Graphic::Scaled(unsigned xsz, unsigned ysz)  {
  Graphic ret;
  int xi, yi;
  double ox, oy;
  ret.DefSize(xsz, ysz);
  for(yi=0; yi<(long)ret.ysize; yi++)  {
//    ret.image[yi] = new unsigned char[ret.xsize];
    for(xi=0; xi<(long)ret.xsize; xi++)  {
      ox = (xi * xsize);
      ox /= ret.xsize;
      oy = (yi * ysize);
      oy /= ret.ysize;
      ret.image[yi][xi] = image[(int)oy][(int)ox];
      }
    }
  ret.xcenter = (xcenter * ret.xsize) / xsize;
  ret.ycenter = (ycenter * ret.ysize) / ysize;
  return ret;
  }

Graphic Graphic::Rotated(double sc, int xa, int ya, int za)  {
  Graphic ret;
  int sz, xsz, ysz;
  sz = (int)(sc*zsize*(1.75)+1.5);
  xsz = (int)(sc*xsize*(1.75)+1.5);
  ysz = (int)(sc*ysize*(1.75)+1.5);
  if(ysz > sz) sz = ysz;
  if(xsz > sz) sz = xsz;
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

  unsigned char point, *curline;

  for(ctr=0; ctr<(int)ret.ysize; ctr++)  {
//    ret.image[ctr] = new unsigned char[ret.xsize];
    memset(ret.image[ctr], 0, ret.xsize);
    }
  for(ctr=0; ctr<(int)ret.ysize; ctr++)  {
    curline = ret.image[ctr];
    b2x = b1x;  b2y = b1y;  b2z = b1z;
    for(ctr2=0; ctr2<(int)ret.xsize; ctr2++)  {
      cx = b2x;  cy = b2y;  cz = b2z;  point=0;
      int ctr3 = -(int)(sz/sc);
      for(; point==0 && ctr3<=(sz/sc); ctr3++)  {
	if(cz>=0 && cz<zsize && cy>=0 && cy<ysize && cx>=0 && cx<xsize)  {
	  point = image3d[(int)cz][(int)cy][(int)cx];
	  if(point != 0) curline[ctr2] = point;
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

Graphic Graphic::Rotated(int angle)  {
  Graphic ret;
  ret.SetRotated(*this, angle);
  return ret;
  }

void Graphic::SetRotated(Graphic &in, int angle) {
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
  unsigned char *curpt;
  for(yi=0; yi<(long)ysize; yi++)  {
    curpt = image[yi];
    curx = basex;  cury = basey;
    for(xi=0; xi<(long)xsize; xi++)  {
      dx=(int)curx;	dy=(int)cury;
      if((dx>=0) && (dy>=0) && (dx < (long)in.xsize) && (dy < (long)in.ysize)) {
	*curpt = in.image[dy][dx];
	}
      else  {
	*curpt = 0;
	}
      curpt++;
      curx+=incxx;
      cury+=incyx;
      }
    basex+=incxy;
    basey+=incyy;
    }
  }

Graphic::Graphic(const Graphic &from) {
  int ctr, ctr2;
  xdef = 0;  ydef = 0;  zdef = 0;
//  DefSize(from.xsize, from.ysize, from.zsize);
  DefSize(from.xsize, from.ysize);
  for(ctr=0; ctr<(int)from.zsize; ctr++)  {
    for(ctr2=0; ctr2<(int)from.ysize; ctr2++)  {
      memcpy(image3d[ctr][ctr2], from.image3d[ctr][ctr2], from.xsize);
//      DefLin((char*)from.image3d[ctr][ctr2]);
      }
    }

//  unsigned char **cur, **curf;
//  cur = image;	curf = from.image;
//  for(ctr=0; ctr<(long)from.ysize; ctr++)  {
//    *cur = new unsigned char[xsize];
//    memcpy(*cur, *curf, xsize);
//    cur++;	curf++;
//    }
  xcenter = from.xcenter;
  ycenter = from.ycenter;
  zcenter = from.zcenter;
  }

void Graphic::operator =(const Graphic &from) {
  int ctr, ctr2;
  unsigned char **cur;
  cur = image;
//  if(image != NULL)  {
//    for(ctr=0; ctr<(long)ysize; ctr++)  {
//      if(*cur != NULL)  delete(*cur);
//      cur++;
//      }
//    delete(image);
//    }

//  DefSize(from.xsize, from.ysize, from.zsize);
  DefSize(from.xsize, from.ysize);
  for(ctr=0; ctr<(int)from.zsize; ctr++)  {
    for(ctr2=0; ctr2<(int)from.ysize; ctr2++)  {
      memcpy(image3d[ctr][ctr2], from.image3d[ctr][ctr2], from.xsize);
//      DefLin((char*)from.image3d[ctr][ctr2]);
      }
    }

//  cur = image;	curf = from.image;
//  for(ctr=0; ctr<(long)from.ysize; ctr++)  {
//    *cur = new unsigned char[xsize];
//    memcpy(*cur, *curf, xsize);
//    cur++;	curf++;
//    }
  xcenter = from.xcenter;
  ycenter = from.ycenter;
  zcenter = from.zcenter;
  }

Graphic Graphic::operator +(const Graphic &from)  {
  Graphic ret;
  unsigned char tcol;
//  if(__Da_Screen != NULL)  tcol = __Da_Screen->deftran;
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
//    ret.image[ctry+yc] = new unsigned char[xc+xe];
    for(ctrx=-xc; ctrx<xe; ctrx++)  {
      if((ctrx+from.xcenter >= 0) && (ctry+from.ycenter >= 0) && 
		(ctrx+from.xcenter < (long)from.xsize) && 
		(ctry+from.ycenter < (long)from.ysize) &&
		(from.image[ctry+from.ycenter][ctrx+from.xcenter] != tcol))
	ret.image[ctry+yc][ctrx+xc] =
		from.image[ctry+from.ycenter][ctrx+from.xcenter];
      else if((ctrx+xcenter >= 0) && (ctry+ycenter >= 0) && 
		(ctrx+xcenter < (long)xsize) && (ctry+ycenter < (long)ysize) &&
		(image[ctry+ycenter][ctrx+xcenter] != tcol))
	ret.image[ctry+yc][ctrx+xc] = image[ctry+ycenter][ctrx+xcenter];
      else ret.image[ctry+yc][ctrx+xc] = 0;
      }
    }
  return ret;
  }

void Graphic::XFlip() {
  int ctr, ctr2;
  unsigned char *tmp;
  for(ctr = 0; ctr < (long)ysize; ctr++)  {
    tmp = image[ctr];
    image[ctr] = new unsigned char[xsize];
    for(ctr2 = 0; ctr2 < (long)xsize; ctr2++)  {
      image[ctr][ctr2] = tmp[(xsize-1) - ctr2];
      }
    delete tmp;
    }
  xcenter = (xsize-1)-xcenter;
  }

void Graphic::YFlip() {
  int ctr;
  unsigned char *tmp;
  for(ctr = 0; ctr < ((long)ysize/2); ctr++)  {
    tmp = image[ctr];
    image[ctr] = image[(ysize-1) - ctr];
    image[(ysize-1) - ctr] = tmp;
    }
  ycenter = (ysize-1)-ycenter;
  }

void Graphic::Trim() {
  int yb = ysize, ye = 0, xb = xsize, xe = 0, ctrx, ctry, ctr;
  int yclear = 1;
  if((ysize == 0) || (xsize == 0) || zsize == 0)  {
    if(image != NULL)  {
      for(ctr=0; ctr<(long)ysize; ctr++)  if(image[ctr] != NULL)  free(image[ctr]);
      free(image);
      }
    image3d = new unsigned char**[1];
    image3d[0] = new unsigned char*[1];
    image3d[0][0] = new unsigned char[1];
    image3d[0][0][0] = 0;
    image = image3d[0];
    xsize = 1;  ysize = 1;  zsize = 1;
    xdef = 1;  ydef = 1;  zdef = 1;
    xcenter = 0;  ycenter = 0;  zcenter = 0;
    return;
    }
  for(ctry = 0; ctry < (long)ysize; ctry++)  {
    int xclear = 1;
    for(ctrx = 0; ctrx < (long)xsize; ctrx++)  {
      if(image[ctry][ctrx] != 0)  {
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
    return;
    }
  if((yb > ye) || (xb > xe))  {
    if(image != NULL)  {
      for(ctr=0; ctr<(long)ysize; ctr++)  if(image[ctr] != NULL)  free(image[ctr]);
      free(image);
      }
    image3d = new unsigned char**[1];
    image3d[0] = new unsigned char*[1];
    image3d[0][0] = new unsigned char[1];
    image3d[0][0][0] = 0;
    image = image3d[0];
    xsize = 1;  ysize = 1;  zsize = 1;
    xdef = 1;  ydef = 1;  zdef = 1;
    xcenter = 0;  ycenter = 0;  zcenter = 0;
    return;
    }
  unsigned char **tmpimage = image;
  int tmpysz = ysize;
  int tmpxcn = xcenter - xb;
  int tmpycn = ycenter - yb;
  xdef = 0;  ydef = 0; zdef = 0;
  DefSize((xe-xb)+1, (ye-yb)+1);
  for(ctry = yb; ctry < (ye+1); ctry++)  {
    DefLin((char *)&tmpimage[ctry][xb]);
    }
  for(ctr=0; ctr<tmpysz; ctr++)  free(tmpimage[ctr]);
  free(tmpimage);
  xcenter = tmpxcn;
  ycenter = tmpycn;
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
  int ctr, ctr2;
  xcenter = 0;		ycenter = 0;	linedef = 0;
  ysize = ysz;          xsize = xsz;	zsize = 1;
  if(xsz <= (int)xdef && ysz <= (int)ydef && 1 == zdef)  return;
  if(zdef > 0)  {
    if(ydef > 0)  {
      for(ctr=0; ctr<(int)zdef; ctr++)  {
	if(xdef > 0)  {
	  for(ctr2=0; ctr2<(int)ydef; ctr2++)  {
	    delete image3d[ctr][ctr2];
	    }
	  }
        delete image3d[ctr];
	}
      }
    delete image3d;
    }
  ysize = ((unsigned)ysz>ydef)?ysz:ydef;  xsize = ((unsigned)xsz>xdef)?xsz:xdef;
  image = new unsigned char*[ysize];
  for(ctr=0; ctr<(long)ysize; ctr++)
    image[ctr] = new unsigned char[xsize];
  image3d = new unsigned char**[1];
  image3d[0] = image;
  ydef = ysize;  xdef = xsize;  zdef = 1;
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
	    delete image3d[ctr][ctr2];
	    }
	  }
        delete image3d[ctr];
	}
      }
    delete image3d;
    }

  ysize = ((unsigned)ysz>ydef)?ysz:ydef;
  xsize = ((unsigned)xsz>xdef)?xsz:xdef;
  zsize = ((unsigned)zsz>zdef)?zsz:zdef;
  image3d = new unsigned char**[zsize];
  for(ctr=0; ctr<(long)zsize; ctr++)  {
    image3d[ctr] = new unsigned char*[ysize];
    for(ctr2=0; ctr2<(long)ysize; ctr2++)
	image3d[ctr][ctr2] = new unsigned char[xsize];
    }
  image = image3d[0];
  ydef = ysize;  xdef = xsize;  zdef = zsize;
  }

Graphic::~Graphic() {
  int ctr;
  if(image != NULL)  {
    for(ctr=0; ctr<(long)((ysize>ydef)?ysize:ydef); ctr++)
      if(image[ctr] != NULL)  delete(image[ctr]);
    delete(image);
    }
  }

void Graphic::DefLin(char *data)  {
  if(ysize == 0 || xsize == 0 || zsize == 0)  return;
  int plane = linedef/ysize, line = linedef%ysize;
  plane %= zsize;
//  image3d[plane][line] = new unsigned char[xsize];
  memcpy(image3d[plane][line], data, xsize);
  linedef++;
  }

void Graphic::DefLinH(char *data)  {
  int ctr;
  int tmp, tmp2;
//  image[linedef] = new unsigned char[xsize];
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
    image[linedef][ctr] = tmp;
    }
  linedef++;
  if(linedef >= (long)ysize)  linedef = 0;
  }

Graphic::Graphic(char *fn)  {
 xdef = 0;  ydef = 0;  zdef = 0;
 int bmp, colused;
 unsigned size2, width, height, off = 0;
 unsigned char buffer[1280];
 unsigned detect;
 int ctr;
 {
#ifdef O_BINARY
  bmp = open(fn, O_RDONLY|O_BINARY);
#else
  bmp = open(fn, O_RDONLY);
#endif

  read(bmp, buffer, 16);
  if((buffer[0] != 'B') || (buffer[1] != 'M'))  {
    Exit(1, "\"%s\" Not Found, or Not A Bitmap file!\n", fn);
    }
  size2 = buffer[14]+256*(buffer[15]);
  read(bmp, buffer, (size2 - 2));
  width = buffer[2]+256*(buffer[3]);
  if((width %4) != 0)  off = 4 - (width % 4);
  height = buffer[6]+256*(buffer[7]);
  detect = buffer[10]+256*(buffer[11]);
  if(detect != 1)  {
    Exit(1, "I only suport 1 plane Bitmap files, \"%s\" is %d-plane!\n",
        fn, detect);
    }
  detect = buffer[12]+256*(buffer[13]);
  if(detect != 8)  {
    Exit(1, "I only support 8 bit Bitmap files, \"%s\" is %d-bit!\n",
        fn, detect);
    }
  detect = buffer[14]+256*(buffer[15]);
  if(detect != 0)  {
    Exit(1, "I do not support compressed Bitmap files, \"%s\" is compressed!\n",
        fn);
    }
  colused = buffer[30]+256*(buffer[31]);
  if(colused == 0)  colused = 256;
  read(bmp, buffer, colused*4);
  DefSize(width, height);
//  linedef = height - 1;
//  for(ctr = height; ctr > 0; ctr--)  {
//    int tmp = read(bmp, buffer, width+off);
//    if(tmp != ((long)width+(long)off))  {
//      Exit(1, "Read error in file \"%s\"\n", fn);
//      }
//    DefLin((char *)buffer);
//    linedef -=2;
//    }
  for(ctr = height; ctr > 0; ctr--)  {
    int tmp = read(bmp, image[ctr-1], width+off);
    if(tmp != ((long)width+(long)off))  {
      Exit(1, "Read error in file \"%s\"\n", fn);
      }
    }
  linedef = height;
  close(bmp);
  }
 }

#define fpint(f, i) \
	fprintf(f, "%c%c%c%c", i&255, (i>>8)&255, (i>>16)&255, (i>>24)&255)

void Graphic::SaveBMP(char *fn, const Palette &pal)  {
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
  fpint(bmp, xsize);				// X Sizer
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
      fprintf(bmp, "%c", image[ctr][ctr2]);
    for(; (ctr2 & 3) != 0; ctr2++)
      fprintf(bmp, "%c", 0);
    }
  fclose(bmp);
//  printf("Xsize = %d, Ysize = %d\n  Size ?= (%d)\n", xsize, ysize,
//	(ysize * ((xsize+3) - ((xsize+3)&3))) + (pal.coldec*4) + 54);
  }
 }

