#include <stdio.h>

#include "chunk.h"

mfmt Chunk::ch[CHUNK_MAX];

void Chunk::DeleteAll() {
  int size;
  for(size=0; size<CHUNK_MAX; ++size) {
    while(ch[size].u32) {
      unsigned int *tmp;
      tmp = ch[size].u32;
      ch[size].u8 = (unsigned char *)(*tmp);
      delete tmp;
      }
    }
  }
