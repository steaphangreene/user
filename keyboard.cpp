#include "config.h"

#include "input.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <ctype.h>

#ifdef DOS
#include <pc.h>
#include <bios.h>
#include <sys/segments.h>

#include "asm_crap.h"

#define KEYBOARD_INT 9
#endif

#include "screen.h"
#include "keyboard.h"

extern Screen *__Da_Screen;
extern Keyboard *__Da_Keyboard;
extern InputQueue *__Da_InputQueue;

#ifdef DOS
const char *__Char_Lookup = "\0\E1234567890-=\b\tqwertyuiop[]\n\0asdfghjkl;'`\0\\zxcvbnm,./\0*\0 \0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0-\0\0+\0\0\0\0\b\0\0";
const char *__Shift_Char_Lookup = "\0\E!@#$%^&*()_+\b\tQWERTYUIOP{}\n\0ASDFGHJKL:\"~\0\\ZXCVBNM<>?\0*\0 \0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0-\0\0+\0\0\0\0\b\0\0";
#endif

Keyboard::Keyboard() {
  if(__Da_Screen == NULL) Exit(-1, "Must create Screen before Keyboard!\n");
  memset(KeyRemap, 0, KB_BUF_SIZE*(sizeof(Sprite *)));
  memset(KeyStats, 0, KB_BUF_SIZE);
  memset((char *)key_stat, 0, KEY_MAX);
  memset((char *)modkey, 0, 32*sizeof(int));
  modkey[0] = KEY_LSHIFT;
  modkey[1] = KEY_RSHIFT;
  modkey[2] = KEY_LCTRL;
  modkey[3] = KEY_RCTRL;
  modkey[4] = KEY_LALT;
  modkey[5] = KEY_RALT;

#ifdef DOS
  handler_area.handle = (unsigned long)_my_cs();
  handler_area.size = (unsigned long)dummy_marker_function
        - (long)keyboard_handler;
  handler_area.address = (unsigned long)keyboard_handler;

  __dpmi_get_protected_mode_interrupt_vector(KEYBOARD_INT, &old_handler);
  __dpmi_lock_linear_region(&handler_area);

  handler_seginfo.selector = _my_cs();
  handler_seginfo.offset32 = (long)keyboard_handler;
  __dpmi_set_protected_mode_interrupt_vector(KEYBOARD_INT, &handler_seginfo);
#endif

  crit = 0;
  __Da_Keyboard = this;
  }

Keyboard::~Keyboard() {
  __Da_Keyboard = NULL;
#ifdef DOS
  __dpmi_set_protected_mode_interrupt_vector(KEYBOARD_INT, &old_handler);
  __dpmi_unlock_linear_region(&handler_area);
#endif
  }

void Keyboard::Update() {
  if(crit) return;
#ifdef X_WINDOWS
  XEvent e;
  XFlush(__Da_Screen->_Xdisplay);
  while(XCheckMaskEvent(__Da_Screen->_Xdisplay,
	KeyPressMask|KeyReleaseMask, &e))  {
    if(e.type == KeyPress)
	Down(XKeycodeToKeysym(__Da_Screen->_Xdisplay, e.xkey.keycode, 0));
    else Up(XKeycodeToKeysym(__Da_Screen->_Xdisplay, e.xkey.keycode, 0));
    XFlush(__Da_Screen->_Xdisplay);
    }
#endif
  }

int Keyboard::IsPressed(int k) {
  Update();
  return(key_stat[k] != 0);
  }

int Keyboard::WaitForKey() {
  crit = 1;
  int ret = GetAKey();
  if(__Da_Screen == NULL)
    while(ret==0) { ret = GetAKey(); }
  else
    while(ret==0) { __Da_Screen->RefreshFast(); ret = GetAKey(); }
  crit = 0;
  return ret;
  }

int Keyboard::GetAKey(int t) {
  crit = 1;
  int ret = GetAKey();
  if(__Da_Screen == NULL)
    while(t>0&&ret==0) { t-=100; usleep(100000); ret = GetAKey(); }
  else  {
    time_t dest; long udest;
    timeval tv;
    gettimeofday(&tv, NULL);
    dest = t/1000;
    udest = (t-dest*1000)*1000;
    dest += tv.tv_sec;
    udest += tv.tv_usec;
    if(udest > 1000000) { udest -= 1000000; dest += 1; }
    while((tv.tv_sec<dest ||(tv.tv_sec==dest && tv.tv_usec<udest)) && ret==0) {
      __Da_Screen->RefreshFast(); ret = GetAKey(); gettimeofday(&tv, NULL);
      }
    }
  crit = 0;
  return ret;
  }

char Keyboard::WaitForChar() {
  crit = 1;
  char ret = GetAChar();
  if(__Da_Screen == NULL)
    while(ret==0) { ret = GetAChar(); }
  else
    while(ret==0) { __Da_Screen->RefreshFast(); ret = GetAChar(); }
  crit = 0;
  return ret;
  }

char Keyboard::GetAChar(int t) {
  crit = 1;
  char ret = GetAChar();
  if(__Da_Screen == NULL)
    while(t>0&&ret==0) { t-=100; usleep(100000); ret = GetAChar(); }
  else  {
    time_t dest; long udest;
    timeval tv;
    gettimeofday(&tv, NULL);
    dest = t/1000;
    udest = (t-dest*1000)*1000;
    dest += tv.tv_sec;
    udest += tv.tv_usec;
    if(udest > 1000000) { udest -= 1000000; dest += 1; }
    while((tv.tv_sec<dest ||(tv.tv_sec==dest && tv.tv_usec<udest)) && ret==0) {
      __Da_Screen->RefreshFast(); ret = GetAChar(); gettimeofday(&tv, NULL);
      }
    }
  crit = 0;
  return ret;
  }

int Keyboard::GetAKey() {
  int ret=0;
#ifdef X_WINDOWS
  XEvent e;
  XFlush(__Da_Screen->_Xdisplay);
  while(ret == 0 && XCheckMaskEvent(__Da_Screen->_Xdisplay,
	KeyPressMask|KeyReleaseMask, &e)) {
    if(e.type == KeyPress)  {
      ret = XKeycodeToKeysym(__Da_Screen->_Xdisplay, e.xkey.keycode, 0);
      key_stat[ret] = 1;
      }
    else  {
      key_stat[XKeycodeToKeysym(__Da_Screen->_Xdisplay, e.xkey.keycode, 0)]=0;
      }
    }
#endif
#ifdef DOS
  if(buf_ind>0)  {
    unsigned int ctr;
    ret = keyboard_buf[0];
    for(ctr=0; ctr<buf_ind; ctr++)  {
      keyboard_buf[ctr] = keyboard_buf[ctr+1];
      }
    --buf_ind;
    }
#endif
  return ret;
  }


char Keyboard::GetAChar() {
  int key = GetAKey();
  return KeyToChar(key);
  }

char Keyboard::KeyToChar(int key) {
  int ret;
  switch(key)  {
    case(0): return 0; break;
    case(KEY_INSERT):  ret = CHAR_INSERT;  break;
    case(KEY_DEL):  ret = CHAR_DELETE;  break;
    case(KEY_BACKSP):  ret = CHAR_BACKSPACE;  break;
    case(KEY_UP):  ret = CHAR_UP;  break;
    case(KEY_DOWN):  ret = CHAR_DOWN;  break;
    case(KEY_LEFT):  ret = CHAR_LEFT;  break;
    case(KEY_RIGHT):  ret = CHAR_RIGHT;  break;
    case(KEY_ESC):  ret = CHAR_ESCAPE;  break;
    case(KEY_END):  ret = CHAR_END;  break;
    case(KEY_HOME):  ret = CHAR_HOME;  break;
    case(KEY_PGUP):  ret = CHAR_PGUP;  break;
    case(KEY_PGDN):  ret = CHAR_PGDN;  break;
#ifdef DOS
    default:  ret=__Char_Lookup[key];  break;
#endif
#ifdef X_WINDOWS
    case(KEY_RETURN):
    case(KEY_ENTER):  ret = '\n'; break;
    case(KEY_TAB):  ret = '\t'; break;
    case(KEY_SPACE):  ret = ' '; break;
    case(KEY_QUOTE):  ret = '\''; break;
    case(KEY_BQUOTE):  ret = '`'; break;
    case(KEY_DOT):  ret = '.'; break;
    case(KEY_COMA):  ret = ','; break;
    case(KEY_BSLASH):  ret = '\\'; break;
    case(KEY_SLASH):  ret = '/'; break;
    case(KEY_EQUALS):  ret = '='; break;
    case(KEY_MINUS):  ret = '-'; break;
    case(KEY_CAPS):
    case(KEY_LCTRL):
    case(KEY_RCTRL):
    case(KEY_RSHIFT):
    case(KEY_LSHIFT):
    case(KEY_LALT):
    case(KEY_RALT):  ret = 0; break;
    default:  {
      if(key_stat[KEY_LSHIFT]  || key_stat[KEY_RSHIFT])  {
        ret = toupper(XKeysymToString(key)[0]);
	}
      else {
        ret = XKeysymToString(key)[0];
	}
      }break;
#endif
    }
  return ret;
  }

void Keyboard::DisableQueue()  {
#ifdef DOS
  buf_ind=0;
#endif
  queue_keys=0;
  }

void Keyboard::EnableQueue()  {
  queue_keys=1;
  }

void Keyboard::MapKeyToControl(int k, Control &c)  {
  MapKeyToControl(k, &c);
  }

void Keyboard::MapKeyToControl(int k, Control *c)  {
  KeyRemap[k] = c;
  }

void Keyboard::MapKeyToControl(int k, int c)  {
  if(__Da_Screen != NULL)
	MapKeyToControl(k, (Control *)__Da_Screen->GetSpriteByNumber(c));
  }

void Keyboard::Down(int k)  {
  if(key_stat[k] == 1) return;
  key_stat[k] = 1;
  InputAction a;
  a.k.type = INPUTACTION_KEYDOWN;
  a.k.modkeys = ModKeys();
  a.k.key = k;
  a.k.chr = KeyToChar(k);
  if(__Da_InputQueue != NULL) __Da_InputQueue->ActionOccurs(&a);
  }

void Keyboard::Up(int k)  {
  if(key_stat[k] == 0) return;
  key_stat[k] = 0;
  InputAction a;
  a.k.type = INPUTACTION_KEYUP;
  a.k.modkeys = ModKeys();
  a.k.key = k;
  a.k.chr = 0;
  if(__Da_InputQueue != NULL) __Da_InputQueue->ActionOccurs(&a);
  }

int Keyboard::ModKeys() {
  int ctr, ret=0;
  for(ctr=0; ctr<32; ctr++) {
    if(modkey[ctr] > 0 && key_stat[modkey[ctr]]) ret |= (1<<ctr);
    }
  return ret;
  }

int Keyboard::ModKey(int k) {
  int ctr;
  for(ctr=0; ctr<32; ctr++) {
    if(modkey[ctr] == k) return (1<<ctr);
    }
  return 0;
  }

void Keyboard::AddModKey(int k) {
  int ctr;
  for(ctr=0; ctr<32 && modkey[ctr] != 0; ctr++);
  if(ctr >= 32) Exit(-1, "Too many ModKeys (max=32)\n");
  modkey[ctr] = k;
  }

void Keyboard::RemoveModKey(int k) {
  int ctr;
  for(ctr=0; ctr<32 && modkey[ctr] != k; ctr++);
  if(ctr >= 32) Exit(-1, "Tried to Remove Non-Existant ModKey (%d)\n", k);
  modkey[ctr] = 0;
  }

#ifdef DOS

volatile void Keyboard::keyboard_handler()  {
  START_INTERRUPT();

  __asm__ __volatile__ (
	"inb     $0x60, %al
	xorl	%edx, %edx
	movb    %al, %dl
	cmpb	$0xE1, %al
	jne	not_ext2
	movb	$2, %cl
	movb	%cl, __8Keyboard$in_ext
	jmp	skip_to_end
not_ext2:
	cmpb	$0xE0, %al
	jne	not_ext
	movb	$1, %cl
	movb	%cl, __8Keyboard$in_ext
	jmp	skip_to_end
not_ext:
	andl    $0x7f, %edx
	xorb	%cl, %cl
	cmpb	%cl, __8Keyboard$in_ext
	je	not_was_ext
	decb	__8Keyboard$in_ext
	movb	$0, %cl
	cmpb	%cl, __8Keyboard$in_ext
	jne	skip_to_end
	orl	$0x80, %edx
not_was_ext:
	movb	$170, %cl
	cmpb	%cl, %dl
	je	skip_to_end
	movb	$198, %cl
	cmpb	%cl, %dl
	jne	done_remaping
	movb	$197, %dl
done_remaping:
	movb    __8Keyboard$key_stat(%edx), %ch
	testb   $0x80, %al
	setz    __8Keyboard$key_stat(%edx)
	movb    __8Keyboard$key_stat(%edx), %cl
	cmpb	%cl, %ch
	je	skip_to_end
	cmpb	$0, %cl
	je	skip_to_end
	movb	%dl, %al
	movb	%cl, %ah
	xorb	$0x1, %ah
	movb	__8Keyboard$queue_keys, %cl
	cmpb	$0, %cl
	je	skip_to_end
	movl	__8Keyboard$buf_ind, %edx
	addl	%edx, %edx
	movw    %ax, __8Keyboard$keyboard_buf(%edx)
	incl	%edx
	movl	%edx, __8Keyboard$buf_ind
skip_to_end:
"	);

  ACKNOWLEDGE_KEYBOARD_INTERRUPT();
  END_INTERRUPT();
  }

volatile unsigned short Keyboard::ModKey[10];

#endif
volatile int Keyboard::modkey[32];
volatile char Keyboard::key_stat[KEY_MAX];
volatile char Keyboard::queue_keys = 1;
#ifdef DOS

volatile unsigned short Keyboard::keyboard_buf[KB_BUF_SIZE];
volatile unsigned short Keyboard::modkey_buf[KB_BUF_SIZE];
volatile unsigned long Keyboard::buf_ind = 0;
volatile char Keyboard::in_ext = 0;

volatile void Keyboard::dummy_marker_function()  {}
#endif

