#include <time.h>

int main()  {
  float test1 = 3.14;
  double test2 = 3.14;
  long double test3 = 3.14;
  int ctr;
  uclock_t begin, end;
  begin = uclock();
  for(ctr=0; ctr<1000; ctr++)
    test1+=test1;
  end = uclock();
  printf("      Floats took %Ld\n", (end-begin));

  begin = uclock();
  for(ctr=0; ctr<1000; ctr++)
    test2+=test2;
  end = uclock();
  printf("     Doubles took %Ld\n", (end-begin));

  begin = uclock();
  for(ctr=0; ctr<1000; ctr++)
    test3+=test3;
  end = uclock();
  printf("Long Doubles took %Ld\n", (end-begin));

  return 0;
  }
