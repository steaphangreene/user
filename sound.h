#ifndef INSOMNIA_USER_SOUND_H
#define INSOMNIA_USER_SOUND_H

#include "config.h"
#include "mfmt.h"

class Sound  {
  public:
  Sound(char *);
  ~Sound();
  int Play();
  int Loop();
  void Stop(int);
  void ConvertTo(int /*bits*/, int /*channels*/, int /*freq*/);
  int Bits() { return bits; };
  int Freq() { return freq; };
  int Channels() { return channels; };

  private:
  Sound();
  unsigned long len;
  mfmt data;
  int freq;
  char channels, bits;
  friend class Speaker;
  friend class NewResFile;
  friend class ResFile;
  };

#endif
