#include <stdio.h>

#include "chunk.h"

mfmt Chunk::ch[CHUNK_MAX];

void Chunk::DeleteAll() {
  int size;
  for(size=0; size<CHUNK_MAX; ++size) {
    while(ch[size].ul) {
      unsigned long *tmp;
      tmp = ch[size].ul;
      ch[size].c = (char *)(*tmp);
      delete tmp;
      }
    }
  }
