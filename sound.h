#ifndef INSOMNIA_USER_SOUND_H
#define INSOMNIA_USER_SOUND_H

#include "config.h"
#include "mfmt.h"

class Sound  {
  public:
  Sound(char *);
  ~Sound();
  void Play();
  void ConvertTo(int /*bits*/, int /*stereo?*/, int /*freq*/);
  int Bits() { return bits; };
  int Freq() { return freq; };
  int Stereo() { return stereo; };

  private:
  Sound();
  unsigned long len;
  mfmt data;
  int freq;
  char stereo, bits;
  friend class Speaker;
  friend class NewResFile;
  friend class ResFile;
  };

#endif
