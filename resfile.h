#ifndef INSOMNIA_USER_RESFILE_H
#define INSOMNIA_USER_RESFILE_H

#include "config.h"
#include "engine.h"

#include <stdio.h>

class Graphic;
class Palette;
class Sound;
class LongBag;
class ShortBag;
class CharBag;

class NewResFile  {
  public:
  NewResFile(const char *);
  ~NewResFile();
  void Add(const Graphic *);
  void Add(const Palette *);
  void Add(const Sound *);
  void Add(const LongBag *);
  void Add(const ShortBag *);
  void Add(const CharBag *);

  private:
  void Write(const void *, int);
  void WriteInt(int);
  void WriteShort(short);
  void WriteChar(char);
  U2_File rf;
  char *fn;
  };

class ResFile  {
  public:
  ResFile(const char *, const char *);
  ResFile(const char *);
  ~ResFile();
  void *Get();
  Graphic *GetGraphic();
  Palette *GetPalette();
  Sound *GetSound();
  LongBag *GetLongBag();
  ShortBag *GetShortBag();
  CharBag *GetCharBag();

  private:
  void Open(const char *, const char *);
  int TryToOpen(const char *, const char *);
  Graphic *GrabGraphic();
  Palette *GrabPalette();
  Sound *GrabSound();
  LongBag *GrabLongBag();
  ShortBag *GrabShortBag();
  CharBag *GrabCharBag();
  void Read(void *, int);
  int ReadInt();
  short ReadShort();
  char ReadChar();
  U2_File rf;
  char *fn;
  long version;
  };

#endif
