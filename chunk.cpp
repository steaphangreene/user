#include <stdio.h>

#include "chunk.h"

mfmt Chunk::ch[CHUNK_MAX];

void Chunk::DeleteAll() {
  int size;
  for(size=0; size<CHUNK_MAX; ++size) {
    while(ch[size].u8) {
      unsigned char *tmp;
      tmp = ch[size].u8;
      ch[size].u8 = (unsigned char *)(*tmp);
      delete [] tmp;
      }
    }
  }
