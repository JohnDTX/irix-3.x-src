#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#	makefile for canon/imagen Imprint-10 driver, fonts, etc.
#
# DSL 2

# to use the draw.c that take advantage of the Imagen graphics primitives,
#	change the definition of "DRAW" in the next line to "impdraw"
#	N.B  This will only work with Imagen versions 1.9 or later
DRAW = ../draw

OL = $(ROOT)/
#LCFLAGS = -O
LCFLAGS = 
LDFLAGS = -s
IFLAG = -i
INS = :
YPLIBS = -lyp -lrpc -lbsd -ldbm
INSDIR = $(OL)usr/bin
FONTHOME = $(OL)usr/lib/font
FONTDIR = $(OL)usr/lib/font/devi10
MAKEDEV = ../makedev
FFILES = [A-Z] [A-Z][0-9A-Z] DESC
OFILES = [A-Z].out [A-Z][0-9A-Z].out DESC.out
UNSUP = buildrast makefonts makei10 printrast readrast

all:	di10 i10_fonts i10_rasts

di10 dimpress:	dimpress.o $(DRAW).o glob.o misc.o rast.o oldrast.o
	$(CC) $(IFLAG) $(LDFLAGS) $(FFLAG) -o $@ dimpress.o $(DRAW).o \
		glob.o misc.o rast.o oldrast.o -lm $(YPLIBS)
	$(INS) $@ $(INSDIR)
	cd $(INSDIR); chmod 775 $@; $(CH) chgrp bin $@; chown bin $@

dimpress.o:	ext.h gen.h ../dev.h impcodes.h dimpress.h spectab.h
	if [ "$(DRAW)" = "impdraw" ] ; then \
		$(CC) -DIMPDRAW -I../ -c dimpress.c ; \
	else \
		$(CC) -I../ -c dimpress.c ; \
	fi
glob.o:	gen.h init.h
misc.o:	ext.h gen.h
rast.o:	ext.h gen.h rast.h
readrast.o:	ext.h gen.h rast.h
printrast.o:	ext.h gen.h rast.h impcodes.h
buildrast.o:	ext.h gen.h init.h rast.h buildrast.h
editrast.o:	gen.h rast.h buildrast.h editrast.h
oldrast.o:	ext.h gen.h glyph.h impcodes.h dimpress.h
impdraw.o:	gen.h impcodes.h

../draw.o:	../draw.c
	cd ..;  $(MAKE) draw.o


i10_fonts:	makedir $(MAKEDEV)
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
	if [ ! -d $(FONTHOME) ] ; then rm -f $(FONTHOME);  mkdir $(FONTHOME); \
		chmod 755 $(FONTHOME);  fi
	if [ ! -d $(FONTDIR) ] ; then rm -f $(FONTDIR);  mkdir $(FONTDIR); \
		chmod 755 $(FONTDIR);  fi

$(MAKEDEV):	$(MAKEDEV).c ../dev.h
	cc -I../ $(LDFLAGS) -o $(MAKEDEV) $(MAKEDEV).c

i10_rasts:	$(MAKEDEV)
	cd rasti10; $(MAKE) -f rasti10.mk all ROOT=$(ROOT) CH=$(CH) \
		INS=$(INS) LCFLAGS=$(LCFLAGS) LDFLAGS=$(LDFLAGS)

unsup:	$(UNSUP)

makei10:	makei10.c ../dev.h
	$(CC) $(LDFLAGS) -I../ -o makei10 makei10.c
makefonts:	makefonts.c ../dev.h
	$(CC) $(FFLAG) $(LDFLAGS) -I../ -o makefonts makefonts.c

readrast:	readrast.o glob.o misc.o rast.o
	$(CC) $(LDFLAGS) -o readrast readrast.o glob.o misc.o rast.o
printrast:	printrast.o glob.o misc.o rast.o
	$(CC) $(LDFLAGS) -o printrast printrast.o glob.o misc.o rast.o
buildrast:	buildrast.o glob.o misc.o rast.o editrast.o
	$(CC) $(LDFLAGS) -o buildrast buildrast.o glob.o misc.o rast.o editrast.o

install:
	$(MAKE) -f devi10.mk INS=cp all ROOT=$(ROOT) CH=$(CH) \
		LCFLAGS=$(LCFLAGS) LDFLAGS=$(LDFLAGS)

clean:
	rm -f *.o

clobber:	clean
	rm -f $(OFILES) di10 dimpress $(UNSUP)
	cd rasti10;  $(MAKE) -f rasti10.mk clobber
