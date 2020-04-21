#	lib4014.a make file

#
#
include ${ROOT}/usr/include/make/commondefs
#	SCCS:	@(#)t4014.mk	1.3
# @(#)$Header: /d2/3.7/src/usr.lib/libplot/t4014/RCS/Makefile,v 1.1 89/03/27 18:38:50 root Exp $
# $Log:	Makefile,v $
# Revision 1.1  89/03/27  18:38:50  root
# Initial check-in for 3.7
# 
# Revision 1.2  87/05/04  11:45:07  andre
# 3.6 Product, New 5.3 Style Installs Added, Native Installs done
# 

LIB	= lib4014.a
AR	= ar
SOURCE	= arc.c box.c circle.c close.c dot.c erase.c label.c \
	  line.c linemod.c move.c open.c point.c scale.c space.c subr.c
OFILES	= arc.o box.o circle.o close.o dot.o erase.o label.o \
	  line.o linemod.o move.o open.o point.o scale.o space.o subr.o

all:	$(LIB)

$(LIB):	$(OFILES)
	rm -f $(LIB)
	$(AR) rv $(LIB) $(OFILES)

install:all
	${INSTALL} -u bin -g bin -m 644 -F /usr/lib \
	-idb "std.sw.unix mr" "${LIB}" 
	ranlib ${ROOT}/usr/lib/${LIB}

clean:
	rm -f *.o

clobber:clean
	rm -f $(LIB)
