#include <stdio.h>

#include "engine.h"
#include "screen.h"
#include "sprite.h"
#include "graphic.h"
#include "list.h"

extern Screen *__Da_Screen;

Sprite::Sprite() {
  if(__Da_Screen == NULL)  Exit(-1, "Must create Screen before Sprite!\n");
  pan = 0;
  drawn = 0;
  flags = 0;
  ownimage = 0;
  angle = 0;
  inum = 0;
  priority = 0;
  remap = NULL;
  image = NULL;
  trueimage = NULL;
  snum = __Da_Screen->RegisterSprite(this);
  }

Sprite::Sprite(const Graphic &g) {
  if(__Da_Screen == NULL)  Exit(-1, "Must create Screen before Sprite!\n");
  pan = 0;
  drawn = 0;
  flags = 0;
  ownimage = 0;
  angle = 0;
  inum = 0;
  priority = 0;
  remap = NULL;
  image = NULL;
  trueimage = NULL;
  snum = __Da_Screen->RegisterSprite(this);
  SetImage(g);
  }

void Sprite::SetImage(const Graphic *g) {
  SetImage(*g);
  }

void Sprite::SetImage(const Graphic &g) {
  Debug("User:Sprite:SetImage() Begin");
  int dr = drawn;
  if(dr) Erase();
  if(ownimage && image != NULL) delete image;
  if(ownimage && trueimage != NULL && image != trueimage) delete trueimage;
  ownimage = 1;
  image = new Graphic(g);
  inum = 1;
  trueimage = image;
//  if(dr) Draw();
  Debug("User:Sprite:SetImage() End");
  }

void Sprite::UseImage(const Graphic &g) {
  SetImage(&g);
  }

void Sprite::UseImage(const Graphic *g) {
  int dr = drawn;
  if(dr) Erase();
  if(ownimage && image != NULL) delete image;
  if(ownimage && trueimage != NULL && image != trueimage) delete trueimage;
  ownimage = 0;
  image = (Graphic *)g;
  inum = 1;
  trueimage = image;
//  if(dr) Draw();
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
  xpos = x; ypos = y; drawn = 1;
  return __Da_Screen->CollideRectangle(snum,
	xpos, ypos, image->xsize, image->ysize);
  }

IntList Sprite::CDraw(int x, int y) {
  if(drawn || image == NULL) { IntList ret; return ret; }
  x-=image->xcenter; y-=image->ycenter;
  __Da_Screen->DrawTransparentGraphicFG(*image, x, y, pan);
  xpos = x; ypos = y; drawn = 1;
  return __Da_Screen->CollideRectangle(snum,
	xpos, ypos, image->xsize, image->ysize);
  }

IntList Sprite::CDraw() {
  if(drawn || image == NULL) { IntList ret; return ret; }
  __Da_Screen->DrawTransparentGraphicFG(*image, xpos, ypos, pan);
  drawn = 1;
  return __Da_Screen->CollideRectangle(snum,
	xpos, ypos, image->xsize, image->ysize);
  }

void Sprite::Move(int x, int y) {
  Debug("User:Sprite:Move(x,y) Begin");
  if(image == NULL) return;
  Debug("User:Sprite:Move(x,y) Before Erase");
  Erase();
  Debug("User:Sprite:Move(x,y) Before Draw");
  Draw(x, y);
  Debug("User:Sprite:Move(x,y) End");
  }

void Sprite::Position(int x, int y) {
  Debug("User:Sprite:Position(x,y) Begin");
  if(image == NULL) return;
  Debug("User:Sprite:Position(x,y) Before Erase");
  Erase();
  Debug("User:Sprite:Position(x,y) Before x,y");
  x-=image->xcenter; y-=image->ycenter;
  xpos = x; ypos = y;
  Debug("User:Sprite:Position(x,y) End");
  }

void Sprite::Draw(int x, int y, int a) {
  if(drawn || image == NULL) return;
  if(angle != a) {
    if(image != trueimage) delete image;
    image = new Graphic(trueimage->Rotated(a));
    }
  x-=image->xcenter; y-=image->ycenter;
  __Da_Screen->DrawTransparentGraphicFG(*image, x, y, pan);
  xpos = x; ypos = y; drawn = 1;
  if(image != NULL && __Da_Screen != NULL)
    __Da_Screen->RestoreRectangle(xpos, ypos, image->xsize, image->ysize);
  }

void Sprite::Draw(int x, int y) {
  if(drawn || image == NULL) return;
  Debug("User:Sprite:Draw(x,y) Begin");
  x-=image->xcenter; y-=image->ycenter;
//  __Da_Screen->DrawTransparentGraphicFG(*image, x, y, pan);
  Debug("User:Sprite:Draw(x,y) Middle");
  xpos = x; ypos = y; drawn = 1;
  if(image != NULL && __Da_Screen != NULL)
    __Da_Screen->RestoreRectangle(xpos, ypos, image->xsize, image->ysize);
  Debug("User:Sprite:Draw(x,y) End");
  }

void Sprite::Draw() {
  if(drawn || image == NULL) return;
//  __Da_Screen->DrawTransparentGraphicFG(*image, xpos, ypos, pan);
  drawn = 1;
  if(image != NULL && __Da_Screen != NULL)
    __Da_Screen->RestoreRectangle(xpos, ypos, image->xsize, image->ysize);
  }

void Sprite::Position() {
  Debug("User:Sprite:Position() Begin");
  if(image == NULL) return;
  Debug("User:Sprite:Position() Before Erase");
  Erase();
  drawn=1;
  Debug("User:Sprite:Position() End");
  }

void Sprite::Remove() {
  drawn = 0;
  }

void Sprite::Erase() {
  if(!drawn) return;
  drawn = 0;
  if(image != NULL && __Da_Screen != NULL)
    __Da_Screen->RestoreRectangle(xpos, ypos, image->xsize, image->ysize);
  }

void Sprite::DefLinH(char *l) {
  if(image == NULL) Exit(-1, "DefLinH on undeclared Graphic in Sprite!\n");
  trueimage->DefLinH(l);
  }

void Sprite::DefLin(char *l) {
  if(image == NULL) Exit(-1, "DefLin on undeclared Graphic in Sprite!\n");
  trueimage->DefLin(l);
  }

void Sprite::DefSize(int x, int y) {
  if(image == NULL) { image = new Graphic(); trueimage = image; }
  trueimage->DefSize(x, y);
  }

int Sprite::Hits(int x, int y, int xs, int ys) {
  Debug("User:Sprite:Hits2 0000");
  int ctrx, ctry;

  Debug("User:Sprite:Hits2 0500");
  if(image == NULL) Exit(-1, "Hitting Nothing!\n");

  if(image->depth == 8)  {
    for(ctry=ypos; ctry < ((ypos+image->ysize) <? (y+ys)); ctry++)  {
      for(ctrx=xpos; ctrx < ((xpos+image->xsize) <? (x+xs)); ctrx++)  {
	if(image->image[ctry-ypos].uc[ctrx-xpos] != image->tcolor) {
	  return 1;
	  }
	}
      }
    }
  else if(image->depth == 16)  {
    for(ctry=ypos; ctry < ((ypos+image->ysize) <? (y+ys)); ctry++)  {
      for(ctrx=xpos; ctrx < ((xpos+image->xsize) <? (x+xs)); ctrx++)  {
	if(image->image[ctry-ypos].us[ctrx-xpos] != image->tcolor) {
	  return 1;
	  }
	}
      }
    }
  else if(image->depth == 32)  {
    for(ctry=ypos; ctry < ((ypos+image->ysize) <? (y+ys)); ctry++)  {
      for(ctrx=xpos; ctrx < ((xpos+image->xsize) <? (x+xs)); ctrx++)  {
	Debug("User:Sprite:Hits2 0600");
	if(image->image[ctry-ypos].uc[(ctrx-xpos)*4+3])  {
	  return 1;
	  }
	Debug("User:Sprite:Hits2 0605");
	}
      }
    }
  else Exit(-1, "Unknown Depth Error (%d) in %s\n", image->depth, __PRETTY_FUNCTION__);
  Debug("User:Sprite:Hits2 1000");
  return 0;
  }

int Sprite::Hits(Sprite *s) {
  Debug("User:Sprite:Hits 0000");
  int ctrx, ctry;

  Debug("User:Sprite:Hits 0500");
  if(image->depth != s->image->depth)
	Exit(-1, "Depth Mismatch %d->%d!\n", image->depth, s->image->depth);
  if(image == NULL || s->image == NULL) Exit(-1, "Hitting Nothing!\n");

  if(image->depth == 8)  {
    for(ctry=(ypos >? s->ypos); ctry <
		((ypos+image->ysize) <? (s->ypos+s->image->ysize)); ctry++)  {
      for(ctrx=(xpos >? s->xpos); ctrx <
		((xpos+image->xsize) <? (s->xpos+s->image->xsize)); ctrx++)  {
	if((image->image[ctry-ypos].uc[ctrx-xpos] != image->tcolor)
	  	&& (s->image->image[ctry-(s->ypos)].uc[ctrx-(s->xpos)]
		!= s->image->tcolor)) {
	  return 1;
	  }
	}
      }
    }
  else if(image->depth == 32)  {
    for(ctry=(ypos >? s->ypos); ctry <
		((ypos+image->ysize) <? (s->ypos+s->image->ysize)); ctry++)  {
      for(ctrx=(xpos >? s->xpos); ctrx <
		((xpos+image->xsize) <? (s->xpos+s->image->xsize)); ctrx++)  {
	Debug("User:Sprite:Hits 0600");
	if(image->image[ctry-ypos].uc[(ctrx-xpos)*4+3] != image->tcolor)  {
	  Debug("User:Sprite:Hits 0602");
	  if(s->image->image[ctry-(s->ypos)].uc[(ctrx-(s->xpos))*4+3]
		!= s->image->tcolor) {
	    Debug("User:Sprite:Hits 0999");
	    return 1;
	    }
	  }
	Debug("User:Sprite:Hits 0605");
	}
      }
    }
  else Exit(-1, "Unknown Depth Error (%d) in %s\n", image->depth, __PRETTY_FUNCTION__);
  Debug("User:Sprite:Hits 1000");
  return 0;
  }

Sprite::~Sprite() {
  Debug("User:Sprite:~Sprite Begin");
  Erase();
  if(ownimage && image != NULL) delete image;
  if(ownimage && trueimage != NULL && image != trueimage) delete trueimage;
  if(__Da_Screen != NULL) __Da_Screen->RemoveSprite(snum, this);
  Debug("User:Sprite:~Sprite End");
  }

void Sprite::RedrawArea(int x, int y, int xs, int ys)  {
//  int XP = xpos-image->xcenter, YP = ypos-image->ycenter;
  int XP = xpos, YP = ypos;
  int XS = image->xsize, YS = image->ysize;
  if(x > XP+XS || y > YP+YS || x+xs < XP || y+ys < YP) return;

//  printf("(%d,%d)-%dx%d, (%d,%d)-%dx%d\n", XP, YP, XS, YS, x, y, xs, ys);

  x -= XP; y -= YP;
  if(x<0) { xs+=x; x=0; }
  if(y<0) { ys+=y; y=0; }
  if(x+xs > XS) xs = XS-x;
  if(y+ys > YS) ys = YS-y;

//  printf("(%d,%d), (%d,%d)-%dx%d\n", XP, YP, x, y, xs, ys);
  if(flags&(SPRITE_RECTANGLE|SPRITE_SOLID))
    __Da_Screen->DrawPartialGraphicFG(*image,XP,YP,x,y,xs,ys,pan);
  else
    __Da_Screen->DrawPartialTransparentGraphicFG(*image,XP,YP,x,y,xs,ys,pan);
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
  remap = cm;
  }

void Sprite::SetLine(int x, int y, int d, color c) {
  Erase();
  if(image == NULL) image=new Graphic;
  image->SetLine(x, y, d, c);
  }
