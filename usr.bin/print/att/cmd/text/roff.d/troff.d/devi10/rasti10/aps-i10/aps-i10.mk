#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	makefile for aps-5 version of Imagen Imprint-10 raster tables -
#		for aps compatable printing on an Imagen
#
# DSL 2

OL = $(ROOT)/
INS = :
FONTDIR = $(OL)usr/lib/font/devi10/rasti10/devaps
MAKEDEV = ../../../makedev
FFILES = [A-Z] [A-Z][0-9A-Z] DESC
OFILES = [A-Z].out [A-Z][0-9A-Z].out DESC.out

all:	aps_fonts

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
	$(INS) $(OFILES) $(FONTDIR);
	cd $(FONTDIR); chmod 644 $(OFILES);  \
		$(CH) chgrp bin $(OFILES); chown bin $(OFILES)
	-if u3b2 ; \
	then \
	cd $(FONTDIR); pack $(OFILES); \
	fi

makedir:
	if [ ! -d $(FONTDIR) ] ; then rm -f $(FONTDIR);  mkdir $(FONTDIR); \
		chmod 755 $(FONTDIR);  fi

$(MAKEDEV):	$(MAKEDEV).c
	cc $(CFLAGS) $(LDFLAGS) -o $(MAKEDEV) $(MAKEDEV).c

install:
	$(MAKE) -f aps-i10.mk INS=cp all ROOT=$(ROOT) CH=$(CH)

clean:
	rm -f *.o

clobber:	clean
	rm -f *.out
