#include <user/screen.h>
#include <user/sound.h>
#include <user/keyboard.h>
#include <dos.h>

int  main()  {
  SoundCard sb(22050);
  Screen screen;
  User user;
  user.SetModifyerKeys();
  DigSample ds("test.wav");
  printf("DSP Revision Compatability, version %1.2f\n", sb.SBVersion());
  printf("Press and hold Q key to quit.\n");
  ds.Play();
  ds.Play();
  while(!(user.IsPressed(SCAN_Q)))  {
    delay(100);
    sb.Update();
    }
  }
