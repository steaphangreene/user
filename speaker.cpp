#include <sys/types.h>
#include <sys/stat.h>
#include <sys/soundcard.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "speaker.h"
#include "sound.h"
#include "engine.h"

#define SOUND_BUF_SIZE 16384 // Must
#define SOUND_BUF_POWER 14 // Agree

Speaker::Speaker(int stro, int bts, int fr)  {
  buf.uc = new unsigned char[SOUND_BUF_SIZE];
  if(__Da_Speaker != NULL) Exit(-1, "Duplicate Speaker!\n");
  if(Configure(stro, bts, fr)) __Da_Speaker = this;
  }

void Speaker::Reconfigure(int stro, int bts, int fr)  {
  if(dsp>0) close(dsp);
  Configure(stro, bts, fr);
  }

int Speaker::Configure(int stro, int bts, int fr)  {
  int ctr;

  writenext = -(SOUND_BUF_SIZE);
  cur = new Playing[SOUND_NUM];
  cur_num = 0; cur_alloc = SOUND_NUM;
  for(ctr=0; ctr<SOUND_NUM; ctr++) { cur[ctr].left=-1; cur[ctr].pos=NULL; }
  ambient = -1; ambientp.v = NULL;

  stereo=stro; bits=bts; freq=fr;
  dsp=open("/dev/dsp", O_WRONLY);
  if(dsp==-1) {
    perror("User::Speaker Error on /dev/dsp");
    fprintf(stderr, "Sound will not be played.\n");
    return 0;
    }
  int tmp = 0x00020000+SOUND_BUF_POWER; //1024 frags
  if(ioctl(dsp, SNDCTL_DSP_SETFRAGMENT, &tmp)==-1) {
    perror("User");
    fprintf(stderr, "Error setting frag size\n");
    return 0;
    }
  if(ioctl(dsp, SNDCTL_DSP_STEREO, &stereo)==-1) {
    perror("User");
    fprintf(stderr, "Error setting stereo/mono\n");
    return 0;
    }
  if(bits==8) tmp = AFMT_U8;
  else if(bits==16) tmp = AFMT_S16_LE;
  else Exit(-1, "Bits = %d!?\n", bits);
  if(ioctl(dsp, SNDCTL_DSP_SETFMT, &tmp)==-1) {
    perror("User");
    fprintf(stderr, "Error setting format\n");
    return 0;
    }
  if(ioctl(dsp, SNDCTL_DSP_SPEED, &freq)==-1) {
    perror("User");
    fprintf(stderr, "Error setting frequency\n");
    return 0;
    }
  if(ioctl(dsp, SNDCTL_DSP_NONBLOCK)==-1) {
    perror("User");
    fprintf(stderr, "Error setting non-blocking mode\n");
    return 0;
    }
  return 1;
  }

Speaker::~Speaker()  {
  __Da_Speaker = NULL;
  if(dsp>0) close(dsp);
  }

void Speaker::FinishQueue() {
  ioctl(dsp, SNDCTL_DSP_SYNC);
  }

void Speaker::Play(Sound &samp) {
  int ctr;
  samp.ConvertTo(bits, stereo, freq);
//  write(dsp, samp.data, samp.len);
  for(ctr=0; ctr<cur_alloc && cur[ctr].left>=0; ctr++);
  if(ctr>=cur_alloc) Exit(-1, "Unhandled cur expansion!\n");
  cur[ctr].left = samp.len;  cur[ctr].pos = samp.data.uc;
  }

void Speaker::SetAsAmbient(Sound &samp) {
  int ctr;
  samp.ConvertTo(bits, stereo, freq);
//  write(dsp, samp.data, samp.len);
  for(ctr=0; ctr<cur_alloc && cur[ctr].left>=0; ctr++);
  if(ctr>=cur_alloc) Exit(-1, "Unhandled cur expansion!\n");
  cur[ctr].left = samp.len;  cur[ctr].pos = samp.data.uc;
  ambient = ctr; ambientp.uc = samp.data.uc;
  }

void Speaker::Update() {
  Debug("User:Speaker:Update Begin");
  count_info tmp;
  if(ioctl(dsp, SNDCTL_DSP_GETOPTR, &tmp)==-1) {
    perror("User");
    Exit(-1, "Error checking progress\n");
    }
//  printf("Bytes = %d\n", writenext);
  if(tmp.bytes < writenext) return;
  writenext += SOUND_BUF_SIZE;
//  printf("Bytes = %d\n", tmp.bytes);

  int ctr, ctr2;//, mix=0;
  if(bits==8) memset(buf.uc, 128, SOUND_BUF_SIZE);
  else memset(buf.s, 0, SOUND_BUF_SIZE);
  for(ctr=0; ctr<cur_alloc; ctr++)  {
    if(cur[ctr].left>=0)  {
//      memcpy(buf.uc, cur[ctr].pos, SOUND_BUF_SIZE <? cur[ctr].left);
      if(bits==8) {
	for(ctr2=0; ctr2<(SOUND_BUF_SIZE <? cur[ctr].left); ctr2++) {
	  unsigned char ob = buf.uc[ctr2];
	  buf.uc[ctr2]+=cur[ctr].pos[ctr2]; buf.uc[ctr2] ^= 128;
	  if(buf.uc[ctr2] < cur[ctr].pos[ctr2] && ob > 128) buf.uc[ctr] = 255;
	  if(buf.uc[ctr2] > cur[ctr].pos[ctr2] && ob < 128) buf.uc[ctr] = 0;
	  }
	}
      else {
//	if(!mix) {
//	  memcpy(buf, cur[ctr].pos, SOUND_BUF_SIZE <? cur[ctr].left);
//	  mix=1;
//	  }
//	else {
	  for(ctr2=0; ctr2<((SOUND_BUF_SIZE <? cur[ctr].left)>>1); ctr2++)  {
	    short ob = buf.s[ctr2];
	    buf.s[ctr2] += ((short*)(cur[ctr].pos))[ctr2];
	    if(buf.s[ctr2] < ((short*)(cur[ctr].pos))[ctr2] && ob > 0)
		buf.s[ctr2] = 32767;
	    if(buf.s[ctr2] > ((short*)(cur[ctr].pos))[ctr2] && ob < 0)
		buf.s[ctr2] = -32768;
//	    }
	  }
	}
      if(cur[ctr].left <= SOUND_BUF_SIZE) {
	if(ctr==ambient)  {
	  cur[ctr].left += cur[ctr].pos-ambientp.uc;
	  cur[ctr].pos = ambientp.uc;
	  }
	else {
	  cur[ctr].left = -1;
	  }
	}
      else  {
	cur[ctr].left -= SOUND_BUF_SIZE;
	cur[ctr].pos += SOUND_BUF_SIZE;
	}
      }
    }
  write(dsp, buf.uc, SOUND_BUF_SIZE);
  Debug("User:Speaker:Update End");
  }

void Speaker::StopByBuffer(mfmt ptr, int sz)  {
  int ctr;
  for(ctr=0; ctr<cur_alloc; ctr++)  {
    if(cur[ctr].pos >= ptr.uc && cur[ctr].pos < (ptr.uc+sz))  {
      cur[ctr].left = -1;
      if(ctr==ambient)  {
	ambient = -1;
	ambientp.v = NULL;
	}
      }
    }
  }

int Speaker::Active()  {
  return(__Da_Speaker == this);
  }
