#
# This is the new version that creates separate "list.root" and "list.usr"
# from what used to be "list.unix", for selective boot tape loading.
#
# Note:  The "print" target does not currently work - dcs.
#

include ${ROOT}/usr/include/make/commondefs

# The list of known normal images; excludes boot

# The % tag is for maintainence releases.
entries = root usr upd man mail games gifts gltut demos ftn \
	pas nfs xns troff sys sysnfs

# Scripts to be installed

scripts = Install Read Remove Write Verify \
	instcmd.cln instcmd.sys instcmd.sysnfs

# Programs to be installed

progs =	Have Spchk isefs ieck newstat

# Standalone programs

stand = ipfex mdfex sifex

# Miscellaneous files to be installed

files = configs list.final list.ftnpas nsizes desc README

# Fluff should be generated for the following images

# removed for maintainence releases.

dofluff = upd man games demos ftn pas nfs xns sys sysnfs \
	gltut gifts troff mail

L = ../lists.3000

#
# Build and Install
# 

install all: versions images fluff ranlibs $(files) $(scripts) $(progs) $(stand)
	ROOT=$(DIST) $(INSTALL) -F /dist -m 0444 -u root -g sys \
		"../`L=$L ./getvers dist` list.*"
	ROOT=$(DIST) $(INSTALL) -F /dist -m 0444 -u root -g sys \
		"$(files)"
	ROOT=$(DIST) $(INSTALL) -F /dist -m 0755 -u root -g sys \
		"$(progs) $(scripts)"
	ROOT=$(DIST) $(INSTALL) -F /dist -m 0755 -u root -g sys \
		"$(stand)"

umph:
	ROOT=$(DIST) $(INSTALL) -F /dist -m 0444 -u root -g sys \
		"../`L=$L ./getvers dist` list.*"
	ROOT=$(DIST) $(INSTALL) -F /dist -m 0444 -u root -g sys \
		"$(files)"
	ROOT=$(DIST) $(INSTALL) -F /dist -m 0755 -u root -g sys \
		"$(progs) $(scripts)"
	ROOT=$(DIST) $(INSTALL) -F /dist -m 0755 -u root -g sys \
		"$(stand)"

#
# Remove all recreatable files
#

clean clobber:
	rm -f $(progs) $(stand) mklist *.o
	rm -f list.* fluff.* configs nsizes* desc
	rm -f ../Versions/*

versions:
	while read name pnum desc ; \
	do echo "$$name	$$pnum	$(RELEASE) $$desc" > ../Versions/$$pnum ;\
	done < $L/versions

#
# generate images.  We want to see the error message from this command,
# because it may have some interesting warnings, but we don't want it to
# stop the Make.  Thus "-", which is not a usual practice with me.
#

images:
	-./mkimage -v -d $(DIST) -i $(sbase)/idb -s $(sbase) -m 3000 $(entries)

fluff: mklist
	rm -f fluff.*
	./genfluff $L $(dofluff)
	if test -f $L/prefluff ; then cp $L/prefluff fluff.pre ; fi

ranlibs:
	./chkranlib -a -i $(sbase)/idb -s $(sbase)

configs:
	./genconfigs -m $(machine)

list.final:
	cp $L/final list.final

list.ftnpas:
	comm -12 list.ftn list.pas > list.ftnpas

nsizes:
	sort -u +1 -o nsizes nsizes.*

desc:
	ln desc.3000 desc

Have:	Have.c
	${CC} $(CFLAGS) $(LDFLAGS) Have.c -o Have

Spchk:	Spchk.c nstat.o
	${CC} $(CFLAGS) $(LDFLAGS) Spchk.c nstat.o -o Spchk

mklist:	mklist.c
	${CC} $(CFLAGS) $(LDFLAGS) mklist.c -o mklist

isefs:	isefs.c
	${CC} $(CFLAGS) $(LDFLAGS) isefs.c -o isefs

ieck:	ieck.c  superblk.o
	${CC} $(CFLAGS) $(LDFLAGS) ieck.c superblk.o -o ieck

newstat: newstat.c
	$(CC) $(CFLAGS) $(LDFLAGS) newstat.c -o newstat

superblk.o: superblk.c
	${CC} $(CFLAGS) $(LDFLAGS) -c superblk.c

nstat.o: nstat.c
	${CC} $(CFLAGS) $(LDFLAGS) -c nstat.c

$(stand):
	cp $(SRCROOT)/stand/cmd/$@/$@ .
