#include "bag.h"
#include "engine.h"

#include <stdio.h>

LongBag::LongBag(int sz)  {
  contents = new long[sz];
  if(contents == NULL)
    U2_Exit(1, "Not enough memory for LongBag!\n");
  size = sz;
  }

LongBag::~LongBag()  {
  if(contents != NULL) delete contents;
  size = -1;
  }

long &LongBag::operator [](int ind)  {
  if(size <= 0)  
    U2_Exit(1, "Empty LongBag accessed!\n");
  else if(ind < 0 || ind >= size)
    U2_Exit(1, "LongBag reference out of range!\n");
  else return contents[ind];
  }

long *LongBag::GiveUpContents()  {
  long *ret;
  ret = contents;
  contents = NULL;
  size = 0;
  return ret;
  }

IntBag::IntBag(int sz)  {
  contents = new int[sz];
  if(contents == NULL)
    U2_Exit(1, "Not enough memory for IntBag!\n");
  size = sz;
  }

IntBag::~IntBag()  {
  if(contents != NULL) delete contents;
  size = -1;
  }

int &IntBag::operator [](int ind)  {
  if(size <= 0)  
    U2_Exit(1, "Empty IntBag accessed!\n");
  else if(ind < 0 || ind >= size)
    U2_Exit(1, "IntBag reference out of range!\n");
  else return contents[ind];
  }

int *IntBag::GiveUpContents()  {
  int *ret;
  ret = contents;
  contents = NULL;
  size = 0;
  return ret;
  }

ShortBag::ShortBag(int sz)  {
  contents = new short[sz];
  if(contents == NULL)
    U2_Exit(1, "Not enough memory for ShortBag!\n");
  size = sz;
  }

ShortBag::~ShortBag()  {
  if(contents != NULL) delete contents;
  size = -1;
  }

short &ShortBag::operator [](int ind)  {
  if(size <= 0)  
    U2_Exit(1, "Empty ShortBag accessed!\n");
  else if(ind < 0 || ind >= size)
    U2_Exit(1, "ShortBag reference out of range!\n"); 
  else return contents[ind];
  }

short *ShortBag::GiveUpContents()  {
  short *ret;
  ret = contents;
  contents = NULL;
  size = 0;
  return ret;
  }

CharBag::CharBag(int sz)  {
  contents = new char[sz];
  if(contents == NULL)
    U2_Exit(1, "Not enough memory for CharBag!\n");
  size = sz;
  }

CharBag::~CharBag()  {
  if(contents != NULL) delete contents;
  size = -1;
  }

char &CharBag::operator [](int ind)  {
  if(size <= 0)  
    U2_Exit(1, "Empty CharBag accessed!\n");
  else if(ind < 0 || ind >= size)
    U2_Exit(1, "CharBag reference out of range!\n");
  else return contents[ind];
  }

char *CharBag::GiveUpContents()  {
  char *ret;
  ret = contents;
  contents = NULL;
  size = 0;
  return ret;
  }

