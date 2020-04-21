#
#ident	"\%W\%"
#
# Notes:
#	makefile for workstation print directory
#

#
# Common Definitions
#
include	$(ROOT)/usr/include/make/commondefs

#
# Compile Time Options
#

#
# Local Definitions
#

#
# Targets/Rules
#
DIRS	= att lp.spool trscript

all default install clean:	locclean
	set DEST=${ROOT}; export DEST
	for i in $(DIRS); do (cd $$i;  $(MAKE) -f $$i.mk $@ ); done

clobber:	locclean
	for i in $(DIRS); do (cd $$i; $(MAKE) -f $$i.mk $@ ); done
	rm -f makedev makedev_host .B* .emacs_? .emacs_??

FRC:

#
# Specific Target/Rules follow
#

boot:
	cd att/cmd/text/roff.d/troff.d ; $(MAKE) -f troff.mk boot
	cd transcript ; . ./config ; cd src ; make boot

install:


locclean:
	
