CC:=	gcc -O6 -mpentium -ffast-math -funroll-loops -malign-double -mstack-align-double -s -Wall
#CC:=	egcs -O6 -ffast-math -malign-double -s -Wall
ALL:=	Makefile graphic.h screen.h user.h
#LIBS:=	-lm -lX11
#LIBS:=	-lm -lX11 -lvga
LIBS:=	-lm -lX11 -lXxf86dga -lXext
OBJS:=	graphic.o engine.o sound.o screen.o sprite.o user.o mouse.o net.o resfile.o palette.o
USER:=	/usr/include/user/libuser.a
TSTR:=	$(shell date +"%Y%m%d%H%M")

all:	user vb tst

tar:    screen.h
	cd .. ; tar czhvf ~/c/archive/user.$(TSTR).tar.gz \
	  user/*.cpp user/*.[hc] user/Makefile user/*.bmp user.h
#	  user/*.cpp user/*.[hc] user/Makefile user/*.bmp user/*.wav user.h

tst:	test.cpp $(ALL) $(USER)
	$(CC) -o tst test.cpp $(USER) $(LIBS)

user:	$(USER)
	@echo -n

$(USER):	$(ALL) $(OBJS)
	ar rcs $(USER) $(OBJS)

vb:	$(OBJS) vb.o $(ALL)
	$(CC) -o vb vb.o $(OBJS) $(LIBS)

vb.o:	vb.cpp $(ALL)
	$(CC) -c vb.cpp

net.o:	net.cpp $(ALL)
	$(CC) -c net.cpp

resfile.o:	resfile.cpp $(ALL)
	$(CC) -c resfile.cpp

engine.o:	engine.cpp $(ALL)
	$(CC) -c engine.cpp

sound.o:	sound.cpp $(ALL)
	$(CC) -c sound.cpp

sprite.o:	sprite.cpp $(ALL)
	$(CC) -c sprite.cpp

screen.o:	screen.cpp $(ALL)
	$(CC) -c screen.cpp

graphic.o:	graphic.cpp $(ALL)
	$(CC) -c graphic.cpp

user.o:	user.cpp $(ALL)
	$(CC) -c user.cpp

mouse.o:	mouse.cpp $(ALL)
	$(CC) -c mouse.cpp

palette.o:	palette.cpp $(ALL)
	$(CC) -c palette.cpp


