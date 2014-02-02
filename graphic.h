// *************************************************************************
// graphic.h
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

#ifndef INSOMNIA_GRAPHIC_H
#define INSOMNIA_GRAPHIC_H

#include "mfmt.h"
#include "palette.h"
#include "chunk.h"

typedef unsigned int color;

class Graphic {
  public:
  Graphic();
  Graphic(int, int);
  Graphic(const Graphic &);
  Graphic(const char *);
  Graphic(const char *, Palette &);
  void SaveBMP(const char *);
  void SaveBMP(const char *, const Palette &);
  ~Graphic();
  void operator =(const Graphic &from);
  void operator +=(const Graphic &from);
  void operator -=(const Graphic &from);
  Graphic operator +(const Graphic &from);
  Graphic operator -(const Graphic &from);
  void PasteGraphic(Graphic &g, int x=0, int y=0);
  void PasteGraphic(Graphic *g, int x=0, int y=0);
  void PasteTransparentGraphic(Graphic &g, int x=0, int y=0);
  void PasteTransparentGraphic(Graphic *g, int x=0, int y=0);
  void SetRotated(Graphic &, int);
  void SetScaled(Graphic &, double);
  void SetScaled(Graphic &, int, int);
  void SetPixel(int, color);
  void SetLine(int, int, int, color);
  void SetRect(int, int, int, color);
  void SetFillRect(int, int, int, color);
  void DrawPixel(int, int, int, color);
  void DrawLine(int, int, int, int, int, color);
  void DrawRect(int, int, int, int, int, color);
  void DrawFillRect(int, int, int, int, int, color);
  void ClearArea(int, int, int, int);
  void Clear();
  Graphic Rotated(int);
  Graphic Rotated(int, int);
  Graphic Rotated(double, int, int, int);
  Graphic Rotated(double, int, int, int, int);
  Graphic RotatedClock();
  Graphic RotatedCounterClock();
  Graphic RotatedFull();
  Graphic Scaled(double);
  Graphic Scaled(int, int);
  Graphic Partial(int, int, int, int);
  void Undo3ds();
  void PaletteConvert(const Palette &, const Palette &);
  void DepthConvert(int, const Palette &);
  void FindTrueCenter();
  void Trim();
  void XFlip();
  void YFlip();
  void SetCenter(int, int);
  void DefSize(int, int);
  void DefLin(const char *);
  void DefLinH(const char *);
  color GetPixel(int, int);
  Graphic HalfSize() {return Scaled(xsize>>1, ysize>>1);};
  Graphic DoubleSize() {return Scaled(xsize<<1, ysize<<1);};
  Graphic Hashed(color);
  Graphic OffHashed(color); 
  Graphic DoubleX();
  Graphic DoubleY();
  int xsize, ysize, depth;
  int xcenter, ycenter;
  mfmt * image;
  unsigned int tcolor;

  private:
  void Init(const char *);
  void Init24(const char *, Palette &);
  void InitTGA32(const char *);
  int linedef;
  int xdef, ydef;
  Chunk chunk;
  };

#endif
