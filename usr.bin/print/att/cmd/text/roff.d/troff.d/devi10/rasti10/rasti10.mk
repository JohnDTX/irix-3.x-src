#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	makefile for raster tables for Imagen Imprint-10 typesetter
#
# DSL 2

OL = $(ROOT)/
CFLAGS = -O
LDFLAGS = -s
INS = :
RASTDIR = $(OL)usr/lib/font/devi10/rasti10
FILES = [A-Z].[0-9]* [A-Z][A-Z0-9].[0-9]*
3BRASTDIR = 3b-rast

all:	i10_rasts aps

i10_rasts:	make3brast
	for i in $(FILES); \
	do	if [ ! -s $$i ]; \
		then echo ERROR: bad raster file $$i; \
		fi; \
	done
	if vax ;  then : ; \
	  else	if [ ! -d $(3BRASTDIR) ]; \
			then rm -f $(3BRASTDIR); mkdir $(3BRASTDIR); \
		fi ; \
		for i in $(FILES); \
		do	if [ ! -r $(3BRASTDIR)/$$i ] || [ -n "`find $$i -newer $(3BRASTDIR)/$$i -print`" ]; \
			   then	echo $$i;  ./make3brast $$i > $(3BRASTDIR)/$$i; \
			fi; \
		done; \
	fi
	if [ ! -d $(RASTDIR) ] ; then rm -f $(RASTDIR);  mkdir $(RASTDIR); \
		chmod 755 $(RASTDIR);  fi
	if vax ;  then $(INS) $(FILES) $(RASTDIR); \
		 else  cd $(3BRASTDIR);  $(INS) $(FILES) $(RASTDIR); \
	fi
	$(INS) RASTERLIST $(RASTDIR)
	cd $(RASTDIR); chmod 644 $(FILES) RASTERLIST; \
		$(CH) chgrp bin $(FILES) RASTERLIST; chown bin $(FILES) RASTERLIST
	-if u3b2 ; \
	then \
	cd $(RASTDIR); pack $(FILES) RASTERLIST; \
	fi

make3brast:	make3brast.c
	cc $(CFLAGS) $(LDFLAGS) -o make3brast make3brast.c

aps:
	cd aps-i10; $(MAKE) -f aps-i10.mk all INS=$(INS) ROOT=$(ROOT) CH=$(CH)

fdump:	fdump.c ../glyph.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o fdump fdump.c

fbuild:	fbuild.c ../glyph.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o fbuild fbuild.c

install:
	$(MAKE) -f rasti10.mk INS=cp all ROOT=$(ROOT) CH=$(CH)

clean:
	rm -f *.o
	cd aps-i10;  $(MAKE) -f aps-i10.mk clean

clobber:	clean
	rm -f make3brast fdump fbuild
	rm -rf $(3BRASTDIR)
	cd aps-i10;  $(MAKE) -f aps-i10.mk clobber
