
include ${ROOT}/usr/include/make/commondefs

OBJS=	main.o manager.o window.o select.o junk.o unix.o scanner.o desktop.o
MERCS=	mexrc pf4mexrc
LCINCS=	-I${ROOT}/usr/include/gl2
LCOPTS=	-O -DUNIX -D${machine}
LLDLIBS = -lport -lgl -lm -l${GLHDWR}
GRPOWN  = -g bin -u bin
IDBTAG  = -idb "std.sw.unix"

all: mex
	
clobber clean:
	rm -f mex *.o

install: all
	${INSTALL} ${GRPOWN} ${IDBTAG} -m 755 -F /usr//bin mex
	${INSTALL} ${GRPOWN} ${IDBTAG} -m 644 -F /usr/lib/gl2 "${MERCS}"

mex: ${OBJS}
	${CC} ${CFLAGS} -o mex ${OBJS} ${LDFLAGS}

main.o: win.h misc.h
	if test '$(machine)' = iris ;\
	then ${CCF} -DPM2 -c main.c ;\
	else ${CCF} -DIP2 -c main.c ; fi
window.o: win.h misc.h
select.o: win.h misc.h
scanner.o: win.h misc.h
manager.o: win.h misc.h
	if test '$(machine)' = iris ;\
	then ${CCF} -DPM2 -c manager.c ;\
	else ${CCF} -DIP2 -c manager.c ; fi
unix.o: win.h misc.h
junk.o: win.h misc.h
bkgrnd.o: win.h
desktop.o: win.h misc.h
