#	Development make file for nroff/troff

#
#
include ${ROOT}/usr/include/make/commondefs
#
# $Source: /d2/3.7/src/usr.bin/text/roff.d/RCS/Makefile,v $
# @(#)$Revision: 1.1 $
# $Date: 89/03/27 18:27:22 $
#
# Regular make options:
#	nroff	make nroff
#	troff	make troff
#	suftab	make suftab
#	lint	lint the nroff/troff source code
#	clean	remove temporary object files
#	clobber	remove all object files
#
# Development make options:
#	dnroff	make debug version of nroff (dnroff)
#	dtroff	make debug version of troff (dtroff)
#	bldtab	generate name table builder program

NTSOURCE= hytab.c n1.c n10.c n[2-9i].c nii.c ntab.c strs.hd t10.c \
	  t6.c tab3.c tdef.hd tw.hd uns.hd def.nr.names def.rq.names
SMSOURCE= suftabmake.c suftab.c
SOURCE	= $(NTSOURCE) $(SMSOURCE)

NFILES	= n1.o n2.o n3.o n4.o n5.o n6.o n7.o n8.o n9.o n10.o ni.o \
	  nii.o ntab.o hytab.o
TFILES	= n1.o n2.o n3.o n4.o n5.o t6.o n7.o n8.o n9.o t10.o ni.o \
	  nii.o tab3.o hytab.o

B	= /usr/bin
L	= /usr/lib
F	= /lib/font
T	= /lib/term

MAGIC	=
X	=
Y	= not_yet

all:	nroff troff suftab
	cd fonts.d ; make $@
	cd terms.d ; make $@

#	Don't depend on all so that we don't do subdirectories twice.
install:nroff troff suftab
	${INSTALL} -u bin -g bin -m 755 -F /usr/bin  \
	-idb "std.sw.unix mr" "nroff troff"
	${INSTALL} -u bin -g bin -m 644 -F /usr/lib  \
	-idb "std.sw.unix mr" "suftab "
	cd fonts.d ; make $@
	cd terms.d ; make $@

clean:
	rm -f *.o suftab suftabmake
	rm -f bldtab ??tab.dt
	cd fonts.d ; make $@
	cd terms.d ; make $@

#	Don't depend on clean so that we don't do subdirectories twice.
clobber:
	rm -f *.o suftab suftabmake
	rm -f bldtab ??tab.dt
	rm -f nroff troff suftab dnroff dtroff
	cd fonts.d ; make $@
	cd terms.d ; make $@

nroff$X:ALWAYS
	$(MAKE) $@ X=done Y= MAGIC=-DNROFF

nroff$Y:switch.$$@ $(NFILES)
	$(CCF) $(NFLAG) $(LDFLAGS) -o $@ $(NFILES)

troff:	switch.$$@ $(TFILES)
	$(CCF) $(NFLAG) $(LDFLAGS) -o $@ $(TFILES)

switch.nroff switch.troff:
	rm -f switch.nroff switch.troff *.o
	touch $@

ALWAYS:

.c.o:
	$(CCF) $(MAGIC) -c $<

n[18].o::tdef.hd strs.hd tw.hd uns.hd

n[2-79].o n10.o nii.o::tdef.hd strs.hd tw.hd

t6.o t10.o::tdef.hd strs.hd

ni.o::	tdef.hd nrtab.dt rqtab.dt


#	***** NOTE *****  the 300 in the next line is hardwired for (NN)
nrtab.dt:bldtab def.nr.names
	./bldtab 300 def.nr.names > nrtab.dt

#	***** NOTE *****  the 350 in the next line is hardwired for (NM)
rqtab.dt:bldtab def.rq.names
	./bldtab 350 def.rq.names > rqtab.dt

#	Run suftabmake on Source Machine
#	suftabmake.c includes suftab.c
suftab:	suftabmake
	./suftabmake
	chmod 644 $@

#	Make suftabmake on Source Machine
#	suftabmake.c includes suftab.c
suftabmake:suftabmake.c suftab.c
	cc -I${ROOT}/usr/include -O -o $@ suftabmake.c

#	Run bldtab on Source Machine
bldtab:	bldtab.c tdef.hd
	cc -s -I${ROOT}/usr/include -O -o $@ bldtab.c

dnroff:	$(NFILES)
	$(CCF) $(NFLAG) -o $@ $(NFILES)

dtroff:	$(TFILES)
	$(CCF) $(NFLAG) -o $@ $(TFILES)

lint:	n1.c
	@lint -DNROFF -haxcs n[1-9].c n10.c ni.c nii.c ntab.c hytab.c | \
	  pr -h "lint of nroff"
	@lint -haxcs n[1-5].c t6.c n[7-9].c t10.c ni.c nii.c tab3.c hytab.c | \
	  pr -h "lint of troff"
