CC:=	gcc -O3 -s `User-CFlags`
OBJS:=	screen.o engine.o graphic.o palette.o sound.o speaker.o sprite.o input.o keyboard.o mouse.o resfile.o bag.o
ALL:=	*.h Makefile
LIBS:=	`User-Libs`
CCC:=	$(CC)
TSTR:=  $(shell date +"%Y%m%d%H%M")

all:	stest mtest $(ALL)
#all:	libuser.a

stest:	libuser.a stest.o $(ALL)
	$(CC) -o stest stest.o $(LIBS)

stest.o:	stest.cpp $(ALL)
	$(CC) -c stest.cpp

mtest:	libuser.a mtest.o $(ALL)
	$(CC) -o mtest mtest.o $(LIBS)

mtest.o:	mtest.cpp $(ALL)
	$(CC) -c mtest.cpp

libuser.a:	$(OBJS) $(ALL)
	ar rcs libuser.a $(OBJS)

screen.o:	screen.cpp $(ALL)
	$(CC) -c screen.cpp

engine.o:	engine.cpp $(ALL)
	$(CC) -c engine.cpp

graphic.o:	graphic.cpp $(ALL)
	$(CC) -c graphic.cpp

palette.o:	palette.cpp $(ALL)
	$(CC) -c palette.cpp

sound.o:	sound.cpp $(ALL)
	$(CC) -c sound.cpp

speaker.o:	speaker.cpp $(ALL)
	$(CC) -c speaker.cpp

sprite.o:	sprite.cpp $(ALL)
	$(CC) -c sprite.cpp

input.o:	input.cpp $(ALL)
	$(CC) -c input.cpp

keyboard.o:	keyboard.cpp $(ALL)
	$(CC) -c keyboard.cpp

mouse.o:	mouse.cpp $(ALL)
	$(CC) -c mouse.cpp

bag.o:	bag.cpp $(ALL)
	$(CC) -c bag.cpp

resfile.o:	resfile.cpp $(ALL)
	$(CC) -c resfile.cpp

tar:	screen.h
	cd .. ; tar czhvf ~/c/archive/user-2.0-pre$(TSTR).tar.gz \
	user/*.cpp user/*.[hc] user/Makefile user/*.bmp user/*.wav user.h \
	user/User-*
