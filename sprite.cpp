#include <stdio.h>

#include "engine.h"
#include "screen.h"
#include "sprite.h"
#include "graphic.h"
#include "list.h"

extern Screen *__Da_Screen;

Sprite::Sprite() {
  if(__Da_Screen == NULL)  Exit(-1, "Must create Screen before Sprite!\n");
  collisions = 1;
  drawn = 0;
  image = NULL;
  trueimage = NULL;
  snum = __Da_Screen->RegisterSprite(this);
  }

Sprite::Sprite(const Graphic &g) {
  if(__Da_Screen == NULL)  Exit(-1, "Must create Screen before Sprite!\n");
  collisions = 1;
  drawn = 0;
  image = NULL;
  trueimage = NULL;
  snum = __Da_Screen->RegisterSprite(this);
  SetImage(g);
  }

void Sprite::SetImage(const Graphic *g) {
  SetImage(*g);
  }

void Sprite::SetImage(const Graphic &g) {
  int dr = drawn;
  if(dr) Erase();
  if(image != NULL) delete image;
  if(trueimage != NULL && image != trueimage) delete trueimage;
  image = new Graphic(g);
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

IntList Sprite::CDraw(int x, int y) {
  if(drawn || image == NULL) { IntList ret; return ret; }
  __Da_Screen->DrawTransparentGraphicFG(*image, x, y);
  xpos = x; ypos = y; drawn = 1;
  return __Da_Screen->CollideRectangle(snum,
	xpos, ypos, image->xsize, image->ysize);
  }

IntList Sprite::CDraw() {
  if(drawn || image == NULL) { IntList ret; return ret; }
  __Da_Screen->DrawTransparentGraphicFG(*image, xpos, ypos);
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

void Sprite::Draw(int x, int y) {
  if(drawn || image == NULL) return;
  Debug("User:Sprite:Draw(x,y) Begin");
  __Da_Screen->DrawTransparentGraphicFG(*image, x, y);
  Debug("User:Sprite:Draw(x,y) Middle");
  xpos = x; ypos = y; drawn = 1;
  Debug("User:Sprite:Draw(x,y) End");
  }

void Sprite::Draw() {
  if(drawn || image == NULL) return;
  __Da_Screen->DrawTransparentGraphicFG(*image, xpos, ypos);
  drawn = 1;
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

int Sprite::Hits(Sprite *s) {
  Debug("User::Sprite::Hits 0000");
  int ctrx, ctry;

  Debug("User::Sprite::Hits 0500");
  if(image->depth != s->image->depth)
	Exit(-1, "Depth Mismatch %d->%d!\n", image->depth, s->image->depth);
  if(image == NULL || s->image == NULL) Exit(-1, "Hitting Nothing!\n");

  if(image->depth == 8)  {
    for(ctry=(ypos >? s->ypos); ctry <
		((ypos+image->ysize) <? (s->ypos+s->image->ysize)); ctry++)  {
      for(ctrx=(xpos >? s->xpos); ctrx <
		((xpos+image->xsize) <? (s->xpos+s->image->xsize)); ctrx++)  {
	if((image->image[ctry-ypos][ctrx-xpos] != image->tcolor)
	  	&& (s->image->image[ctry-(s->ypos)][ctrx-(s->xpos)]
		!= s->image->tcolor)) {
	  return 1;
	  }
	}
      }
    }
  else if(image->depth == 24 || image->depth == 32)  {
    for(ctry=(ypos >? s->ypos); ctry <
		((ypos+image->ysize) <? (s->ypos+s->image->ysize)); ctry++)  {
      for(ctrx=(xpos >? s->xpos); ctrx <
		((xpos+image->xsize) <? (s->xpos+s->image->xsize)); ctrx++)  {
	Debug("User::Sprite::Hits 0600");
	if(((image->image[ctry-ypos][(ctrx-xpos)*3] << 16)
	    + (image->image[ctry-ypos][(ctrx-xpos)*3+1] << 8)
	    + image->image[ctry-ypos][(ctrx-xpos)*3+2]) != image->tcolor)  {
	  Debug("User::Sprite::Hits 0602");
//printf("%d, %d | %d, %d\n", ctrx-s->xpos, ctry-s->ypos, s->image->xsize, s->image->ysize);
	  if(((s->image->image[ctry-(s->ypos)][(ctrx-(s->xpos))*3] << 16)
	    + (s->image->image[ctry-(s->ypos)][(ctrx-(s->xpos))*3+1] << 8)
	    + s->image->image[ctry-(s->ypos)][(ctrx-(s->xpos))*3+2])
	    != s->image->tcolor) {
	    Debug("User::Sprite::Hits 0999");
	    return 1;
	    }
	  }
	Debug("User::Sprite::Hits 0605");
	}
      }
    }
  Debug("User::Sprite::Hits 1000");
  return 0;
  }

Sprite::~Sprite() {
  Debug("User:Sprite:~Sprite Begin");
  Erase();
  Debug("User:Sprite:~Sprite Middle");
  if(__Da_Screen != NULL) __Da_Screen->RemoveSprite(snum, this);
  Debug("User:Sprite:~Sprite End");
  }

//  Graphic *image, *trueimage;
