#ifndef INSOMNIA_USER_SPEAKER_H
#define INSOMNIA_USER_SPEAKER_H

#include "config.h"
#include "mfmt.h"

#ifdef OSS_MMAP_SOUND
#include <pthread.h>
#endif

#define SOUND_NONE	0
#define SOUND_DOS	1
#define SOUND_OSS	2
#define SOUND_OSS_MMAP	3
#define SOUND_ESD	4

#define SOUND_NUM 16

#ifdef DOS_SOUND
#include <dpmi.h>
#endif

class Sound;
//class Music;

struct Playing {
  mfmt pos; 
  int left; 
  };

class Speaker {
  public:
  Speaker(int /*stereo?*/, int /*bits*/, int /*freq*/);
  ~Speaker();
  void Reconfigure(int /*stereo?*/, int /*bits*/, int /*freq*/);
  void FinishQueue();
  void Update();
  int Play(Sound &);
  int Loop(Sound &);
  void MakeFriendly(Sound &);
  void SetAsAmbient(Sound &);
//  void Play(Music &);
  void Stop(int);
  void StopByBuffer(mfmt, int);
  int Active();

  private:
  int stype;
  int Configure(int, int, int);
  void ExpandCur();
  int bufsize, stereo, cur_num, cur_alloc, ambient;
  long writenext;
  mfmt buf, *samp;
  Playing *cur;
  int *loop;
  static volatile int freq;

#ifdef OSS_SOUND
  int dsp;
#ifdef OSS_MMAP_SOUND
  void *dma_buf;
  pthread_t mmap_thread;
  friend void *__do_dma(void *);
  long writenow;
#endif
#else
#ifdef ESD_SOUND
  int dsp;
#endif
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
