#ifndef INSOMNIA_USER_MFMT_H
#define INSOMNIA_USER_MFMT_H

union mfmt {
  void *v;
  signed char *s8;
  signed short *s16;
  signed int *s32;
  signed long *s64;
  signed long L;
  unsigned char *u8;
  unsigned short *u16;
  unsigned int *u32;
  unsigned long *u64;
  unsigned long UL;
  };

#endif
