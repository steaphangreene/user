#include "config.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef OSS_SOUND
#include <sys/soundcard.h>
#ifdef OSS_MMAP_SOUND
#include <sys/mman.h>
#endif
#endif

#ifdef ESD_SOUND
#include <esd.h>
#endif

#ifdef SDL_SOUND
#include "SDL.h"
#include "SDL_mixer.h"
#endif

#ifdef DOS_SOUND
#include <sys/segments.h>
#include <pc.h>
#include <dpmi.h>
#include <dos.h>

#include "asm_crap.h"

#define	S_MIX_INDEX	(0x0004 | sb_addr)
#define	S_MIX_DATA	(0x0005 | sb_addr)
#define	S_RESET		(0x0006 | sb_addr)
#define S_READ		(0x000A | sb_addr)
#define	S_WRITE		(0x000C | sb_addr)
#define	S_DATA_AVAIL	(0x000E | sb_addr)
#define	S_ACK8		(0x000E | sb_addr)
#define	S_ACK16		(0x000F | sb_addr)

#define ReadDSP(data)	do { while((inportb(S_DATA_AVAIL) & 128) == 0); data = inportb(S_READ); } while(0)
#define WriteDSP(data)  do { while((inportb(S_WRITE) & 128) != 0); outportb(S_WRITE, data); } while(0)

//#define SB16		(0)
//#define SB16		(1)
#define SB16		(dsp_version >= 4.00)

const int DMA_PAGE[8] = {0x87, 0x83, 0x81, 0x82, 0x8F, 0x8B, 0x89, 0x8A};
const int IRQ_INT[16] = {0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF,
			0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77};
#endif

#include "speaker.h"
#include "sound.h"
#include "engine.h"

//#define SOUND_BUF_SIZE 16384 // Must
//#define SOUND_BUF_POWER 14 // Agree

//#define SOUND_BUF_SIZE 8192 // Must
//#define SOUND_BUF_POWER 13 // Agree

//#define SOUND_BUF_SIZE 4096 // Must
//#define SOUND_BUF_POWER 12 // Agree

//#define SOUND_BUF_SIZE 2048 // Must
//#define SOUND_BUF_POWER 11 // Agree

//#define SOUND_BUF_SIZE 1024 // Must
//#define SOUND_BUF_POWER 10 // Agree

#define SOUND_BUF_SIZE 512 // Must
#define SOUND_BUF_POWER 9 // Agree

//#define SOUND_BUF_SIZE 256 // Must
//#define SOUND_BUF_POWER 8 // Agree

#ifdef OSS_MMAP_SOUND
void *__do_dma(void *);
#endif

Speaker::Speaker(int chan, int bts, int fr, int bfsz)  {
#ifdef OSS_MMAP_SOUND
  dma_buf = NULL;
#endif /*OSS_MMAP_SOUND*/
  buf.v = NULL;
  if(__Da_Speaker != NULL) U2_Exit(-1, "Duplicate Speaker!\n");
  if(Configure(chan, bts, fr, bfsz)) __Da_Speaker = this;
  }

void Speaker::Reconfigure(int chan, int bts, int fr, int bfsz)  {
  if(this != __Da_Speaker) return;
#ifdef OSS_SOUND
#ifdef OSS_MMAP_SOUND
  if(stype == SOUND_OSS_MMAP) {
    struct audio_buf_info abinfo;
    ioctl(dsp, SNDCTL_DSP_GETOSPACE, &abinfo);
    munmap(dma_buf, abinfo.fragstotal * abinfo.fragsize);
    }
#endif
  if(dsp>0) { close(dsp); dsp = 0; }
#endif
#ifdef ESD_SOUND
  if(dsp>0) { close(dsp); dsp = 0; }
#endif

#ifdef SDL_SOUND
  Mix_CloseAudio();
  SDL_Quit();
#endif

  Configure(chan, bts, fr, bfsz);
  }

int Speaker::Configure(int chan, int bts, int fr, int bfsz)  {
  int ctr;

  cur = new Playing[SOUND_NUM];
  samp = new mfmt[SOUND_NUM];
  loop = new int[SOUND_NUM];
  cur_num = 0; cur_alloc = SOUND_NUM;
  for(ctr=0; ctr<SOUND_NUM; ctr++)
    { samp[ctr].v=NULL; cur[ctr].left=-1; cur[ctr].pos.v=NULL; loop[ctr]=0; }
  ambient = -1;

  channels=channels; bits=bts; freq=fr;

#ifdef DOS_SOUND
  count = 0; old_count = -1;
  char *blaster, *tmp;
  blaster = getenv("BLASTER");
  if(blaster == NULL)  {
    fprintf(stderr, "BLASTER variable not set.  ");
    fprintf(stderr, "Run sound card setup utility.\n");
    fprintf(stderr,"Sound will not be played.\n");
    return 0;
    }
  for(tmp = blaster; (*tmp != 'I') && (*tmp != 0); tmp++);
  if(*tmp == 0)  {
    fprintf(stderr, "BLASTER variable invalid (no 'I' value).  ");
    fprintf(stderr, "Run sound card setup utility.\n");
    fprintf(stderr,"Sound will not be played.\n");
    return 0;
    }
  sb_irq = strtol(tmp+1, NULL, 16);
  if(sb_irq==2) sb_irq=9;

  for(tmp = blaster; (*tmp != 'D') && (*tmp != 0); tmp++);
  if(*tmp == 0) sb_dma8 = -1;
  else sb_dma8 = strtol(tmp+1, NULL, 16);

  for(tmp = blaster; (*tmp != 'H') && (*tmp != 0); tmp++);
  if(*tmp == 0) sb_dma16 = -1;
  else sb_dma16 = strtol(tmp+1, NULL, 16);

  sb_dma16 = -1; //DISABLED 16-BIT *********

  if(sb_dma8 == -1 && sb_dma16 == -1)  {
    fprintf(stderr, "BLASTER variable invalid (no 'H' or 'D' value).  ");
    fprintf(stderr, "Run sound card setup utility.\n");
    fprintf(stderr,"Sound will not be played.\n");
    return 0;
    }

  for(tmp = blaster; (*tmp != 'A') && (*tmp != 0); tmp++);
  if(*tmp == 0) sb_addr = 0;
  else sb_addr = strtol(tmp+1, NULL, 16);

  if(sb_addr == 0 || sb_addr<0x220 || sb_addr>0x240 || (sb_addr&0x0F)!=0)  {
    fprintf(stderr, "BLASTER variable invalid (missing or bad 'A' section.  ");
    fprintf(stderr, "Run sound card setup utility.\n");
    fprintf(stderr,"Sound will not be played.\n");
    return 0;
    }

  unsigned char in1, in2;
  outportb(S_RESET, 1);
  delay(1);
  outportb(S_RESET, 0);
  in1 = inportb(S_DATA_AVAIL);
  in2 = inportb(S_READ);
  for(ctr = 0; (((in1 & 128) == 0) || (in2 != 0xAA)) && (ctr < 200); ctr++)  {
    delay(1);
    in1 = inportb(S_DATA_AVAIL);
    in2 = inportb(S_READ);
    }
  if(((in1 & 128) == 0) || (in2 != 0xAA))  {
    fprintf(stderr, "Sound init failed.  Run sound card setup utility.\n");
    fprintf(stderr, "Sound will not be played.\n");
    return 0;
    }

  unsigned char tmpb;
  WriteDSP(0xE1);
  ReadDSP(tmpb); dsp_version = tmpb;
  ReadDSP(tmpb); dsp_version += (tmpb / 100.0);

  if(dsp_version < 2.00)  {
    fprintf(stderr, "Extremely old SoundBlaster (DSP >= 2.00 required).  ");
    fprintf(stderr, "Get a newer sound card.\n");
    fprintf(stderr, "Sound will not be played.\n");
    return 0;
    }

  WriteDSP(0xD1);

  if(!SB16) bits=8;

  if(sb_dma8==-1 && bits==8) bits=16;
  else if(sb_dma16==-1 && bits==16) bits=8;

  if(dsp_version < 3.00) channels=1;

  if(sb_dma8 == -1) {
    printf("SB v%.2f, irq=%X, dma=-/%X, io=%X\n",
	dsp_version, sb_irq, sb_dma16, sb_addr);
    }
  else if(sb_dma16 == -1) {
    printf("SB v%.2f, irq=%X, dma=%X/-, io=%X\n",
	dsp_version, sb_irq, sb_dma8, sb_addr);
    }
  else {
    printf("SB v%.2f, irq=%X, dma=%X/%X, io=%X\n",
	dsp_version, sb_irq, sb_dma8, sb_dma16, sb_addr);
    }

  wav_seg = __dpmi_allocate_dos_memory(bfsz >> 3, &wav_handle);
  if(wav_seg == -1)  {
    fprintf(stderr, "Sound init failed.  Not enough DOS memory.\n");
    fprintf(stderr, "Program will run without sound.\n");
    return 0;
    }

  if(bits==8)  {
    seglsb = ((wav_seg << 4) & 255);
    segmsb = ((wav_seg >> 4) & 255);
    segpg = wav_seg >> 12;
    }
  else  {
    seglsb = ((wav_seg << 3) & 255);
    segmsb = ((wav_seg >> 5) & 255);
    segpg = wav_seg >> 12;
    }

  if(sb_irq < 8) outportb(0x21,inportb(0x21) | (1 << (sb_irq & 7)));
  else {
//    outportb(0x21,inportb(0x21) | 4);  // IRQ 2 = Cascade
    outportb(0xA1,inportb(0xA1) | (1 << (sb_irq & 7)));
    }

  LOCK_VARIABLE(sb_dma8);
  LOCK_VARIABLE(sb_dma16);
  LOCK_VARIABLE(sb_irq);
  LOCK_VARIABLE(sb_addr);
  LOCK_VARIABLE(dsp_version);
  LOCK_VARIABLE(wav_seg);
  LOCK_VARIABLE(seglsb);
  LOCK_VARIABLE(segmsb);
  LOCK_VARIABLE(segpg);
  LOCK_VARIABLE(freq);
  LOCK_VARIABLE(count);

  handler_area.handle = (unsigned long)_my_cs();
  handler_area.size = (unsigned long)dummy_marker_function
	- (unsigned long)sound_handler;
  handler_area.address = (unsigned long)sound_handler;

  __dpmi_get_protected_mode_interrupt_vector(IRQ_INT[sb_irq], &old_handler);
  __dpmi_lock_linear_region(&handler_area);

  handler_seginfo.selector = _my_cs();
  handler_seginfo.offset32 = (long)sound_handler;
  __dpmi_set_protected_mode_interrupt_vector(IRQ_INT[sb_irq], &handler_seginfo);

/*  
  fprintf(stderr, "DOS sound not yet implemented in User 2.0.\n");
  fprintf(stderr,"Sound will not be played.\n");
  return 0;
*/

  unsigned char blank[bufsize];
  memset(blank, 0, bufsize);
  dosmemput((void *)blank, bufsize, (wav_seg<<4));
  dosmemput((void *)blank, bufsize, (wav_seg<<4)+bufsize);

  disable();
  if(bits==8)  {
    outportb(0x0A, 0x04|sb_dma8);
    outportb(0x0C, 0x00);
    outportb(0x0B, 0x58|sb_dma8);
    outportb(DMA_PAGE[sb_dma8], segpg);
    outportb((sb_dma8 << 1), seglsb);
    outportb((sb_dma8 << 1), segmsb);
    outportb((sb_dma8 << 1) + 1, ((bufsize<<1)-1) & 255);
    outportb((sb_dma8 << 1) + 1, ((bufsize-1)>>7));
    outportb(0x0A, sb_dma8);
    }
  else {
    outportb(0xD4, 0x04|(sb_dma16&3));
    outportb(0xD8, 0x00);
    outportb(0xD6, 0x58|(sb_dma16&3));
    outportb(DMA_PAGE[sb_dma16], segpg);
    outportb(0xB8+(sb_dma16 << 2), seglsb);
    outportb(0xB8+(sb_dma16 << 2), segmsb);
    outportb(0xBA+(sb_dma16 << 2), (bufsize-1) & 255);
    outportb(0xBA+(sb_dma16 << 2), (bufsize-1) >> 8);
    outportb(0xD4, (sb_dma16&3));
    }

  if(SB16) {
    WriteDSP(0x41);
    WriteDSP(freq >> 8);
    WriteDSP(freq & 255);
    if(bits==8)  {
      WriteDSP(0xC6);
      if(channels > 1) WriteDSP(0x20);	// Unsigned Stereo
      else WriteDSP(0x00);		// Unsigned Mono
      WriteDSP((bufsize-1) & 255);
      WriteDSP((bufsize-1) >> 8);
      }
    else {
      WriteDSP(0xB6);
      if(channels > 1) WriteDSP(0x30);	// Signed Stereo
      else WriteDSP(0x10);		// Signed Mono
      WriteDSP(((bufsize>>1)-1) & 255);
      WriteDSP((bufsize-2)>>9);
      }
    }
  else {
    if(channels > 1) {
      outportb(S_MIX_INDEX, 0x0E);
      outportb(S_MIX_DATA, inportb(S_MIX_DATA)|0x02);
      }
    else {
      outportb(S_MIX_INDEX, 0x0E);
      outportb(S_MIX_DATA, inportb(S_MIX_DATA)&0xFD);
      }
    WriteDSP(0x40);
    WriteDSP(256-(1000000/(channels * freq)));
    WriteDSP(0x48);
    WriteDSP((bufsize-1) & 255);
    WriteDSP((bufsize-1) >> 8);
    WriteDSP(0x90);
    }

  if(sb_irq < 8) outportb(0x21,inportb(0x21) & (255^(1 << (sb_irq & 7))));
  else {
    outportb(0x21,inportb(0x21) & 251);  // IRQ 2 = Cascade
    outportb(0xA1,inportb(0xA1) & (255^(1 << (sb_irq & 7))));
    }
  enable();
  if(bits == 8) outportb(S_WRITE, 0xD0);
  else outportb(S_WRITE, 0xD5);
  paused = 1;
#endif

#ifdef SDL_SOUND
  stype = SOUND_SDL;
  SDL_Init(SDL_INIT_AUDIO);
  if(Mix_OpenAudio(44100, AUDIO_S16, 2, 2048)) {
    fprintf(stderr, "Error initializing SDL Audio!\n");
    fprintf(stderr, "Continuing without sound.\n");
    return 0;
    }
#endif

#ifdef OSS_SOUND
  stype = SOUND_OSS;
#ifdef OSS_MMAP_SOUND
  dsp=open("/dev/dsp", O_RDWR);
#else
  dsp=open("/dev/dsp", O_WRONLY);
#endif
  if(dsp<1) {
#endif

#ifdef ESD_SOUND
    stype = SOUND_ESD;
    esd_format_t format = ESD_STREAM | ESD_PLAY;
    if(bits==16) format |= ESD_BITS16;
    else format |= ESD_BITS8;
    if(channels > 1) format |= ESD_STEREO;
    else format |= ESD_MONO;
    dsp=esd_play_stream_fallback( format, freq, NULL, "User Engine 2.0" );
    if(dsp<1) {
#ifdef OSS_SOUND
      perror("User::Speaker ESD failed, and Error on /dev/dsp");
#else
      fprintf(stderr, "User::Speaker ESD failed\n");
#endif
      fprintf(stderr, "Sound will not be played.\n");
      stype = SOUND_NONE; return 0;
      }
    if(fcntl(dsp, F_SETFL, O_NONBLOCK)==-1) {
      perror("User");
      fprintf(stderr, "Error setting non-blocking mode\n");
      stype = SOUND_NONE; return 0;
      }
    return 1;
#else
    perror("User::Speaker Error on /dev/dsp");
    fprintf(stderr, "Sound will not be played.\n");
    stype = SOUND_NONE; return 0;
#endif

#ifdef OSS_SOUND
    }
  int tmp, tmp2, caps;
  if(ioctl(dsp, SNDCTL_DSP_GETCAPS, &caps) == -1) {
    perror("User");
    fprintf(stderr, "Error getting soundcard capabilities\n");
    stype = SOUND_NONE; return 0;
    }
  if(bfsz) { 
    int bufpow=0, bufstub=1;
    while(bufstub != 0 && bufstub < bfsz) { ++bufpow; bufstub <<= 1; }
    if(bufstub != bfsz) {
      U2_Exit(-1, "User:Speaker: buf_size (%d) not a power of 2!\n", bfsz);
      }
    tmp = 0x00020000+bufpow;
    }
  if(ioctl(dsp, SNDCTL_DSP_SETFRAGMENT, &tmp)==-1) {
    perror("User");
    fprintf(stderr, "Error setting frag size\n");
    stype = SOUND_NONE; return 0;
    }
  if(bits==8) tmp = AFMT_U8;
  else if(bits==16) tmp = AFMT_S16_LE;
  else U2_Exit(-1, "Bits = %ld!?\n", bits);
  tmp2 = tmp;
  if(ioctl(dsp, SNDCTL_DSP_SETFMT, &tmp)==-1 || tmp != tmp2) {
    perror("User");
    fprintf(stderr, "Error setting format\n");
    stype = SOUND_NONE; return 0;
    }
  if(ioctl(dsp, SNDCTL_DSP_CHANNELS, &channels)==-1) {
    perror("User");
    fprintf(stderr, "Error setting stereo/mono\n");
    stype = SOUND_NONE; return 0;
    }
  tmp = freq;  tmp2 = tmp;
  if(ioctl(dsp, SNDCTL_DSP_SPEED, &tmp)==-1 || tmp != tmp2) {
    perror("User");
    fprintf(stderr, "Error setting frequency\n");
    stype = SOUND_NONE; return 0;
    }
  struct audio_buf_info abinfo;
  ioctl(dsp, SNDCTL_DSP_GETOSPACE, &abinfo);
  bufsize = abinfo.fragsize;
#ifdef OSS_MMAP_SOUND
  if((caps & DSP_CAP_MMAP) && (caps & DSP_CAP_TRIGGER)) {
    if((dma_buf = mmap(NULL, abinfo.fragstotal * abinfo.fragsize,
	PROT_WRITE, MAP_FILE | MAP_SHARED, dsp, 0)) == (void*)-1) {
      perror("User:Speaker:mmap");
      fprintf(stderr, "Speaker: mmap failed - using old OSS mode\n");
      }
    else {
      writenext = 0;  writenow = 0;
      memset(dma_buf, 0, abinfo.fragstotal * abinfo.fragsize);
      pthread_create(&mmap_thread, NULL, __do_dma, this);
      stype = SOUND_OSS_MMAP;
      }
    }
#endif
  writenext = -(bufsize);
#endif
  if(buf.u8 != NULL) delete [] buf.u8;
  buf.u8 = new unsigned char[bufsize];

  return 1;
  }

#ifdef OSS_MMAP_SOUND
void *__do_dma(void *t) {
  ((Speaker *)t)->dma_handler();
  return NULL;
  }

void Speaker::dma_handler() {
  int tmp;
  fd_set ws;
  timeval tm;
  count_info count;
  tmp = 0;
  if (ioctl(dsp, SNDCTL_DSP_SETTRIGGER, &tmp) == -1) {
    perror("ioctl: SNDCTL_DSP_SETTRIGGER");
    U2_Exit(-1);
    }
  tmp = PCM_ENABLE_OUTPUT;
  if (ioctl(dsp, SNDCTL_DSP_SETTRIGGER, &tmp) == -1) {
    perror("ioctl: SNDCTL_DSP_SETTRIGGER");
    U2_Exit(-1);
    }
  while(1) {
    FD_ZERO(&ws);
    FD_SET(dsp, &ws);
    tm.tv_sec=10;  tm.tv_usec = 0;
    tmp = select(dsp+1, NULL, &ws, NULL, &tm);
    if(tmp == -1) {
      perror("User:Speaker:select");
      U2_Exit(-1);
      }
    if(tmp == 0) {
      U2_Exit(-1, "User:Speaker:select: timeout.\n");
      }
    if(ioctl(dsp, SNDCTL_DSP_GETOPTR, &count)==-1) {
      perror("User:Speaker:ioctl(SNDCTL_DSP_GETOPTR)");
      U2_Exit(-1);
      }
    while(count.blocks > 0) {
      char *target = (char *)dma_buf;
      if(writenow & 1) target += bufsize;
      memcpy(target, buf.v, bufsize);
      ++writenow;
      --count.blocks;
      }
    }
  }
#endif

Speaker::~Speaker()  {
#ifdef DOS_SOUND
  disable();
  if(wav_seg != -1)
    __dpmi_free_dos_memory(wav_handle);
  if(__Da_Speaker == this)  {
    outportb(S_WRITE, 0xD3);
    outportb(S_WRITE, 0xD0);
    outportb(S_WRITE, 0xDA);
    outportb(S_WRITE, 0xD0);

    if(sb_irq < 8) outportb(0x21,inportb(0x21) | (1 << (sb_irq & 7)));
    else {
//      outportb(0x21,inportb(0x21) | 4);  // IRQ 2 = Cascade
      outportb(0xA1,inportb(0xA1) | (1 << (sb_irq & 7)));
      }

    __dpmi_set_protected_mode_interrupt_vector(IRQ_INT[sb_irq], &old_handler);
//    __dpmi_unlock_linear_region(&handler_area);
    }
  enable();
#endif

#ifdef OSS_SOUND
  if(dsp>0) { close(dsp); dsp = 0; }
#else
#ifdef ESD_SOUND
  if(dsp>0) { close(dsp); dsp = 0; }
#endif
#endif
  __Da_Speaker = NULL;
  }

void Speaker::FinishQueue() {
#ifdef OSS_SOUND
  if(stype == SOUND_OSS) ioctl(dsp, SNDCTL_DSP_SYNC);
#endif
#ifdef ESD_SOUND
  if(stype == SOUND_ESD) fcntl(dsp, F_SETFL, O_SYNC);
#endif
  }

void Speaker::MakeFriendly(Sound &smp) {
  smp.ConvertTo(bits, channels, freq);
  }

int Speaker::Play(Sound &smp) {
  int ctr;
  smp.ConvertTo(bits, channels, freq);
//  write(dsp, smp.data, smp.len);
  for(ctr=0; ctr<cur_alloc && cur[ctr].left>=0; ctr++);
  if(ctr>=cur_alloc) /* ExpandCur(); */ return -1; 
  cur[ctr].left = smp.len;  cur[ctr].pos.u8 = smp.data.u8;
  loop[ctr] = 0; samp[ctr].u8 = smp.data.u8;

#ifdef DOS_SOUND
  if(paused) {
    if(bits == 8) outportb(S_WRITE, 0xD4);
    else outportb(S_WRITE, 0xD6);
    paused = 0;
    }
#endif
  return ctr;
  }

int Speaker::Loop(Sound &smp) {
  int ctr;
  smp.ConvertTo(bits, channels, freq);
//  write(dsp, smp.data, smp.len);
  for(ctr=0; ctr<cur_alloc && cur[ctr].left>=0; ctr++);
  if(ctr>=cur_alloc) /* ExpandCur(); */ return -1; 
  cur[ctr].left = smp.len;  cur[ctr].pos.u8 = smp.data.u8;
  loop[ctr] = 1; samp[ctr].u8 = smp.data.u8;

#ifdef DOS_SOUND
  if(paused) {
    if(bits == 8) outportb(S_WRITE, 0xD4);
    else outportb(S_WRITE, 0xD6);
    paused = 0;
    }
#endif
  return ctr;
  }

void Speaker::SetAsAmbient(Sound &smp) {
  int ctr;
  smp.ConvertTo(bits, channels, freq);
//  write(dsp, smp.data, smp.len);
  for(ctr=0; ctr<cur_alloc && cur[ctr].left>=0; ctr++);
  if(ctr>=cur_alloc) /* ExpandCur(); */ return; 
  cur[ctr].left = smp.len;  cur[ctr].pos.u8 = smp.data.u8;
  ambient = ctr; samp[ctr].u8 = smp.data.u8;
  loop[ctr] = 1;

#ifdef DOS_SOUND
  if(paused) {
    if(bits == 8) outportb(S_WRITE, 0xD4);
    else outportb(S_WRITE, 0xD6);
    paused = 0;
    }
#endif
  }

void Speaker::Update() {
  UserDebug("User:Speaker:Update Begin");
  int ctr, ctr2;

#ifdef SDL_SOUND
  if(stype == SOUND_SDL) return;
#endif

#ifdef DOS_SOUND
  if(!paused) {
    for(ctr=0; ctr<cur_alloc && cur[ctr].left<0; ctr++);
    if(ctr==cur_alloc) {
      if(bits == 8) outportb(S_WRITE, 0xD0);
      else outportb(S_WRITE, 0xD5);
      paused = 1;
      }
    }
  if(count <= old_count) return;
//  printf("count = %ld\n", count);
//  U2_Exit(0, "count = %ld\n", count);
  old_count=count;
#endif

#ifdef OSS_SOUND
  if(stype == SOUND_OSS) {
    count_info tmp;
    if(ioctl(dsp, SNDCTL_DSP_GETOPTR, &tmp)==-1) {
      perror("User");
      U2_Exit(-1, "Error checking progress\n");
      }
    if(tmp.bytes < writenext) return;
    writenext += bufsize;
    }
#ifdef OSS_MMAP_SOUND
  else if(stype == SOUND_OSS_MMAP) {
    if(writenext > writenow) return;
    ++writenext;
//    printf("Inc writenext\n"); fflush(stdout);
    }
#endif
#endif

//  if(bits==8) memset(buf.u8, 128, bufsize);
//  if(bits==16) memset(buf.s16, 0, bufsize);
  if(bits==8)  {
    register int smp;
    for(ctr2=0; ctr2 < bufsize; ctr2++) {
      smp = 128;
      for(ctr=0; ctr<cur_alloc; ctr++)  {
	if(ctr2 < cur[ctr].left) { smp += cur[ctr].pos.u8[ctr2]; smp -= 128; }
	}
      if(smp<0) smp=0; else if(smp>255) smp=255;
      buf.u8[ctr2] = smp;
      }
    }
  else if(bits==16) for(ctr=0; ctr<cur_alloc; ctr++)  {
    register int smp;
    for(ctr2=0; ctr2 < (bufsize>>1); ctr2++) {
      smp = 0;
      for(ctr=0; ctr<cur_alloc; ctr++)  {
	if(ctr2 < ((cur[ctr].left)>>1)) { smp += cur[ctr].pos.s16[ctr2]; }
	}
      if(smp<-32768) smp=-32768; else if(smp>32767) smp=32767;
      buf.s16[ctr2] = smp;
      }
    }
  else U2_Exit(1, "Unknown sound depth error!\n");

  for(ctr=0; ctr<cur_alloc; ctr++)  {
    if(cur[ctr].left>=0)  {
      if(cur[ctr].left <= bufsize) {
	if(loop[ctr])  {
	  cur[ctr].left += cur[ctr].pos.u8-samp[ctr].u8;
	  cur[ctr].pos.u8 = samp[ctr].u8;
	  }
	else {
	  cur[ctr].left = -1;
	  }
	}
      else  {
	cur[ctr].left -= bufsize;
	cur[ctr].pos.u8 += bufsize;
	}
      }
    }

#ifdef DOS_SOUND
  if(count&1) dosmemput(buf.v, bufsize, (wav_seg<<4));
  else dosmemput(buf.v, bufsize, (wav_seg<<4)+bufsize);
#endif

#ifdef OSS_SOUND
  if(stype == SOUND_OSS) write(dsp, buf.u8, bufsize);
#endif
#ifdef ESD_SOUND
  if(stype == SOUND_ESD) write(dsp, buf.u8, bufsize);
//  if(stype == SOUND_ESD) fcntl(dsp, F_SETFL, O_SYNC);
#endif

  UserDebug("User:Speaker:Update End");
  }

void Speaker::Stop(int s)  {
  cur[s].left = -1;
  loop[s] = 0;
  samp[s].v = NULL;
  if(s==ambient)  {
    ambient = -1;
    }
  }

void Speaker::StopByBuffer(mfmt ptr, int sz)  {
  int ctr;
  for(ctr=0; ctr<cur_alloc; ctr++)  {
    if(cur[ctr].pos.u8 >= ptr.u8 && cur[ctr].pos.u8 < (ptr.u8+sz))  {
      cur[ctr].left = -1;
      loop[ctr] = 0;
      samp[ctr].v = NULL;
      if(ctr==ambient)  {
	ambient = -1;
	}
      }
    }
  }

int Speaker::Active()  {
  return(__Da_Speaker == this);
  }

void Speaker::ExpandCur()  {
  int ctr;

  cur_alloc += SOUND_NUM;
  Playing *oldcur=cur;
  cur = new Playing[cur_alloc];
  loop = new int[cur_alloc];
  for(ctr=0; ctr<(cur_alloc-SOUND_NUM); ctr++) {
     cur[ctr] = oldcur[ctr];
     loop[ctr] = loop[ctr];
     }
  for(; ctr<cur_alloc; ctr++) {
    cur[ctr].left=-1;
    cur[ctr].pos.v=NULL;
    loop[ctr]=0;
    }
  delete oldcur;
  }

#ifdef DOS_SOUND
volatile void Speaker::sound_handler()  {
  START_INTERRUPT();
  __asm__ __volatile__ (
	"
/  if(bits==8) inportb(S_ACK8);
	movl	__7Speaker$sb_addr, %edx
	movl	__7Speaker$bits, %eax
	testb	$0x08, %al
	jz	bit16
	orw	$0xE, %dx
	inb	%dx, %al
	jmp	was_bit8

/  else inportb(S_ACK16)
bit16:
	orw	$0xF, %dx
	inb	%dx, %al

/  outportb(0x20, 0x20);
was_bit8:
	movb	$0x20, %al
	outb	%al, $0x20

/  if(sb_irq | 8) outportb(0xA0, 0x20);
	movl	__7Speaker$sb_irq, %eax
	testl	$0x08, %eax
	jz	s8b
	movb	$0x20, %al
	outb	%al, $0xA0
s8b:

/  ++count;
	movl	__7Speaker$count, %eax
	incl	%eax
	movl	%eax, __7Speaker$count
	");
  END_INTERRUPT();
  }

#endif
volatile int Speaker::freq;
volatile int Speaker::bits;

#ifdef DOS_SOUND
volatile int Speaker::sb_dma8;
volatile int Speaker::sb_dma16;
volatile int Speaker::sb_irq;
volatile int Speaker::sb_addr;
volatile double Speaker::dsp_version;
volatile int Speaker::wav_seg;
volatile int Speaker::count;
volatile int Speaker::seglsb;
volatile int Speaker::segmsb;
volatile int Speaker::segpg;
volatile int Speaker::section = 0;

volatile void Speaker::dummy_marker_function()  {}
#endif
