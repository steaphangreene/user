#ifndef INSOMNIA_USER_MFMT_H
#define INSOMNIA_USER_MFMT_H

union mfmt {
  void *v;
  char *c;
  short *s;
  long *l;
  unsigned char *uc;
  unsigned short *us;
  unsigned long *ul;
  };

#endif
