// *************************************************************************
// palette.h
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

#ifndef INSOMNIA_PALETTE_H
#define INSOMNIA_PALETTE_H

typedef unsigned char cval;
typedef unsigned char Color;
typedef unsigned long TrueColor;

class Palette  {
  public:
  Palette();
  Palette(const char *);
  Palette(cval *);
  Palette(const Palette &);
  Palette operator =(const Palette &);
  void SetPalette(cval *);
  void SetPaletteEntry(cval el, cval r, cval g, cval b);
  cval GetRedEntry(cval el);
  cval GetGreenEntry(cval el);
  cval GetBlueEntry(cval el);
  TrueColor GetTrueColor(cval el);
  void SetTrueColor(cval el, TrueColor tc);
  void GetPSPPalette(const char *fn);
  void GetBMPPalette(const char *fn);
  cval GetClosestColor(cval r, cval g, cval b);
  cval colors[768];
  TrueColor tcols[256];
  int coldec;
  };

#endif
