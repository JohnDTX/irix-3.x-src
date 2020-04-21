#
#ident	"\%W\%"
#
# Notes:
#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	makefile for aps-5 driver, fonts, etc.
#
# DSL 2

#

#
# Common Definitions
#
include	$(ROOT)/usr/include/make/commondefs

#
# Compile Time Options
#
#LCFLAGS = -O
LCFLAGS =
LDFLAGS = -s
YPLIBS = -lyp -lrpc -lbsd -ldbm

#
# Local Definitions
#
OL = $(ROOT)/
INS = :
INSDIR = $(OL)usr/bin
FONTHOME = $(OL)usr/lib/font
FONTDIR = $(OL)usr/lib/font/devaps
MAKEDEV = ./makedev
FFILES = [A-Z] [A-Z][0-9A-Z] DESC
OFILES = [A-Z].[oa][ud][td] [A-Z][0-9A-Z].[oa][ud][td] DESC.out

#
# Targets/Rules
#

default:    all

clean:
	rm -f *.o

clobber:	clean
	rm -f $(OFILES) daps makedev

FRC:

#
# Specific Target/Rules follow
#
install: default

all:	daps aps_fonts

daps:	daps.o ../draw.o build.o
	$(CC) $(CFLAGS) $(LDFLAGS) $(FFLAG) -o daps daps.o ../draw.o build.o -lm $(YPLIBS)
	$(INS)  daps $(INSDIR)
	$(CH) cd $(INSDIR); \
	chmod 755 daps; chgrp bin daps; chown bin daps

daps.o:	aps.h ../dev.h daps.h daps.g
	$(CC) $(CFLAGS) -I../ -c daps.c

../draw.o:	../draw.c
	cd ..;  $(MAKE) draw.o

aps_fonts:	makedir $(MAKEDEV)
	$(MAKEDEV) DESC
	for i in $(FFILES); \
	do	if [ ! -r $$i.out ] || [ -n "`find $$i -newer $$i.out -print`" ]; \
		   then	$(MAKEDEV) $$i; \
		fi; \
	done
	-if [ -r LINKFILE ]; then \
	    sh ./LINKFILE; \
	fi
	$(INS) $(OFILES) version $(FONTDIR);
	cd $(FONTDIR); chmod 644 $(OFILES) version;  \
		$(CH) chgrp bin $(OFILES) version; chown bin $(OFILES) version

$(MAKEDEV):	$(MAKEDEV).c ../dev.h
	cc -I../ $(LDFLAGS) -o $(MAKEDEV) $(MAKEDEV).c
makedir:
	if [ ! -d $(FONTHOME) ] ; then rm -f $(FONTHOME);  mkdir $(FONTHOME); \
		chmod 755 $(FONTHOME);  fi
	if [ ! -d $(FONTDIR) ] ; then rm -f $(FONTDIR);  mkdir $(FONTDIR); \
		chmod 755 $(FONTDIR);  fi

#install:
#	$(MAKE) -f devaps.mk INS=cp all ROOT=$(ROOT) CH=$(CH) \
#		LCFLAGS=$(LCFLAGS) LDFLAGS=$(LDFLAGS)

