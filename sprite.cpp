#include <stdio.h>
#include <algorithm>
using std::min;
using std::max;

#include "engine.h"
#include "screen.h"
#include "sprite.h"
#include "graphic.h"
#include "list.h"

extern Screen *__Da_Screen;

Sprite::Sprite() {
  if(__Da_Screen == NULL)  U2_Exit(-1, "Must create Screen before Sprite!\n");
  pan = 0;
  drawn = 0;
  flags = 0;
  ownimage = 0;
  angle = 0;
  inum = 0;
  priority = 0;
  next = NULL;
  prev = NULL;
  remap.v = NULL;
  image = NULL;
  trueimage = NULL;
  snum = __Da_Screen->RegisterSprite(this);
  }

Sprite::Sprite(const Graphic &g) {
  if(__Da_Screen == NULL)  U2_Exit(-1, "Must create Screen before Sprite!\n");
  pan = 0;
  drawn = 0;
  flags = 0;
  ownimage = 0;
  angle = 0;
  inum = 0;
  priority = 0;
  next = NULL;
  prev = NULL;
  remap.v = NULL;
  image = NULL;
  trueimage = NULL;
  snum = __Da_Screen->RegisterSprite(this);
  SetImage(g);
  }

void Sprite::SetImage(const Graphic *g) {
  SetImage(*g);
  }

void Sprite::SetImage(const Graphic &g) {
  UserDebug("User:Sprite:SetImage() Begin");
  int dr = drawn;
  if(dr) Erase();
  if(ownimage && image != NULL) delete image;
  if(ownimage && trueimage != NULL && image != trueimage) delete trueimage;
  ownimage = 1;
  image = new Graphic(g);
  inum = 1;
  trueimage = image;
//  if(dr) Draw();
  SetupBinFlags();
  UserDebug("User:Sprite:SetImage() End");
  }

void Sprite::UseImage(const Graphic &g) {
  SetImage(&g);
  }

void Sprite::UseImage(const Graphic *g) {
  UserDebug("User:Sprite:UseImage() Begin");
  int dr = drawn;
  if(dr) Erase();
  if(ownimage && image != NULL) delete image;
  if(ownimage && trueimage != NULL && image != trueimage) delete trueimage;
  ownimage = 0;
  image = (Graphic *)g;
  inum = 1;
  trueimage = image;
//  if(dr) Draw();
  SetupBinFlags();
  UserDebug("User:Sprite:UseImage() End");
  }

IntList Sprite::CMove(int x, int y) {
  if(image == NULL) { IntList ret; return ret; }
  Erase();
  Draw(x, y);
  return __Da_Screen->CollideRectangle(snum,
	xpos, ypos, image->xsize, image->ysize);
  }

IntList Sprite::CMove(int x, int y, int a) {
  if(image == NULL) { IntList ret; return ret; }
  Erase();
  Draw(x, y, a);
  return __Da_Screen->CollideRectangle(snum,
	xpos, ypos, image->xsize, image->ysize);
  }

IntList Sprite::CDraw(int x, int y, int a) {
  if(drawn || image == NULL) { IntList ret; return ret; }
  if(angle != a) {
    if(image != trueimage) delete image;
    image = new Graphic(trueimage->Rotated(a));
    }
  x-=image->xcenter; y-=image->ycenter;
  if(flags&(SPRITE_RECTANGLE|SPRITE_SOLID))
    __Da_Screen->DrawGraphicFG(*image, x, y, pan);
//  else if(flags&(SPRITE_RECTANGLE))
//    __Da_Screen->DrawTransparentGraphicFG(*image, x, y, pan);
//  else if(flags&(SPRITE_SOLID))
//    __Da_Screen->DrawTransparentGraphicFG(*image, x, y, pan);
  else
    __Da_Screen->DrawTransparentGraphicFG(*image, x, y, pan);
  xpos = x; ypos = y; drawn = 1; __Da_Screen->DropSprite(this); 
  return __Da_Screen->CollideRectangle(snum,
	xpos, ypos, image->xsize, image->ysize);
  }

IntList Sprite::CDraw(int x, int y) {
  if(drawn || image == NULL) { IntList ret; return ret; }
  x-=image->xcenter; y-=image->ycenter;
  __Da_Screen->DrawTransparentGraphicFG(*image, x, y, pan);
  xpos = x; ypos = y; drawn = 1; __Da_Screen->DropSprite(this);
  return __Da_Screen->CollideRectangle(snum,
	xpos, ypos, image->xsize, image->ysize);
  }

IntList Sprite::CDraw() {
  if(drawn || image == NULL) { IntList ret; return ret; }
  __Da_Screen->DrawTransparentGraphicFG(*image, xpos, ypos, pan);
  drawn = 1; __Da_Screen->DropSprite(this);
  return __Da_Screen->CollideRectangle(snum,
	xpos, ypos, image->xsize, image->ysize);
  }

void Sprite::Move(int x, int y, int a) {
  if(image == NULL) return;
  Erase();
  Draw(x, y, a);
  }

void Sprite::Move(int x, int y) {
  UserDebug("User:Sprite:Move(x,y) Begin");
  if(image == NULL) return;
  UserDebug("User:Sprite:Move(x,y) Before Erase");
  Erase();
  UserDebug("User:Sprite:Move(x,y) Before Draw");
  Draw(x, y);
  UserDebug("User:Sprite:Move(x,y) End");
  }

void Sprite::Position(int x, int y) {
  UserDebug("User:Sprite:Position(x,y) Begin");
  if(image == NULL) return;
  UserDebug("User:Sprite:Position(x,y) Before First Invalid");
//  if(drawn && __Da_Screen != NULL)
//    __Da_Screen->InvalidateRectangle(xpos, ypos, image->xsize, image->ysize);
  UserDebug("User:Sprite:Position(x,y) Before x,y");
  x-=image->xcenter; y-=image->ycenter;
  if(drawn) __Da_Screen->LiftSprite(this);
  xpos = x; ypos = y; drawn=1; __Da_Screen->DropSprite(this);
  UserDebug("User:Sprite:Position(x,y) Before Second Invalid");
//  if(__Da_Screen != NULL)
//    __Da_Screen->InvalidateRectangle(xpos, ypos, image->xsize, image->ysize);
  UserDebug("User:Sprite:Position(x,y) End");
  }

void Sprite::Draw(int x, int y, int a) {
  if(drawn || image == NULL) return;
  if(angle != a) {
    if(image != trueimage) delete image;
    image = new Graphic(trueimage->Rotated(a));
    }
  x-=image->xcenter; y-=image->ycenter;
//  __Da_Screen->DrawTransparentGraphicFG(*image, x, y, pan);
  xpos = x; ypos = y; drawn = 1; __Da_Screen->DropSprite(this);
  if(image != NULL && __Da_Screen != NULL)
    __Da_Screen->RestoreRectangle(xpos, ypos, image->xsize, image->ysize);
  }

void Sprite::Draw(int x, int y) {
  if(drawn || image == NULL) return;
  UserDebug("User:Sprite:Draw(x,y) Begin");
  x-=image->xcenter; y-=image->ycenter;
//  __Da_Screen->DrawTransparentGraphicFG(*image, x, y, pan);
  UserDebug("User:Sprite:Draw(x,y) Middle");
  xpos = x; ypos = y; drawn = 1; __Da_Screen->DropSprite(this);
  if(image != NULL && __Da_Screen != NULL)
    __Da_Screen->RestoreRectangle(xpos, ypos, image->xsize, image->ysize);
  UserDebug("User:Sprite:Draw(x,y) End");
  }

void Sprite::Draw() {
  if(drawn || image == NULL) return;
//  __Da_Screen->DrawTransparentGraphicFG(*image, xpos, ypos, pan);
  drawn = 1; __Da_Screen->DropSprite(this);
  if(image != NULL && __Da_Screen != NULL)
    __Da_Screen->RestoreRectangle(xpos, ypos, image->xsize, image->ysize);
  }

void Sprite::Position() {
  UserDebug("User:Sprite:Position() Begin");
  if(image == NULL) return;
  if(drawn) __Da_Screen->LiftSprite(this);
  drawn=1; __Da_Screen->DropSprite(this);
  UserDebug("User:Sprite:Position() End");
  }

void Sprite::Remove() {
  if(drawn) __Da_Screen->LiftSprite(this);
  drawn = 0;
  }

void Sprite::Erase() {
  if(!drawn) return;
  drawn = 0; __Da_Screen->LiftSprite(this);
  if(image != NULL && __Da_Screen != NULL)
    __Da_Screen->RestoreRectangle(xpos, ypos, image->xsize, image->ysize);
  }

void Sprite::DefLinH(char *l) {
  if(image == NULL) U2_Exit(-1, "DefLinH on undeclared Graphic in Sprite!\n");
  trueimage->DefLinH(l);
  }

void Sprite::DefLin(char *l) {
  if(image == NULL) U2_Exit(-1, "DefLin on undeclared Graphic in Sprite!\n");
  trueimage->DefLin(l);
  }

void Sprite::DefSize(int x, int y) {
  if(image == NULL) { image = new Graphic(); trueimage = image; }
  trueimage->DefSize(x, y);
  }

int Sprite::Hits(int x, int y, int xs, int ys) {
  UserDebug("User:Sprite:Hits(x,y,z) Start");
  int ctrx, ctry;

  if(image == NULL) U2_Exit(-1, "Hitting Nothing!\n");

  if(image->depth == 8)  {
    UserDebug("User:Sprite:Hits(x,y,z) 8-bit Loop Start");
    for(ctry=max(ypos, y); ctry < min(ypos+image->ysize, y+ys); ctry++)  {
      for(ctrx=max(xpos, x); ctrx < min(xpos+image->xsize, x+xs); ctrx++)  {
	if(image->image[ctry-ypos].uc[ctrx-xpos] != image->tcolor) {
	  return 1;
	  }
	}
      }
    UserDebug("User:Sprite:Hits(x,y,z) 8-bit Loop End");
    }
  else if(image->depth == 16)  {
    UserDebug("User:Sprite:Hits(x,y,z) 16-bit Loop Start");
    for(ctry=max(ypos, y); ctry < min(ypos+image->ysize, y+ys); ctry++)  {
      for(ctrx=max(xpos, x); ctrx < min(xpos+image->xsize, x+xs); ctrx++)  {
	if(image->image[ctry-ypos].us[ctrx-xpos] != image->tcolor) {
	  return 1;
	  }
	}
      }
    UserDebug("User:Sprite:Hits(x,y,z) 16-bit Loop End");
    }
  else if(image->depth == 32)  {
    UserDebug("User:Sprite:Hits(x,y,z) 32-bit Loop Start");
    for(ctry=max(ypos, y); ctry < min(ypos+image->ysize, y+ys); ctry++)  {
      for(ctrx=max(xpos, x); ctrx < min(xpos+image->xsize, x+xs); ctrx++)  {
	if(image->image[ctry-ypos].uc[((ctrx-xpos)<<2)+3]) {
	  return 1;
	  }
	}
      }
    UserDebug("User:Sprite:Hits(x,y,z) 32-bit Loop End");
    }
  else U2_Exit(-1, "Unknown Depth Error (%ld) in %s\n", image->depth, __PRETTY_FUNCTION__);
  UserDebug("User:Sprite:Hits(x,y,z) End");
  return 0;
  }

int Sprite::Hits(Sprite *s) {
  UserDebug("User:Sprite:Hits 0000");
  int ctrx, ctry;

  UserDebug("User:Sprite:Hits 0500");
  if(image->depth != s->image->depth)
	U2_Exit(-1, "Depth Mismatch %ld->%ld!\n", image->depth, s->image->depth);
  if(image == NULL || s->image == NULL) U2_Exit(-1, "Hitting Nothing!\n");

  if(image->depth == 8)  {
    for(ctry=max(ypos, s->ypos); ctry <
		min(ypos+image->ysize, s->ypos+s->image->ysize); ctry++)  {
      for(ctrx=max(xpos, s->xpos); ctrx <
		min(xpos+image->xsize, s->xpos+s->image->xsize); ctrx++)  {
	if((image->image[ctry-ypos].uc[ctrx-xpos] != image->tcolor)
	  	&& (s->image->image[ctry-(s->ypos)].uc[ctrx-(s->xpos)]
		!= s->image->tcolor)) {
	  return 1;
	  }
	}
      }
    }
  else if(image->depth == 32)  {
    for(ctry=max(ypos, s->ypos); ctry <
		min(ypos+image->ysize, s->ypos+s->image->ysize); ctry++)  {
      for(ctrx=max(xpos, s->xpos); ctrx <
		min(xpos+image->xsize, s->xpos+s->image->xsize); ctrx++)  {
	UserDebug("User:Sprite:Hits 0600");
	if(image->image[ctry-ypos].uc[(ctrx-xpos)*4+3] != image->tcolor)  {
	  UserDebug("User:Sprite:Hits 0602");
	  if(s->image->image[ctry-(s->ypos)].uc[(ctrx-(s->xpos))*4+3]
		!= s->image->tcolor) {
	    UserDebug("User:Sprite:Hits 0999");
	    return 1;
	    }
	  }
	UserDebug("User:Sprite:Hits 0605");
	}
      }
    }
  else U2_Exit(-1, "Unknown Depth Error (%ld) in %s\n", image->depth, __PRETTY_FUNCTION__);
  UserDebug("User:Sprite:Hits 1000");
  return 0;
  }

Sprite::~Sprite() {
  UserDebug("User:Sprite:~Sprite Begin");
  Erase();
  if(ownimage && image != NULL) delete image;
  if(ownimage && trueimage != NULL && image != trueimage) delete trueimage;
  if(__Da_Screen != NULL) __Da_Screen->RemoveSprite(snum, this);
  UserDebug("User:Sprite:~Sprite End");
  }

void Sprite::RedrawArea(int x, int y, int xs, int ys)  {
  int XP = xpos, YP = ypos;
  int XS = image->xsize, YS = image->ysize;
  if(x > XP+XS || y > YP+YS || x+xs <= XP || y+ys <= YP) return;

  x -= XP; y -= YP;
  if(x<0) { xs+=x; x=0; }
  if(y<0) { ys+=y; y=0; }
  if(x+xs > XS) xs = XS-x;
  if(y+ys > YS) ys = YS-y;

  if(flags&(SPRITE_RECTANGLE|SPRITE_SOLID))
    if(remap.v == NULL)
      __Da_Screen->DrawPartialGraphicFG(*image,XP,YP,x,y,xs,ys,pan);
    else
      __Da_Screen->RCDrawPartialGraphicFG(*image,remap,XP,YP,x,y,xs,ys,pan);
  else
    if(remap.v == NULL)
      __Da_Screen->DrawPartialTransparentGraphicFG(*image,XP,YP,x,y,xs,ys,pan);
    else
      __Da_Screen->RCDrawPartialTransparentGraphicFG(*image,remap,XP,YP,x,y,xs,ys,pan);
  }

int Sprite::XPos() {
  if(image==NULL) return xpos;
  else return xpos + image->xcenter;
  }

int Sprite::YPos() {
  if(image==NULL) return ypos;
  else return ypos + image->ycenter;
  }

int Sprite::XCenter() {
  if(image==NULL) return 0;
  else return image->xcenter;
  }

int Sprite::YCenter() {
  if(image==NULL) return 0;
  else return image->ycenter;
  }

void Sprite::SetColormap(unsigned long *cm) {
  remap.ul = cm;
  }

void Sprite::SetColormap(unsigned char *cm) {
  remap.uc = cm;
  }

void Sprite::SetColormap(unsigned short *cm) {
  remap.us = cm;
  }

void Sprite::SetLine(int x, int y, int d, color c) {
  Erase();
  if(image == NULL) image=new Graphic;
  image->SetLine(x, y, d, c);
  SetupBinFlags();
  }

void Sprite::SetupBinFlags() {
  if(image == NULL) return;
  if(image->xsize>(BIN_SIZE+1) || image->ysize>(BIN_SIZE+1))
    SetFlag(SPRITE_LARGE);
  else ClearFlag(SPRITE_LARGE);
  if(image->xsize>(LARGE_BIN_SIZE+1) || image->ysize>(LARGE_BIN_SIZE+1))
    SetFlag(SPRITE_HUGE);
  else ClearFlag(SPRITE_HUGE);
  }
