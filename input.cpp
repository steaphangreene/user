#include "engine.h"
#include "input.h"
#include "mouse.h"
#include "keyboard.h"
#include "screen.h"

#include <stdio.h>
#include <string.h>

extern InputQueue *__Da_InputQueue;
extern Keyboard *__Da_Keyboard;
extern Screen *__Da_Screen;
extern Mouse *__Da_Mouse;

InputQueue::InputQueue() {
  if(__Da_InputQueue != NULL) U2_Exit(-1, "Multiple InputQueues Declared!\n");
  __Da_InputQueue = this;
  head = 0;
  tail = 0;
  memset(keymap, 0, sizeof(keymap));
  }

InputQueue::~InputQueue() {
  __Da_InputQueue = NULL;
  }

void InputQueue::Update() {
  if(__Da_Keyboard != NULL) __Da_Keyboard->Update();
  if(__Da_Mouse != NULL) __Da_Mouse->Update();
  }
 
void InputQueue::ActionOccurs(InputAction *a) {
  if((tail+1)&1023 == head) return;
  if(a->k.type == INPUTACTION_KEYDOWN && keymap[a->k.key % 256] != NULL) {
    RemapedKey *cur = keymap[a->k.key % 256];
    while(cur != NULL) {
      if(cur->k == a->k.key) {
	cur->c->Click(1);
	return;
	}
      cur = cur->next;
      }
    }
  if(a->k.type == INPUTACTION_KEYUP && keymap[a->k.key % 256] != NULL) {
    RemapedKey *cur = keymap[a->k.key % 256];
    while(cur != NULL) {
      if(cur->k == a->k.key) {
	cur->c->UnClick(1);
	return;
	}
      cur = cur->next;
      }
    }
  queue[tail] = *a;
  ++tail; tail &= 1023;
  }

InputAction *InputQueue::NextAction() {
  if(tail == head) return NULL;
  int ohead = head;
  ++head; head &= 1023;
  return &(queue[ohead]);
  }

InputAction *InputQueue::WaitForNextAction() {
  while(tail == head) __Da_Screen->Refresh();
  int ohead = head;
  ++head; head &= 1023;
  return &(queue[ohead]);
  }

InputAction *InputQueue::PeekNextAction() {
  if(tail == head) return NULL;
  return &(queue[head]);
  }

void InputQueue::MapKeyToControl(int k, int cn) {
  UserDebug("InputQueue::MapKeyToControl(int k, int c)  Start");
  MapKeyToControl(k, (Control *)__Da_Screen->GetSpriteByNumber(cn));
  UserDebug("InputQueue::MapKeyToControl(int k, int c)  End");
  }

void InputQueue::MapKeyToControl(int k, Control &c) {
  UserDebug("InputQueue::MapKeyToControl(int k, Control &c)  Start");
  MapKeyToControl(k, &c);
  UserDebug("InputQueue::MapKeyToControl(int k, Control &c)  End");
  }

void InputQueue::MapKeyToControl(int k, Control *c) {
  UserDebug("InputQueue::MapKeyToControl(int k, Control *c)  Start");
  UnmapKey(k);
  if(c == NULL) return;
  RemapedKey **cur = keymap + (k % 256);
  while(*cur != NULL) cur = &((*cur)->next);
  (*cur) = new RemapedKey;
  (*cur)->k = k;
  (*cur)->c = c;
  (*cur)->next = NULL;
  UserDebug("InputQueue::MapKeyToControl(int k, Control *c)  End");
  }

void InputQueue::UnmapKey(int k) {
  RemapedKey **cur = keymap + (k % 256);
  while(*cur != NULL) {
    if((*cur)->k == k) {
      RemapedKey *tmp = *cur;
      *cur = (*cur)->next;
      delete tmp;
      }
    else cur = &((*cur)->next);
    }
  }

void InputQueue::UnmapControl(Control &c) {
  UnmapControl(&c);
  }

void InputQueue::UnmapControl(Control *c) {
  int ctr;
  RemapedKey **cur;
  for(ctr=0; ctr<256; ctr++) {
    cur = keymap + ctr;
    while(*cur != NULL) {
      if((*cur)->c == c) {
	RemapedKey *tmp = *cur;
	*cur = (*cur)->next;
	delete tmp;
	}
      else cur = &((*cur)->next);
      }
    }
  }
