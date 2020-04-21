# Makefile for rogue for System V using standard conventions

# $Source: /d2/3.7/src/games/rogue/RCS/Makefile,v $
# @(#)$Revision: 1.1 $
# $Date: 89/03/27 15:44:53 $

install: install.$(GLHDWR)

install.both:
	cd libCurses ; $(MAKE) install
	cd obj.gl1 ; GLHDWR=gl1 $(MAKE) install
	cd obj.gl2 ; GLHDWR=gl2 $(MAKE) install

install.gl1:
	cd libCurses ; $(MAKE) install
	cd obj.gl1 ; $(MAKE) install

install.gl2:
	cd libCurses ; $(MAKE) install
	cd obj.gl2 ; $(MAKE) install

clean clobber:
	cd libCurses ; $(MAKE) clean
	cd obj.gl1 ; $(MAKE) clean
	cd obj.gl2 ; $(MAKE) clean
