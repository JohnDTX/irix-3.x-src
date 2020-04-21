#
#
include ${ROOT}/usr/include/make/commondefs

# Makefile for constructing the xns library
#
#
LIB	= libxns.a
LCDEFS	= -DSYSTEMV
OBJS	= trdwri.o ttymodes.o xcmd.o xnsfile.o xnsread.o xnswrite.o \
	  xnsconnect.o makeproc.o utmp.o conserr.o mpr.o xnslisten.o \
	  xnseof.o

all:	${LIB}

install:all
	${INSTALL} -u bin -g bin -m 644 -F /usr/lib \
	-idb "std.sw.unix mr" "${LIB}"
	ranlib ${ROOT}/usr/lib/"${LIB}"

clean:
	rm -f *.o a.out core

clobber:clean
	rm -f ${LIB}

${LIB}:${OBJS}
	ar rv ${LIB} ${OBJS}

.c.o:
	${CCF} -c $<
