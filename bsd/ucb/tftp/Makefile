# "$Header: /d2/3.7/src/bsd/ucb/tftp/RCS/Makefile,v 1.1 89/03/27 15:00:48 root Exp $"
#
include ${ROOT}/usr/include/make/commondefs

ALL=	tftp
LCINCS=	-I$(ROOT)/usr/include/bsd
LCOPTS=	-O 
LLDLIBS= -lsun -lbsd

all: $(ALL)

tftp:	main.o tftp.o tftpsubs.o
	${CCF} main.o tftp.o tftpsubs.o -o tftp ${LDFLAGS}

clean clobber:
	rm -f $(ALL) *.o *.s errs core a.out t.?

install: all
	${INSTALL} -F /usr/bin -idb "std.sw.unix mr" tftp
