#ifndef INSOMNIA_USER_SPEAKER_H
#define INSOMNIA_USER_SPEAKER_H

#include "config.h"
#include "mfmt.h"

#define SOUND_NUM 16

#ifdef DOS_SOUND
#include <dpmi.h>
#endif

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
  void MakeFriendly(Sound &);
  void SetAsAmbient(Sound &);
//  void Play(Music &);
  void StopByBuffer(mfmt, int);
  int Active();

  private:
  int Configure(int, int, int);
  int bufsize, stereo, cur_num, cur_alloc, ambient;
  long writenext;
  mfmt buf, ambientp;
  Playing *cur;
  static volatile int freq;

#ifdef OSS_SOUND
  int dsp;
#endif

#ifdef DOS_SOUND
  static volatile void sound_handler();

  __dpmi_meminfo handler_area;
  __dpmi_paddr old_handler;
  __dpmi_paddr handler_seginfo;

  int wav_handle, old_count, paused;
#endif

  static volatile long bits;

#ifdef DOS_SOUND
  static volatile long wav_seg, count;
  static volatile int seglsb, segmsb, segpg;
  static volatile double dsp_version;

  static volatile int sb_dma8, sb_dma16, sb_irq, sb_addr, section;

  static volatile void dummy_marker_function();
#endif
  };

extern Speaker *__Da_Speaker;

#endif
