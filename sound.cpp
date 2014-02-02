#include "config.h"

#include <stdio.h>
#include <unistd.h>
#include "SDL.h"
#include "SDL_mixer.h"

#include "engine.h"
#include "speaker.h"
#include "sound.h"

#define FRead(fl, data, ammt) {\
  if(ammt > 0)  if(U2_FRead(data, 1, ammt, fl) != ammt) {\
    U2_Exit(1, "Read failure in WAVE file!\n");\
    }\
  }

Sound::Sound()  {
  data.v = NULL;
  }

Sound::Sound(char *fn)  {
#ifdef SDL_SOUND
  data.v = Mix_LoadWAV(fn);
#else
  data.v = NULL;
  U2_File wave;
  wave = U2_FOpenRead(fn);
  if(wave == NULL)  {
    U2_Exit(-1, "\"%s\" not found.\n", fn);
    }
  int headsize;
  short tmp;
  char buffer[256];
  FRead(wave, buffer, 16);
  if((buffer[0] != 'R') || (buffer[1] != 'I') ||
        (buffer[2] != 'F') || (buffer[3] != 'F'))  {
    U2_Exit(1, "\"%s\" is not a WAVE file (no RIFF).\n", fn);
    }
  if((buffer[8] != 'W') || (buffer[9] != 'A') ||
        (buffer[10] != 'V') || (buffer[11] != 'E'))  {
    U2_Exit(1, "\"%s\" is not a WAVE file (no WAVE).\n", fn);
    }
  if((buffer[12] != 'f') || (buffer[13] != 'm') || (buffer[14] != 't'))  {
    U2_Exit(1, "\"%s\" is not a WAVE file (no fmt).\n", fn);
    }
  FRead(wave, &headsize, 4);
  FRead(wave, &tmp, 2);
  if(tmp != 1)  {
    U2_Exit(1, "\"%s\" is not a PCM WAVE file (unsupported).\n", fn);
    }
  FRead(wave, &tmp, 2);
  channels = tmp;
  FRead(wave, &freq, 4);
  FRead(wave, buffer, 6);
  FRead(wave, &tmp, 2);
  bits=tmp;
  if(headsize > 16)  FRead(wave, buffer, (headsize-16));
//  if(bits != 8)  {
//    U2_Exit(1, "\"%s\" is a %d-bit WAVE file (unsupported).\n", fn, bits);
//    }
  FRead(wave, buffer, 4);
  if((buffer[0] == 'f') && (buffer[1] == 'a') &&
        (buffer[2] == 'c') && (buffer[3] == 't'))  {
    FRead(wave, &headsize, 4);
    FRead(wave, buffer, headsize);
    FRead(wave, buffer, 4);
    }
  if((buffer[0] != 'd') || (buffer[1] != 'a') ||
        (buffer[2] != 't') || (buffer[3] != 'a'))  {
    U2_Exit(1, "\"%s\" is not a WAVE file (no data).\n", fn);
    }
  FRead(wave, &len, 4); //len *= channels; len *=(bits>>3);
  data.u8 = new unsigned char[len];
//  printf("Len = %d\n", len);
  FRead(wave, data.u8, (long)len);

  U2_FClose(wave);
#endif
  }

Sound::~Sound()  {
#ifdef SDL_SOUND
  Mix_FreeChunk((Mix_Chunk*)data.v);
#else
  UserDebug("User:Sound:~Sound Begin");
  if(__Da_Speaker != NULL) __Da_Speaker->StopByBuffer(data, (long)len);
  UserDebug("User:Sound:~Sound Middle");
  if(data.v != NULL) {
    if(bits == 16) delete [] data.s16;
    else delete [] data.u8;
    }
  data.v = NULL;
  UserDebug("User:Sound:~Sound End");
#endif
  }

int Sound::Play()  {
#ifdef SDL_SOUND
  return Mix_PlayChannel(-1, (Mix_Chunk *)(data.v), 0);
#else
  if(__Da_Speaker != NULL) return __Da_Speaker->Play(*this);
  else return -1;
#endif
  }

int Sound::Loop()  {
#ifdef SDL_SOUND
  return Mix_PlayChannel(-1, (Mix_Chunk *)(data.v), -1);
#else
  if(__Da_Speaker != NULL) return __Da_Speaker->Loop(*this);
  else return -1;
#endif
  }

void Sound::Stop(int s)  {
#ifdef SDL_SOUND
  Mix_HaltChannel(s);
#else
  if(__Da_Speaker != NULL) __Da_Speaker->Stop(s);
#endif
  }

void Sound::ConvertTo(int bts, int chan, int fr)  {
  UserDebug("User:Sound:ConvertTo Begin");
#ifndef SDL_SOUND
  if(bits == bts && chan == channels && fr == freq) return;
  int ctr; mfmt odata; odata.v = data.v;
  if(bits == 8 && bts == 16)  {
    data.s16 = new short[len];
    for(ctr=0; ctr<(int)len; ctr++)  {
      data.s16[ctr] = odata.u8[ctr];
      data.s16[ctr] -= 128;
      data.s16[ctr] <<= 8;
      }
    len <<= 1; bits = 16;
    delete [] odata.u8; odata.v = data.v;
    }
  else if(bits == 16 && bts == 8)  {
    len >>= 1;
    data.u8 = new unsigned char[len];
    for(ctr=0; ctr<(int)len; ctr++)  {
      odata.s16[ctr] >>= 8;
      odata.s16[ctr] += 128;
      data.u8[ctr] = odata.s16[ctr];
      }
    bits = 8;
    delete [] odata.s16; odata.v = data.v;
    }

  if(bits == 8 && channels == 1 && chan == 2)  {
    data.u8 = new unsigned char[len<<1];
    for(ctr=0; ctr<(int)len; ctr++)  {
      data.u8[(ctr<<1)] = odata.u8[ctr];
      data.u8[(ctr<<1)+1] = odata.u8[ctr];
      }
    len <<= 1; channels = 2;
    delete [] odata.u8; odata.v = data.v;
    }
  else if(bits == 8 && channels == 2 && chan == 1)  {
    len >>= 1;
    data.u8 = new unsigned char[len];
    for(ctr=0; ctr<(int)len; ctr++)  {
      data.u8[ctr] = (odata.u8[(ctr<<1)]>>1)+(odata.u8[(ctr<<1)+1]>>1);
      }
    channels = 1;
    delete [] odata.u8; odata.v = data.v;
    }
  else if(bits == 16 && channels == 1 && chan == 2)  {
    data.s16 = new short[len];
    for(ctr=0; ctr<(int)(len>>1); ctr++)  {
      data.s16[(ctr<<1)] = odata.s16[ctr];
      data.s16[(ctr<<1)+1] = odata.s16[ctr];
      }
    len <<= 1; channels = 2;
    delete [] odata.s16; odata.v = data.v;
    }
  else if(bits == 16 && channels == 2 && chan == 1)  {
    len >>= 1;
    data.s16 = new short[len>>1];
    for(ctr=0; ctr<(int)(len>>1); ctr++)  {
      data.s16[ctr] = (odata.s16[(ctr<<1)]>>1)+(odata.s16[(ctr<<1)+1]>>1);
      }
    channels = 1;
    delete [] odata.s16; odata.v = data.v;
    }

  if(bits == 8 && freq == (fr<<2))  {
    len >>= 2;
    data.u8 = new unsigned char[len];
    for(ctr=0; ctr<(int)len; ctr++)  {
      data.u8[ctr] = odata.u8[ctr<<2];
      }
    freq = fr;
    delete [] odata.u8; odata.v = data.v;
    }
  else if(bits == 8 && freq == (fr<<1))  {
    len >>= 1;
    data.u8 = new unsigned char[len];
    for(ctr=0; ctr<(int)len; ctr++)  {
      data.u8[ctr] = odata.u8[ctr<<1];
      }
    freq = fr;
    delete [] odata.u8; odata.v = data.v;
    }
  else if(bits == 8 && channels == 2 && freq == (fr>>1))  {
    len = (len<<1)-2;
    data.u8 = new unsigned char[len];
    unsigned char tmp2a, tmpa = odata.u8[0];
    unsigned char tmp2b, tmpb = odata.u8[1];
    data.u8[0] = tmpa;
    data.u8[1] = tmpb;
    for(ctr=4; ctr<(int)len; ctr+=4)  {
      tmp2a = odata.u8[(ctr>>1)];
      tmp2b = odata.u8[(ctr>>1)+1];
      data.u8[ctr-1] = ((tmpa+tmp2a)>>1);
      data.u8[ctr] = tmp2a;
      data.u8[ctr-3] = ((tmpb+tmp2b)>>1);
      data.u8[ctr-2] = tmp2b;
      tmpa = tmp2a; tmpb = tmp2b;
      }
    freq = fr;
    delete [] odata.u8; odata.v = data.v;
    }
  else if(bits == 8 && freq == (fr>>1))  {
    len = (len<<1)-1;
    data.u8 = new unsigned char[len];
    unsigned char tmp2, tmp = odata.u8[0];
    data.u8[0] = tmp;
    for(ctr=2; ctr<(int)len; ctr+=2)  {
      tmp2 = odata.u8[ctr>>1];
      data.u8[ctr-1] = ((tmp+tmp2)>>1);
      data.u8[ctr] = tmp2; tmp = tmp2;
      }
    freq = fr;
    delete [] odata.u8; odata.v = data.v;
    }
  else if(bits == 8 && channels == 2 && freq == (fr>>2))  {
    len = (len<<2)-6;
    data.u8 = new unsigned char[len];
    unsigned long tmp2a, tmpa = odata.u8[0];
    unsigned long tmp2b, tmpb = odata.u8[1];
    data.u8[0] = tmpa;
    data.u8[1] = tmpb;
    for(ctr=8; ctr<(int)len; ctr+=8)  {
      tmp2a = odata.u8[(ctr>>2)];
      tmp2b = odata.u8[(ctr>>2)+1];
      data.u8[ctr-3] = ((tmpb+tmp2b+tmp2b+tmp2b)>>2);
      data.u8[ctr-2] = ((tmpb+tmp2b)>>1);
      data.u8[ctr-1] = ((tmpb+tmpb+tmpb+tmp2b)>>2);
      data.u8[ctr] = tmp2b; tmpb = tmp2b;
      data.u8[ctr-7] = ((tmpa+tmp2a+tmp2a+tmp2a)>>2);
      data.u8[ctr-6] = ((tmpa+tmp2a)>>1);
      data.u8[ctr-5] = ((tmpa+tmpa+tmpa+tmp2a)>>2);
      data.u8[ctr-4] = tmp2a; tmpa = tmp2a;
      }
    freq = fr;
    delete [] odata.u8; odata.v = data.v;
    }
  else if(bits == 8 && freq == (fr>>2))  {
    len = (len<<2)-3;
    data.u8 = new unsigned char[len];
    unsigned long tmp2, tmp = odata.u8[0];
    data.u8[0] = tmp;
    for(ctr=4; ctr<(int)len; ctr+=4)  {
      tmp2 = odata.u8[ctr>>2];
      data.u8[ctr-3] = ((tmp+tmp2+tmp2+tmp2)>>2);
      data.u8[ctr-2] = ((tmp+tmp2)>>1);
      data.u8[ctr-1] = ((tmp+tmp+tmp+tmp2)>>2);
      data.u8[ctr] = tmp2; tmp = tmp2;
      }
    freq = fr;
    delete [] odata.u8; odata.v = data.v;
    }
  else if(bits == 16 && channels == 2 && freq == (fr<<2))  {
    len >>= 2;
    data.s16 = new short[len>>1];
    for(ctr=0; ctr<(int)(len>>1); ctr+=2)  {
      data.s16[ctr] = odata.s16[ctr<<2];
      data.s16[ctr+1] = odata.s16[(ctr<<2)+1];
      }
    freq = fr;
    delete [] odata.s16; odata.v = data.v;
    }
  else if(bits == 16 && freq == (fr<<2))  {
    len >>= 2;
    data.s16 = new short[len>>1];
    for(ctr=0; ctr<(int)(len>>1); ctr++)  {
      data.s16[ctr] = odata.s16[ctr<<2];
      }
    freq = fr;
    delete [] odata.s16; odata.v = data.v;
    }
  else if(bits == 16 && channels == 2 && freq == (fr<<1))  {
    len >>= 1;
    data.s16 = new short[len>>1];
    for(ctr=0; ctr<(int)(len>>1); ctr+=2)  {
      data.s16[ctr] = odata.s16[ctr<<1];
      data.s16[ctr+1] = odata.s16[(ctr<<1)+1];
      }
    freq = fr;
    delete [] odata.s16; odata.v = data.v;
    }
  else if(bits == 16 && freq == (fr<<1))  {
    len >>= 1;
    data.s16 = new short[len>>1];
    for(ctr=0; ctr<(int)(len>>1); ctr++)  {
      data.s16[ctr] = odata.s16[ctr<<1];
      }
    freq = fr;
    delete [] odata.s16; odata.v = data.v;
    }
  else if(bits == 16 && channels == 2 && freq == (fr>>1))  {
    len = (len<<1)-4;
    data.s16 = new short[len>>1];
    short tmp2a, tmpa = odata.s16[0];
    short tmp2b, tmpb = odata.s16[1];
    data.s16[0] = tmpa;
    data.s16[1] = tmpb;
    for(ctr=4; ctr<(int)(len>>1); ctr+=4)  {
      tmp2a = odata.s16[(ctr>>1)];
      tmp2b = odata.s16[(ctr>>1)+1];
      data.s16[ctr-1] = (tmpa+tmp2a)>>1;
      data.s16[ctr] = tmp2a; tmpa = tmp2a;
      data.s16[ctr-3] = (tmpb+tmp2b)>>1;
      data.s16[ctr-2] = tmp2b; tmpb = tmp2b;
      }
    freq = fr;
    delete [] odata.s16; odata.v = data.v;
    }
  else if(bits == 16 && freq == (fr>>1))  {
    len = (len<<1)-2;
    data.s16 = new short[len>>1];
    short tmp2, tmp = odata.s16[0];
    data.s16[0] = tmp;
    for(ctr=2; ctr<(int)(len>>1); ctr+=2)  {
      tmp2 = odata.s16[ctr>>1];
      data.s16[ctr-1] = (tmp+tmp2)>>1;
      data.s16[ctr] = tmp2; tmp = tmp2;
      }
    freq = fr;
    delete [] odata.s16; odata.v = data.v;
    }
  else if(bits == 16 && channels == 2 && freq == (fr>>2))  {
    len = (len<<2)-12;
    data.s16 = new short[len>>1];
    long tmp2a, tmpa = odata.s16[0];
    long tmp2b, tmpb = odata.s16[1];
    data.s16[0] = tmpa;
    data.s16[1] = tmpb;
    for(ctr=8; ctr<(int)(len>>1); ctr+=8)  {
      tmp2a = odata.s16[(ctr>>2)];
      tmp2b = odata.s16[(ctr>>2)+1];
      data.s16[ctr-3] = (tmpa+tmp2a+tmp2a+tmp2a)>>2;
      data.s16[ctr-2] = (tmpa+tmp2a)>>1;
      data.s16[ctr-1] = (tmpa+tmpa+tmpa+tmp2a)>>2;
      data.s16[ctr] = tmp2a; tmpa = tmp2a;
      data.s16[ctr-7] = (tmpb+tmp2b+tmp2b+tmp2b)>>2;
      data.s16[ctr-6] = (tmpb+tmp2b)>>1;
      data.s16[ctr-5] = (tmpb+tmpb+tmpb+tmp2b)>>2;
      data.s16[ctr-4] = tmp2b; tmpb = tmp2b;
      }
    freq = fr;
    delete [] odata.s16; odata.v = data.v;
    }
  else if(bits == 16 && freq == (fr>>2))  {
    len = (len<<2)-6;
    data.s16 = new short[len>>1];
    long tmp2, tmp = odata.s16[0];
    data.s16[0] = tmp;
    for(ctr=4; ctr<(int)(len>>1); ctr+=4)  {
      tmp2 = odata.s16[ctr>>2];
      data.s16[ctr-3] = (tmp+tmp2+tmp2+tmp2)>>2;
      data.s16[ctr-2] = (tmp+tmp2)>>1;
      data.s16[ctr-1] = (tmp+tmp+tmp+tmp2)>>2;
      data.s16[ctr] = tmp2; tmp = tmp2;
      }
    freq = fr;
    delete [] odata.s16; odata.v = data.v;
    }
#endif
  UserDebug("User:Sound:ConvertTo End");
  }
