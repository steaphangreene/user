#ifndef INSOMNIA_RESFILE_H
#define INSOMNIA_RESFILE_H

#include <user/screen.h>
#include <user/sound.h>
#include <user/bag.h>

class NewResFile  {
  public:
  NewResFile(const char *);
  ~NewResFile();
  void Add(const Graphic *);
  void Add(const DigSample *);
  void Add(const LongBag *);
  void Add(const ShortBag *);
  void Add(const CharBag *);

  private:
  void Write(const void *, int);
  void WriteInt(int);
  void WriteShort(short);
  void WriteChar(char);
  FILE *rf;
  char *fn;
  };

class ResFile  {
  public:
  ResFile(const char *, const char *);
  ResFile(const char *);
  ~ResFile();
  void *Get();
  Graphic *GetGraphic();
  DigSample *GetSound();
  LongBag *GetLongBag();
  ShortBag *GetShortBag();
  CharBag *GetCharBag();

  private:
  void Open(const char *, const char *);
  int TryToOpen(const char *, const char *);
  Graphic *GrabGraphic();
  DigSample *GrabSound();
  LongBag *GrabLongBag();
  ShortBag *GrabShortBag();
  CharBag *GrabCharBag();
  void Read(void *, int);
  int ReadInt();
  short ReadShort();
  char ReadChar();
  FILE *rf;
  char *fn;
  };

#endif
