#	makefile for workstation print directory
#	$Source: /d2/3.7/src/usr.bin/print/lp.spool/RCS/Makefile,v $
#	@(#)$Revision: 1.1 $
#	$Date: 89/03/27 18:16:14 $

DIRS	= etc lib 

FILES	= config printer
PUTDIR	= $(DESTDIR)/usr/spool/lp/sgi

all:
	for i in $(DIRS); do (cd $$i; make $@); done

install: 
	for i in $(DIRS); do (cd $$i; make $@); done
	cp $(FILES) $(PUTDIR)
	(cd $(PUTDIR); chown lp $(FILES) )
	(cd $(PUTDIR); chgrp lp $(FILES) )
	mkdir $(PUTDIR)/log
	chown lp $(PUTDIR)/log
	chgrp lp $(PUTDIR)/log

clean: FRC
	for i in $(DIRS); do (cd $$i; make $@); done

clobber:FRC
	for i in $(DIRS); do (cd $$i; make $@); done


FRC:

