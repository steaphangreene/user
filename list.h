#ifndef	INSOMNIA_LIST_H
#define	INSOMNIA_LIST_H

#include <stdlib.h>
#include <stdio.h>

class IntList  {
  public:
  IntList()  {
    size = 0;
    data = NULL;
    }

  IntList(const IntList &from)  {
    int ctr;
    size = from.size;
    if(size == 0)  {
      data = NULL;
      return;
      }
    else  {
      data = new long int[size];
      for(ctr = 0; ctr < size; ctr++)
	data[ctr] = from.data[ctr];
      }
    }

  ~IntList()  {
    if(data != NULL)  delete data;
    }

  void operator =(const IntList &from)  {
    int ctr;
    if(data != NULL)  delete data;
    size = from.size;
    if(size == 0)  {
      data = NULL;
      return;
      }
    else  {
      data = new long int[size];
      for(ctr = 0; ctr < size; ctr++)
        data[ctr] = from.data[ctr];
      }
    }

  void Print()  {
    int ctr;
    for(ctr = 0; ctr < size; ctr++)
      printf("%ld ", data[ctr]);
    }

  void operator +=(int newval)  {
    if(*this == newval)  return;
    if(size < 1)  {
      data = new long[1];
      data[0] = newval;
      size = 1;
      }
    else  {
      int ctr2;
      long *tmp;
      tmp = new long[size+1];
      for(ctr2 = 0; ctr2 < size; ctr2++)
        tmp[ctr2] = data[ctr2];
      tmp[size] = newval;
      delete data;
      size++;
      data = tmp;
      tmp = NULL;
      }
    }

  void operator -=(int killval)  {
    int ctr;
    for(ctr=0; ctr<size; ++ctr)  {
      if(data[ctr] == killval)  {
	size--;
	for(; ctr<size; ctr++)  {
	  data[ctr] = data[ctr+1];
	  }
	}
      }
    }

  void operator +=(const IntList &IL)  {
    int ctr;
    for(ctr = 0; ctr < IL.size; ctr++)
      (*this) += IL.data[ctr];
    }

  IntList operator +(IntList IL)  {
    IntList ret;
    int ctr;
    ret.size = IL.size + size;
    ret.data = new long[size];
    for(ctr = 0; ctr < size; ctr++)
      ret.data[ctr] = data[ctr];
    for(ctr = size; ctr < size+IL.size; ctr++)
      ret.data[ctr] = IL.data[ctr-size];
    return ret;
    }

  int operator !=(int val)  {
    int ctr;
    for(ctr = 0; ctr < size; ctr++)
      if(data[ctr] == val)  return (1 == 2);
    return (1 == 1);
    }

  int operator ==(int val)  {
    int ctr;
    for(ctr = 0; ctr < size; ctr++)
      if(data[ctr] == val)  return (1 == 1);
    return (1 == 2);
    }

  int Size()  {
    return (size);
    }

  int operator >(int val)  {
    return (size > val);
    }

  int operator <(int val)  {
    return (size < val);
    }

  int operator >=(int val)  {
    return (size >= val);
    }

  int operator <=(int val)  {
    return (size <= val);
    }

  unsigned long GetSize()  {
    return size;
    }

  long operator[] (unsigned long num)  {
    if(num < (unsigned long)size)  return data[num];
    else return 0;
    }

  void Clear()  {
    if(data != NULL)  delete data;
    data = NULL;
    size = 0;
    }

  void RotateToElement(int elnum)  {
    int ctr;
    if(elnum >= size || elnum < 0)  return;
    int *tmpl = new int[elnum];
    for(ctr = 0; ctr < elnum; ctr++)  {
      tmpl[ctr] = data[ctr];
      }
    for(; ctr < size; ctr++)  {
      data[ctr-elnum] = data[ctr];
      }
    for(; ctr < (size+elnum); ctr++)  {
      data[ctr-elnum] = tmpl[ctr-size];
      }
    }

  protected:
  long size;
  long *data;
  };

#endif
