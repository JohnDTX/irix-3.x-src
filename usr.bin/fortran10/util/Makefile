#
#

include ${ROOT}/usr/include/make/commondefs

.SUFFIXES: .o .p
OBJS= asm68k.o 

all:	asm68k

install:	all
	${INSTALL}  -F ../bin -idb "std.sw.unix mr" asm68k

asm68k:	$(OBJS)
	CC_OPTS="${CC_OPTS} -ZH" $(PC) $(PCFLAGS) -o asm68k $(OBJS)

.p.o:
	CC_OPTS="${CC_OPTS} -ZH" $(PC) $(PCFLAGS) -c $<


clean:
	rm -f *.o *.obj *.j

clobber:	clean
	rm -f asm68k
