#	makefile for workstation print directory
#	$Source: /d2/3.7/src/usr.bin/print/att/cmd/RCS/Makefile,v $
#	@(#)$Revision: 1.1 $
#	$Date: 89/03/27 18:06:17 $

DIRS	= text

all:	
	for i in $(DIRS); do (cd $$i; make -f $$i.mk $@ ); done

install: 
	for i in $(DIRS); do (cd $$i; make -f $$i.mk $@ ); done

clean: FRC
	for i in $(DIRS); do (cd $$i; make -f $$i.mk $@ ); done

clobber:FRC
	for i in $(DIRS); do (cd $$i; make -f $$i.mk $@ ); done


FRC:

