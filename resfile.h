#ifndef INSOMNIA_RESFILE_H
#define INSOMNIA_RESFILE_H

#include <user/screen.h>
#include <user/sound.h>

class NewResFile  {
  public:
  NewResFile(const char *);
  ~NewResFile();
  void Add(const Graphic *);
  void Add(const DigSample *);

  private:
  void Write(const void *, int);
  void WriteInt(int);
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

  private:
  void Open(const char *, const char *);
  int TryToOpen(const char *, const char *);
  Graphic *GrabGraphic();
  DigSample *GrabSound();
  void Read(void *, int);
  int ReadInt();
  FILE *rf;
  char *fn;
  };

#endif
