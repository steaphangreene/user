#include <stdio.h>
#include <unistd.h>

#include "engine.h"
#include "speaker.h"
#include "sound.h"

#define FRead(fl, data, ammt) {\
  if(ammt > 0)  if(read(fileno(fl), data, ammt) != ammt) {\
    Exit(1, "Read failure in WAVE file!\n");\
    }\
  }

Sound::Sound()  {
  data.v = NULL;
  }

Sound::Sound(char *fn)  {
  data.v = NULL;
  FILE *wave;
  wave = fopen(fn, "rb");
  if(wave == NULL)  {
    Exit(-1, "\"%s\" not found.\n", fn);
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
//  if(stereo)  {
//    Exit(1, "\"%s\" is a stereo WAVE file (unsupported).\n", fn);
//    }
  FRead(wave, &freq, 4);
  FRead(wave, buffer, 6);
  FRead(wave, &tmp, 2);
  bits=tmp;
  if(headsize > 16)  FRead(wave, buffer, (headsize-16));
//  if(bits != 8)  {
//    Exit(1, "\"%s\" is a %d-bit WAVE file (unsupported).\n", fn, bits);
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
    Exit(1, "\"%s\" is not a WAVE file (no data).\n", fn);
    }
  FRead(wave, &len, 4); //len *=(stereo+1); len *=(bits>>3);
  data.uc = new unsigned char[len];
//  printf("Len = %d\n", len);
  FRead(wave, data.uc, (long)len);
  
  fclose(wave);
  }

Sound::~Sound()  {
  Debug("User:Sound:~Sound Begin");
  if(__Da_Speaker != NULL) __Da_Speaker->StopByBuffer(data, (long)len);
  Debug("User:Sound:~Sound Middle");
  if(data.v != NULL) delete data.v;
  data.v = NULL;
  Debug("User:Sound:~Sound End");
  }

void Sound::Play()  {
  if(__Da_Speaker != NULL) __Da_Speaker->Play(*this);
  }

void Sound::ConvertTo(int bts, int stro, int fr)  {
  Debug("User:Sound:ConvertTo Begin");
  if(bits == bts && stro == stereo && fr == freq) return;
  int ctr; mfmt odata; odata.v = data.v;
  if(bits == 8 && bts == 16)  {
    data.s = new short[len];
    for(ctr=0; ctr<len; ctr++)  {
      data.s[ctr] = odata.uc[ctr];
      data.s[ctr] ^= 128;
      data.s[ctr] <<= 8;
      }
    len <<= 1; bits = 16;
    delete odata.v; odata.v = data.v;
    }
  else if(bits == 16 && bts == 8)  {
    len >>= 1;
    data.uc = new unsigned char[len];
    for(ctr=0; ctr<len; ctr++)  {
      odata.s[ctr] >>= 8;
      odata.s[ctr] ^= 128;
      data.uc[ctr] = odata.s[ctr];
      }
    bits = 8;
    delete odata.v; odata.v = data.v;
    }

  if(bits == 8 && stereo == 0 && stro == 1)  {
    data.uc = new unsigned char[len<<1];
    for(ctr=0; ctr<len; ctr++)  {
      data.uc[(ctr<<1)] = odata.uc[ctr];
      data.uc[(ctr<<1)+1] = odata.uc[ctr];
      }
    len <<= 1; stereo = 1;
    delete odata.v; odata.v = data.v;
    }
  else if(bits == 8 && stereo == 1 && stro == 0)  {
    len >>= 1;
    data.uc = new unsigned char[len];
    for(ctr=0; ctr<len; ctr++)  {
      data.uc[ctr] = (odata.uc[(ctr<<1)]>>1)+(odata.uc[(ctr<<1)+1]>>1);
      }
    stereo = 1;
    delete odata.v; odata.v = data.v;
    }
  else if(bits == 16 && stereo == 0 && stro == 1)  {
    data.s = new short[len];
    for(ctr=0; ctr<(len>>1); ctr++)  {
      data.s[(ctr<<1)] = odata.s[ctr];
      data.s[(ctr<<1)+1] = odata.s[ctr];
      }
    len <<= 1; stereo = 1;
    delete odata.v; odata.v = data.v;
    }
  else if(bits == 16 && stereo == 1 && stro == 0)  {
    len >>= 1;
    data.s = new short[len>>1];
    for(ctr=0; ctr<len>>1; ctr++)  {
      data.s[ctr] = (odata.s[(ctr<<1)]>>1)+(odata.s[(ctr<<1)+1]>>1);
      }
    stereo = 1;
    delete odata.v; odata.v = data.v;
    }

  if(bits == 8 && freq == (fr<<2))  {
    len >>= 2;
    data.uc = new unsigned char[len];
    for(ctr=0; ctr<len; ctr++)  {
      data.uc[ctr] = odata.uc[ctr<<2];
      }
    freq = fr;
    delete odata.v; odata.v = data.v;
    }
  else if(bits == 8 && freq == (fr<<1))  {
    len >>= 1;
    data.uc = new unsigned char[len];
    for(ctr=0; ctr<len; ctr++)  {
      data.uc[ctr] = odata.uc[ctr<<1];
      }
    freq = fr;
    delete odata.v; odata.v = data.v;
    }
  else if(bits == 8 && freq == (fr>>1))  {
    len = (len<<1)-1;
    data.uc = new unsigned char[len];
    unsigned char tmp2, tmp = odata.uc[0];
    data.uc[0] = tmp;
    for(ctr=2; ctr<len; ctr+=2)  {
      tmp2 = odata.uc[ctr>>1];
      data.uc[ctr-1] = ((tmp+(tmp2^128))>>1);
      data.uc[ctr] = tmp2; tmp = tmp2;
      }
    freq = fr;
    delete odata.v; odata.v = data.v;
    }
  else if(bits == 8 && freq == (fr>>2))  {
    len = (len<<2)-3;
    data.uc = new unsigned char[len];
    unsigned char tmp2, tmp = odata.uc[0];
    data.uc[0] = tmp;
    for(ctr=4; ctr<len; ctr+=4)  {
      tmp2 = odata.uc[ctr>>2];
      data.uc[ctr-3] = (tmp+tmp2+tmp2+tmp2)>>2;
      data.uc[ctr-2] = ((tmp+(tmp2^128))>>1);
      data.uc[ctr-1] = (tmp+tmp+tmp+tmp2)>>2;
      data.uc[ctr] = tmp2; tmp = tmp2;
      }
    freq = fr;
    delete odata.v; odata.v = data.v;
    }
  else if(bits == 16 && freq == (fr<<2))  {
    len >>= 2;
    data.s = new short[len>>1];
    for(ctr=0; ctr<len>>1; ctr++)  {
      data.s[ctr] = odata.s[ctr<<2];
      }
    freq = fr;
    delete odata.v; odata.v = data.v;
    }
  else if(bits == 16 && freq == (fr<<1))  {
    len >>= 1;
    data.s = new short[len>>1];
    for(ctr=0; ctr<len>>1; ctr++)  {
      data.s[ctr] = odata.s[ctr<<1];
      }
    freq = fr;
    delete odata.v; odata.v = data.v;
    }
  else if(bits == 16 && freq == (fr>>1))  {
    len = (len<<1)-2;
    data.s = new short[len>>1];
    short tmp2, tmp = odata.s[0];
    data.s[0] = tmp;
    for(ctr=2; ctr<(len>>1); ctr+=2)  {
      tmp2 = odata.s[ctr>>1];
      data.s[ctr-1] = (tmp+tmp2)>>1;
      data.s[ctr] = tmp2; tmp = tmp2;
      }
    freq = fr;
    delete odata.v; odata.v = data.v;
    }
  else if(bits == 16 && freq == (fr>>2))  {
    len = (len<<2)-6;
    data.s = new short[len>>1];
    short tmp2, tmp = odata.s[0];
    data.s[0] = tmp;
    for(ctr=4; ctr<(len>>1); ctr+=4)  {
      tmp2 = odata.s[ctr>>2];
      data.s[ctr-3] = (tmp+tmp2+tmp2+tmp2)>>2;
      data.s[ctr-2] = (tmp+tmp2)>>1;
      data.s[ctr-1] = (tmp+tmp+tmp+tmp2)>>2;
      data.s[ctr] = tmp2; tmp = tmp2;
      }
    freq = fr;
    delete odata.v; odata.v = data.v;
    }
  Debug("User:Sound:ConvertTo End");
  }
