# Copyright (c) 1983 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#
include ${ROOT}/usr/include/make/commondefs

ALL=	routed
OBJS=	af.o if.o input.o main.o output.o startup.o tables.o timer.o \
	trace.o inet.o
LCINCS = -I${ROOT}/usr/include/bsd 
LCDEFS = -O
LLDLIBS= -lsun -lbsd -ldbm
IDBTAG = -idb "std.sw.unix mr"

default all:	${ALL}

routed: ${OBJS}
	${CCF} -o routed ${OBJS} ${LDFLAGS} 

install:	all
	${INSTALL} -o -F /usr/etc ${IDBTAG} "${ALL}"

clean:
	rm -f routed *.o core a.out
    
clobber:	clean
	rm -f ${ALL}
