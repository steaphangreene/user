#include "config.h"

#include <stdio.h>
#include <unistd.h>

#include "engine.h"
#include "speaker.h"
#include "sound.h"

#define FRead(fl, data, ammt) {\
  if(ammt > 0)  if(read(fileno(fl), data, ammt) != ammt) {\
    U2_Exit(1, "Read failure in WAVE file!\n");\
    }\
  }

Sound::Sound()  {
  data.v = NULL;
  }

Sound::Sound(char *fn)  {
  data.v = NULL;
  FILE *wave;
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
  stereo = (tmp == 2);
//  if(stereo)  {
//    U2_Exit(1, "\"%s\" is a stereo WAVE file (unsupported).\n", fn);
//    }
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
  FRead(wave, &len, 4); //len *=(stereo+1); len *=(bits>>3);
  data.uc = new unsigned char[len];
//  printf("Len = %d\n", len);
  FRead(wave, data.uc, (long)len);
  
  fclose(wave);
  }

Sound::~Sound()  {
  UserDebug("User:Sound:~Sound Begin");
  if(__Da_Speaker != NULL) __Da_Speaker->StopByBuffer(data, (long)len);
  UserDebug("User:Sound:~Sound Middle");
  if(data.v != NULL) {
    if(bits == 16) delete [] data.s;
    else delete [] data.uc;
    }
  data.v = NULL;
  UserDebug("User:Sound:~Sound End");
  }

int Sound::Play()  {
  if(__Da_Speaker != NULL) return __Da_Speaker->Play(*this);
  else return -1;
  }

int Sound::Loop()  {
  if(__Da_Speaker != NULL) return __Da_Speaker->Loop(*this);
  else return -1;
  }

void Sound::Stop(int s)  {
  if(__Da_Speaker != NULL) __Da_Speaker->Stop(s);
  }

void Sound::ConvertTo(int bts, int stro, int fr)  {
  UserDebug("User:Sound:ConvertTo Begin");
  if(bits == bts && stro == stereo && fr == freq) return;
  int ctr; mfmt odata; odata.v = data.v;
  if(bits == 8 && bts == 16)  {
    data.s = new short[len];
    for(ctr=0; ctr<(int)len; ctr++)  {
      data.s[ctr] = odata.uc[ctr];
      data.s[ctr] -= 128;
      data.s[ctr] <<= 8;
      }
    len <<= 1; bits = 16;
    delete [] odata.uc; odata.v = data.v;
    }
  else if(bits == 16 && bts == 8)  {
    len >>= 1;
    data.uc = new unsigned char[len];
    for(ctr=0; ctr<(int)len; ctr++)  {
      odata.s[ctr] >>= 8;
      odata.s[ctr] += 128;
      data.uc[ctr] = odata.s[ctr];
      }
    bits = 8;
    delete [] odata.s; odata.v = data.v;
    }

  if(bits == 8 && stereo == 0 && stro == 1)  {
    data.uc = new unsigned char[len<<1];
    for(ctr=0; ctr<(int)len; ctr++)  {
      data.uc[(ctr<<1)] = odata.uc[ctr];
      data.uc[(ctr<<1)+1] = odata.uc[ctr];
      }
    len <<= 1; stereo = 1;
    delete [] odata.uc; odata.v = data.v;
    }
  else if(bits == 8 && stereo == 1 && stro == 0)  {
    len >>= 1;
    data.uc = new unsigned char[len];
    for(ctr=0; ctr<(int)len; ctr++)  {
      data.uc[ctr] = (odata.uc[(ctr<<1)]>>1)+(odata.uc[(ctr<<1)+1]>>1);
      }
    stereo = 1;
    delete [] odata.uc; odata.v = data.v;
    }
  else if(bits == 16 && stereo == 0 && stro == 1)  {
    data.s = new short[len];
    for(ctr=0; ctr<(int)(len>>1); ctr++)  {
      data.s[(ctr<<1)] = odata.s[ctr];
      data.s[(ctr<<1)+1] = odata.s[ctr];
      }
    len <<= 1; stereo = 1;
    delete [] odata.s; odata.v = data.v;
    }
  else if(bits == 16 && stereo == 1 && stro == 0)  {
    len >>= 1;
    data.s = new short[len>>1];
    for(ctr=0; ctr<(int)(len>>1); ctr++)  {
      data.s[ctr] = (odata.s[(ctr<<1)]>>1)+(odata.s[(ctr<<1)+1]>>1);
      }
    stereo = 1;
    delete [] odata.s; odata.v = data.v;
    }

  if(bits == 8 && freq == (fr<<2))  {
    len >>= 2;
    data.uc = new unsigned char[len];
    for(ctr=0; ctr<(int)len; ctr++)  {
      data.uc[ctr] = odata.uc[ctr<<2];
      }
    freq = fr;
    delete [] odata.uc; odata.v = data.v;
    }
  else if(bits == 8 && freq == (fr<<1))  {
    len >>= 1;
    data.uc = new unsigned char[len];
    for(ctr=0; ctr<(int)len; ctr++)  {
      data.uc[ctr] = odata.uc[ctr<<1];
      }
    freq = fr;
    delete [] odata.uc; odata.v = data.v;
    }
  else if(bits == 8 && stereo && freq == (fr>>1))  {
    len = (len<<1)-2;
    data.uc = new unsigned char[len];
    unsigned char tmp2a, tmpa = odata.uc[0];
    unsigned char tmp2b, tmpb = odata.uc[1];
    data.uc[0] = tmpa;
    data.uc[1] = tmpb;
    for(ctr=4; ctr<(int)len; ctr+=4)  {
      tmp2a = odata.uc[(ctr>>1)];
      tmp2b = odata.uc[(ctr>>1)+1];
      data.uc[ctr-1] = ((tmpa+tmp2a)>>1);
      data.uc[ctr] = tmp2a;
      data.uc[ctr-3] = ((tmpb+tmp2b)>>1);
      data.uc[ctr-2] = tmp2b;
      tmpa = tmp2a; tmpb = tmp2b;
      }
    freq = fr;
    delete [] odata.uc; odata.v = data.v;
    }
  else if(bits == 8 && freq == (fr>>1))  {
    len = (len<<1)-1;
    data.uc = new unsigned char[len];
    unsigned char tmp2, tmp = odata.uc[0];
    data.uc[0] = tmp;
    for(ctr=2; ctr<(int)len; ctr+=2)  {
      tmp2 = odata.uc[ctr>>1];
      data.uc[ctr-1] = ((tmp+tmp2)>>1);
      data.uc[ctr] = tmp2; tmp = tmp2;
      }
    freq = fr;
    delete [] odata.uc; odata.v = data.v;
    }
  else if(bits == 8 && stereo && freq == (fr>>2))  {
    len = (len<<2)-6;
    data.uc = new unsigned char[len];
    unsigned long tmp2a, tmpa = odata.uc[0];
    unsigned long tmp2b, tmpb = odata.uc[1];
    data.uc[0] = tmpa;
    data.uc[1] = tmpb;
    for(ctr=8; ctr<(int)len; ctr+=8)  {
      tmp2a = odata.uc[(ctr>>2)];
      tmp2b = odata.uc[(ctr>>2)+1];
      data.uc[ctr-3] = ((tmpb+tmp2b+tmp2b+tmp2b)>>2);
      data.uc[ctr-2] = ((tmpb+tmp2b)>>1);
      data.uc[ctr-1] = ((tmpb+tmpb+tmpb+tmp2b)>>2);
      data.uc[ctr] = tmp2b; tmpb = tmp2b;
      data.uc[ctr-7] = ((tmpa+tmp2a+tmp2a+tmp2a)>>2);
      data.uc[ctr-6] = ((tmpa+tmp2a)>>1);
      data.uc[ctr-5] = ((tmpa+tmpa+tmpa+tmp2a)>>2);
      data.uc[ctr-4] = tmp2a; tmpa = tmp2a;
      }
    freq = fr;
    delete [] odata.uc; odata.v = data.v;
    }
  else if(bits == 8 && freq == (fr>>2))  {
    len = (len<<2)-3;
    data.uc = new unsigned char[len];
    unsigned long tmp2, tmp = odata.uc[0];
    data.uc[0] = tmp;
    for(ctr=4; ctr<(int)len; ctr+=4)  {
      tmp2 = odata.uc[ctr>>2];
      data.uc[ctr-3] = ((tmp+tmp2+tmp2+tmp2)>>2);
      data.uc[ctr-2] = ((tmp+tmp2)>>1);
      data.uc[ctr-1] = ((tmp+tmp+tmp+tmp2)>>2);
      data.uc[ctr] = tmp2; tmp = tmp2;
      }
    freq = fr;
    delete [] odata.uc; odata.v = data.v;
    }
  else if(bits == 16 && stereo && freq == (fr<<2))  {
    len >>= 2;
    data.s = new short[len>>1];
    for(ctr=0; ctr<(int)(len>>1); ctr+=2)  {
      data.s[ctr] = odata.s[ctr<<2];
      data.s[ctr+1] = odata.s[(ctr<<2)+1];
      }
    freq = fr;
    delete [] odata.s; odata.v = data.v;
    }
  else if(bits == 16 && freq == (fr<<2))  {
    len >>= 2;
    data.s = new short[len>>1];
    for(ctr=0; ctr<(int)(len>>1); ctr++)  {
      data.s[ctr] = odata.s[ctr<<2];
      }
    freq = fr;
    delete [] odata.s; odata.v = data.v;
    }
  else if(bits == 16 && stereo && freq == (fr<<1))  {
    len >>= 1;
    data.s = new short[len>>1];
    for(ctr=0; ctr<(int)(len>>1); ctr+=2)  {
      data.s[ctr] = odata.s[ctr<<1];
      data.s[ctr+1] = odata.s[(ctr<<1)+1];
      }
    freq = fr;
    delete [] odata.s; odata.v = data.v;
    }
  else if(bits == 16 && freq == (fr<<1))  {
    len >>= 1;
    data.s = new short[len>>1];
    for(ctr=0; ctr<(int)(len>>1); ctr++)  {
      data.s[ctr] = odata.s[ctr<<1];
      }
    freq = fr;
    delete [] odata.s; odata.v = data.v;
    }
  else if(bits == 16 && stereo && freq == (fr>>1))  {
    len = (len<<1)-4;
    data.s = new short[len>>1];
    short tmp2a, tmpa = odata.s[0];
    short tmp2b, tmpb = odata.s[1];
    data.s[0] = tmpa;
    data.s[1] = tmpb;
    for(ctr=4; ctr<(int)(len>>1); ctr+=4)  {
      tmp2a = odata.s[(ctr>>1)];
      tmp2b = odata.s[(ctr>>1)+1];
      data.s[ctr-1] = (tmpa+tmp2a)>>1;
      data.s[ctr] = tmp2a; tmpa = tmp2a;
      data.s[ctr-3] = (tmpb+tmp2b)>>1;
      data.s[ctr-2] = tmp2b; tmpb = tmp2b;
      }
    freq = fr;
    delete [] odata.s; odata.v = data.v;
    }
  else if(bits == 16 && freq == (fr>>1))  {
    len = (len<<1)-2;
    data.s = new short[len>>1];
    short tmp2, tmp = odata.s[0];
    data.s[0] = tmp;
    for(ctr=2; ctr<(int)(len>>1); ctr+=2)  {
      tmp2 = odata.s[ctr>>1];
      data.s[ctr-1] = (tmp+tmp2)>>1;
      data.s[ctr] = tmp2; tmp = tmp2;
      }
    freq = fr;
    delete [] odata.s; odata.v = data.v;
    }
  else if(bits == 16 && stereo && freq == (fr>>2))  {
    len = (len<<2)-12;
    data.s = new short[len>>1];
    long tmp2a, tmpa = odata.s[0];
    long tmp2b, tmpb = odata.s[1];
    data.s[0] = tmpa;
    data.s[1] = tmpb;
    for(ctr=8; ctr<(int)(len>>1); ctr+=8)  {
      tmp2a = odata.s[(ctr>>2)];
      tmp2b = odata.s[(ctr>>2)+1];
      data.s[ctr-3] = (tmpa+tmp2a+tmp2a+tmp2a)>>2;
      data.s[ctr-2] = (tmpa+tmp2a)>>1;
      data.s[ctr-1] = (tmpa+tmpa+tmpa+tmp2a)>>2;
      data.s[ctr] = tmp2a; tmpa = tmp2a;
      data.s[ctr-7] = (tmpb+tmp2b+tmp2b+tmp2b)>>2;
      data.s[ctr-6] = (tmpb+tmp2b)>>1;
      data.s[ctr-5] = (tmpb+tmpb+tmpb+tmp2b)>>2;
      data.s[ctr-4] = tmp2b; tmpb = tmp2b;
      }
    freq = fr;
    delete [] odata.s; odata.v = data.v;
    }
  else if(bits == 16 && freq == (fr>>2))  {
    len = (len<<2)-6;
    data.s = new short[len>>1];
    long tmp2, tmp = odata.s[0];
    data.s[0] = tmp;
    for(ctr=4; ctr<(int)(len>>1); ctr+=4)  {
      tmp2 = odata.s[ctr>>2];
      data.s[ctr-3] = (tmp+tmp2+tmp2+tmp2)>>2;
      data.s[ctr-2] = (tmp+tmp2)>>1;
      data.s[ctr-1] = (tmp+tmp+tmp+tmp2)>>2;
      data.s[ctr] = tmp2; tmp = tmp2;
      }
    freq = fr;
    delete [] odata.s; odata.v = data.v;
    }
  UserDebug("User:Sound:ConvertTo End");
  }
