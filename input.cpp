#include "engine.h"
#include "input.h"
#include "mouse.h"
#include "keyboard.h"
#include "screen.h"

#include <stdio.h>

extern InputQueue *__Da_InputQueue;
extern Keyboard *__Da_Keyboard;
extern Screen *__Da_Screen;
extern Mouse *__Da_Mouse;

InputQueue::InputQueue() {
  if(__Da_InputQueue != NULL) Exit(-1, "Multiple InputQueues Declared!\n");
  __Da_InputQueue = this;
  head = 0;
  tail = 0;
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
