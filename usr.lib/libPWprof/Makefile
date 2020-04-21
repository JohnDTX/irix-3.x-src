#
# Makefile for /usr/lib directory
# 

DIRS	= libPW libcrypt libcurses libl libplot libstand libtermcap liby \
	    libg libm libmsky libmfpa
PROGS	= atrun clstat.o manprog unittab
FILES	= eign lib.b make.h
#makekey can't be shipped internationally.

BOOTDIRS = libPW libcrypt libl libm liby
BOOTPROGS = clstat.o
BOOTFILES = make.h

#	termcap done in bin/vi

L=	$(DEST)/usr/lib

all:	$(PROGS) $(FILES) FRC
	for i in $(DIRS); do (cd $$i; make $@); done

install: $(PROGS) $(FILES) FRC
	cp $(PROGS) $(FILES) $L/.
	cd $L ; chmod 755 $(PROGS) ; chmod 644 $(FILES)
	cd $L ; chmod 644 unittab
	cd $L ; chmod 644 clstat.o
	cd $L ; chgrp bin $(PROGS) $(FILES)
	cd $L ; chown bin $(PROGS) $(FILES)
	for i in $(DIRS); do (cd $$i; make $@); done

boot: $(BOOTPROGS) $(BOOTFILES)
	cp $(BOOTPROGS) $(BOOTFILES) $(DEST)/usr/lib
	for i in $(BOOTDIRS); do (cd $$i ; make install); done

clean:
	rm -f *.o a.out core y.tab.c y.tab.h
	for i in $(DIRS); do (cd $$i; make $@); done

clobber:
	rm -f *.o a.out core y.tab.c y.tab.h
	rm -f $(PROGS)
	for i in $(DIRS); do (cd $$i; make $@); done

$(DIRS) DIRS: FRC
	echo $(cd $@; make all

FRC:
