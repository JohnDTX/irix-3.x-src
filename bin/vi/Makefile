# @(#) vi.mk 1.8
# $Source: /d2/3.7/src/bin/vi/RCS/Makefile,v $
# @(#)$Revision: 1.1 $
# $Date: 89/03/27 14:56:19 $

all install clean clobber:
	$Ecd misc	; make $@
	$Ecd termlib	; make $@
	$Ecd termcap.db	; make $@
	$Ecd vi		; make $@

misc:	FRC
	$Ecd misc	; make all

termlib:FRC
	$Ecd termlib	; make all

termcap.db:FRC
	$Ecd termcap.db	; make all

vi:	FRC
	$Ecd vi		; make all

FRC:
