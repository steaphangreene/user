#include <ctype.h>
#include <string.h>
#include <user/os_defs.h>
#include <user/screen.h>
#include <user/keyboard.h>
#include <user/engine.h>

#ifdef DOS
#include <sys/segments.h>
#include "asm_crap.h"
#define KEYBOARD_INT 9
#endif

#ifdef SVGALIB
#include <vga.h>
#endif

#ifdef X_WINDOWS
#define Screen X_Screen
#define Window X_Window

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#undef Screen
#undef Window

extern int IN_X;
extern unsigned char *Frame;
extern int ___myscreen;
extern Display *___mydisplay;
extern X_Window ___mywindow;
extern XSizeHints ___myhint;
extern XWMHints ___mywmhint;
extern GC ___mygc;
extern XEvent ___myevent;
extern Colormap ___mymap;
extern XColor ___mypal[256];
extern XImage *___mypic;
#endif

const char *__Char_Lookup = "\0\E1234567890-=\b\tqwertyuiop[]\n\0asdfghjkl;'`\0\\zxcvbnm,./\0*\0 \0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0-\0\0+\0\0\0\0\b\0\0";
const char *__Shift_Char_Lookup = "\0\E!@#$%^&*()_+\b\tQWERTYUIOP{}\n\0ASDFGHJKL:\"~\0\\ZXCVBNM<>?\0*\0 \0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0-\0\0+\0\0\0\0\b\0\0";

extern User *__Da_User;
extern Screen *__Da_Screen;
extern Mouse *__Da_Mouse;

UserAction::UserAction(const UserAction &in)  {
  int ctr;
  modkeys=in.modkeys;
  pan=in.pan;
  type=in.type;
  butt=in.butt;
  startx=in.startx;
  starty=in.starty;
  endx=in.endx;
  endy=in.endy;
  next=NULL;
  size = in.size;
  if(size == 0)  {
    data = NULL;
    }
  else  {
    data = new long int[size];
    for(ctr = 0; ctr < size; ctr++)
      data[ctr] = in.data[ctr];
    }
  }

UserAction::~UserAction()  {
//  if(data != NULL)  delete data;
  }

UserAction::UserAction()  {
  type = USERACTION_NONE;
  modkeys = 0;
  butt=-1;
  startx=-1;
  starty=-1;
  endx=-1;
  endy=-1;
  size=0;
  pan=0;
  data=NULL;
  next=NULL;
  }

UserAction::UserAction(int tp)  {
  type = tp;
  modkeys = 0;
  butt=-1;
  startx=-1;
  starty=-1;
  endx=-1;
  endy=-1;
  size=0;
  pan=0;
  data=NULL;
  next=NULL;
  }

UserAction::UserAction(Panel p, char v1, int v2, int v3)  {
  type = USERACTION_MOUSE;
  modkeys = 0;
  pan = p;
  butt=v1;
  startx=v2;
  starty=v3;
  endx=-1;
  endy=-1;
  size=0;
  data=NULL;
  next=NULL;
  }

void UserAction::SetKeyPressed(int which)  {
  SetKeyPressed(which, 0);
  }

void UserAction::SetKeyReleased(int which)  {
  SetKeyPressed(which, 0);
  type = USERACTION_KEYRELEASED;
  }

void UserAction::SetKeyPressed(int which, int mod)  {
#ifdef X_WINDOWS
  if(IN_X || Frame != NULL)
    which=XKeycodeToKeysym(___mydisplay, which, 0);
#endif
  int shift = 0, ctr;
  for(ctr=0; shift == 0 && ctr<8; ctr++)  {
    if((__Da_User->ModKey[ctr] == SCAN_RSHIFT
		|| __Da_User->ModKey[ctr] == SCAN_LSHIFT)
		&& (mod & (1<<ctr)))
      shift = 1;
    }
  type = USERACTION_KEYPRESSED;

  butt=which;
  switch(butt)  {
    case(SCAN_INSERT):  pan = KEY_INSERT;  break;
    case(SCAN_DEL):  pan = KEY_DELETE;  break;
    case(SCAN_BACKSP):  pan = KEY_BACKSPACE;  break;
    case(SCAN_UP):  pan = KEY_UP;  break;
    case(SCAN_DOWN):  pan = KEY_DOWN;  break;
    case(SCAN_LEFT):  pan = KEY_LEFT;  break;
    case(SCAN_RIGHT):  pan = KEY_RIGHT;  break;
    case(SCAN_ESC):  pan = KEY_ESCAPE;  break;
    case(SCAN_END):  pan = KEY_END;  break;
    case(SCAN_HOME):  pan = KEY_HOME;  break;
    case(SCAN_PGUP):  pan = KEY_PGUP;  break;
    case(SCAN_PGDN):  pan = KEY_PGDN;  break;
#ifdef DOS
    default:  pan=__Char_Lookup[which];  break;
#endif
#ifdef X_WINDOWS
    case(SCAN_RETURN):
    case(SCAN_ENTER):  pan = '\n'; break;
    case(SCAN_TAB):  pan = '\t'; break;
    case(SCAN_SPACE):  pan = ' '; break;
    case(SCAN_QUOTE):  pan = '\''; break;
    case(SCAN_BQUOTE):  pan = '`'; break;
    case(SCAN_DOT):  pan = '.'; break;
    case(SCAN_COMA):  pan = ','; break;
    case(SCAN_BSLASH):  pan = '\\'; break;
    case(SCAN_SLASH):  pan = '/'; break;
    case(SCAN_EQUALS):  pan = '='; break;
    case(SCAN_MINUS):  pan = '-'; break;
    case(SCAN_CAPS):
    case(SCAN_LCTRL):
    case(SCAN_RCTRL):
    case(SCAN_RSHIFT):
    case(SCAN_LSHIFT):
    case(SCAN_LALT):
    case(SCAN_RALT):  pan = 0; break;
    default:    if(IN_X || Frame != NULL)  {
		pan=XKeysymToString(butt)[0];
		if(__Da_User->IsPressed(SCAN_LSHIFT) 
			|| __Da_User->IsPressed(SCAN_RSHIFT))  {
		  pan = toupper(pan);
		  }
		}
	      else  {
		pan=__Char_Lookup[which];
		}
	      break;
#endif
    }
  modkeys = mod;
  size=0;
  data=NULL;
  next=NULL;
  }

void UserAction::SetKeyReleased(int which, int mod)  {
  SetKeyPressed(which, mod);
  type = USERACTION_KEYRELEASED;
  }

void UserAction::SetButtonPressed(int which)  {
  type = USERACTION_BUTTONPRESSED;
  butt=which;
  modkeys = 0;
  size=0;
  data=NULL;
  next=NULL;
  }

void UserAction::SetButtonReleased(int which)  {
  type = USERACTION_BUTTONRELEASED;
  butt=which;
  modkeys = 0;
  size=0;
  data=NULL;
  next=NULL;
  }

UserAction::UserAction(Panel p, char v1, int v2, int v3, int v4, int v5)  {
  type = USERACTION_MOUSE;
  modkeys = 0;
  pan = p;
  butt=v1;
  startx=v2;
  starty=v3;
  endx=v4;
  endy=v5;
  size=0;
  data=NULL;
  next=NULL;
  }

UserAction UserAction::operator =(const UserAction &in)  {
  int ctr;
  modkeys=in.modkeys;
  pan=in.pan;
  type=in.type;
  butt=in.butt;
  startx=in.startx;
  starty=in.starty;
  endx=in.endx;
  endy=in.endy;
  next=NULL;
  if(data != NULL)  delete data;
  size = in.size;
  if(size == 0)  {
    data = NULL;
    }
  else  {
    data = new long int[size];
    for(ctr = 0; ctr < size; ctr++)
      data[ctr] = in.data[ctr];
    }
  return *this;
  }

UserAction UserAction::operator =(const IntList &in)  {
  int ctr;
  if(data != NULL)  delete data;
  size = ((IntList &)in).Size();
  if(size == 0)  {
    data = NULL;
    }
  else  {
    data = new long int[size];
    for(ctr = 0; ctr < size; ctr++)
      data[ctr] = ((IntList &)in)[ctr];
    }
  return *this;
  }

User::User()  {
  if(__Da_Screen == NULL)  {
    Exit(0, "User needs a Screen (create Screen before User)!\r\n");
    }
  bzero((char *)ModKey, 10);
  InitKey();

#ifdef X_WINDOWS
  if(IN_X)  {
    closeatom = XInternAtom(___mydisplay, "WM_DELETE_WINDOW", 1);
    if(closeatom)
      XSetWMProtocols(___mydisplay, ___mywindow, (Atom *)&closeatom, 1);
    }
#endif
  }

void User::KeyRepeatOn()  {
#ifdef X_WINDOWS
  if(IN_X || Frame != NULL)  {
    Display *md;
    md = XOpenDisplay("");
    XAutoRepeatOn(md);
    XCloseDisplay(md);
    }
#endif
  }

void User::KeyRepeatOff()  {
#ifdef X_WINDOWS
  if(IN_X || Frame != NULL)  {
    Display *md;
    md = XOpenDisplay("");
    XAutoRepeatOff(md);
    XCloseDisplay(md);
    }
#endif
  }

unsigned long User::GetModKeyStatus()  {
  unsigned long mk = 0;
  int ctr;
  for(ctr=0; ctr<8 && ModKey[ctr]; ctr++)  {
    if(keyboard_map[ModKey[ctr]])  mk += (1 << ctr);
    }
  return mk;
  }

void User::SetModifyerKeys()  {
  SetModifyerKeys(0, 0, 0, 0, 0, 0, 0, 0);
  }

void User::SetModifyerKeys(int m1)  {
  SetModifyerKeys(m1, 0, 0, 0, 0, 0, 0, 0);
  }

void User::SetModifyerKeys(int m1, int m2)  {
  SetModifyerKeys(m1, m2, 0, 0, 0, 0, 0, 0);
  }

void User::SetModifyerKeys(int m1, int m2, int m3)  {
  SetModifyerKeys(m1, m2, m3, 0, 0, 0, 0, 0);
  }

void User::SetModifyerKeys(int m1, int m2, int m3, int m4)  {
  SetModifyerKeys(m1, m2, m3, m4, 0, 0, 0, 0);
  }

void User::SetModifyerKeys(int m1, int m2, int m3, int m4, int m5)  {
  SetModifyerKeys(m1, m2, m3, m4, m5, 0, 0, 0);
  }

void User::SetModifyerKeys(int m1, int m2, int m3, int m4, int m5, int m6)  {
  SetModifyerKeys(m1, m2, m3, m4, m5, m6, 0, 0);
  }

void User::SetModifyerKeys(int m1, int m2, int m3, int m4, int m5, int m6,
	int m7)  {
  SetModifyerKeys(m1, m2, m3, m4, m5, m6, m7, 0);
  }

void User::SetModifyerKeys(int m1, int m2, int m3, int m4, int m5, int m6,
	int m7, int m8)  {
#ifdef DOS
  ModKey[0] = m1;	ModKey[1] = m2;		ModKey[2] = m3;
  ModKey[3] = m4;	ModKey[4] = m5;		ModKey[5] = m6;
  ModKey[6] = m7;	ModKey[7] = m8;
#endif

#ifdef X_WINDOWS
  if(IN_X || Frame != NULL)  {
    ModKey[0] = XKeysymToKeycode(___mydisplay, m1);
    ModKey[1] = XKeysymToKeycode(___mydisplay, m2);
    ModKey[2] = XKeysymToKeycode(___mydisplay, m3);
    ModKey[3] = XKeysymToKeycode(___mydisplay, m4);
    ModKey[4] = XKeysymToKeycode(___mydisplay, m5);
    ModKey[5] = XKeysymToKeycode(___mydisplay, m6);
    ModKey[6] = XKeysymToKeycode(___mydisplay, m7);
    ModKey[7] = XKeysymToKeycode(___mydisplay, m8);
    }
  else  {
    ModKey[0] = m1;	ModKey[1] = m2;		ModKey[2] = m3;
    ModKey[3] = m4;	ModKey[4] = m5;		ModKey[5] = m6;
    ModKey[6] = m7;	ModKey[7] = m8;
    }
#endif
  }

void User::Update()  {
  updating = 1;

#ifdef X_WINDOWS
  if((!IN_X) && (Frame == NULL))  {
#endif
   int ctr, ctr2, ismod;
   for(ctr=0; ctr<buf_ind; ctr++)  {
    ismod = 0;
    for(ctr2=0; ismod == 0 && ctr2<8; ctr2++)  {
      if(ModKey[ctr2] == (keyboard_buf[ctr] & 127))  ismod = 1;
      }
    if(!ismod)  {
      if(keyboard_buf[ctr] & 128)  {
        keyboard_buf[ctr] &= 127;
	if(KeyRemap[keyboard_buf[ctr]])  {
	  if(KeyRemap[keyboard_buf[ctr]]->MouseInteraction() == SPRITE_BUTTON
	  	&& ((Button *)KeyRemap[keyboard_buf[ctr]])->IsPressed())
	    ((Button *)KeyRemap[keyboard_buf[ctr]])->Click();
	  }
	else
	  JustReleasedKey(keyboard_buf[ctr], modkey_buf[ctr]);
	}
      else  {
	if(KeyRemap[keyboard_buf[ctr]])  {
	  if(KeyRemap[keyboard_buf[ctr]]->MouseInteraction() == SPRITE_SBUTTON
	  	|| (!((Button *)KeyRemap[keyboard_buf[ctr]])->IsPressed()))
	    ((Button *)KeyRemap[keyboard_buf[ctr]])->Click();
	  }
	else
	  JustPressedKey(keyboard_buf[ctr], modkey_buf[ctr]);
	}
      }
    }
   buf_ind = 0;

#ifdef X_WINDOWS
   }
  else  {
    XFlush(___mydisplay);
    while(XCheckMaskEvent(___mydisplay, KeyReleaseMask | KeyPressMask,
	&___myevent)){
      if(___myevent.type == KeyPress)  {
        JustPressedKey(___myevent.xkey.keycode);
        keyboard_map[___myevent.xkey.keycode] = 1;
        }
      else  if(___myevent.type == KeyRelease)  {
        JustReleasedKey(___myevent.xkey.keycode);
        keyboard_map[___myevent.xkey.keycode] = 0;
        }
      else  printf("Got Unknown event of type %d!\r\n", ___myevent.type);
      }
    while(XCheckTypedEvent(___mydisplay, ClientMessage, &___myevent))  {
      if(((unsigned long)___myevent.xclient.data.l[0]) == closeatom)  {
        UserAction *tmpa;
        tmpa = new UserAction(USERACTION_SYSTEM_QUIT);
        ActionOccured(tmpa);
        }
      }
    }
//  else  {
//    // ********************************
//    }
#endif
  updating = 0;
  }

void User::MapKeyToButton(int which, int tobutt)  {
  MapKeyToButton(which, __Da_Screen->SpriteList[tobutt]);
  }

void User::MapKeyToButton(int which, Sprite &tobutt)  {
  MapKeyToButton(which, &tobutt);
  }

void User::MapKeyToButton(int which, Sprite *tobutt)  {
#ifdef DOS
  KeyRemap[which] = tobutt;
#endif

#ifdef X_WINDOWS
  if(IN_X || Frame != NULL)
    KeyRemap[XKeysymToKeycode(___mydisplay, which)] = tobutt;
  else KeyRemap[which] = tobutt;
#endif
  }

void User::UnmapKey(int which)  {
  KeyRemap[which] = NULL;
  }

void User::UnmapButton(Sprite *which)  {
  int ctr;
  for(ctr=0; ctr<128; ctr++)  {
    if(KeyRemap[ctr] == which)  KeyRemap[ctr] = NULL;
    }
  }

void User::JustPressedKey(int which)  {
  if(KeyRemap[which] == NULL)  {
    UserAction *tmpa;
    tmpa = new UserAction();
    tmpa->SetKeyPressed(which);
    ActionOccured(tmpa);
    }
  else  {
    if(keyboard_map[which] == 0 && (!((Button *)KeyRemap[which])->IsPressed()))
      ((Button *)KeyRemap[which])->Click();
    }
  }

void User::JustReleasedKey(int which)  {
  if(KeyRemap[which] == NULL)  {
    UserAction *tmpa;
    tmpa = new UserAction();
    tmpa->SetKeyReleased(which);
    ActionOccured(tmpa);
    }
  else  {
    if(keyboard_map[which] != 0 && ((Button *)KeyRemap[which])->IsPressed())
      ((Button *)KeyRemap[which])->Click();
    }
  }

void User::JustPressedKey(int which, int mod)  {
  if(KeyRemap[which] == NULL)  {
    UserAction *tmpa;
    tmpa = new UserAction();
    tmpa->SetKeyPressed(which, mod);
    ActionOccured(tmpa);
    }
  else  {
    if(keyboard_map[which] == 0 && (!((Button *)KeyRemap[which])->IsPressed()))
      ((Button *)KeyRemap[which])->Click();
    }
  }

void User::JustReleasedKey(int which, int mod)  {
  if(KeyRemap[which] == NULL)  {
    UserAction *tmpa;
    tmpa = new UserAction();
    tmpa->SetKeyReleased(which, mod);
    ActionOccured(tmpa);
    }
  else  {
    if(keyboard_map[which] != 0 && ((Button *)KeyRemap[which])->IsPressed())
      ((Button *)KeyRemap[which])->Click();
    }
  }

void User::JustPressedButton(int which)  {
  UserAction *tmpa;
  tmpa = new UserAction();
  tmpa->SetButtonPressed(which);
  ActionOccured(tmpa);
  }

void User::JustReleasedButton(int which)  {
  UserAction *tmpa;
  tmpa = new UserAction();
  tmpa->SetButtonReleased(which);
  ActionOccured(tmpa);
  }

void User::ActionOccured(const UserAction *inact)  {
  if(((UserAction *)inact)->modkeys == 0 && __Da_User != NULL)  {
    ((UserAction *)inact)->modkeys = __Da_User->GetModKeyStatus();
    }
  if(head == NULL)  {
    head = (UserAction *)inact;
    tail = (UserAction *)inact;
    }
  else  {
    tail->next = (UserAction *)inact;
    tail = (UserAction *)inact;
    }
  }

UserAction User::Action()  {
  UserAction ret;
  if(head == NULL)  return ret;
  ret = *head;
  ret.next = NULL;
  UserAction *tmph = head;
  head = head->next;
  delete tmph;
  if(head == NULL)  tail = NULL;
  return ret;
  }

UserAction User::WaitForAction()  {
  UserAction ret;
  while(head == NULL)  {
#ifdef X_WINDOWS
    XNextEvent(___mydisplay, &___myevent);
    XPutBackEvent(___mydisplay, &___myevent);
#endif
    if(__Da_Screen != NULL) __Da_Screen->RefreshFast();
    }
  ret = *head;
  ret.next = NULL;
  UserAction *tmph = head;
  head = head->next;
  delete tmph;
  if(head == NULL)  tail = NULL;
  return ret;
  }

void User::InitKey()  {
  currkey = 0;
  currscan = 0;
  updating = 0;
  if(__Da_User != NULL)  {
    delete this;
    Exit(1, "Two Users!  One per machine pal!\n");
    }
  bzero(KeyRemap, 128*(sizeof(Sprite *)));
  bzero(KeyStats, 128);
  bzero((char *)keyboard_map, 128);
  __Da_User = this;
  head = NULL;
  tail = NULL;

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
#ifdef X_WINDOWS
  if(!IN_X)  {
    if(Frame != NULL)  {
      XGrabKeyboard(___mydisplay, ___mywindow, True,
        GrabModeAsync, GrabModeAsync, CurrentTime);
      }
#ifdef SVGALIB
    else  {
      keyboard_init();
      keyboard_seteventhandler(vkh);
      }
#endif
    }
#endif

  KeyRepeatOff();
  SetModifyerKeys(SCAN_RSHIFT, SCAN_LSHIFT, SCAN_RCTRL,
	SCAN_LCTRL, SCAN_RALT, SCAN_LALT);
  }

User::~User()  {

#ifdef DOS
  __dpmi_set_protected_mode_interrupt_vector(KEYBOARD_INT, &old_handler);
  __dpmi_unlock_linear_region(&handler_area);
#endif

  KeyRepeatOn();
  UserAction *tmp=head;
#ifdef X_WINDOWS
#ifdef SVGALIB
  if((!IN_X) && (Frame == NULL))  {
    keyboard_close();
    keyboard_setdefaulteventhandler();
    }
#endif
#endif
  while(tmp != NULL)  {
    tmp=tmp->next;
    delete head;
    head=tmp;
    }
  __Da_User = NULL;
  }

void User::KeyCurrent()  {
  queue_keys = 0;
  }

void User::KeyQueue()  {
  queue_keys = 1;
  }

void User::ClearQueue()  {
  UserAction curact;
  curact = Action();
  while(curact.Type() != USERACTION_NONE)  {
    curact = Action();
    }
  }

char User::IsPressed(int key)  {

#ifdef X_WINDOWS
  if(updating == 0) Update();
  if(IN_X || Frame != NULL)
    return keyboard_map[XKeysymToKeycode(___mydisplay, key)];
  else return keyboard_map[key];
#endif

#ifdef DOS
  return keyboard_map[key];
#endif
  }

int User::GetAChar()  {
#ifdef DOS
  int ret = 0;
  while(!ret)  {
    ret = GetAChar2();
    }
  return ret;
#endif

#ifdef X_WINDOWS
#ifdef SVGALIB
  if(IN_X || Frame != NULL)
#endif
    XMaskEvent(___mydisplay, KeyPressMask, &___myevent);
    return (XKeysymToString(XLookupKeysym((XKeyEvent *)&___myevent, 0)))[0];
#ifdef SVGALIB
    }
  else  {
    return vga_getch();
    }
#endif
#endif
  }

int User::GetAChar2()  {
  Update();
  int ret = 0;
  UserAction curact;
  curact = Action();
  while(curact.Type() != USERACTION_KEYPRESSED
	&& curact.Type() != USERACTION_NONE)  {
    curact = Action();
    }
  if(curact.Type() == USERACTION_KEYPRESSED)  {
    ret = curact.Char();
    }
  return ret;
  }

int User::GetAChar(int in_delay)  {
  int ret = 0;

#ifdef DOS
  uclock_t time_started, to_delay = 0;
  time_started = uclock();
  to_delay = ((in_delay * UCLOCKS_PER_SEC) / 1);	//was / 1000
  while((!ret) && ((uclock() - time_started) < to_delay))  {
    ret = GetAChar2();
    }
#endif

#ifdef X_WINDOWS
  in_delay=0;	//UNUSED!!!
#ifdef SVGALIB
  if(IN_X || Frame != NULL)
#endif
    return GetAChar();
#ifdef SVGALIB
    }
  else  {
    return vga_getkey();
    }
#endif
#endif
  return ret;
  }

int User::GetAKey()  {
  int theinp = 1;
  while(theinp != 0)  {
    theinp = GetAKey2();
    if(__Da_Screen !=NULL)  __Da_Screen->Refresh();
    }
  while(theinp == 0)  {
    theinp = currscan;
    while(theinp == currscan)  {
      theinp = GetAKey2();
      if(__Da_Screen !=NULL)  __Da_Screen->Refresh();
      }
    currscan = theinp;
    }
  while(theinp != 0)  {
    theinp = GetAKey2();
    if(__Da_Screen !=NULL)  __Da_Screen->Refresh();
    }
  return currscan;
  }

char *User::GetClipboardString()  {
#ifdef DOS
  return NULL;
#endif
#ifdef X_WINDOWS
  Atom tmps = XInternAtom(___mydisplay, "UserText", False);
  XConvertSelection(___mydisplay, XA_PRIMARY, XA_STRING, tmps, ___mywindow, 0);
  XFlush(___mydisplay);
  XNextEvent(___mydisplay, &___myevent);
  while(___myevent.type != SelectionNotify)  {
    printf("%d<--\n", ___myevent.type);
    XNextEvent(___mydisplay, &___myevent);
    }
  if(___myevent.xselection.property == None)  return NULL;
  unsigned char *data;
  unsigned long ret_remaining, ret_length;
  long ret, datalength = 100;
  Atom ret_atom;
  int ret_format;
  ret=XGetWindowProperty(___mydisplay, ___mywindow,
	___myevent.xselection.property, 0L, datalength, False,
	AnyPropertyType, &ret_atom, &ret_format, &ret_length,
	&ret_remaining, &data);
  return (char *)data;
#endif
  }

int User::GetAKey2()  {
  if(keyboard_map[SCAN_A])  return SCAN_A;
  if(keyboard_map[SCAN_B])  return SCAN_B;
  if(keyboard_map[SCAN_C])  return SCAN_C;
  if(keyboard_map[SCAN_D])  return SCAN_D;
  if(keyboard_map[SCAN_E])  return SCAN_E;
  if(keyboard_map[SCAN_F])  return SCAN_F;
  if(keyboard_map[SCAN_G])  return SCAN_G;
  if(keyboard_map[SCAN_H])  return SCAN_H;
  if(keyboard_map[SCAN_I])  return SCAN_I;
  if(keyboard_map[SCAN_J])  return SCAN_J;
  if(keyboard_map[SCAN_K])  return SCAN_K;
  if(keyboard_map[SCAN_L])  return SCAN_L;
  if(keyboard_map[SCAN_M])  return SCAN_M;
  if(keyboard_map[SCAN_N])  return SCAN_N;
  if(keyboard_map[SCAN_O])  return SCAN_O;
  if(keyboard_map[SCAN_P])  return SCAN_P;
  if(keyboard_map[SCAN_Q])  return SCAN_Q;
  if(keyboard_map[SCAN_R])  return SCAN_R;
  if(keyboard_map[SCAN_S])  return SCAN_S;
  if(keyboard_map[SCAN_T])  return SCAN_T;
  if(keyboard_map[SCAN_U])  return SCAN_U;
  if(keyboard_map[SCAN_V])  return SCAN_V;
  if(keyboard_map[SCAN_W])  return SCAN_W;
  if(keyboard_map[SCAN_X])  return SCAN_X;
  if(keyboard_map[SCAN_Y])  return SCAN_Y;
  if(keyboard_map[SCAN_Z])  return SCAN_Z;

  if(keyboard_map[SCAN_0])  return SCAN_0;
  if(keyboard_map[SCAN_1])  return SCAN_1;
  if(keyboard_map[SCAN_2])  return SCAN_2;
  if(keyboard_map[SCAN_3])  return SCAN_3;
  if(keyboard_map[SCAN_4])  return SCAN_4;
  if(keyboard_map[SCAN_5])  return SCAN_5;
  if(keyboard_map[SCAN_6])  return SCAN_6;
  if(keyboard_map[SCAN_7])  return SCAN_7;
  if(keyboard_map[SCAN_8])  return SCAN_8;
  if(keyboard_map[SCAN_9])  return SCAN_9;

  if(keyboard_map[SCAN_SPACE])  return SCAN_SPACE;
  if(keyboard_map[SCAN_DOT])  return SCAN_DOT;
  if(keyboard_map[SCAN_COMA])  return SCAN_COMA;
  if(keyboard_map[SCAN_ENTER])  return SCAN_ENTER;
  if(keyboard_map[SCAN_ESC])  return SCAN_ESC;
  if(keyboard_map[SCAN_DEL])  return SCAN_DEL;
  if(keyboard_map[SCAN_BACKSP])  return SCAN_BACKSP;
  if(keyboard_map[SCAN_LEFT])  return SCAN_LEFT;
  if(keyboard_map[SCAN_RIGHT])  return SCAN_RIGHT;
  if(keyboard_map[SCAN_UP])  return SCAN_UP;
  if(keyboard_map[SCAN_DOWN])  return SCAN_DOWN;
  return 0;
  }

#ifdef X_WINDOWS
void User::vkh(int key, int on)  {
  key += ((1-on) << 7);
  keyboard_buf[buf_ind] = key;
  modkey_buf[buf_ind] = 0;
  if(ModKey[0] && keyboard_map[ModKey[0]])
    modkey_buf[buf_ind] |= 1;
  if(ModKey[1] && keyboard_map[ModKey[1]])
    modkey_buf[buf_ind] |= 2;
  if(ModKey[2] && keyboard_map[ModKey[2]])
    modkey_buf[buf_ind] |= 4;
  if(ModKey[3] && keyboard_map[ModKey[3]])
    modkey_buf[buf_ind] |= 8;
  if(ModKey[4] && keyboard_map[ModKey[4]])
    modkey_buf[buf_ind] |= 16;
  if(ModKey[5] && keyboard_map[ModKey[5]])
    modkey_buf[buf_ind] |= 32;
  if(ModKey[6] && keyboard_map[ModKey[6]])
    modkey_buf[buf_ind] |= 64;
  if(ModKey[7] && keyboard_map[ModKey[7]])
    modkey_buf[buf_ind] |= 128;
  buf_ind++;
  }
#endif

volatile unsigned char User::ModKey[10];
volatile char User::keyboard_map[KB_BUF_SIZE];
volatile char User::keyboard_buf[KB_BUF_SIZE];
volatile char User::modkey_buf[KB_BUF_SIZE];
volatile int User::buf_ind = 0;
volatile char User::queue_keys = 1;
