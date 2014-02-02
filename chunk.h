#ifndef INSOMNIA_USER_CHUNK_H
#define INSOMNIA_USER_CHUNK_H

#include <stdio.h>

#include "config.h"
#include "mfmt.h"

class Chunk {
  public:
  Chunk() { data.s8 = NULL; size = 0; };
  Chunk(int sz) { data.s8 = NULL; size = 0; Get(sz); };

  void SetSize(int sz) {
    if(size == sz) return;
    Drop();
    Get(sz);
    };

  ~Chunk() {
    Drop();
    };

  int Size() const { return size; };
  mfmt Data() const { return data; };

  private:
  int size;
  mfmt data;

  void Drop() {
    if(data.s8 != NULL) {
      delete [] data.s8;
      data.s8 = NULL;
      size = 0;
      }
    };

  void Get(int sz) {
    size = sz;
    data.u8 = new unsigned char[sz];
    };
  };

#endif
