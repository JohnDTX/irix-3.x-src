#
#
include ${ROOT}/usr/include/make/commondefs


OBJS=dbg.o ../lib/umem.o ../lib/20muinit.o

all:	dbg



install:	all
	${INSTALL} -u bin -g bin -m 755 -F /usr/bin \
	-idb "std.sw.unix mr" dbg

dbg:	$(OBJS)
	$(PC) -o dbg $(OBJS)

../lib/umem.o:	../lib/umem.asm
	cd ../lib; $(MAKE) $(MAKEFLAGS) umem.o

../lib/20muinit.o:	../lib/20muinit.asm
	cd ../lib; $(MAKE) $(MAKEFLAGS) 20muinit.o

dbg.o:	dbg.p
	$(PC) -c dbg.p


clean:
	rm -f dbg.o *.obj *.j

clobber:	clean
	rm -f dbg
