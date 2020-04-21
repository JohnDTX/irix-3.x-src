#	libplot.a make file
#
#
include ${ROOT}/usr/include/make/commondefs
#	SCCS:	@(#)plot.mk	1.3

I	= $(DEST)/usr/lib
LIB	= libplot.a
AR	= ar

SOURCE = arc.c box.c circle.c close.c cont.c dot.c erase.c label.c \
	 line.c linmod.c move.c open.c point.c putsi.c space.c
OFILES = arc.o box.o circle.o close.o cont.o dot.o erase.o label.o \
	 line.o linmod.o move.o open.o point.o putsi.o space.o

all:	$(LIB)

$(LIB):	$(OFILES)
	rm -f $@
	$(AR) rv $@ $(OFILES)
	ranlib $@

install:all
	${INSTALL} -g bin -u bin -m 644 -F /usr/lib \
	-idb "std.sw.unix mr" "${LIB}"
	ranlib ${ROOT}/usr/lib/"${LIB}"

clean:
	rm -f *.o

clobber:clean
	rm -f $(LIB)
