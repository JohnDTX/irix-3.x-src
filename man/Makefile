# @(#)$Header: /d2/3.7/src/man/RCS/Makefile,v 1.1 89/03/27 16:19:32 root Exp $
# $Log:	Makefile,v $
# Revision 1.1  89/03/27  16:19:32  root
# Initial check-in for 3.7
# 
# Revision 1.1  86/10/04  15:31:36  kyle
# Initial revision
# 
# Revision 1.4  86/10/04  15:26:25  kyle
# Added print/troff option manuals
# 
# Revision 1.3  85/05/02  17:18:31  donl
# Just a little test...
# 
# Revision 1.2  85/03/21  18:51:41  bob
# Changed to create/chmod/chown dest dirs as needed.
# 
# Revision 1.1  85/01/28  16:00:57  bob
# Initial revision
# 

D	= $(DEST)/usr/man
P	= $(DEST)/../print

MANS	= $D/a_man $D/u_man $D/a_man/man1 $D/a_man/man7 $D/a_man/man8 \
	  $D/u_man/man1 $D/u_man/man2 $D/u_man/man3 $D/u_man/man4 \
	  $D/u_man/man5 $D/u_man/man6 \
	  $P/u_man $P/u_man/man1 $P/u_man/man4 $P/u_man/man5

CATS	= $D/a_man/cat1 $D/a_man/cat7 $D/a_man/cat8 \
    	  $D/u_man/cat1 $D/u_man/cat2 $D/u_man/cat3 $D/u_man/cat4 \
	  $D/u_man/cat5 $D/u_man/cat6 \
	  $P/u_man/cat1 $P/u_man/cat4 $P/u_man/cat5

all clean clobber:
	cd a_man		; make $@
	cd u_man		; make $@
	cd troff		; make $@

install: FRC $(MANS) $(CATS)
	chmod 755 $(MANS) ; chmod 777 $(CATS)
	chgrp bin $(MANS) $(CATS)
	chown bin $(MANS) $(CATS)
	cd a_man		; make $@
	cd u_man		; make $@
	cd troff		; make $@

a_man:	FRC
	cd a_man		; make all

u_man:	FRC
	cd u_man		; make all

FRC:

$(MANS) $(CATS):
	mkdir $@
