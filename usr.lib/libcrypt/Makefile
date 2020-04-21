# Makefile for usr.lib/libcrypt to isolate DES encryption algorithms for the
# convenience of the CIA.

include ${ROOT}/usr/include/make/commondefs

# @(#)$Header: /d2/3.7/src/usr.lib/libcrypt/RCS/Makefile,v 1.1 89/03/27 18:34:28 root Exp $
# $Log:	Makefile,v $
# Revision 1.1  89/03/27  18:34:28  root
# Initial check-in for 3.7
# 
# Revision 1.28  87/05/04  00:05:37  andre
# /ws/staff/andre/check2
# 
# Revision 1.2  86/03/04  12:14:02  root
# deleted DES stuff for local custs.
# 
# Revision 1.1  85/06/04  19:36:16  bob
# Initial revision
# 

all:	libcrypt.a libcrypt_p.a

install:all
#	For the Master Make
	${INSTALL} -u bin -g bin -m 644 -F /usr/lib  \
	-idb "std.sw.unix mr" "libcrypt.a libcrypt_p.a"
	ranlib ${ROOT}/usr/lib/libcrypt.a
	ranlib ${ROOT}/usr/lib/libcrypt_p.a
#	For domestic customers wanting DES
#	${INSTALL} -u bin -g bin -m 644 -F /usr/src/des  \
#	-idb "std.sw.unix mr" "libcrypt.a libcrypt_p.a"
#	ranlib ${ROOT}/usr/lib/libcrypt.a
#	ranlib ${ROOT}/usr/lib/libcrypt_p.a

clean:
	rm -f crypt.o

clobber:clean
	rm -f libcrypt.a libcrypt_p.a

libcrypt.a:crypt.c
	rm -f $@
	$(CC) $(CFLAGS) -c crypt.c
	ar r $@ crypt.o
	rm -f crypt.o

libcrypt_p.a:crypt.c
	rm -f $@
	$(CC) $(CFLAGS) -c -p crypt.c
	ar r $@ crypt.o
	rm -f crypt.o
