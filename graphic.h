// *************************************************************************
// graphic.h
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

#ifndef INSOMNIA_GRAPHIC_H
#define INSOMNIA_GRAPHIC_H

#include "list.h"
#include "palette.h"

class Graphic {
  public:
  Graphic();
  Graphic(int, int);
  Graphic(int, int, int);
  Graphic(const Graphic &);
  Graphic(char *);
  Graphic(char *, Palette &);
  void SaveBMP(char *);
  void SaveBMP(char *, const Palette &);
  ~Graphic();
  void operator =(const Graphic &from);
  void operator +=(const Graphic &from);
  void operator -=(const Graphic &from);
  Graphic operator +(const Graphic &from);
  Graphic operator -(const Graphic &from);
  Graphic HalfSize() {return Scaled(xsize>>1, ysize>>1);};
  Graphic DoubleSize() {return Scaled(xsize<<1, ysize<<1);};
  Graphic Hashed(); 
  Graphic OffHashed(); 
  void SetRotated(Graphic &, int);
  void SetScaled(Graphic &, double);
  void SetScaled(Graphic &, int, int);
  Graphic Rotated(int);
  Graphic Rotated(int, int);
  Graphic Rotated(double, int, int, int);
  Graphic Rotated(double, int, int, int, int);
  Graphic RotatedClock();
  Graphic RotatedCounterClock();
  Graphic RotatedFull();
  Graphic Scaled(double);
  Graphic Scaled(unsigned, unsigned);
  Graphic Partial(int, int, int, int);
  void PaletteConvert(const Palette &, const Palette &);
  void FindTrueCenter();
  void Trim();
  void XFlip();
  void YFlip();
  void SetCenter(int, int);
  void DefSize(int, int);
  void DefSize(int, int, int);
  void DefLin(char*);
  void DefLinH(char*);
  unsigned xsize, ysize, zsize, depth;
  int xcenter, ycenter, zcenter;
  unsigned char ** image;
  unsigned char *** image3d;
  int tcolor;

  private:
  void Init(char *);
  void Init24(char *, Palette &);
  int linedef;
  unsigned xdef, ydef, zdef;
  };

#endif
