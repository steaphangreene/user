#ifndef INSOMNIA_SCREEN_H
#define INSOMNIA_SCREEN_H

#include <user/os_defs.h>
#include <user/palette.h>

#define MOD_RSHIFT       0
#define MOD_LSHIFT       1
#define MOD_RCTRL        2
#define MOD_LCTRL        3
#define MOD_RALT         4
#define MOD_LALT         5

#define USERACTION_NONE			-1
#define USERACTION_MOUSE		0
#define USERACTION_KEYPRESSED		1
#define USERACTION_KEYRELEASED		2
#define USERACTION_BUTTONPRESSED	3
#define USERACTION_BUTTONRELEASED	4
#define USERACTION_SYSTEM_QUIT		5
#define USERACTION_SYSTEM_RESIZE	6

#define SPRITE_BUTTON	1
#define SPRITE_SBUTTON	2

#define KB_BUF_SIZE     128

#ifdef DOS
#include <dpmi.h>
#endif

#include <time.h>
#include <user/graphic.h>
#include <user/sound.h>
#include <user/keyboard.h>
#include "list.h"

#define MAX_PANEL	10
#define MAX_SPRITES     16384
#define WITH_SPRITES    (0x0F2)

#define MODE_320x200	0x13
#define MODE_640x400	0x100
#define MODE_640x480	0x101
#define MODE_800x600	0x103
#define MODE_1024x768	0x105
#define MODE_1280x1024	0x107

#include "engine.h"

typedef int Panel;

class Sprite {
  public:
  Sprite();
  Sprite(const Graphic &);
  Sprite(const Graphic *);
  Sprite(int, int);
  ~Sprite();
  IntList Move(int, int);
  IntList Move(int, int, int);
  IntList Move(int, int, int, int, int);
  IntList Move(int, int, double, int, int, int);
  void SetPanel(Panel);
  void SetImage(const Graphic *);
  void SetImage(const Graphic &);
  void UseImage(const Graphic *);
  void UseImage(const Graphic &);
  void SetPriority(int);
  void SetMouseInteraction(unsigned long inp)  { mouseinter = inp; };
  void DefSize(int, int);
  void Trim()  {image->Trim();};
  void FindTrueCenter()  {image->FindTrueCenter();};
  void EnableCollisions()  { collisions = 1; };
  void DisableCollisions()  { collisions = 0; };
  void EnableScrolling()  { scrolls = 1; };
  void DisableScrolling()  { scrolls = 0; };
  void DefLin(char*);
  void DefLinH(char*);
  void DefBac();
  int SpriteNumber()  {  return spnum; };
  int Visible()  {  return visible; };
  IntList Draw();
  IntList Draw(int, int);
  void Erase();
  int XSize() { return image->xsize; };
  int YSize() { return image->ysize; };
  int XPos() { return xpos+image->xcenter; };
  int YPos() { return ypos+image->ycenter; };
  int XCenter() { return image->xcenter; };
  int YCenter() { return image->ycenter; };
  void SetColormap(char *);
  void SetTransparentColor(int);

  protected:
  IntList PutBlock(int, int);
  void DrawBlock(int, int);
  IntList HitBlock(int, int, unsigned long, int);
  short xpos, ypos, angle;
  unsigned bgysz;
  unsigned bgsz;
  unsigned xblocks;
  Graphic* image;
  Graphic* trueimage;
  unsigned long ** collide;
  unsigned priority;
  char boff;
  char ownimage;
  char collisions;
  char scrolls;
  char visible;
  char panel;
  unsigned char tcol;
  int spnum;
  unsigned long mouseinter;
  unsigned xsize, ysize;
  Sprite ***behind;
  Sprite ***infront;
  friend class Mouse;
  friend class Screen;
  char *colormap;
  };

class Button : public Sprite {
  public:
  Button();
  Button(int, int, const char *, Color, Color, Color, Color);
  ~Button();
  void SetImage(const Graphic &, const Graphic &);
  void SetImage(const Graphic *, const Graphic *);
  void UseImage(const Graphic &, const Graphic &);
  void UseImage(const Graphic *, const Graphic *);
  void Click();
  void StealthClick();
  void Disable()  { disabled = 1; };
  void Enable()  { disabled = 0; };
  int IsEnabled()  { return (1-disabled); };
  int IsDisabled()  { return disabled; };
  void SetSound(DigSample *pw, DigSample *rw)
	{ PressWave=pw; ReleaseWave=rw; };
  int IsPressed()  { return (image == clickedg);};
  void Create(int, int, const char *, Color, Color, Color, Color);

  private:
  Graphic *normg, *clickedg;
  DigSample *PressWave, *ReleaseWave;
  char disabled;
  };

#include <user/mouse.h>

class UserAction : public IntList {
  public:
  UserAction(const UserAction &in);
  UserAction();
  ~UserAction();
  UserAction(int);
  UserAction(Panel, char, int, int);
  UserAction(Panel, char, int, int, int, int);
  UserAction operator =(const UserAction &in);
  UserAction operator =(const IntList &in);
  void SetKeyPressed(int);
  void SetKeyReleased(int);
  void SetKeyPressed(int, int);
  void SetKeyReleased(int, int);
  void SetButtonPressed(int);
  void SetButtonReleased(int);
  char Type()  {return type;};
  int Startx()  {return startx;};
  int Starty()  {return starty;};
  int Endx()  {return endx;};
  int Endy()  {return endy;};
  int ButtonPressed()  {return butt;};
  int Key()  {return butt;};
  int Char()  {return pan;};
  int Pan()  {return pan;};
  int ModKeyPressed(int n)  { return modkeys & (1 << n); };
  int ModKeys()  { return modkeys; };

  private:
  unsigned long modkeys;
  char type;
  int butt;
  int pan;
  int startx;
  int starty;
  int endx;
  int endy;
  UserAction *next;
  friend class User;
  };

class Screen {
  public: 
  Screen();
  Screen(const char *);
  Screen(int);
  Screen(const char *, int);
  Screen(int, int);
  Screen(const char *, int, int);
  ~Screen();
  Sprite *SpriteList[MAX_SPRITES];
  void ScrollPanel32(Panel, int, int);
  void Scroll32(int, int);
  void ScrollPanel(Panel, int, int);
  void Scroll(int, int);
  void Scroll1(int, int);
  void Scroll2(int, int);
  void SaveBack(int, int);
  void RestoreBack(int, int);
  void DeleteBack(int, int);
  IntList EraseSpritesHere(int, int);
  void Redraw(int, int);
  void RedrawSprites(const IntList &);
  unsigned GetXSize()  {  return pxend[0];  };
  unsigned GetYSize()  {  return pyend[0];  };
  void PastePartialGraphic(const Graphic &, int, int, int, int, int, int);
  void PasteGraphic(const Graphic &, int, int);
  void FullScreenGraphic(const Graphic &);
  void FullScreenBMP(char *);
  void GetBMPPalette(char *);
  void GetPSPPalette(char *);
  void PasteBMP(char *, int, int);
  void ClearScreen();
  void BlankScreen();
  void ShowScreen();
  void FadeIn();
  void FadeIn(int);
  void FadeOut();
  void FadeOut(int);
  void FillRectangle(int, int, int, int, Color);
  void BorderRectangle(int, int, int, int, Color);
  void SetPoint(int, int, Color);
  Color GetPoint(int, int);
  void BSetPoint(int, int, Color);
  void GetFriendlyColor(Color &a, unsigned char b, unsigned char c, 
	unsigned char d)  {GetFriendlyColor(&a, b, c, d);};
  void GetFriendlyColor(Color *, unsigned char, unsigned char, 
	unsigned char);
  void SetPaletteEntry(Color, unsigned char, unsigned char, 
	unsigned char);
  void SetBlock(int a, int b, unsigned char *c)  {
    SetBlock32(a, b, c);
    }
  void SetBlock16(int, int, unsigned char *);
  void SetBlock32(int, int, unsigned char *);
  void SetBlock64(int, int, unsigned char *);
  void SSetBlock(int, int, int, unsigned char *);
  void BSetBlock(int, int, unsigned char *);
  unsigned char *GetBlock16(int, int);
  unsigned char *GetBlock32(int, int);
  unsigned char *GetBlock64(int, int);
  void Refresh();
  void RefreshFull();
  void RefreshFast();
  int AddSpriteToList(Sprite *);
  void RemoveSpriteFromList(int);
  Panel NewPanel(int, int, int, int);
  void DeletePanel(Panel);
  void ErasePanelBackground(Panel);
  void RestorePanelBackground(Panel);
  IntList ErasePanelSprites(Panel);
  unsigned char *BackupPanel(Panel);
  void RestorePanel(Panel, unsigned char *);
  int PanelXStart(Panel pan)  { return pxstart[pan]; };
  int PanelYStart(Panel pan)  { return pystart[pan]; };
  int PanelXEnd(Panel pan)  { return pxend[pan]; };
  int PanelYEnd(Panel pan)  { return pyend[pan]; };
  void SetSize(int, int);
  void SetFrameRate(int);
  int ModeSupported(int);
  int SetFont(char *);
  int SetFont(char *, char *);
  int print(Color, Color, char *);
  int printf(Color, Color, const char *, ...)
	__attribute__ ((format (printf, 4, 5)));
  Graphic gprintf(Color, Color, const char *, ...)
	__attribute__ ((format (printf, 4, 5)));
  Graphic String2Graphic(Color, Color, const char *);
  void PositionCursor(int, int);
  void SetCursor(const Graphic &);
  void HideCursor();
  void SetTextPanel(Panel);
  int XPos();
  int YPos();
  unsigned char *DoubleBuffer();
  unsigned char *DoubleBuffer(int);
  unsigned char *DoubleBuffer(int, int);
  void GiveBlock(unsigned char *, unsigned char *);
  void TakeBlock(unsigned char *, unsigned char *);
  void InvalidateRectangle(int, int, int, int);
  Sprite *GetSpriteByNumber(int n)  {
    return SpriteList[n];
    }
  Button *GetButtonByNumber(int n)  {
    return (Button*)SpriteList[n];
    }
  void SetDefaultTransparentColor(int);
  int deftran;
  void ClipToPanel(int &x, int &y, Panel w);
  int DefaultXSize();
  int DefaultYSize();
  int MaxXSize();
  int MaxYSize();

  private:
  char *Name;
  Sprite *Cursor;
  Panel textp;
  void AlignCursor();
  Graphic **font;
  int curx, cury;
  short *vmode;
  unsigned char *VESABlock;
  void InitScreen();

#ifdef DOS
  void SetPage(int);
  int GetLogicalLineLength();
  int SetLogicalLineLength(int);
  char **update;
  void SetMode(int);
#endif
#ifdef X_WINDOWS
  int upx1, upy1, upx2, upy2;
#endif

  void WaitForNextFrame();
  int SetScreenMode(int);
  int curpage;
  int pages;
  int numvmodes;
  int granularity;
  Sprite ***spritehere;
  unsigned char ***backg;
  int pxstart[MAX_PANEL];
  int pystart[MAX_PANEL];
  int pxend[MAX_PANEL];
  int pyend[MAX_PANEL];
  int Xlen;
  char nextpanel;
  unsigned char blank;
  unsigned char graph;
  void SetPalette(unsigned char *);
  void SetPalette(const Palette &);
  unsigned long VidMem;
  unsigned char *VidBuf;
  unsigned mode;
  long XYsize;
#ifdef DOS
  uclock_t framedelay;
  uclock_t lasttime;
#endif
#ifdef X_WINDOWS
  clock_t framedelay;
  clock_t lasttime;
#endif
  Palette palette;
//  unsigned char Palette[768];
  friend class SoundCard;
  friend class Sprite;
  friend class Button;
  friend class Mouse;
  };

class User {
  public: 
  User();
  ~User();
  unsigned long GetModKeyStatus();
  void SetModifyerKeys();
  void SetModifyerKeys(int);
  void SetModifyerKeys(int, int);
  void SetModifyerKeys(int, int, int);
  void SetModifyerKeys(int, int, int, int);
  void SetModifyerKeys(int, int, int, int, int);
  void SetModifyerKeys(int, int, int, int, int, int);
  void SetModifyerKeys(int, int, int, int, int, int, int);
  void SetModifyerKeys(int, int, int, int, int, int, int, int);
  void KeyRepeatOn();
  void KeyRepeatOff();
  char IsPressed(int key);
  int GetAChar(int);
  int GetAChar();
  int GetAKey();
  void MapKeyToButton(int, int);
  void MapKeyToButton(int, Sprite *);
  void MapKeyToButton(int, Sprite &);
  void UnmapKey(int);
  void UnmapButton(Sprite *);
  void JustPressedKey(int);
  void JustReleasedKey(int);
  void JustPressedKey(int, int);
  void JustReleasedKey(int, int);
  void JustPressedButton(int);
  void JustReleasedButton(int);
  void KeyCurrent();
  void KeyQueue();
  void ClearQueue();
  UserAction Action();
  UserAction WaitForAction();
  UserAction *NextAction()  {return head;};
  void Update();
  void GenerateAction(const UserAction &);
  char *GetClipboardString();

  private:

#ifdef DOS
  __dpmi_meminfo handler_area;
  __dpmi_paddr old_handler;
  __dpmi_paddr handler_seginfo;

  static volatile void keyboard_handler();
  static volatile void dummy_marker_function();
#endif

#ifdef X_WINDOWS
  unsigned long closeatom;
  void vkh(int, int);
#endif

  void ActionOccured(const UserAction *);
  static volatile unsigned char ModKey[10];
  static volatile char keyboard_map[128];
  static volatile char keyboard_buf[128];
  static volatile char modkey_buf[128];
  static volatile int buf_ind;
  static volatile char queue_keys;

  Sprite *KeyRemap[128];
  char KeyStats[128];
  UserAction *head;
  UserAction *tail;
  void InitKey();
  int GetAKey2();
  int GetAChar2();
  int currkey;
  int currscan;
  int updating;
  friend class UserAction;
  friend class Mouse;
  };

#endif
