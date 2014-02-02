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

/* Be sure to keep this EXACTLY the same length if changed! */
#define FILE_HEADER	"Insomnia's User Engine 2.01 Resource File!\n"

/* Older headers for backward-compatibility detection */
#define O200_HEADER	"Insomnia's User Engine 2.0 Resource File!\n\0"

/* Tags for Resfile element types */
#define GRAPHIC_TAG	"GRAP"
#define PALETTE_TAG	"PALT"
#define SOUND_TAG	"DIGS"
#define LONGBAG_TAG	"LBAG"
#define INTBAG_TAG	"IBAG"
#define SHORTBAG_TAG	"SBAG"
#define CHARBAG_TAG	"CBAG"
#define NULL_TAG	"NULL"
#define TAG_SIZE	4

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
  rf = U2_FOpenRead(fn);
  if(rf == NULL)  {
    U2_Exit(1, "\"%s\" not found!\n", filen);
    }
  if(U2_FRead(buffer, 1, sizeof(FILE_HEADER), rf) < (long)sizeof(FILE_HEADER)) {
    U2_Exit(1, "Failed to read ResFile header from \"%s\".\n", fn);
    }
  else if(!strncmp(buffer, FILE_HEADER, sizeof(FILE_HEADER)))  {
    version = 0x0201;
    }
  else if(!strncmp(buffer, O200_HEADER, sizeof(FILE_HEADER)))  {
    fprintf(stderr, "%s: Version 2.0 - BW Compat Enabled.\n", fn);
    version = 0x0200;
    }
  else {
    buffer[sizeof(FILE_HEADER)-2] = 0;
    U2_Exit(1, "\"%s\" is not a resource file, it's header is \"%s\"!\n", fn,
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
  rf = U2_FOpenRead(fn);
  if(rf == NULL)  {
    delete fn;
    return 0;
    }
  return 1;
  }

void *ResFile::Get()  {
  char buf[TAG_SIZE];
  Read(buf, TAG_SIZE);
  if(version == 0x0200) {
    char tmp;
    Read(&tmp, 1);
    if(tmp != 0) U2_Exit(-1, "BW Compat: Bad 2.0 Resfile.\n");
    }
  if(!strncmp(buf, GRAPHIC_TAG, TAG_SIZE))  {
    return GrabGraphic();
    }
  if(!strncmp(buf, SOUND_TAG, TAG_SIZE))  {
    return GrabSound();
    }
  else  {
    if(!(strncmp(buf, NULL_TAG, TAG_SIZE)))  return NULL;
    U2_Exit(1, "Bad ResFile order, attempt to get Graphic on non-Graphic");
    }
  }

LongBag *ResFile::GetLongBag()  {
  char buf[TAG_SIZE];
  Read(buf, TAG_SIZE);
  if(version == 0x0200) {
    char tmp;
    Read(&tmp, 1);
    if(tmp != 0) U2_Exit(-1, "BW Compat: Bad 2.0 Resfile.\n");
    }
  if(strncmp(buf, LONGBAG_TAG, TAG_SIZE))  {
    if(!(strncmp(buf, NULL_TAG, TAG_SIZE)))  return NULL;
    U2_Exit(1, "Bad ResFile order, attempt to get LongBag on non-LongBag");
    }
  return GrabLongBag();
  }

IntBag *ResFile::GetIntBag()  {
  char buf[TAG_SIZE];
  Read(buf, TAG_SIZE);
  if(version == 0x0200) {
    char tmp;
    Read(&tmp, 1);
    if(tmp != 0) U2_Exit(-1, "BW Compat: Bad 2.0 Resfile.\n");
    }
  if(strncmp(buf, INTBAG_TAG, TAG_SIZE))  {
    if(!(strncmp(buf, NULL_TAG, TAG_SIZE)))  return NULL;
    U2_Exit(1, "Bad ResFile order, attempt to get IntBag on non-IntBag");
    }
  return GrabIntBag();
  }

ShortBag *ResFile::GetShortBag()  {
  char buf[TAG_SIZE];
  Read(buf, TAG_SIZE);
  if(version == 0x0200) {
    char tmp;
    Read(&tmp, 1);
    if(tmp != 0) U2_Exit(-1, "BW Compat: Bad 2.0 Resfile.\n");
    }
  if(strncmp(buf, SHORTBAG_TAG, TAG_SIZE))  {
    if(!(strncmp(buf, NULL_TAG, TAG_SIZE)))  return NULL;
    U2_Exit(1, "Bad ResFile order, attempt to get ShortBag on non-ShortBag");
    }
  return GrabShortBag();
  }

CharBag *ResFile::GetCharBag()  {
  char buf[TAG_SIZE];
  Read(buf, TAG_SIZE);
  if(version == 0x0200) {
    char tmp;
    Read(&tmp, 1);
    if(tmp != 0) U2_Exit(-1, "BW Compat: Bad 2.0 Resfile.\n");
    }
  if(strncmp(buf, CHARBAG_TAG, TAG_SIZE))  {
    if(!(strncmp(buf, NULL_TAG, TAG_SIZE)))  return NULL;
    U2_Exit(1, "Bad ResFile order, attempt to get CharBag on non-CharBag");
    }
  return GrabCharBag();
  }

Graphic *ResFile::GetGraphic()  {
  char buf[TAG_SIZE];
//  printf("Reading Tag\n"); fflush(stdout);
  Read(buf, TAG_SIZE);
  if(version == 0x0200) {
    char tmp;
    Read(&tmp, 1);
    if(tmp != 0) U2_Exit(-1, "BW Compat: Bad 2.0 Resfile.\n");
    }
//  printf("Read Tag \"%c%c%c%c\"\n", buf[0], buf[1], buf[2], buf[3]); fflush(stdout);
  if(strncmp(buf, GRAPHIC_TAG, TAG_SIZE))  {
    if(!(strncmp(buf, NULL_TAG, TAG_SIZE)))  return NULL;
    U2_Exit(1, "Bad ResFile order, attempt to get Graphic on non-Graphic");
    }
//  printf("Grabbing Graphic\n"); fflush(stdout);
  return GrabGraphic();
//  printf("Grabbed Graphic\n"); fflush(stdout);
  }

Palette *ResFile::GetPalette()  {
  char buf[TAG_SIZE];
  Read(buf, TAG_SIZE);
  if(version == 0x0200) {
    char tmp;
    Read(&tmp, 1);
    if(tmp != 0) U2_Exit(-1, "BW Compat: Bad 2.0 Resfile.\n");
    }
  if(strncmp(buf, PALETTE_TAG, TAG_SIZE))  {
    if(!(strncmp(buf, NULL_TAG, TAG_SIZE)))  return NULL;
    U2_Exit(1, "Bad ResFile order, attempt to get Palette on non-Palette");
    }
  return GrabPalette();
  }

Sound *ResFile::GetSound()  {
  char buf[TAG_SIZE];
  Read(buf, TAG_SIZE);
  if(version == 0x0200) {
    char tmp;
    Read(&tmp, 1);
    if(tmp != 0) U2_Exit(-1, "BW Compat: Bad 2.0 Resfile.\n");
    }
  if(strncmp(buf, SOUND_TAG, TAG_SIZE))  {
    if(!(strncmp(buf, NULL_TAG, TAG_SIZE)))  return NULL;
    U2_Exit(1, "Bad ResFile order, attempt to get Sound on non-Sound");
    }
  return GrabSound();
  }

LongBag *ResFile::GrabLongBag()  {
  LongBag *ret;
  int sz, ctr;
  sz = ReadLong();
  ret = new LongBag(sz);
  for(ctr=0; ctr<sz; ctr++)  {
    (*(LongBag*)ret)[ctr] = ReadLong();
    }
  return ret;
  }

IntBag *ResFile::GrabIntBag()  {
  IntBag *ret;
  int sz, ctr;
  sz = ReadInt();
  ret = new IntBag(sz);
  for(ctr=0; ctr<sz; ctr++)  {
    (*(IntBag*)ret)[ctr] = ReadInt();
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
    U2_Exit(1, "Out of Memory for Graphic!\r\n");
    }

  ret->depth=dp;	ret->tcolor=tc;
  ret->DefSize(xs, ys);
  ret->xcenter=xc;	ret->ycenter=yc;

  for(ctr=0; ctr<ys; ctr++)  {
    Read(ret->image[ctr].v, xs*(dp>>3));
    }
  return ret;
  }

Palette *ResFile::GrabPalette()  {
  Palette *ret;
  ret = new Palette;
  if(ret == NULL)  {
    U2_Exit(1, "Out of Memory for Palette!\r\n");
    }
  ret->coldec = ReadInt(); 
  Read(ret->colors, ((long)ret->coldec)*3);
  return ret;
  }

Sound *ResFile::GrabSound()  {
  Sound *ret = new Sound;
  if(ret == NULL)  {
    U2_Exit(1, "Out of Memory for Sound Sample!\r\n");
    }
  ret->len = ReadInt();
  ret->freq = ReadInt();
  ret->bits = ReadChar();
  ret->channels = ReadChar();
  ret->data.u8 = new unsigned char[ret->len];
  Read(ret->data.u8, ret->len);
  return ret;
  }

void ResFile::Read(void *data, int ammt)  {
  if(U2_FRead(data, 1, ammt, rf) != (unsigned int)ammt)  {
    perror("User:ResFile:Read");
    U2_Exit(1, "Read failure on \"%s\"!\n", fn);
    }
  }

long ResFile::ReadLong()  {
  unsigned char tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;
  if(U2_FRead(&tmp1, 1, 1, rf) != 1) U2_Exit(1, "Read failure on \"%s\"!\n",fn);
  if(U2_FRead(&tmp2, 1, 1, rf) != 1) U2_Exit(1, "Read failure on \"%s\"!\n",fn);
  if(U2_FRead(&tmp3, 1, 1, rf) != 1) U2_Exit(1, "Read failure on \"%s\"!\n",fn);
  if(U2_FRead(&tmp4, 1, 1, rf) != 1) U2_Exit(1, "Read failure on \"%s\"!\n",fn);
  if(U2_FRead(&tmp5, 1, 1, rf) != 1) U2_Exit(1, "Read failure on \"%s\"!\n",fn);
  if(U2_FRead(&tmp6, 1, 1, rf) != 1) U2_Exit(1, "Read failure on \"%s\"!\n",fn);
  if(U2_FRead(&tmp7, 1, 1, rf) != 1) U2_Exit(1, "Read failure on \"%s\"!\n",fn);
  if(U2_FRead(&tmp8, 1, 1, rf) != 1) U2_Exit(1, "Read failure on \"%s\"!\n",fn);
  return (((long)(tmp8))<<56)
       + (((long)(tmp7))<<48)
       + (((long)(tmp6))<<40)
       + (((long)(tmp5))<<32)
       + (((long)(tmp4))<<24)
       + (((long)(tmp3))<<16)
       + (((long)(tmp2))<<8)
       + (((long)(tmp1))<<0);
  }

int ResFile::ReadInt()  {
  unsigned char tmp1, tmp2, tmp3, tmp4;
  if(U2_FRead(&tmp1, 1, 1, rf) != 1) U2_Exit(1, "Read failure on \"%s\"!\n",fn);
  if(U2_FRead(&tmp2, 1, 1, rf) != 1) U2_Exit(1, "Read failure on \"%s\"!\n",fn);
  if(U2_FRead(&tmp3, 1, 1, rf) != 1) U2_Exit(1, "Read failure on \"%s\"!\n",fn);
  if(U2_FRead(&tmp4, 1, 1, rf) != 1) U2_Exit(1, "Read failure on \"%s\"!\n",fn);
  return (((long)(tmp4))<<24)
       + (((long)(tmp3))<<16)
       + (((long)(tmp2))<<8)
       + (((long)(tmp1))<<0);
  }

short ResFile::ReadShort()  {
  unsigned char tmp1, tmp2;
  if(U2_FRead(&tmp1, 1, 1, rf) != 1) U2_Exit(1, "Read failure on \"%s\"!\n",fn);
  if(U2_FRead(&tmp2, 1, 1, rf) != 1) U2_Exit(1, "Read failure on \"%s\"!\n",fn);
  return (((long)(tmp2))<<8)
       + (((long)(tmp1))<<0);
  }

char ResFile::ReadChar()  {
  unsigned char ret;
  if(U2_FRead(&ret, 1, 1, rf) != 1) U2_Exit(1, "Read failure on \"%s\"!\n",fn);
  return ret;
  }

ResFile::~ResFile()  {
  delete fn;
  U2_FClose(rf);
  }

NewResFile::NewResFile(const char *filen)  {
  fn = new char[strlen(filen)+1];
  strcpy(fn, filen);
  rf = U2_FOpenWrite(fn);
  if(rf == NULL)  {
    U2_Exit(1, "Cannot create \"%s\"!\n", fn);
    }
  if(U2_FWrite(FILE_HEADER, 1, sizeof(FILE_HEADER), rf)
	< (size_t)sizeof(FILE_HEADER)) {
    U2_Exit(1, "Not enough drive space for \"%s\"!\n", fn);
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
    WriteLong((*(LongBag*)in)[ctr]);
    }
  }

void NewResFile::Add(const IntBag *in)  {
  if(in == NULL)  {
    Write(NULL_TAG, TAG_SIZE);
    return;
    }
  int ctr;
  Write(INTBAG_TAG, TAG_SIZE);
  WriteInt(in->Size());
  for(ctr=0; ctr<in->Size(); ctr++)  {
    WriteInt((*(IntBag*)in)[ctr]);
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
    Write(in->image[ctr].v, in->xsize*((in->depth)>>3));
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
  WriteChar(in->channels);
  Write(in->data.u8, in->len);
  } 

NewResFile::~NewResFile()  {
  delete fn;
  U2_FClose(rf);
  }

void NewResFile::Write(const void *data, int ammt)  {
  if(U2_FWrite(data, 1, ammt, rf) != (size_t)ammt)  {
    U2_Exit(1, "Write failure on \"%s\"!\n", fn);
    }
  }

void NewResFile::WriteLong(long data)  {
  unsigned char tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8;
  tmp8 = (data>>56) & 255;
  tmp7 = (data>>48) & 255;
  tmp6 = (data>>40) & 255;
  tmp5 = (data>>32) & 255;
  tmp4 = (data>>24) & 255;
  tmp3 = (data>>16) & 255;
  tmp2 = (data>>8) & 255;
  tmp1 = data & 255;
  if(U2_FPutC(tmp1, rf) < 0) U2_Exit(1,"Write failure on \"%s\"!\n",fn); 
  if(U2_FPutC(tmp2, rf) < 0) U2_Exit(1,"Write failure on \"%s\"!\n",fn); 
  if(U2_FPutC(tmp3, rf) < 0) U2_Exit(1,"Write failure on \"%s\"!\n",fn); 
  if(U2_FPutC(tmp4, rf) < 0) U2_Exit(1,"Write failure on \"%s\"!\n",fn); 
  if(U2_FPutC(tmp5, rf) < 0) U2_Exit(1,"Write failure on \"%s\"!\n",fn); 
  if(U2_FPutC(tmp6, rf) < 0) U2_Exit(1,"Write failure on \"%s\"!\n",fn); 
  if(U2_FPutC(tmp7, rf) < 0) U2_Exit(1,"Write failure on \"%s\"!\n",fn); 
  if(U2_FPutC(tmp8, rf) < 0) U2_Exit(1,"Write failure on \"%s\"!\n",fn); 
  }

void NewResFile::WriteInt(int data)  {
  unsigned char tmp1, tmp2, tmp3, tmp4;
  tmp4 = (data>>24) & 255;
  tmp3 = (data>>16) & 255;
  tmp2 = (data>>8) & 255;
  tmp1 = data & 255;
  if(U2_FPutC(tmp1, rf) < 0) U2_Exit(1,"Write failure on \"%s\"!\n",fn); 
  if(U2_FPutC(tmp2, rf) < 0) U2_Exit(1,"Write failure on \"%s\"!\n",fn); 
  if(U2_FPutC(tmp3, rf) < 0) U2_Exit(1,"Write failure on \"%s\"!\n",fn); 
  if(U2_FPutC(tmp4, rf) < 0) U2_Exit(1,"Write failure on \"%s\"!\n",fn); 
  }

void NewResFile::WriteShort(short data)  {
  unsigned char tmp1, tmp2;
  tmp2 = (data>>8) & 255;
  tmp1 = data & 255;
  if(U2_FPutC(tmp1, rf) < 0) U2_Exit(1,"Write failure on \"%s\"!\n",fn); 
  if(U2_FPutC(tmp2, rf) < 0) U2_Exit(1,"Write failure on \"%s\"!\n",fn); 
  }

void NewResFile::WriteChar(char data)  {
  if(U2_FPutC(data, rf) < 0) U2_Exit(1,"Write failure on \"%s\"!\n",fn); 
  }
