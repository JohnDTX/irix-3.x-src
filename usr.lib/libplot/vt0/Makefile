#	libvt0.a makefile

#
#
include ${ROOT}/usr/include/make/commondefs

#	SCCS:	@(#)vt0.mk	1.3
# $Source: /d2/3.7/src/usr.lib/libplot/vt0/RCS/Makefile,v $
# @(#)$Revision: 1.1 $
# $Date: 89/03/27 18:39:28 $

I	= $(DEST)/usr/lib
LIB	= libvt0.a
AR	= ar
SOURCE	= arc.c box.c circle.c close.c dot.c erase.c frame.c label.c \
	  line.c linmod.c move.c open.c point.c space.c subr.c
OFILES	= arc.o box.o circle.o close.o dot.o erase.o frame.o label.o \
	  line.o linmod.o move.o open.o point.o space.o subr.o

all:	$(LIB)

$(LIB):	$(OFILES)
	rm -f $(LIB)
	$(AR) rv $(LIB) $(OFILES)

install:all
	${INSTALL} -u bin -g bin -m 644 -F /usr/lib \
	-idb "std.sw.unix mr" "${LIB}"
	ranlib ${ROOT}/usr/lib/"${LIB}"

clean:
	rm -f *.o

clobber:clean
	rm -f $(LIB)
