#include <sys/times.h>

main()  {
  struct tms tmp;
  printf("%d:", CLOCKS_PER_SEC);
  printf("%d\r\n", times(&tmp));
  }
