#
#
include ${ROOT}/usr/include/make/commondefs
# model makefile


#----- parameters -----
ARCHIVE=	ark.a
CFLAGS=		-O -UDEBUG -n
INCLUDES=	-I${ROOT}/usr/include
MAKEFILE=	Makefile
#-----


#----- tool names -----
AR=		ar
AS=		as
CC=		cc
CPP=		cpp
LD=		ld
RANLIB=		ranlib
#-----


#----- abbreviations -----
C=		${CC} ${CFLAGS} ${INCLUDES}
E=		@echo mk
I=		@echo>>Install
M=		@echo link $@;$C ${LDFLAGS} -o $@ $@.o ${LIB}
T=		@echo>>Targets
X=		ar x ${ARCHIVE}
#-----


#----- default rules -----
.SUFFIXES:
.SUFFIXES:	.c .i .o .s

default: all

.c.o:
	$C -c $*.c
#	@-mv -f `basename $*.o` $*.o
#	@${LD} -T 0 -o ${AOUT} -r -x $*.o;mv -f ${AOUT} $*.o
.c.s:
	$C -S $*.c
.s.o:
	$C -c $*.s
.c.i:
	$C -E $*.c > $*.i
#-----


#----- required targets -----
all:	iib ibtab

install: all
	strip iib
	${INSTALL} -u root -g sys -m 755 -F /etc \
	-idb "std.sw.unix mr" "iib ibtab"

clean clobber:
	-rm -f *.b *.o iib
release:
	exit 1
#-----


#----- peculiar targets -----

cib:	cib.o fib.o _ib.o ${MAKEFILE}
	$C -o $@ cib.o fib.o _ib.o
	-./fixhex $@
	chmod 755 $@

dib:	dib.o fib.o _ib.o ${MAKEFILE}
	$C -o $@ dib.o fib.o _ib.o
	-./fixhex $@
	chmod 755 $@

xib:	xib.o _ib.o ${MAKEFILE}
	-rm -f $@
	$C -o $@ xib.o _ib.o
	./fixhex $@
	chmod 755 $@

tib:	tib.o _ib.o ${MAKEFILE}
	-rm -f $@
	$C -o $@ tib.o _ib.o
	-./fixhex $@
	chmod 755 $@

iib:	iib.o ibtab.o _ib.o ${MAKEFILE}
	-rm -f $@
	$C -o $@ iib.o ibtab.o _ib.o
	-./fixhex $@
	chmod 04555 iib; chown root iib

rib:	rib.o
	-rm -f $@
	$C -o $@ rib.o
	chmod 04555 $@; chown root $@
#-----
