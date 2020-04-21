#	makefile for workstation print directory
#	$Source: /d2/3.7/src/usr.bin/print/att/RCS/Makefile,v $
#	@(#)$Revision: 1.1 $
#	$Date: 89/03/27 18:06:13 $

DIRS	= cmd


all:	
	for i in $(DIRS); do (cd $$i; make $@); done

install: 
	for i in $(DIRS); do (cd $$i; make $@); done

clean: FRC
	for i in $(DIRS); do (cd $$i; make $@); done

clobber:FRC
	for i in $(DIRS); do (cd $$i; make $@); done


FRC:

