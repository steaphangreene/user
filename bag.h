#ifndef INSOMNIA_USER_BAG_H
#define INSOMNIA_USER_BAG_H

class LongBag {
  public:
  LongBag(int sz);
  ~LongBag();
  long &operator [](int ind);
  long *GiveUpContents();
  int Size() const { return size; };

  private:
  long *contents;
  int size;
  };

class ShortBag {
  public:
  ShortBag(int sz);
  ~ShortBag();
  short &operator [](int ind);
  short *GiveUpContents();
  int Size() const { return size; };

  private:
  short *contents;
  int size;
  };

class CharBag {
  public:
  CharBag(int sz);
  ~CharBag();
  char &operator [](int ind);
  char *GiveUpContents();
  int Size() const { return size; };

  private:
  char *contents;
  int size;
  };

#endif
