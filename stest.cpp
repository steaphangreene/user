#include <unistd.h>

#include "engine.h"
#include "sound.h"
#include "speaker.h"

#ifdef DOS
#include <dos.h>
#endif

int  main(int argc, char **argv)  {
  InitUserEngine(argc, argv);

//  Speaker *sb = new Speaker(0, 16, 11025);
//  Speaker *sb = new Speaker(1, 16, 11025);
//  Speaker *sb = new Speaker(0, 16, 22050);
//  Speaker *sb = new Speaker(1, 16, 22050);
//  Speaker *sb = new Speaker(0, 16, 44100);
  Speaker *sb = new Speaker(1, 16, 44100);
//  Speaker *sb = new Speaker(0, 8, 11025);
//  Speaker *sb = new Speaker(1, 8, 11025);
//  Speaker *sb = new Speaker(0, 8, 22050);
//  Speaker *sb = new Speaker(1, 8, 22050);
//  Speaker *sb = new Speaker(0, 8, 44100);
//  Speaker *sb = new Speaker(1, 8, 44100);

  Sound ds1("test1.wav");
//  Sound ds1("test2.wav");

  ds1.Play();

//  sb.Reconfigure(0, 8, 11025);
//  ds2.Play();
  while(1) sb->Update();
  delete sb;
  }
