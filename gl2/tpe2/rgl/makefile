
include ${ROOT}/usr/include/make/commondefs

CINCS = -I$(GLROOT)/include/$(GLHDWR) \
		-I$(GLROOT)/include/vkernel
LIBS = $(GLROOT)/lib/libVgl2.a $(GLROOT)/lib/libV.a 
VKERNEL = $(GLROOT)/lib/$(GLHDWR)/vkernel
VKERNELR = $(GLROOT)/lib/$(GLHDWR)/vkernel.r
LINT=lint
CTAGS=ctags
LCOPTS  = -V -OBPS -N -O -DV -DGL2TERM -DRGL -DGL2 -DNEWSTDIO -DDEBUG

OBJECTS=conio.o dbgmenu.o display.o dispxlat.o emulator.o emulint.o \
	functions.o pxdio.o
FILES=conio.c dbgmenu.c display.c dispxlat.c emulator.c emulint.c \
	functions.c pxdio.c

OBJS3279 = term3279.o special3279.o irisio3279.o hostio3279.o
SRCS3279 =  term3279.c special3279.c irisio3279.c hostio3279.c

TERMOBJ= ctype.o context.o dispatch.o callfunc.o ret.o \
	 getline.o bogus.o textio.o

SRCS =   ctype.c context.c dispatch.c callfunc.c ret.c \
	 getline.c bogus.c textio.c

all: term3279 


term3279.r: $(OBJECTS) $(TERMOBJ) $(OBJS3279)
	$(CC) -V --n -e _TeamRoot -N -T 40000 $(OBJECTS) $(TERMOBJ) $(OBJS3279)\
		-o term3279.r $(LIBS)

term3279:	term3279.r
	makeboot term3279.r temp.boot -l 40000 -h 3FFE0
	glueboot $(VKERNEL) temp.boot -o term3279
	rm temp.boot 
	mv term3279 /mnt/usr/local/boot/vrgl
	@echo 


lint:	${FILES}
	${LINT} ${DFLAGS} ${INCLUDES} ${SRCS} ${SRCS3279} ${FILES} > linterrs
	@echo 

flowterm:	${FILES}
	cflow ${DFLAGS} ${INCLUDES} ${SRCS3279} ${SRCS} ${FILES} > cflow.doc
	@echo 

tags:	${FILES}
	rm -f tags
	${CTAGS} ${SRCS3279} ${SRCS} ${FILES}
	@echo 


conio.o:	conio.c pxw.h

dbgmenu.o:	pxw.h

display.o:	pxw.h

emulator.o:	pxw.h

emulint.o:	pxw.h

functions.o:	pxw.h

hostio3279.o:	pxw.h hostio.h

pxdio.o:	pxw.h hostio.h

term3279.o:	pxw.h hostio.h

t3279.o:	pxw.h hostio.h



install: all 

clean:
	rm -f *.o *.BAK *.ls *.r escape.doc

.SUFFIXES: .o .ls .c .s

.c.o:	
	$(CC) $(DFLAGS) -c $*.c

.c.s:	
	$(CC) -S $(DFLAGS) -c $*.c

