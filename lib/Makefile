# makefile for making all of /lib
# $Source: /d2/3.7/src/lib/RCS/Makefile,v $
# @(#)$Revision: 1.1 $
# $Date: 89/03/27 16:12:37 $

# libc is  made by wrapper

all install clean:
	cd c2		; make $@
	cd ccom		; make $@
	cd cpp		; make $@

clobber:
	cd libc		; make clean
	cd c2		; make clean
	cd ccom		; make clean
	cd cpp		; make clean
