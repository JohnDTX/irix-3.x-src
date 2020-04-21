#
# Copyright (c) 1983 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#	@(#)Makefile	1.2 (Berkeley) 2/7/86
#
#
include ${ROOT}/usr/include/make/commondefs

OBJS=	tftpd.o tftpsubs.o
LCINCS=	-I$(ROOT)/usr/include/bsd
LCOPTS=	-O 
LLDLIBS= -lsun -lbsd
TGTS=	tftpd
IDBTAG = -idb "upd nfs "

all:	$(TGTS)

tftpd:	$(OBJS)
	$(CCF) -o tftpd $(OBJS) $(LDFLAGS)  

install:	all
	${INSTALL} -F /usr/etc ${IDBTAG} "${TGTS}"

clean clobber:
	rm -f $(TGTS) *.o *.s errs core a.out t.?
