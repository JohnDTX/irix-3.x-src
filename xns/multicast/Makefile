#
#
include ${ROOT}/usr/include/make/commondefs
# Makefile for constructing the zmsg library
#
# $Source: /d2/3.7/src/xns/multicast/RCS/Makefile,v $
# $Revision: 1.1 $
# $Date: 89/03/27 18:41:43 $
#
PROGS=	libzmsg.a
OBJS= zinit.o zsend.o zrecv.o mpr.o

all:	${PROGS}

libzmsg.a:	${OBJS}
	ar rcv libzmsg.a ${OBJS}
	strip libzmsg.a
	ranlib libzmsg.a

clean clobber:
	rm -f $(OBJS) libzmsg.a

install: $(PROGS)
	${INSTALL} -src zmsg.h -u root -g sys -m 444 \
		-F /usr/include/xns -idb "std.sw.unix mr" Xzmsg.h
	${INSTALL} -u bin -g bin -m 644 -F /usr/lib \
		-idb "std.sw.unix mr" libzmsg.a
	ranlib ${ROOT}/usr/lib/libzmsg.a
