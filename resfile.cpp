#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "engine.h"
#include "screen.h"
#include "resfile.h"
#include "bag.h"
#include "sound.h"
#include "palette.h"
#include "graphic.h"

#define FILE_HEADER	"Insomnia's User Engine 2.0 Resource File!\n\0"
#define GRAPHIC_TAG	"GRAP"
#define PALETTE_TAG	"PALT"
#define SOUND_TAG	"DIGS"
#define LONGBAG_TAG	"LBAG"
#define SHORTBAG_TAG	"SBAG"
#define CHARBAG_TAG	"CBAG"
#define NULL_TAG	"NULL"
#define TAG_SIZE	(sizeof(GRAPHIC_TAG))


ResFile::ResFile(const char *filen)  {
  Open(filen, "");
  }

ResFile::ResFile(const char *filen, const char *com)  {
  Open(filen, com);
  }

void ResFile::Open(const char *filen, const char *com)  {
  fn = new char[strlen(filen)+1];
  strcpy(fn, filen);
  char buffer[256];
  rf = fopen(fn, "rb");
  if(rf == NULL)  {
//    printf("Tried \"%s\"\n", fn);
    delete fn;
    TryToOpen(filen, com);
    if(rf == NULL)  {
      char *tmpp1, *tmpp2, *path, delim = ':';
      int fail = 0;
      tmpp2 = getenv("PATH");
      path = new char[strlen(tmpp2)+1];
      tmpp1 = path;
      strcpy(path, tmpp2);
      while((!fail) && rf == NULL)  {
	tmpp2 = tmpp1;
	while(*tmpp2 != delim && *tmpp2 != 0) tmpp2++;
	if(*tmpp2 == delim)  {
	  char tmpc = tmpp2[1];
	  *tmpp2 = '/';
	  tmpp2[1] = 0;
	  TryToOpen(filen, tmpp1);
	  tmpp2[1] = tmpc;
	  }
	else  {
	  tmpp2 = tmpp1;
	  fail = 1;
	  }
	tmpp1 = ++tmpp2;
	}
      delete path;
      if(rf == NULL)  {
	Exit(1, "\"%s\" not found!\n", filen);
	}
      }
    }
  if(read(fileno(rf), buffer, sizeof(FILE_HEADER)) < (long)sizeof(FILE_HEADER)
	|| strncmp(buffer, FILE_HEADER, sizeof(FILE_HEADER)))  {
    Exit(1, "\"%s\" is not a resource file, it's header is \"%s\"!\n", fn,
	buffer);
    }
  }

int ResFile::TryToOpen(const char *filen, const char *com)  {
  int pos;
  fn = new char[strlen(filen)+strlen(com)+1];
  sprintf(fn, "%s%c", com, 0);
  for(pos=strlen(fn)-1; pos>=0 && fn[pos]!='/' && fn[pos]!='\\'; pos--);
  if(pos >= 0)  fn[pos] = 0;
  strcat(fn, "/");
  strcat(fn, filen);
  rf = fopen(fn, "rb");
  if(rf == NULL)  {
    delete fn;
    return 0;
    }
  return 1;
  }

void *ResFile::Get()  {
  char buf[TAG_SIZE];
  Read(buf, TAG_SIZE);
  if(!strncmp(buf, GRAPHIC_TAG, TAG_SIZE))  {
    return GrabGraphic();
    }
  if(!strncmp(buf, SOUND_TAG, TAG_SIZE))  {
    return GrabSound();
    }
  else  {
    if(!(strncmp(buf, NULL_TAG, TAG_SIZE)))  return NULL;
    Exit(1, "Bad ResFile order, attempt to get Graphic on non-Graphic");
    }
  }

LongBag *ResFile::GetLongBag()  {
  char buf[TAG_SIZE];
  Read(buf, TAG_SIZE);
  if(strncmp(buf, LONGBAG_TAG, TAG_SIZE))  {
    if(!(strncmp(buf, NULL_TAG, TAG_SIZE)))  return NULL;
    Exit(1, "Bad ResFile order, attempt to get LongBag on non-LongBag");
    }
  return GrabLongBag();
  }

ShortBag *ResFile::GetShortBag()  {
  char buf[TAG_SIZE];
  Read(buf, TAG_SIZE);
  if(strncmp(buf, SHORTBAG_TAG, TAG_SIZE))  {
    if(!(strncmp(buf, NULL_TAG, TAG_SIZE)))  return NULL;
    Exit(1, "Bad ResFile order, attempt to get ShortBag on non-ShortBag");
    }
  return GrabShortBag();
  }

CharBag *ResFile::GetCharBag()  {
  char buf[TAG_SIZE];
  Read(buf, TAG_SIZE);
  if(strncmp(buf, CHARBAG_TAG, TAG_SIZE))  {
    if(!(strncmp(buf, NULL_TAG, TAG_SIZE)))  return NULL;
    Exit(1, "Bad ResFile order, attempt to get CharBag on non-CharBag");
    }
  return GrabCharBag();
  }

Graphic *ResFile::GetGraphic()  {
  char buf[TAG_SIZE];
//  printf("Going\n");
  Read(buf, TAG_SIZE);
//  printf("Going2\n");
  if(strncmp(buf, GRAPHIC_TAG, TAG_SIZE))  {
    if(!(strncmp(buf, NULL_TAG, TAG_SIZE)))  return NULL;
    Exit(1, "Bad ResFile order, attempt to get Graphic on non-Graphic");
    }
  return GrabGraphic();
  }

Palette *ResFile::GetPalette()  {
  char buf[TAG_SIZE];
  Read(buf, TAG_SIZE);
  if(strncmp(buf, PALETTE_TAG, TAG_SIZE))  {
    if(!(strncmp(buf, NULL_TAG, TAG_SIZE)))  return NULL;
    Exit(1, "Bad ResFile order, attempt to get Palette on non-Palette");
    }
  return GrabPalette();
  }

Sound *ResFile::GetSound()  {
  char buf[TAG_SIZE];
  Read(buf, TAG_SIZE);
  if(strncmp(buf, SOUND_TAG, TAG_SIZE))  {
    if(!(strncmp(buf, NULL_TAG, TAG_SIZE)))  return NULL;
    Exit(1, "Bad ResFile order, attempt to get Sound on non-Sound");
    }
  return GrabSound();
  }

LongBag *ResFile::GrabLongBag()  {
  LongBag *ret;
  int sz, ctr;
  sz = ReadInt();
  ret = new LongBag(sz);
  for(ctr=0; ctr<sz; ctr++)  {
    (*(LongBag*)ret)[ctr] = ReadInt();
    }
  return ret;
  }

ShortBag *ResFile::GrabShortBag()  {
  ShortBag *ret;
  int sz, ctr;
  sz = ReadInt();
  ret = new ShortBag(sz);
  for(ctr=0; ctr<sz; ctr++)  {
    (*(ShortBag*)ret)[ctr] = ReadShort();
    }
  return ret;
  }

CharBag *ResFile::GrabCharBag()  {
  CharBag *ret;
  int sz, ctr;
  sz = ReadInt();
  ret = new CharBag(sz);
  for(ctr=0; ctr<sz; ctr++)  {
    (*(CharBag*)ret)[ctr] = ReadChar();
    }
  return ret;
  }

Graphic *ResFile::GrabGraphic()  {
  Graphic *ret;
  int ctr, xc, yc, xs, ys, tc, dp;
  xc = ReadInt();
  yc = ReadInt();
  xs = ReadInt();
  ys = ReadInt();
  dp = ReadInt();
  tc = ReadInt();

  ret = new Graphic;
  if(ret == NULL)  {
    Exit(1, "Out of Memory for Graphic!\r\n");
    }

  ret->depth=dp;	ret->tcolor=tc;
  ret->DefSize(xs, ys);
  ret->xcenter=xc;	ret->ycenter=yc;

  unsigned char buff[xs+2];
//  printf("Depth = %d\n", dp);
  for(ctr=0; ctr<ys; ctr++)  {
    Read(buff, xs*(dp>>3));
    ret->DefLin((char *)buff);
    }
  return ret;
  }

Palette *ResFile::GrabPalette()  {
  Palette *ret;
  ret = new Palette;
  if(ret == NULL)  {
    Exit(1, "Out of Memory for Palette!\r\n");
    }
  ret->coldec = ReadInt(); 
  Read(ret->colors, ((long)ret->coldec)*3);
  return ret;
  }

Sound *ResFile::GrabSound()  {
  Sound *ret = new Sound;
  if(ret == NULL)  {
    Exit(1, "Out of Memory for Sound Sample!\r\n");
    }
  ret->len = ReadInt();
  ret->freq = ReadInt();
  ret->bits = ReadChar();
  ret->stereo = ReadChar();
  ret->data.uc = new unsigned char[ret->len];
  Read(ret->data.uc, ret->len);
  return ret;
  }

void ResFile::Read(void *data, int ammt)  {
  if(read(fileno(rf), data, ammt) != ammt)  {
    perror("User:ResFile:Read");
    Exit(1, "Read failure on \"%s\"!\n", fn);
    }
  }

int ResFile::ReadInt()  {
  unsigned char tmp1, tmp2, tmp3, tmp4;
  if(read(fileno(rf), &tmp1, 1) != 1) Exit(1, "Read failure on \"%s\"!\n",fn); 
  if(read(fileno(rf), &tmp2, 1) != 1) Exit(1, "Read failure on \"%s\"!\n",fn); 
  if(read(fileno(rf), &tmp3, 1) != 1) Exit(1, "Read failure on \"%s\"!\n",fn); 
  if(read(fileno(rf), &tmp4, 1) != 1) Exit(1, "Read failure on \"%s\"!\n",fn); 
  return((tmp4<<24)+(tmp3<<16)+(tmp2<<8)+tmp1);
  }

short ResFile::ReadShort()  {
  unsigned char tmp1, tmp2;
  if(read(fileno(rf), &tmp1, 1) != 1) Exit(1, "Read failure on \"%s\"!\n",fn); 
  if(read(fileno(rf), &tmp2, 1) != 1) Exit(1, "Read failure on \"%s\"!\n",fn); 
  return((tmp2<<8)+tmp1);
  }

char ResFile::ReadChar()  {
  unsigned char ret;
  if(read(fileno(rf), &ret, 1) != 1) Exit(1, "Read failure on \"%s\"!\n",fn); 
  return ret;
  }

ResFile::~ResFile()  {
  delete fn;
  fclose(rf);
  }

NewResFile::NewResFile(const char *filen)  {
  fn = new char[strlen(filen)+1];
  strcpy(fn, filen);
  rf = fopen(fn, "wb");
  if(rf == NULL)  {
    Exit(1, "Cannot create \"%s\"!\n", fn);
    }
  if(write(fileno(rf), FILE_HEADER, sizeof(FILE_HEADER))
	< (long)sizeof(FILE_HEADER)) {
    Exit(1, "Not enough drive space for \"%s\"!\n", fn);
    }
  }

void NewResFile::Add(const LongBag *in)  {
  if(in == NULL)  {
    Write(NULL_TAG, TAG_SIZE);
    return;
    }
  int ctr;
  Write(LONGBAG_TAG, TAG_SIZE);
  WriteInt(in->Size());
  for(ctr=0; ctr<in->Size(); ctr++)  {
    WriteInt((*(LongBag*)in)[ctr]);
    }
  }

void NewResFile::Add(const ShortBag *in)  {
  if(in == NULL)  {
    Write(NULL_TAG, TAG_SIZE);
    return;
    }
  int ctr;
  Write(SHORTBAG_TAG, TAG_SIZE);
  WriteInt(in->Size());
  for(ctr=0; ctr<in->Size(); ctr++)  {
    WriteShort((*(ShortBag*)in)[ctr]);
    }
  }

void NewResFile::Add(const CharBag *in)  {
  if(in == NULL)  {
    Write(NULL_TAG, TAG_SIZE);
    return;
    }
  int ctr;
  Write(CHARBAG_TAG, TAG_SIZE);
  WriteInt(in->Size());
  for(ctr=0; ctr<in->Size(); ctr++)  {
    WriteChar((*(CharBag*)in)[ctr]);
    }
  }

void NewResFile::Add(const Graphic *in)  {
  if(in == NULL)  {
    Write(NULL_TAG, TAG_SIZE);
    return;
    }
  int ctr;
  Write(GRAPHIC_TAG, TAG_SIZE);
  WriteInt(in->xcenter);
  WriteInt(in->ycenter);
  WriteInt(in->xsize);
  WriteInt(in->ysize);
  WriteInt(in->depth);
  WriteInt(in->tcolor);
  for(ctr=0; ctr<(long)in->ysize; ctr++)  {
    Write(in->image[ctr], in->xsize*((in->depth)>>3));
    }
  }

void NewResFile::Add(const Palette *in)  {
  if(in == NULL)  {
    Write(NULL_TAG, TAG_SIZE);
    return;
    }
  Write(PALETTE_TAG, TAG_SIZE);
  WriteInt(in->coldec);
  Write(in->colors, ((long)in->coldec)*3);
  }

void NewResFile::Add(const Sound *in)  {
  if(in == NULL)  {
    Write(NULL_TAG, TAG_SIZE);
    return;
    }
  Write(SOUND_TAG, TAG_SIZE);
  WriteInt(in->len);
  WriteInt(in->freq);
  WriteChar(in->bits);
  WriteChar(in->stereo);
  Write(in->data.uc, in->len);
  } 

NewResFile::~NewResFile()  {
  delete fn;
  fclose(rf);
  }

void NewResFile::Write(const void *data, int ammt)  {
  if(write(fileno(rf), data, ammt) != ammt)  {
    Exit(1, "Write failure on \"%s\"!\n", fn);
    }
  }

void NewResFile::WriteInt(int data)  {
  unsigned char tmp1, tmp2, tmp3, tmp4;
  tmp4 = (data>>24) & 255;
  tmp3 = (data>>16) & 255;
  tmp2 = (data>>8) & 255;
  tmp1 = data & 255;
  if(write(fileno(rf), &tmp1, 1) != 1) Exit(1,"Write failure on \"%s\"!\n",fn); 
  if(write(fileno(rf), &tmp2, 1) != 1) Exit(1,"Write failure on \"%s\"!\n",fn); 
  if(write(fileno(rf), &tmp3, 1) != 1) Exit(1,"Write failure on \"%s\"!\n",fn); 
  if(write(fileno(rf), &tmp4, 1) != 1) Exit(1,"Write failure on \"%s\"!\n",fn); 
  }

void NewResFile::WriteShort(short data)  {
  unsigned char tmp1, tmp2;
  tmp2 = (data>>8) & 255;
  tmp1 = data & 255;
  if(write(fileno(rf), &tmp1, 1) != 1) Exit(1,"Write failure on \"%s\"!\n",fn); 
  if(write(fileno(rf), &tmp2, 1) != 1) Exit(1,"Write failure on \"%s\"!\n",fn); 
  }

void NewResFile::WriteChar(char data)  {
  if(write(fileno(rf), &data, 1) != 1) Exit(1,"Write failure on \"%s\"!\n",fn); 
  }
