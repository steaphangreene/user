#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <user/os_defs.h>
#include <user/screen.h>
#include <user/sound.h>

#ifdef DOS
#include <sys/segments.h>
#include <dpmi.h>
#include <go32.h>
#include <dos.h>
#include "asm_crap.h"
#endif

extern User * __Da_User;
extern Screen * __Da_Screen;
extern SoundCard * __Da_SoundCard;

#define	S_MIX_INDEX	(0x0004 | sb_addr)
#define	S_MIX_WRITE	(0x0005 | sb_addr)
#define	S_RESET		(0x0006 | sb_addr)
#define S_READ		(0x000A | sb_addr)
#define	S_WRITE		(0x000C | sb_addr)
#define	S_DATA_AVAIL	(0x000E | sb_addr)

#define ReadDSP(data)	do { while((inportb(S_DATA_AVAIL) & 128) == 0); data = inportb(S_READ); } while(0)
#define WriteDSP(data)  do { while((inportb(S_WRITE) & 128) != 0); outportb(S_WRITE, data); } while(0)

#define TRAN_SIZE	(SOUND_BLOCK_SIZE-1)
#define AUTO_TRAN_SIZE	(SOUND_BLOCK_SIZE-1)

//#define SB16		(0)
//#define autoinit	(0)
#define SB16		(dsp_version >= 4.00)
#define autoinit	(dsp_version > 2.00)

static int count = 0;

const int DMA_PAGE[4] = {0x87, 0x83, 0x81, 0x82};
const int IRQ_INT[16] = {0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF,
			0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77};

void FRead(FILE *fl, void *data, int ammt)  {
  if(ammt > 0)  if(read(fileno(fl), data, ammt) != ammt)  {
    Exit(1, "Read failure in WAVE file!\n");
    }
  }

DigSample::DigSample()  {
  freq = 0;
  wav_data = NULL;
  }

#ifdef DOS
DigSample::DigSample(char *fn)  {
  FILE *wave;
  wave = fopen(fn, "rb");
  if(wave == NULL)  {
    Exit(1, "\"%s\" not found.\n", fn);
    }
  int headsize;
  short tmp;
  char buffer[256];
  FRead(wave, buffer, 16);
  if((buffer[0] != 'R') || (buffer[1] != 'I') ||
	(buffer[2] != 'F') || (buffer[3] != 'F'))  {
    Exit(1, "\"%s\" is not a WAVE file (no RIFF).\n", fn);
    }
  if((buffer[8] != 'W') || (buffer[9] != 'A') ||
	(buffer[10] != 'V') || (buffer[11] != 'E'))  {
    Exit(1, "\"%s\" is not a WAVE file (no WAVE).\n", fn);
    }
  if((buffer[12] != 'f') || (buffer[13] != 'm') || (buffer[14] != 't'))  {
    Exit(1, "\"%s\" is not a WAVE file (no fmt).\n", fn);
    }
  FRead(wave, &headsize, 4);
  FRead(wave, &tmp, 2);
  if(tmp != 1)  {
    Exit(1, "\"%s\" is not a PCM WAVE file (unsupported).\n", fn);
    }
  FRead(wave, &tmp, 2);
  stereo = (tmp == 2);
  if(stereo)  {
    Exit(1, "\"%s\" is a stereo WAVE file (unsupported).\n", fn);
    }
  FRead(wave, &freq, 4);
  FRead(wave, buffer, 6);
  FRead(wave, &tmp, 2);
  if(headsize > 16)  FRead(wave, buffer, (headsize-16));
  if(tmp != 8)  {
    Exit(1, "\"%s\" is a %d-bit WAVE file (only 8-bit supported).\n", fn, tmp);
    }
  FRead(wave, buffer, 4);
  if((buffer[0] == 'f') && (buffer[1] == 'a') &&
	(buffer[2] == 'c') && (buffer[3] == 't'))  {
    FRead(wave, &headsize, 4);
    FRead(wave, buffer, headsize);
    FRead(wave, buffer, 4);
    }
  if((buffer[0] != 'd') || (buffer[1] != 'a') ||
	(buffer[2] != 't') || (buffer[3] != 'a'))  {
    Exit(1, "\"%s\" is not a WAVE file (no data).\n", fn);
    }
  FRead(wave, &len, 4);

  int tmpl = len;
  DigBlock *ind;
  ind = new DigBlock;
  if(ind == NULL)  {
    Exit(1, "Not enough memory to load \"%s\".\n", fn);
    }
  disable();
  wav_data = ind;
  while(tmpl >= SOUND_BLOCK_SIZE)  {
    FRead(wave, (unsigned char *)ind->data, SOUND_BLOCK_SIZE);
    tmpl -= SOUND_BLOCK_SIZE;
    if(tmpl > 0)  {
      ind->next = new DigBlock;
      ind = (DigBlock *)ind->next;
      if(ind == NULL)  {
	Exit(1, "Not enough memory to load \"%s\".\n", fn);
	}
      }
    }
  if(tmpl > 0)  {
    FRead(wave, (unsigned char *)ind->data, tmpl);
    memset((void *)&(ind->data[tmpl]), 0x80, SOUND_BLOCK_SIZE-tmpl);
    }
  ind->next = NULL;
  fclose(wave);
  enable();
  }

DigSample::~DigSample()  {
  if(wav_data != NULL)  free(wav_data);
  }

void DigSample::Play()  {
  if(__Da_SoundCard != NULL)
    __Da_SoundCard->PlayDig(*this);
  }

void SoundCard::PlayDig(const DigSample &fx)  {
  if(soundinit != 1)  return;

  disable();

//  int len = fx.len, ctr;

  DigBlock *from = fx.wav_data;
  if(queue == NULL)  queue = tail;
  tail = queue;
  while(from != NULL)  {
    memcpy((void *)tail->data, (void *)from->data, SOUND_BLOCK_SIZE);
    from = (DigBlock *)from->next;
    if(from != NULL && tail->link != queue)  {
      tail->next = tail->link;
      tail = tail->next;
      }
    else  {
      tail->next = NULL;
      break;
      }
//    printf("Added 1\r\n");
    }

  enable();

  dosmemput((void *)queue->data, SOUND_BLOCK_SIZE, (wav_seg<<4));

  disable();

  DigBlock *ind;
  ind = new DigBlock;
  if(ind == NULL)  Exit(1, "Out of memory for sound sample.\n");
  queue2 = ind;
  while(from != NULL)  {
    memcpy((void *)ind->data, (void *)from->data, SOUND_BLOCK_SIZE);
    from = (DigBlock *)from->next;
    if(from != NULL)  {
      ind->next = new DigBlock;
      ind = (DigBlock *)ind->next;
      if(ind == NULL)  Exit(1, "Out of memory for sound sample.\n");
      }
    }
  ind->next = NULL;

  enable();

  }

void SoundCard::Update()  {
//  printf("%d\n", count);
  if(tail->link == queue || queue2 == NULL)  {
//    printf("Aborted Update!\r\n");
    return;
    }

  DigBlock *tmpd;
  disable();
  while(tail->link != queue && queue2 != NULL)  {
    tail->next = tail->link;
    tail = tail->next;
    memcpy((void *)tail->data, (void *)queue2->data, SOUND_BLOCK_SIZE);
    tmpd = queue2;
    queue2 = (DigBlock *)queue2->next;
    tmpd->next = NULL;
    delete tmpd;
    }
  tail->next = NULL;
  enable();
//  printf("Done updating\r\n");
  }

SoundCard::SoundCard(int mf)  {
  if(__Da_SoundCard != NULL)  {
    Exit(1, "Error: Multiple SoundCard Objects Created.\n");
    }
  mix_freq = mf;
  wav_seg = -1;
  char *blaster, *tmp;
  int ctr;
  blaster = getenv("BLASTER");
  if(blaster == NULL)  {
    printf("\"BLASTER\" variable not set.  Run sound card setup utility.\n");
    printf("Program will run without sound.  <Press A Key>\n");
    if(__Da_User != NULL)  __Da_User->GetAChar();
    else  getchar();
    return;
    }
  for(tmp = blaster; (*tmp != 'A') && (*tmp != 0); tmp++);
  if(*tmp == 0)  {
    printf("\"BLASTER\" variable invalid (no A field).\n");
    printf("Run sound card setup utility.\n");
    printf("Program will run without sound.  <Press A Key>\n");
    if(__Da_User != NULL)  __Da_User->GetAChar();
    else  getchar();
    return;
    }
  tmp++;
  sb_addr = strtol(tmp, &tmp, 16);
  for(tmp = blaster; (*tmp != 'D') && (*tmp != 0); tmp++);
  if(*tmp == 0)  {
    printf("\"BLASTER\" variable invalid (no D field).\n");
    printf("Run sound card setup utility.\n");
    printf("Program will run without sound.  <Press A Key>\n");
    if(__Da_User != NULL)  __Da_User->GetAChar();
    else  getchar();
    return;
    }
  tmp++;
  sb_dma = strtol(tmp, &tmp, 16);
  for(tmp = blaster; (*tmp != 'I') && (*tmp != 0); tmp++);
  if(*tmp == 0)  {
    printf("\"BLASTER\" variable invalid (no I field).\n");
    printf("Run sound card setup utility.\n");
    printf("Program will run without sound.  <Press A Key>\n");
    if(__Da_User != NULL)  __Da_User->GetAChar();
    else  getchar();
    return;
    }
  tmp++;
  sb_irq = strtol(tmp, &tmp, 16);
  for(tmp = blaster; (*tmp != 'H') && (*tmp != 0); tmp++);
  if(*tmp == 0)  {
    sb_dma16 = 0;
    }
  else  {
    tmp++;
    sb_dma16 = strtol(tmp, &tmp, 16);
    }
  if((sb_addr < 0x220) || (sb_addr > 0x240) ||
	((sb_addr % 0x10) != 0))  {
    printf("\"BLASTER\" variable invalid (A%X not valid).\n", sb_addr);
    printf("Run sound card setup utility.\n");
    printf("Program will run without sound.  <Press A Key>\n");
    if(__Da_User != NULL)  __Da_User->GetAChar();
    else  getchar();
    return;
    }
  unsigned char in1, in2;
  outportb(S_RESET, 1);
  delay(10);
  outportb(S_RESET, 0);
  in1 = inportb(S_DATA_AVAIL);
  in2 = inportb(S_READ);
  for(ctr = 0; (((in1 & 128) == 0) || (in2 != 0xAA)) && (ctr < 200); ctr++)  {
    delay(1);
    in1 = inportb(S_DATA_AVAIL);
    in2 = inportb(S_READ);
    }
  if(((in1 & 128) == 0) || (in2 != 0xAA))  {
    printf("Sound init failed.  Run sound card setup utility.\n");
    printf("Program will run without sound.  <Press A Key>\n");
    if(__Da_User != NULL)  __Da_User->GetAChar();
    else  getchar();
    return;
    }
  wav_seg = __dpmi_allocate_dos_memory(SOUND_BLOCK_SIZE >> 3, &wav_handle);
  if(wav_seg == -1)  {
    printf("Sound init failed.  Not enough DOS memory.\n");
    printf("Program will run without sound.  <Press A Key>\n");
    if(__Da_User != NULL)  __Da_User->GetAChar();
    else  getchar();
    return;
    }
  seglsb = ((wav_seg << 4) & 255);
  segmsb = ((wav_seg >> 4) & 255);
  segpg = wav_seg >> 12;

  unsigned char tmpbyte;
  outportb(S_MIX_INDEX, 0x0E);
  tmpbyte = inportb(S_MIX_WRITE);
  outportb(S_MIX_INDEX, 0x0E);
  outportb(S_MIX_WRITE, tmpbyte|0x22);
  WriteDSP(0xE1);
  ReadDSP(dsp_version);
  ReadDSP(tmpbyte);
  dsp_version += (tmpbyte / 100.0);
  WriteDSP(0xD1);
  soundinit = 1;

  disable();

  tmpbyte = (1 << sb_irq);
  outportb(0x21,inportb(0x21) | tmpbyte);

  handler_area.handle = (unsigned long)_my_cs();
  handler_area.size = (long)dummy_marker_function - (long)sound_handler;
  handler_area.address = (unsigned long)sound_handler;

  if(__dpmi_lock_linear_region(&handler_area))  {
    Exit(1, "Unable to lock required memory!!\n");
    }

  LOCK_VARIABLE(THIS_DS);
  LOCK_VARIABLE(sb_dma);
  LOCK_VARIABLE(sb_dma16);
  LOCK_VARIABLE(sb_irq);
  LOCK_VARIABLE(sb_addr);
  LOCK_VARIABLE(dsp_version);
  LOCK_VARIABLE(wav_seg);
  LOCK_VARIABLE(seglsb);
  LOCK_VARIABLE(segmsb);
  LOCK_VARIABLE(segpg);
  LOCK_VARIABLE(queue);
  LOCK_VARIABLE(tail);
  LOCK_VARIABLE(blank);
  LOCK_VARIABLE(mix_freq);
  LOCK_VARIABLE(section);
  LOCK_ARRAY(queue1, SOUND_QUEUE_SIZE);

  __dpmi_get_protected_mode_interrupt_vector(IRQ_INT[sb_irq], &old_handler);

  handler_seginfo.selector = _my_cs();
  handler_seginfo.offset32 = (long)sound_handler;

  __dpmi_set_protected_mode_interrupt_vector(IRQ_INT[sb_irq], &handler_seginfo);

  tmpbyte = ~tmpbyte;
  outportb(0x21,inportb(0x21) & tmpbyte);

  memset((void *)(blank.data), 0x80, SOUND_BLOCK_SIZE);
  
  for(ctr=0; ctr < (SOUND_QUEUE_SIZE-1); ctr++)  {
    queue1[ctr].link = &queue1[ctr+1];
    queue1[ctr].next = NULL;
    }
  queue1[SOUND_QUEUE_SIZE-1].link = queue1;
  queue1[SOUND_QUEUE_SIZE-1].next = NULL;

  queue = NULL;
  queue2 = NULL;
  tail = queue1;

  THIS_DS = _my_ds();

  dosmemput((void *)blank.data, SOUND_BLOCK_SIZE, (wav_seg<<4));
  dosmemput((void *)blank.data, SOUND_BLOCK_SIZE, (wav_seg<<4)+SOUND_BLOCK_SIZE);

  outportb(0x0A, 0x04+sb_dma);
  outportb(0x0C, 0x00);
  if(autoinit)
    outportb(0x0B, 0x58+sb_dma);
  else
    outportb(0x0B, 0x48+sb_dma);
  outportb((sb_dma << 1), seglsb);
  outportb((sb_dma << 1), segmsb);
  outportb(DMA_PAGE[sb_dma], segpg);
  if(autoinit)  {
    outportb((sb_dma << 1) + 1, (TRAN_SIZE << 1) & 255);
    outportb((sb_dma << 1) + 1, TRAN_SIZE >> 7);
    }
  else  {
    outportb((sb_dma << 1) + 1, TRAN_SIZE & 255);
    outportb((sb_dma << 1) + 1, TRAN_SIZE >> 8);
    }
  outportb(0x0A, sb_dma);

  if(SB16)  {
    WriteDSP(0x41);
    WriteDSP(mix_freq >> 8);
    WriteDSP(mix_freq & 255);
    if(autoinit)  WriteDSP(0xC6);
    else  WriteDSP(0xC0);
    WriteDSP(0x0);
    WriteDSP(TRAN_SIZE & 255);
    WriteDSP(TRAN_SIZE >> 8);
    }
  else  {
    WriteDSP(0x40);
    WriteDSP(256-(1000000/mix_freq));
    if(autoinit)  {
      WriteDSP(0x48);
      WriteDSP(AUTO_TRAN_SIZE & 255);
      WriteDSP(AUTO_TRAN_SIZE >> 8);
      WriteDSP(0x1C);
      }
    else  {
      WriteDSP(0x14);
      WriteDSP(TRAN_SIZE & 255);
      WriteDSP(TRAN_SIZE >> 8);
      }
    }

  enable();

  __Da_SoundCard = this;
//  printf("Did the soundcard thing!\r\n");
//  sound_handler();
  }

SoundCard::~SoundCard()  {
  disable();
  queue = NULL;
  if(wav_seg != -1)
    __dpmi_free_dos_memory(wav_handle);
  if(soundinit == 1)  {
    unsigned char tmpbyte;
    soundinit = 0;
    outportb(S_WRITE, 0xD3);
    outportb(S_WRITE, 0xD0);
    outportb(S_WRITE, 0xDA);
    outportb(S_WRITE, 0xD0);

    tmpbyte = (1 << sb_irq);
    outportb(0x21,inportb(0x21) | tmpbyte);
    __dpmi_set_protected_mode_interrupt_vector(IRQ_INT[sb_irq], &old_handler);
//    __dpmi_unlock_linear_region(&handler_area);
    }
  __Da_SoundCard = NULL;
  enable();
  }

volatile void SoundCard::sound_handler()  {
  START_INTERRUPT();
  __asm__ __volatile__ ("movw %0,%%ds" : : "g" (THIS_DS));
  __asm__ __volatile__ ("movw %0,%%es" : : "g" (THIS_DS));
  volatile static unsigned spos;
  section = 1-section;

  inportb(S_DATA_AVAIL);

  count++;

  if(autoinit)  {
    }
  else  {
    outportb(0x0A, 0x04+sb_dma);
    outportb(0x0C, 0x00);
    outportb(0x0B, 0x48+sb_dma);
    if(section)  {
      outportb((sb_dma << 1), ((wav_seg << 4)+SOUND_BLOCK_SIZE) & 255);
      outportb((sb_dma << 1), (((wav_seg << 4)+SOUND_BLOCK_SIZE) >> 8) & 255);
      }
    else  {
      outportb((sb_dma << 1), seglsb);
      outportb((sb_dma << 1), segmsb);
      }
    outportb(DMA_PAGE[sb_dma], segpg);
    outportb((sb_dma << 1) + 1, TRAN_SIZE & 255);
    outportb((sb_dma << 1) + 1, TRAN_SIZE >> 8);
    outportb(0x0A, sb_dma);

    if(SB16)  {
      WriteDSP(0x41);
      WriteDSP(mix_freq >> 8);
      WriteDSP(mix_freq & 255);
      WriteDSP(0xC0);
      WriteDSP(0x0);
      WriteDSP(TRAN_SIZE & 255);
      WriteDSP(TRAN_SIZE >> 8);
      }
    else  {
      WriteDSP(0x40);
      WriteDSP(256-(1000000/mix_freq));
      WriteDSP(0x14);
      WriteDSP(TRAN_SIZE & 255);
      WriteDSP(TRAN_SIZE >> 8);
      }
    }

  spos = (wav_seg << 4);
  if(!section)  spos += SOUND_BLOCK_SIZE;
  if(queue)  {
    __asm__ __volatile__ ("movl	%0, %%esi\n	movw	%1, %%es
	movl	%2, %%edi\n	movl	%3, %%ecx\n	rep\n	movsb"
	: : "g" ((unsigned)queue->data), "g" ((unsigned short)_dos_ds),
	"g" (spos), "g" ((unsigned)(SOUND_BLOCK_SIZE)));
    if(queue->next == NULL)  {
      queue = NULL;
      }
    else  {
      queue->next = NULL;
      queue = queue->link;
      }
    }
  else  {
    __asm__ __volatile__ ("movl	%0, %%esi\n	movw	%1, %%es
	movl	%2, %%edi\n	movl	%3, %%ecx\n	rep\n	movsb"
	: : "g" ((unsigned)blank.data), "g" ((unsigned short)_dos_ds),
	"g" (spos), "g" ((unsigned)(SOUND_BLOCK_SIZE)));
    }

  outportb(0x20, 0x20);

  END_INTERRUPT();
  }
#endif

#ifdef X_WINDOWS
void DigSample::Play()  {
  }

void SoundCard::PlayDig(const DigSample &fx)  {
  }

volatile void SoundCard::sound_handler()  {
  }

SoundCard::~SoundCard()  {
  }

void SoundCard::Update()  {
  }

DigSample::DigSample(char *fn)  {
  }

SoundCard::SoundCard(int mf)  {
  }

#endif

float SoundCard::SBVersion()  {
  return dsp_version;
  }

SoundCard::SoundCard()  {
  SoundCard(22050);
  }

volatile void SoundCard::dummy_marker_function()  {}

volatile short SoundCard::THIS_DS;
volatile int SoundCard::sb_dma;
volatile int SoundCard::sb_dma16;
volatile int SoundCard::sb_irq;
volatile int SoundCard::sb_addr;
volatile float SoundCard::dsp_version;
volatile long SoundCard::wav_seg;
volatile int SoundCard::seglsb;
volatile int SoundCard::segmsb;
volatile int SoundCard::segpg;
volatile DigBlock *SoundCard::queue = NULL;
volatile DigBlock *SoundCard::tail = NULL;
volatile DigBlock SoundCard::blank;
volatile DigBlock SoundCard::queue1[SOUND_QUEUE_SIZE];
volatile int SoundCard::mix_freq;
volatile int SoundCard::section = 0;
