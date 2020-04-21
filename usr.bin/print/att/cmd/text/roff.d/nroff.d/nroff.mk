#
#ident	"\%W\%"
#
# Notes:
#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	makefile for new nroff.  Also builds terminal tables.
#
# DSL 2.
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
INCORE = -DINCORE
USG = -DUSG
LDFLAGS = -s
# nulled the value of IFLAG for mips port
#IFLAG = -i
IFLAG =

#
# Local Definitions
#
#
I_FLAGS	=-idb "dwb.sw.dwb"
# configuration files that should be discarded if user has one already
NOUPDATE=-idb "dwb.sw.dwb config(noupdate)"
# configuration files that should override, but save, existing version
UPDATE	=-idb "dwb.sw.dwb config(update)"
SU_FLAGS=-m 04555 -u root -g sys $(I_FLAGS)
GU_FLAGS=-m 755 -u bin -g bin $(UPDATE)
DAT_FLAGS=-m 644 -u bin -g bin $(UPDATE)

NROFFLAG = -DNROFF
CFILES=n1.c n2.c n3.c n4.c n5.c n6.c n7.c n8.c n9.c n10.c ni.c nii.c hytab.c \
	suftab.c
HFILES=../tdef.h ../ext.h tw.h
NFILES=n1.o n2.o n3.o n4.o n5.o n6.o n7.o n8.o n9.o n10.o ni.o nii.o hytab.o \
	suftab.o
INSDIR = /usr/bin


#
# Targets/Rules
#
        
default:    all

clean:
	rm -f $(NFILES)

clobber:	clean
	rm -f nroff
	cd terms.d;  $(MAKE) -f terms.mk clobber

FRC:

#
# Specific Target/Rules follow
#
install:
	insnroff insterms

insnroff:	nroff
	$(INSTALL) $(I_FLAGS) -F $(INSDIR) nroff

insterms:
	cd terms.d;  $(MAKE) -f terms.mk install


all:	nroff terms

nroff:	$(NFILES)
	$(CC) $(LDFLAGS) $(IFLAG) $(NFILES) -o nroff

n1.o:	../n1.c ../tdef.h ../ext.h tw.h
	$(CC) $(CFLAGS) $(INCORE) $(NROFFLAG) -I./ -c ../n1.c
n2.o:	../n2.c ../tdef.h ../ext.h tw.h
	$(CC) $(CFLAGS) $(INCORE) $(NROFFLAG) -I./ -c ../n2.c
n3.o:	../n3.c ../tdef.h ../ext.h tw.h
	$(CC) $(CFLAGS) $(INCORE) $(NROFFLAG) -I./ -c ../n3.c
n4.o:	../n4.c ../tdef.h ../ext.h tw.h
	$(CC) $(CFLAGS) $(INCORE) $(NROFFLAG) -I./ -c ../n4.c
n5.o:	../n5.c ../tdef.h ../ext.h
	$(CC) $(CFLAGS) $(USG) $(INCORE) $(NROFFLAG) -c ../n5.c
n6.o:	n6.c ../tdef.h ../ext.h tw.h
	$(CC) $(CFLAGS) $(INCORE) $(NROFFLAG) -I../ -c n6.c
n7.o:	../n7.c ../tdef.h ../ext.h tw.h
	$(CC) $(CFLAGS) $(INCORE) $(NROFFLAG) -I./ -c ../n7.c
n8.o:	../n8.c ../ext.h
	$(CC) $(CFLAGS) $(INCORE) $(NROFFLAG) -c ../n8.c
n9.o:	../n9.c ../tdef.h ../ext.h tw.h
	$(CC) $(CFLAGS) $(INCORE) $(NROFFLAG) -I./ -c ../n9.c
n10.o:	n10.c ../tdef.h ../ext.h tw.h
	$(CC) $(CFLAGS) $(INCORE) $(NROFFLAG) -I../ -c n10.c
ni.o:	../ni.c ../tdef.h
	$(CC) $(CFLAGS) $(INCORE) $(NROFFLAG) -c ../ni.c
nii.o:	../nii.c ../tdef.h ../ext.h tw.h
	$(CC) $(CFLAGS) $(INCORE) $(NROFFLAG) -I./ -c ../nii.c
hytab.o:	../hytab.c
	$(CC) $(CFLAGS) $(INCORE) $(NROFFLAG) -c ../hytab.c
suftab.o:	../suftab.c
	$(CC) $(CFLAGS) $(INCORE) $(NROFFLAG) -c ../suftab.c

terms:
	cd terms.d;  $(MAKE) -f terms.mk

Dnroff:
	$(MAKE) -f nroff.mk nroff CFLAGS="$(CFLAGS) -g -DDEBUG" \
		INCORE=$(INCORE) LDFLAGS=-n
	mv nroff Dnroff
