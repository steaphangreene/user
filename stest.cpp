#include <unistd.h>

#include "sound.h"
#include "speaker.h"

int  main()  {
  int ctr;
  Sound ds1("test1.wav");
  Sound ds2("test2.wav");

  Speaker sb(1, 16, 44100);
//  ds1.Play();
//  for(ctr=0; ctr<100; ctr++) sb.Update();
  sb.Reconfigure(0, 8, 11025);
  ds2.Play();
  while(1) sb.Update();
  }
