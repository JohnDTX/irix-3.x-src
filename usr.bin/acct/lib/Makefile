# makefile for usr.bin/acct/lib

# $Source: /d2/3.7/src/usr.bin/acct/lib/RCS/Makefile,v $
# @(#)$Revision: 1.1 $
# $Date: 89/03/27 17:42:45 $

LIB	= a.a

OBJS	= pnpsplit.o copyn.o devtolin.o expand.o lintodev.o namtouid.o \
	tmless.o tmsecs.o uidtonam.o \
	substr.o regex.o regcmp.o kcore.o

all:	$(LIB)

$(LIB):$(OBJS)
	ar rv $@ $(OBJS)
	ranlib $@

install:all

clean:
	rm -f *.o

clobber:clean
	rm -f a.a

.c.o:
	$(CC) $(CFLAGS) -Dhandy -I.. -I${ROOT}/usr/include  -c $< ; \
	set +e;$(LD) -r -x $@;set -e;test $$? -le 1 ; \
	mv a.out $@

.s.o:
	$(AS) $< ; \
	mv a.out $@ ; \
	set +e;$(LD) -r -x $@;set -e;test $$? -le 1 ; \
	mv a.out $@

.PRECIOUS:$(LIB)
