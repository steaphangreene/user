#include <sys/segments.h>
#include <string.h>
#include <user/screen.h>
#include <user/keyboard.h>
#include <user/engine.h>
#include "asm_crap.h"

volatile void User::keyboard_handler()  {
  static unsigned char inp;
  START_INTERRUPT();

  __asm__ __volatile__ (
	"inb     $0x60, %%al
	movb	%%al, %0
	movb    %%al, %%dl
	andl    $0x7f, %%edx
	testb   $0x80, %%al
	setz    __4User$keyboard_map(%%edx)"
	: "=g" (inp)
	);
  
  if(queue_keys && buf_ind < KB_BUF_SIZE)  {
    keyboard_buf[buf_ind] = inp;
    modkey_buf[buf_ind] = 0;
    if(ModKey[0] && keyboard_map[ModKey[0]])
      modkey_buf[buf_ind] += 1;
    if(ModKey[1] && keyboard_map[ModKey[1]])
      modkey_buf[buf_ind] += 2;
    if(ModKey[2] && keyboard_map[ModKey[2]])
      modkey_buf[buf_ind] += 4;
    if(ModKey[3] && keyboard_map[ModKey[3]])
      modkey_buf[buf_ind] += 8;
    if(ModKey[4] && keyboard_map[ModKey[4]])
      modkey_buf[buf_ind] += 16;
    if(ModKey[5] && keyboard_map[ModKey[5]])
      modkey_buf[buf_ind] += 32;
    if(ModKey[6] && keyboard_map[ModKey[6]])
      modkey_buf[buf_ind] += 64;
    if(ModKey[7] && keyboard_map[ModKey[7]])
      modkey_buf[buf_ind] += 128;
    buf_ind++;
    }
  ACKNOWLEDGE_KEYBOARD_INTERRUPT();
  END_INTERRUPT();
  }

//volatile unsigned char User::ModKey[10];
//volatile char User::keyboard_map[KB_BUF_SIZE];
//volatile char User::keyboard_buf[KB_BUF_SIZE];
//volatile char User::modkey_buf[KB_BUF_SIZE];
//volatile int User::buf_ind = 0;
//volatile char User::queue_keys = 1;

volatile void User::dummy_marker_function()  {}

