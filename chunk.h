#ifndef INSOMNIA_USER_CHUNK_H
#define INSOMNIA_USER_CHUNK_H

#include <stdio.h>

#include "config.h"
#include "mfmt.h"

class Chunk {
  public:
  Chunk() { data.c = NULL; size = 0; };
  Chunk(int sz) { data.c = NULL; size = 0; Get(sz); };

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
    if(data.c != NULL) {
      if(size >= CHUNK_MAX) {
	delete data.c;
	data.c = NULL;
	size = 0;
	}
      else {
	(*(data.ul)) = (unsigned long)ch[size].c;
	ch[size].v = data.v;
	data.c = NULL;
	size = 0;
	}
      }
    };

  void Get(int sz) {
    size = (sz+1023)>>10;
    if(size >= CHUNK_MAX) {
      data.c = new char[size<<10];
      }
    else if(ch[size].v) {
      data.v = ch[size].v;
      ch[size].c = (char *)(*(data.ul));
      }
    else {
      data.c = new char[size<<10];
      }
    };
  };

#endif
