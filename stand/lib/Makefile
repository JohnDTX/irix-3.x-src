#
#
include ${ROOT}/usr/include/make/commondefs

# $Source: /d2/3.7/src/stand/lib/RCS/Makefile,v $
# $Revision: 1.1 $
# $Date: 89/03/27 17:14:11 $
#
#add the subdir and for i stuff


include Make.h

# 
# Local definitons
#
LINCL	= -I../include -I${ROOT}/usr/include

all:	subdirs libstand.a srt0.o

quick:	libstand.a

dq:	subdq libstand.a
	$(ECHO) done

subdq:
	cd dev; make all

clean:
	cd dev; make clean
	cd gl; make clean
	rm -f *.o

clobber:	clean
	rm -f libstand.a srt0.o

install: all

subdirs:
	cd dev; make all
	cd gl; make all


libstand.a: dev/*.o gl/*.o
	$(AR) rv $@ $?
	ranlib $@
	$(ECHO) $@ ready

srt0.o:	srt0.c
	cc $(LOCORE) -DLOCORE $(LINCL) \
	-E srt0.c | egrep -v '^#' > srt0.i
	$(AS) -o $@ srt0.i
	rm -f srt0.i
