
include ${ROOT}/usr/include/make/commondefs

TOUCH=	touch -f -c
GLHDWR = gl2#	# override since this directory is only for GL2
LIBS= $(ROOT)/usr/lib/libgl.a


LCDEFS = -DUNIX -D$(GLHDWR) -D$(machine)
LCINCS  = -I$(ROOT)/usr/include/gl2
LCOPTS  = -O 

FILES=term3279.c conio.c display.c dispxlat.c emulator.c emulint.c \
	functions.c pxdio.c trace.c dbgmenu.c \
	hostio3279.c irisio3279.c local.c ttymodes.c bogus.c \
        dispatch.c errorm.c s3279.c context.c util.c
OBJECTS=display.o dispxlat.o emulator.o emulint.o functions.o \
	pxdio.o trace.o dbgmenu.o \
	hostio3279.o irisio3279.o ret.o local.o ttymodes.o bogus.o \
        dispatch.o errorm.o s3279.o context.o util.o
LINT=lint
CTAGS=ctags

all:	t3279

install: all 
	${INSTALL} -u bin -g bin -m 755 -F /usr/bin  \
	    -idb "std.sw.unix mr" t3279

t3279:	${OBJECTS} conio.o term3279.o
	$(CC) $(LDFLAGS) conio.o term3279.o $(OBJECTS) $(LIBS) \
		-o t3279 -lm -loldmalloc
	@echo '\007'

hexd: hexd.o dispxlat.o
	$(CC) $(LDFLAGS) hexd.o dispxlat.o -o hexd -lm
	@echo '\007'

hexfind: hexfind.o
	$(CC) $(LDFLAGS) hexfind.o -o hexfind -lm
	@echo '\007'

cutr: cutr.o dispxlat.o
	$(CC) $(LDFLAGS) cutr.o dispxlat.o -o cutr -lm
	@echo '\007'

lint:	${FILES} 
	${LINT} ${FILES} > linterrs
	@echo '\007'

flow:	${FILES}
	cflow ${FILES} > flow
	@echo '\007'

tags:	${FILES} 
	rm -f tags
	${CTAGS} ${FILES} 
	@echo '\007'

clobber clean:
	rm -f *.o tags flow linterrs *.out profdata makerrs pxlog \
	pxdtest t3279 cutr hexd LOG* RDD* WRD* REwr*

ret.o: term.h hostio.h
	    $(CCF) -c ret.c

conio.o:	term.h pxw.h
	    $(CCF) -c conio.c


display.o:	pxw.h term.h

emulator.o:	pxw.h term.h

emulint.o:	pxw.h

functions.o:	pxw.h term.h rpc.h

irisio3279.o:	pxw.h hostio.h rpc.h term.h

hostio3279.o:	pxw.h term.h hostio.h

pxdio.o:	pxw.h term.h

term3279.o:	pxw.h hostio.h rpc.h term.h

dbgmenu.o:	pxw.h term.h hostio.h

trace.o:

s3279.o:	term.h

util.o:		term.h

.c.o:	
	$(CC) $(CFLAGS) -c $*.c

.c.s:	
	$(CC) -S $(CFLAGS) -c $*.c

