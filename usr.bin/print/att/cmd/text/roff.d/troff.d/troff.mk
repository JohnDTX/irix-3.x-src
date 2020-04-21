#
#ident	"\%W\%"
#
# Notes:
#
#	Copyright (c) 1984 AT&T
#	  All Rights Reserved


#	makefile for (di) troff.  Also builds subproducts - typesetter
#		drivers, fonts, rasters, etc.
#
# DSL 2.

#
# Common Definitions
#
include	$(ROOT)/usr/include/make/commondefs

# Compile Time Options
#
#LCFLAGS = -O
LCFLAGS =
INCORE = -DINCORE
USG = -DUSG
LLDFLAGS = -s
IFLAG = -i
HCC=	ROOTDIR=/usr /bin/cc

#
# Local Definitions
#

I_FLAGS=-idb "dwb.sw.dwb"
# configuration files that should be discarded if user has one already
NOUPDATE=-idb "dwb.sw.dwb config(noupdate)"
# configuration files that should override, but save, existing version
UPDATE=-idb "dwb.sw.dwb config(update)"
SU_FLAGS=-m 04555 -u root -g sys $(I_FLAGS)
GU_FLAGS=-m 755 -u bin -g bin $(UPDATE)
DAT_FLAGS=-m 644 -u bin -g bin $(UPDATE)

CFILES=n1.c n2.c n3.c n4.c n5.c t6.c n7.c n8.c n9.c t10.c ni.c nii.c hytab.c \
    suftab.c
HFILES=../tdef.h ../ext.h dev.h
TFILES=n1.o n2.o n3.o n4.o n5.o t6.o n7.o n8.o n9.o t10.o ni.o nii.o hytab.o \
    suftab.o 
INSDIR = /usr/bin

all:	troff fonts

default:    all

clean:	hcclean taclean tcclean
	rm -f $(TFILES) draw.o

clobber:	hcclobber taclobber tcclobber
	rm -f $(TFILES) draw.o makedev.o
	rm -f troff tc makedev
	cd devaps;  $(MAKE) -f devaps.mk clobber
	cd devi10;  $(MAKE) -f devi10.mk clobber

FRC:

#
# Specific Target/Rules follow
#
install:	troff fonts
	$(INSTALL) $(I_FLAGS) -F $(INSDIR) "troff makedev tc"

instroff:	troff fonts
	$(INSTALL) $(I_FLAGS) -F $(INSDIR) "troff makedev"
	
insfonts:	fonts
	$(INSTALL) $(I_FLAGS) -F $(INSDIR) tc

boot:	makedev
	cp makedev $(DEST)/usr/bin

troff:	$(TFILES) makedev
	- if vax || u3b || m68000  ; \
	then 	$(CC)  $(IFLAG) $(TFILES) $(LDFLAGS) -o troff; fi
	- if mips ; \
	then $(CC) $(TFILES) $(LLDFLAGS) -o troff; fi

n1.o:	../n1.c ../tdef.h ../ext.h
	pwd
	$(CC) $(CFLAGS)  $(INCORE) -c ../n1.c
n2.o:	../n2.c ../tdef.h ../ext.h
	$(CC) $(CFLAGS)  $(INCORE) -c ../n2.c
n3.o:	../n3.c ../tdef.h ../ext.h
	$(CC) $(CFLAGS)  $(INCORE) -c ../n3.c
n4.o:	../n4.c ../tdef.h ../ext.h
	$(CC) $(CFLAGS)  $(INCORE) -c ../n4.c
n5.o:	../n5.c ../tdef.h ../ext.h
	$(CC) $(CFLAGS)  $(USG) $(INCORE) -c ../n5.c
t6.o:	t6.c ../tdef.h dev.h ../ext.h
	$(CC) $(CFLAGS)  $(INCORE) -I../ -c t6.c
n7.o:	../n7.c ../tdef.h ../ext.h
	$(CC) $(CFLAGS)  $(INCORE) -c ../n7.c
n8.o:	../n8.c ../tdef.h ../ext.h
	$(CC) $(CFLAGS)  $(INCORE) -c ../n8.c
n9.o:	../n9.c ../tdef.h ../ext.h
	$(CC) $(CFLAGS)  $(INCORE) -c ../n9.c
t10.o:	t10.c ../tdef.h dev.h ../ext.h
	$(CC) $(CFLAGS)  $(INCORE) -I../ -c t10.c
ni.o:	../ni.c ../tdef.h
	$(CC) $(CFLAGS)  $(INCORE) -c ../ni.c
nii.o:	../nii.c ../tdef.h ../ext.h
	$(CC) $(CFLAGS)  $(INCORE) -c ../nii.c
hytab.o:	../hytab.c
	$(CC) $(CFLAGS)  $(INCORE) -c ../hytab.c
suftab.o:	../suftab.c
	$(CC) $(CFLAGS)  $(INCORE) -c ../suftab.c

#fonts:	tc aps i10
fonts:	tc

tc:	tc.o draw.o
	$(CC) $(CFLAGS) $(LDFLAGS) $(FFLAG) tc.o draw.o -lm -o tc

#this doesn't seem to be called...
hc:	hc.o draw.o
	$(CC) $(CFLAGS) $(FFLAG)  hc.o draw.o $(LLDFLAGS) -lm -o hc
	$(INS) hc $(INSDIR)
	$(CH) cd $(INSDIR); chmod 755 hc; chgrp bin hc; chown bin hc

ta:	ta.o draw.o
#	$(CC) $(CFLAGS) $(LLDFLAGS) $(FFLAG) ta.o draw.o -lm -o ta
#	$(CC) $(CFLAGS) $(FFLAG) ta.o draw.o $(LLDFLAGS) -lm -o ta
	$(INS) ta $(INSDIR)
	$(CH) cd $(INSDIR); chmod 755 ta; chgrp bin ta; chown bin ta

tc.o:	dev.h
hc.o:	dev.h
ta.o:	dev.h

aps:	draw.o makedev
	cd devaps;  $(MAKE) -f devaps.mk INS=$(INS) ROOT=$(ROOT) CH=$(CH) \
		LCFLAGS=$(LCFLAGS) LLDFLAGS=$(LLDFLAGS)

i10:	draw.o makedev
	cd devi10;  $(MAKE) -f devi10.mk INS=$(INS) ROOT=$(ROOT) CH=$(CH) \
		LCFLAGS=$(LCFLAGS) LLDFLAGS=$(LLDFLAGS)

cleanmkdv:
	- rm -f makedev

# Makedev runs on the build machine to help with the build; it is also
# part of the build.  So, we build a version that runs on the build host
# and install it.  Then we make a version for the target machine (sometimes
# the same host) and leave that in the directory to be installed by the
# general install: target.
# if HOSTENV is defined, we're building two versions, one for a foreign host.
makedev:	cleanmkdv makedev.c dev.h
	if [ -n "$$HOSTENV" ] ; \
	then \
		$(HCC) -c -Dsgi -I/usr/include -DBSD makedev.c ; \
		$(HCC) -DBSD $(LDFLAGS) makedev.o -o makedev; \
		mv makedev $(TOOLROOT)/usr/bin ; \
		$(CC) -c ${CFLAGS}  makedev.c ; \
		$(CC) ${CFLAGS} makedev.o $(LDFLAGS) -o makedev ; \
	else \
		$(CC) -c ${CFLAGS}  makedev.c ; \
		$(CC) ${CFLAGS}  makedev.o $(LDFLAGS) -o makedev; \
		cp makedev $(TOOLROOT)/usr/bin ; \
	fi

Dtroff:
	$(MAKE) -f troff.mk troff LCFLAGS="$(LCFLAGS) -g -DDEBUG" \
		INCORE=$(INCORE) LLDFLAGS=-n INS=: CH=#
	mv troff Dtroff

#install:
#	$(MAKE) -f troff.mk INS=cp all ROOT=$(ROOT) CH=$(CH) \
#		LCFLAGS=$(LCFLAGS) LLDFLAGS=$(LLDFLAGS) INCORE=$(INCORE)

tcinstall:  ;  $(MAKE) -f troff.mk INS=cp tc ROOT=$(ROOT) CH=$(CH)
hcinstall:  ;  $(MAKE) -f troff.mk INS=cp hc ROOT=$(ROOT) CH=$(CH)
tainstall:  ;  $(MAKE) -f troff.mk INS=cp ta ROOT=$(ROOT) CH=$(CH)

	cd devaps;  $(MAKE) -f devaps.mk clean
	cd devi10;  $(MAKE) -f devi10.mk clean
hcclean:  ;  rm -f hc.o
taclean:  ;  rm -f ta.o
tcclean:  ;  rm -f tc.o

hcclobber:	hcclean
	rm -f hc
taclobber:	taclean
	rm -f ta
tcclobber:	tcclean
	rm -f tc
