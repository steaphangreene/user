#ifndef INSOMNIA_SOUND_H
#define INSOMNIA_SOUND_H

#include <user/os_defs.h>

#define MAX_SAMPLE_SIZE		65536
#define SOUND_BLOCK_SIZE	256
#define SOUND_QUEUE_SIZE	256

volatile class DigBlock  {
  public:
  DigBlock() { link = NULL; next = NULL; numadded=0; };
  ~DigBlock() { if(next != NULL && link == NULL)  delete next; };

  private:
  volatile unsigned char data[SOUND_BLOCK_SIZE];
  volatile DigBlock *next, *link;
  volatile int numadded;
  friend class SoundCard;
  friend class DigSample;
  };

class DigSample  {
  public:
  DigSample(char *);
  ~DigSample();
  void Play();

  private:
  DigSample();
  unsigned long len;
  DigBlock *wav_data;
  int freq;
  int stereo;
  friend class SoundCard;
  friend class ResFile;
  friend class NewResFile;
  };

class SoundCard {
  public:
  SoundCard();
  SoundCard(int);
  ~SoundCard();
  float SBVersion();
  void PlayDig(const DigSample &);
  void Update();

  private:
  int wav_handle;
  int soundinit;

  DigBlock *queue2;

  static volatile void sound_handler();
  static volatile void dummy_marker_function();

  static volatile short THIS_DS;
  static volatile int mix_freq;
  static volatile long wav_seg;
  static volatile int seglsb, segmsb, segpg;
  static volatile int sb_dma, sb_dma16, sb_irq, sb_addr, section;
  static volatile float dsp_version;
  static volatile DigBlock *queue, *tail, blank, queue1[SOUND_QUEUE_SIZE];

#ifdef DOS
  __dpmi_meminfo handler_area;
  __dpmi_paddr old_handler;
  __dpmi_paddr handler_seginfo;
#endif

  };

#endif
