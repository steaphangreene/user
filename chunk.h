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
    if(size == ((sz+1023)>>10)) return;
    Drop();
    Get(sz);
    };

  ~Chunk() {
    Drop();
    };

  int Size() const { return size<<10; };
  mfmt Data() const { return data; };
  static void DeleteAll();

  private:
  int size;
  mfmt data;
  static mfmt ch[CHUNK_MAX];

  void Drop() {
    if(data.s8 != NULL) {
      if(size >= CHUNK_MAX) {
	delete [] data.s8;
	data.s8 = NULL;
	size = 0;
	}
      else {
	(*(data.u64)) = ch[size].UL;
	ch[size].v = data.v;
	data.s8 = NULL;
	size = 0;
	}
      }
    };

  void Get(int sz) {
    size = (sz+1023)>>10;
    if(size >= CHUNK_MAX) {
      data.u8 = new unsigned char[size<<10];
      }
    else if(ch[size].v) {
      data.v = ch[size].v;
      ch[size].u8 = data.u8;
      }
    else {
      data.u8 = new unsigned char[size<<10];
      }
    };
  };

#endif
