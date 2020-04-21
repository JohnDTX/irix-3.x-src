
#
#
include ${ROOT}/usr/include/make/commondefs


# sccsid	= @(#) makefile 1.5@(#)

LIB	= libtermcap.a
TDIR	= /usr/lib
AR	= ar
LCDEFS = -DCM_N -DCM_GT -DCM_B -DCM_D 
IDBTAG  = -idb "root upd"

all:	$(LIB)

install:all
	${INSTALL} -m 644 -u bin -g bin -F ${TDIR} ${IDBTAG} ${LIB}
	ranlib ${TDIR}/$(LIB)
	${INSTALL} -ln ${TDIR}/${LIB} -F ${TDIR} ${IDBTAG} libtermlib.a

clean:
	rm -f *.o

clobber:clean
	rm -f $(LIB)

$(LIB): termcap.o tgoto.o tputs.o
	rm -f $@
	$(AR) rv $@ termcap.o tgoto.o tputs.o
	strip $@

termcap.o:../local/uparm.h termcap.c

.c.o:
	$(CCF) -c $<
