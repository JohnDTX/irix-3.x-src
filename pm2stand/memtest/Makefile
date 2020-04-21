#
# Makefile for Bruce's memtest
#

CFLAGS=-O

memtest: memtest.o cmp.o
	ld -o memtest /lib/crtstand.o memtest.o cmp.o /usr/lib/libstand.a \
		/usr/lib/libglxprom.a /usr/lib/libpm2prom.a -lc

install: memtest
	cp memtest $(DEST)/stand
	strip $(DEST)/stand/memtest
	chmod 755 $(DEST)/stand/memtest
	chgrp bin $(DEST)/stand/memtest
	chown bin $(DEST)/stand/memtest

clean:
	rm -f memtest.o cmp.o

clobber: clean
	rm -f memtest
