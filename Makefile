CFLAGS:=$(shell U2-CFlags)
LIBS:=	$(shell U2-Libs)
CC:=	gcc $(CFLAGS)
OBJS:=	screen.o engine.o graphic.o palette.o sound.o speaker.o sprite.o \
	input.o keyboard.o mouse.o resfile.o bag.o chunk.o net.o \
	control.o movable.o clickey.o stickey.o joystick.o
ALL:=	*.h Makefile
CCC:=	$(CC)
TSTR:=  $(shell date +"%Y%m%d%H%M")

all:	stest mtest $(ALL)
#all:	stest $(ALL)

stest:	libu2.a stest.o $(ALL)
	$(CC) -o stest stest.o $(LIBS)

mtest:	libu2.a mtest.o $(ALL)
	$(CC) -o mtest mtest.o $(LIBS)

libu2.a:	$(OBJS) $(ALL)
	ar rcs libu2.a $(OBJS)
	U2-Inst

user:	libu2.a

%.o:	%.cpp $(ALL)
	$(CC) -c $<

speaker.o:	speaker.cpp $(ALL)
	$(CC) -fomit-frame-pointer -c speaker.cpp

keyboard.o:	keyboard.cpp $(ALL)
	$(CC) -fomit-frame-pointer -c keyboard.cpp

tar:	screen.h
	cd .. ; tar czhvf ~/c/archive/user-2.0-pre$(TSTR).tar.gz \
	user/*.cpp user/*.[hc] user/Makefile user/*.bmp user/*.wav user.h \
	user/U2-*

clean:
	rm -f *.o *.a *.so
