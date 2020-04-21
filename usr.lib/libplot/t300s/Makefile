#	lib300s.a make file

#
#
include ${ROOT}/usr/include/make/commondefs
#	SCCS:	@(#)t300s.mk	1.3

LIB	= lib300s.a
AR	= ar
SOURCE	= con.h arc.c box.c circle.c close.c dot.c erase.c label.c \
	 line.c linmod.c move.c open.c point.c space.c subr.c
OFILES	= arc.o box.o circle.o close.o dot.o erase.o label.o \
	 line.o linmod.o move.o open.o point.o space.o subr.o

all:	$(LIB)

$(LIB):	$(OFILES)
	rm -f $(LIB)
	$(AR) rv $(LIB) $(OFILES)
	ranlib $(LIB)

install:all
	${INSTALL} -u bin -g bin -m 644 -F /usr/lib \
	-idb "std.sw.unix" "${LIB}"
	ranlib ${ROOT}/usr/lib/${LIB}

clean:
	rm -f *.o

clobber:clean
	rm -f $(LIB)
