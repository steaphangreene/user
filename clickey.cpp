#include <stdio.h>

#include "screen.h"
#include "graphic.h"
#include "sprite.h"
#include "control.h"
#include "input.h"
#include "keyboard.h"
#include "sound.h"

extern Screen *__Da_Screen;
extern InputQueue *__Da_InputQueue;
extern Keyboard *__Da_Keyboard;

Clickey::Clickey() {
  flags |= SPRITE_ISCONTROL;
  state = 0;
  }

Clickey::Clickey(Graphic g1, Graphic g2) {
  flags |= SPRITE_ISCONTROL;
  state = 0;
  SetImage(&g1, &g2);
  }

Clickey::Clickey(Graphic *g1, Graphic *g2) {
  flags |= SPRITE_ISCONTROL;
  state = 0;
  SetImage(g1, g2);
  }

Clickey::Clickey(int x, int y,
	const char *l, color ct, color cl, color cb, color cd) {
  flags |= SPRITE_ISCONTROL;
  state = 0;
  Create(x, y, l, ct, cl, cb, cd);
  }

void Clickey::SetImage(Graphic g1, Graphic g2) {
  SetImage(&g1, &g2);
  }

void Clickey::SetImage(Graphic *g1, Graphic *g2) {
  Erase();
  inum = 2;
  image = new Graphic[2];
  image[0] = *g1;
  image[1] = *g2;
  SetupBinFlags();
  }

Clickey::~Clickey() {
  __Da_InputQueue->UnmapControl(this);
  }

void Clickey::Click(int b) {
  if(state == 1 || (!enabled)) return;
  int dr = drawn;
  if(dr) Erase();
  Graphic tmpg;
  tmpg = image[0];
  image[0] = image[1];
  image[1] = tmpg;
  if(dr) Draw();
  state = 1; lastb = b;
  if(__Da_InputQueue != NULL) {
    InputAction e;
    e.m.type = INPUTACTION_CONTROLDOWN;
    e.m.button = snum;
    if(__Da_Keyboard != NULL) e.m.modkeys = __Da_Keyboard->ModKeys();
    __Da_InputQueue->ActionOccurs(&e);
    }
  }

void Clickey::UnClick(int b) {
  if(state == 0 || lastb != b) return;
  int dr = drawn;
  if(dr) Erase();
  Graphic tmpg;
  tmpg = image[0];
  image[0] = image[1];
  image[1] = tmpg;
  if(dr) Draw();
  state = 0;
  if(__Da_InputQueue != NULL) {
    InputAction e;
    e.m.type = INPUTACTION_CONTROLUP;
    e.m.button = snum;
    if(__Da_Keyboard != NULL) e.m.modkeys = __Da_Keyboard->ModKeys();
    __Da_InputQueue->ActionOccurs(&e);
    }
  }

void Clickey::RemappedKeyPressed() {
  Click(1);
  }

void Clickey::RemappedKeyReleased() {
  UnClick(1);
  }

void Clickey::SetSound(Sound *s) {
  if(s != NULL) dsnd = new Sound(*s);
  else dsnd = NULL;
  usnd = NULL;
  }

void Clickey::SetSound(Sound *s1, Sound *s2) {
  if(s1 != NULL) dsnd = new Sound(*s1);
  else dsnd = NULL;
  if(s2 != NULL) usnd = new Sound(*s2);
  else usnd = NULL;
  }

void Clickey::SetState(int s) {
  if(state == s) return;
  int dr = drawn;
  if(dr) Erase();
  Graphic tmpg;
  tmpg = image[0];
  image[0] = image[1];
  image[1] = tmpg;
  if(dr) Draw();
  state = s;
  }

void Clickey::Create(int x, int y,
	const char *l, color ct, color cl, color cb, color cd) {
  image = new Graphic[2];
  image[0].SetFillRect(x, y, __Da_Screen->GetApparentDepth(), cl);
  image[0].DrawFillRect(2, 2, x-2, y-2, __Da_Screen->GetApparentDepth(), cd);
  image[0].DrawFillRect(2, 2, x-4, y-4, __Da_Screen->GetApparentDepth(), cb);
  image[0].DrawPixel(1, y-1, __Da_Screen->GetApparentDepth(), cd);
  image[0].DrawPixel(x-1, 0, __Da_Screen->GetApparentDepth(), cd);
  image[0].DrawPixel(x-1, 1, __Da_Screen->GetApparentDepth(), cd);
  image[0].DrawPixel(x-2, 1, __Da_Screen->GetApparentDepth(), cd);
  __Da_Screen->GPrintf(&image[0], 3, 3, cb, ct, l);
  image[1].SetFillRect(x, y, __Da_Screen->GetApparentDepth(), cd);
  image[1].DrawFillRect(4, 4, x-4, y-4, __Da_Screen->GetApparentDepth(), cb);
  __Da_Screen->GPrintf(&image[1], 5, 5, cb, ct, l);
  SetupBinFlags();
  }
