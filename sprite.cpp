#include        <stdlib.h>
#include        <string.h>
#include        <math.h>

#include        "screen.h"

extern Screen *__Da_Screen;
extern User *__Da_User;

Button::Button()  {
  PressWave = NULL;
  ReleaseWave = NULL;
  mouseinter = SPRITE_BUTTON;
  normg = NULL;
  clickedg = NULL;
  disabled = 0;
  }
  
Button::Button(int mxsz, int mysz, const char *text,
        Color ct, Color cl, Color c, Color cd)  {
  normg = NULL;
  clickedg = NULL;
  PressWave = NULL;
  ReleaseWave = NULL;
  mouseinter = SPRITE_BUTTON;
  Create(mxsz, mysz, text, ct, cl, c, cd);
  disabled = 0;
  }

void Button::Create(int mxsz, int mysz, const char *text,
        Color ct, Color cl, Color c, Color cd)  {
  if(__Da_Screen->font == NULL)
    Exit(0, "Must Screen::SetFont before Creating Text Buttons!\r\n");
  Graphic g1, *b1, *b2;
  g1 = __Da_Screen->String2Graphic(c, ct, text);
      
  int ctry;
  int xsz = g1.xsize+8, ysz = __Da_Screen->font[' ']->ysize+6;
  if(g1.ysize < 2)  ysz = 8;
  if(mxsz > xsz) xsz = mxsz;
  if(mysz > ysz) ysz = mysz;
  int xdif = (xsz-(g1.xsize+8))/2;
  int ydif = (ysz-(g1.ysize+6))/2;
  char buf[xsz];
  
  b1 = new Graphic(xsz, ysz);
  b1->SetCenter(4+xdif, 3+ydif);
  memset(buf, cl, xsz);
  buf[xsz-1] = cd;
  b1->DefLin(buf);
  buf[xsz-2] = cd;
  b1->DefLin(buf);
  memset(&buf[2], c, xsz-4);
  for(ctry=2; ctry<(int)((b1->ysize)-2); ctry++)  b1->DefLin(buf);
  memset(&buf[2], cd, xsz-2);
  b1->DefLin(buf);
  buf[1] = cd;
  b1->DefLin(buf);
  *b1 = *b1 + g1;
  b1->SetCenter(0, 0);   
   
  b2 = new Graphic(xsz, ysz);
  b2->SetCenter(6+xdif, 4+ydif);
  memset(buf, cd, xsz);
  for(ctry=0; ctry<3; ctry++)  b2->DefLin(buf);
  memset(&buf[3], c, xsz-4);
  for(ctry=3; ctry<(int)((b2->ysize)-1); ctry++)  b2->DefLin(buf);
  memset(buf, cd, xsz);
  b2->DefLin(buf); 
  *b2 = *b2 + g1;
  b2->SetCenter(0, 0);

  SetImage(*b1, *b2);
  delete b1;
  delete b2;
  }
      
Sprite::Sprite()  {
  if(__Da_Screen == NULL)
    Exit(1, "Must create screen before creating sprites!\n");
  colormap = NULL;
  trueimage = NULL;
  image = NULL;
  ownimage = 0;
  behind = NULL;
  infront = NULL;
  priority = 1000;
  angle = 0;
  xpos = 4000;
  ypos = 4000;
  visible = 0;
  xsize = 0;
  ysize = 0;
  panel = 0;
  collisions = 1;
  scrolls = 1;
  spnum = __Da_Screen->AddSpriteToList(this);
  }

Sprite::Sprite(int xsz, int ysz) {
  Sprite();
  DefSize(xsz, ysz);
  }

Sprite::Sprite(const Graphic *pic) {
  Sprite();
  SetImage(pic);
  }

Sprite::Sprite(const Graphic &pic) {
  Sprite();
  SetImage(pic);
  }

void Sprite::SetPriority(int pri) {
  priority = pri;
  }

void Sprite::DefSize(int xsz, int ysz) {
  if(image != trueimage)  delete image;
  if(ownimage == 1)  delete trueimage;
  ownimage = 1;
  trueimage = new Graphic(xsz, ysz);
  image = trueimage;
  DefBac();
  }

void Sprite::DefBac()  {
  int ctr, ctr2;
  if(behind != NULL)  {
    for(ctr=0; ctr<(long)xblocks; ctr++)  {
      delete behind[ctr];
      delete infront[ctr];
      delete collide[ctr];
      }
    delete behind;
    delete infront;
    delete collide;
    }
  xblocks = ((image->xsize+62) >>5);
  bgysz = image->ysize;
  bgsz = (image->ysize*xblocks);
  behind = new Sprite**[xblocks];
  infront = new Sprite**[xblocks];
  collide = new unsigned long *[xblocks];
  for(ctr=0; ctr<(long)xblocks; ctr++)  {
    behind[ctr] = new Sprite*[bgysz];
    infront[ctr] = new Sprite*[bgysz];
    collide[ctr] = new unsigned long[bgysz];
    for(ctr2=0; ctr2<(long)bgysz; ctr2++)  {
      behind[ctr][ctr2] = NULL;
      infront[ctr][ctr2] = NULL;
      collide[ctr][ctr2] = 0;
      }
    }
  }

Sprite::~Sprite() {
  if(__Da_Screen != NULL)  Erase();
  if(__Da_Screen != NULL)  __Da_Screen->RemoveSpriteFromList(spnum);
  if((image != trueimage) && (image != NULL))  {
    delete image;
    }
  if((ownimage == 1) && (trueimage != NULL))  {
    delete trueimage;
    }
  image = NULL;
  trueimage = NULL;
  if(mouseinter == SPRITE_BUTTON || mouseinter == SPRITE_SBUTTON)  {
    if(__Da_User != NULL)  __Da_User->UnmapButton(this);
    }
  }

void Sprite::DefLin(char *data)  {
  if(image != NULL)  trueimage->DefLin(data);
  }

void Sprite::DefLinH(char *data)  {
  if(image != NULL)  trueimage->DefLinH(data);
  }

int Screen::AddSpriteToList(Sprite *newsp)  {
  int ret;
  for(ret = 0; (ret < MAX_SPRITES) && (SpriteList[ret] != NULL); ret++);
  if(ret == MAX_SPRITES)  { ret = -1;  return ret;  }
  SpriteList[ret] = newsp;
  return ret;
  }

void Sprite::SetImage(const Graphic &img)  {
  SetImage(&img);
  }

void Sprite::SetImage(const Graphic *img)  {
  Erase();
  if((image != trueimage) && (image != NULL))  delete image;
  if((ownimage == 1) && (trueimage != NULL))  delete trueimage;
  trueimage = new Graphic;
  image = trueimage;
  if(image != NULL)  {
    *image = (Graphic &)*img;
    ownimage = 1;
    DefBac();
    }
  else  {
    ownimage = 0;
    image = NULL;
    trueimage = NULL;
    }
  angle = 0;
  }

void Button::Stick()  { mouseinter = SPRITE_SBUTTON; };

void Button::NoStick()  { mouseinter = SPRITE_BUTTON; };

void Button::StealthClick()  {
  if(mouseinter != SPRITE_BUTTON && mouseinter != SPRITE_SBUTTON) return;
  Erase();
  if(trueimage == normg)  {
    trueimage = clickedg;
    image = clickedg;
    }
  else  {
    trueimage = normg;
    image = normg;
    }
  if(image != NULL)  DefBac();
  angle = 0;
  Draw();
  }

void Button::Click()  {
  if(disabled)  return;
  if(mouseinter != SPRITE_BUTTON && mouseinter != SPRITE_SBUTTON) return;
  Erase();
  if(trueimage == normg)  {
    trueimage = clickedg;
    image = clickedg;
    if(__Da_User != NULL)  __Da_User->JustPressedButton(spnum);
    if(PressWave != NULL)  PressWave->Play();
    }
  else  {
    trueimage = normg;
    image = normg;
    if(__Da_User != NULL)  __Da_User->JustReleasedButton(spnum);
    if(ReleaseWave != NULL)  ReleaseWave->Play();
    }
  if(image != NULL)  DefBac();
  angle = 0;
  Draw();
  }

Button::~Button() {
  if(__Da_Screen != NULL)  Erase();
  if(ownimage)  {
    delete clickedg;
    delete normg;
    ownimage = 0;
    }
  }

void Button::SetImage(const Graphic *imgnorm, const Graphic *imgpressed)  {
  SetImage(*imgnorm, *imgpressed);
  }

void Button::SetImage(const Graphic &imgnorm, const Graphic &imgpressed)  {
  Erase();
  int pressed = 0;
  if(image != normg) pressed = 1;
  if((ownimage == 1) && (clickedg != NULL))  delete clickedg;
  if((ownimage == 1) && (normg != NULL))  delete normg;
  clickedg = new Graphic(imgpressed);
  normg = new Graphic(imgnorm);
  if(pressed)  {
    trueimage = clickedg;
    image = clickedg;
    }
  else  {
    trueimage = normg;
    image = normg;
    }
  ownimage = 1;
  if(image != NULL)  DefBac();
  mouseinter = SPRITE_BUTTON;
  angle = 0;
  }

void Button::UseImage(const Graphic &imgnorm, const Graphic &imgpressed)  {
  UseImage(&imgnorm, &imgpressed);
  }

void Button::UseImage(const Graphic *imgnorm, const Graphic *imgpressed)  {
  Erase();
  int pressed = 0;
  if(image != normg) pressed = 1;
  if((ownimage == 1) && (clickedg != NULL))  delete clickedg;
  if((ownimage == 1) && (normg != NULL))  delete normg;
  clickedg = (Graphic *)imgpressed;
  normg = (Graphic *)imgnorm;
  if(pressed)  {
    trueimage = clickedg;
    image = clickedg;
    }
  else  {
    trueimage = normg;
    image = normg;
    }
  ownimage = 0;
  if(image != NULL)  DefBac();
  mouseinter = SPRITE_BUTTON;
  angle = 0;
  }

void Sprite::UseImage(const Graphic &img)  {
  UseImage(&img);
  }

void Sprite::UseImage(const Graphic *img)  {
  if(img == trueimage)  return;
  Erase();
  if((image != trueimage) && (image != NULL))  delete image;
  if((ownimage == 1) && (trueimage != NULL))  delete trueimage;
  trueimage = (Graphic *)img;
  image = trueimage;
  ownimage = 0;
  if(image != NULL)  DefBac();
  angle = 0;
  }

void Screen::RemoveSpriteFromList(int spnum)  {
  SpriteList[spnum] = NULL;
  }

void Sprite::SetPanel(Panel p)  {
  Erase();
  panel = p;
  }

IntList Sprite::Move(int X, int Y) return ret; {
  Erase();
  ret = Draw(X, Y);
  }

IntList Sprite::Move(int X, int Y, int A) return ret; {
  Erase();
  if(A != angle)  {
//    Graphic *tmpg = trueimage;
//    int oi = ownimage;
//    if((image != trueimage) && (image != NULL))  delete image;
//    image = NULL;
//    trueimage = NULL;
//    SetImage(tmpg->Rotated(-A, tcol));
//    trueimage = tmpg;
//    angle = A;
//    ownimage = oi;

    if((image == trueimage) || (image == NULL))  image = new Graphic;
    image->SetRotated(*trueimage, -A);
    angle = A;
    DefBac();
    }
  ret = Draw(X, Y);
  }

IntList Sprite::Move(int X, int Y, int AX, int AY, int AZ) return ret; {
  Erase();
  Graphic *tmpg = trueimage;
  if((image != trueimage) && (image != NULL))  delete image;
  image = NULL;
  trueimage = NULL;
  SetImage(tmpg->Rotated(1, -AX, -AY, -AZ));
  trueimage = tmpg;
  ret = Draw(X, Y);
  }

IntList Sprite::Move(int X, int Y, double SC,
		int AX, int AY, int AZ) return ret; {
  Erase();
  Graphic *tmpg = trueimage;
  if((image != trueimage) && (image != NULL))  delete image;
  image = NULL;
  trueimage = NULL;
  SetImage(tmpg->Rotated(SC, -AX, -AY, -AZ));
  trueimage = tmpg;
  ret = Draw(X, Y);
  }

//void Screen::BSetBlock(int X, int Y, unsigned char *inBlock)  {
//  }

void Screen::SaveBack(int X, int Y)  {
  int ctr;
  unsigned char * tmp;
  if(backg[X][Y] != NULL)  return;
  backg[X][Y] = new unsigned char[32];
  tmp = GetBlock32(X, Y);
  for(ctr = 0; ctr < 32; ctr++)  backg[X][Y][ctr] = tmp[ctr];
  }

void Screen::RestoreBack(int X, int Y)  {
  if(backg[X][Y] != NULL)  memcpy(&VidBuf[(X<<5)+Y*Xlen], backg[X][Y], 32);
  }

IntList Screen::EraseSpritesHere(int X, int Y) return ret; {
  Sprite *tmp = spritehere[X][Y];
  while(tmp!=NULL)  {
    if(tmp->visible)  {
      tmp->Erase();
      ret += tmp->spnum;
      }
    tmp = spritehere[X][Y];
    }
  return ret;
  }

void Screen::DeleteBack(int X, int Y)  {
  if(backg[X][Y] != NULL)  delete backg[X][Y];
  backg[X][Y] = NULL;
  }

void Screen::RedrawSprites(const IntList &them)  {
  int ctr;
  for(ctr=0; ctr<(((IntList &)them).Size()); ctr++)  {
    SpriteList[((IntList &)them)[ctr]]->Draw();
    }
  }

void Screen::Redraw(int X, int Y)  {
  RestoreBack(X, Y);
//-----------------------------------------------------
//  int ctr, valid = 0;
//  for(ctr = 0; ctr < MAX_SPRITES; ctr++)  {
//    if(spritehere[X][Y] == SpriteList[ctr])  valid = 1;
//    }
//  if(valid == 0)  return;
//-----------------------------------------------------
  if(spritehere[X][Y] != NULL)  spritehere[X][Y]->DrawBlock(X, Y);
  InvalidateRectangle(X<<5, Y, (X<<5)+31, Y);
  }

void Sprite::DrawBlock(int X, int Y)  {
  int ctr;
  if(image != NULL)  {
    int tcol = image->tcolor;
    int BX = X<<5;
    int xb = xpos-BX;
    if(xb<0)  xb=0;
    int xe = (xpos+image->xsize)-BX;
    if(xe>32)  xe=32;
    unsigned char *to = &(__Da_Screen->VidBuf[(Y*__Da_Screen->Xlen)+BX+xb]);
    unsigned char *from = &(image->image[Y-ypos][BX+xb-xpos]);
    if(colormap == NULL) for(ctr=xb; ctr<xe; ctr++)  {
      if((*from) != tcol)  *to=*from;
      from++;	to++;
      }
    else for(ctr=xb; ctr<xe; ctr++)  {
      if((*from) != tcol)  *to=colormap[*from];
      from++;	to++;
      }
    }
  if(infront[X-(xpos>>5)][Y-ypos] != NULL)
    infront[X-(xpos>>5)][Y-ypos]->DrawBlock(X, Y);
  }

IntList Sprite::PutBlock(int X, int Y) return ret; {
  int ctr, tcol = image->tcolor;
  unsigned long cblock = 0;
  DrawBlock(X, Y);
  __Da_Screen->InvalidateRectangle(X<<5, Y, (X<<5)+31, Y);

  if(collisions)  {
    for(ctr = (X<<5); ctr < 32+(X<<5); ctr++)  {
      cblock <<= 1;
      if((ctr >= xpos) && (ctr < (xpos + (long)image->xsize)))  {
        if(image->image[Y-ypos][ctr-xpos] != tcol)  cblock ++;
        }
      }
    if(__Da_Screen->spritehere[X][Y] != NULL && cblock)
      ret = __Da_Screen->spritehere[X][Y]->HitBlock(X, Y, cblock, spnum);
    }
  collide[X-(xpos>>5)][Y-ypos] = cblock;
  return ret;
  }

IntList Sprite::HitBlock(int X, int Y, unsigned long block, int insp)
		return ret; {
  if(infront[X-(xpos>>5)][Y-ypos] != NULL)
    ret = infront[X-(xpos>>5)][Y-ypos]->HitBlock(X, Y, block, insp);

  if((insp != spnum) && (collisions > 0) &&
	((block & (collide[X-(xpos>>5)][Y-ypos])) > 0))  {
    ret += spnum;
    }
  return ret;
  }

IntList Sprite::Draw() return ret; {
  if((image == NULL) || (visible == 1))  return ret;
  ret = Draw(xpos+image->xcenter, ypos+image->ycenter);
  return ret;
  }

IntList Sprite::Draw(int X, int Y) return ret; {
  if((image == NULL) || (visible == 1))  return ret;
  visible = 1;
  Sprite ***spritehere = __Da_Screen->spritehere;
  int ctrb, ctry, cutx, xoff;
  xpos = X - image->xcenter;	ypos = Y - image->ycenter;
  cutx = xpos>>5;	xoff = xpos%32;
  xsize = image->xsize;		ysize = image->ysize;
  for(ctrb = 0; ctrb < ((xoff+(long)xsize+31)>>5); ctrb++)
	if(((ctrb+cutx) >= (__Da_Screen->pxstart[panel] >> 5)) &&
		((ctrb+cutx) < ((__Da_Screen->pxend[panel]+31) >> 5)))  {
    for(ctry = 0; ctry < (long)ysize; ctry++)  
	  if(((ctry+ypos) >= __Da_Screen->pystart[panel]) && 
		((ctry+ypos) < __Da_Screen->pyend[panel]))  {
      if(spritehere[cutx+ctrb][ypos+ctry] == NULL)  {
	__Da_Screen->SaveBack(ctrb+cutx, ypos+ctry);
	spritehere[cutx+ctrb][ypos+ctry] = this;
	infront[ctrb][ctry] = NULL;
	behind[ctrb][ctry] = NULL;
	}
      else  {
	Sprite * lastsp = NULL;
	Sprite * cursp = spritehere[cutx+ctrb][ypos+ctry];
	while((cursp != NULL) && ((cursp->priority > priority) ||
		((cursp->priority == priority) && (cursp->spnum > spnum))))  {
	  lastsp = cursp;
	  cursp = cursp->infront[(cutx+ctrb)-(cursp->xpos>>5)]
		[(ypos+ctry)-(cursp->ypos)];
	  }
	infront[ctrb][ctry] = cursp;
	behind[ctrb][ctry] = lastsp;
	if(infront[ctrb][ctry] != NULL)  {
	  infront[ctrb][ctry]->behind[(ctrb+cutx)-
		(infront[ctrb][ctry]->xpos>>5)]	[(ypos+ctry)-
		(infront[ctrb][ctry]->ypos)] = this;
	  }
	if(behind[ctrb][ctry] != NULL)  {
	  behind[ctrb][ctry]->infront[(ctrb+cutx)-
		(behind[ctrb][ctry]->xpos>>5)]	[(ypos+ctry)-
		(behind[ctrb][ctry]->ypos)] = this;
	  }
	else  {
	  spritehere[cutx+ctrb][ypos+ctry] = this;
	  }
	}
      ret += PutBlock(cutx+ctrb, ypos+ctry);
      }
    }
  return ret;
  }

void Sprite::Erase()  {
  if((visible == 0) || (image == NULL))  return;
  visible = 0;
  int ctrb, ctry, cutx, xoff;
  cutx = xpos>>5;	xoff = xpos%32;
  for(ctrb = 0; ctrb < ((xoff+(long)xsize+31)>>5); ctrb++)
	if(((ctrb+cutx) >= (__Da_Screen->pxstart[panel] >> 5)) && 
		((ctrb+cutx) < ((__Da_Screen->pxend[panel]+31) >> 5)))  {
    for(ctry = 0; ctry < (long)ysize; ctry++)  
	if(((ctry+ypos) >= __Da_Screen->pystart[panel]) && 
		((ctry+ypos) < __Da_Screen->pyend[panel]) &&
		(__Da_Screen->spritehere[ctrb+cutx][ctry+ypos] != NULL))  {
      if(infront[ctrb][ctry] != NULL)  {
	infront[ctrb][ctry]->behind[(ctrb+cutx)-
		(infront[ctrb][ctry]->xpos>>5)]	[(ypos+ctry)-
		(infront[ctrb][ctry]->ypos)] = behind[ctrb][ctry];
	}
      if(behind[ctrb][ctry] != NULL)  {
	behind[ctrb][ctry]->infront[(ctrb+cutx)-
		(behind[ctrb][ctry]->xpos>>5)]	[(ypos+ctry)-
		(behind[ctrb][ctry]->ypos)] = infront[ctrb][ctry];
	}
      else  {
	__Da_Screen->spritehere[ctrb+cutx][ctry+ypos] = infront[ctrb][ctry];
	}
      behind[ctrb][ctry] = NULL;
      infront[ctrb][ctry] = NULL;
      __Da_Screen->Redraw(ctrb+cutx, ctry+ypos);
      }  
    }  
  }

void Sprite::SetColormap(char *cm)  {
  colormap = cm;
  }

void Sprite::SetTransparentColor(int tc)  {
  Erase();
  if(image != NULL) image->tcolor = tc;
  if(trueimage != NULL) trueimage->tcolor = tc;
  }

