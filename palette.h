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
typedef unsigned long TrueCol;

class Palette  {
  public:
  Palette();
  Palette(const char *);
  Palette(cval *);
  Palette(const Palette &);
  Palette operator =(const Palette &);
  void SetPalette(cval *);
  void SetPaletteEntry(cval el, cval r, cval g, cval b);
  cval GetRedEntry(cval el) const;
  cval GetGreenEntry(cval el) const;
  cval GetBlueEntry(cval el) const;
  TrueCol GetTrueCol(cval el);
  void SetTrueCol(cval el, TrueCol tc);
  void GetPalette(const char *fn);
  cval GetClosestColor(cval r, cval g, cval b);
  cval colors[768];
  int coldec;

  private:
  void GetMSPalette(const char *fn);
  void GetPSPPalette(const char *fn);
  void GetBMPPalette(const char *fn);
  TrueCol tcols[256];
  };

#endif