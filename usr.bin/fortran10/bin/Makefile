#
#
include ${ROOT}/usr/include/make/commondefs

files = ftncterrs ftnrterrs pascterrs
install:
	${INSTALL} -u bin -g bin -m 644 -F /usr/lib \
	-idb "std.sw.unix mr" "${files}"
	
all:

clean clobber:
	rm -f asm68k *.o

OBJS=asm68k.o #../lib/umem.o ../lib/20muinit.o

asm68k:	$(OBJS)
	$(PC) $(PCFLAGS) -o asm68k $(OBJS)
