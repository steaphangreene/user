#ifndef INSOMNIA_USER_SPEAKER_H
#define INSOMNIA_USER_SPEAKER_H

#include "config.h"
#include "mfmt.h"

#define SOUND_NUM 16

class Sound;
//class Music;

struct Playing {
  unsigned char *pos; 
  int left; 
  };

class Speaker {
  public:
  Speaker(int /*stereo?*/, int /*bits*/, int /*freq*/);
  ~Speaker();
  void Reconfigure(int /*stereo?*/, int /*bits*/, int /*freq*/);
  void FinishQueue();
  void Update();
  void Play(Sound &);
  void SetAsAmbient(Sound &);
//  void Play(Music &);
  void StopByBuffer(mfmt, int);
  int Active();

  private:
//  Speaker();
  int Configure(int, int, int);
  int bufsize, stereo, bits, freq, dsp, cur_num, cur_alloc, ambient;
  long writenext;
  mfmt buf, ambientp;
  Playing *cur;
  };

extern Speaker *__Da_Speaker;

#endif
