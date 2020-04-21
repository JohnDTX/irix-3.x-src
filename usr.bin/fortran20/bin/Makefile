#
#
include ${ROOT}/usr/include/make/commondefs


.SUFFIXES: .p .o

FILES = ftncterrs ftnrterrs pascterrs
IDBTAG = -idb "std.sw.unix"

install:
	${INSTALL} -u bin -g bin -m 644 -F /usr/lib ${IDBTAG} "${FILES}"
	
all clean clobber:

.p.o:
	CC_OPTS="-ZH"; $(PC) $(PCFLAGS) -c $*.p

OBJS=asm68k.o #../lib/umem.o ../lib/20muinit.o

asm68k:	$(OBJS)
	CC_OPTS="-ZH"; $(PC) $(PCFLAGS) -o asm68k $(OBJS)
