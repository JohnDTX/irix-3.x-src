#	makefile for workstation print directory
#ident	"\%W\%"
#
# Notes:
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
DIRS	= text
#
# Targets/Rules
#

default:    all

clean: FRC
	for i in $(DIRS); do (cd $$i; $(MAKE) -f $$i.mk $@ ); done

clobber:FRC
	for i in $(DIRS); do (cd $$i; $(MAKE) -f $$i.mk $@ ); done

FRC:

#
# Specific Target/Rules follow
#
install: 
	for i in $(DIRS); do (cd $$i; $(MAKE) -f $$i.mk $@ ); done


all:	
	for i in $(DIRS); do (cd $$i; $(MAKE) -f $$i.mk $@ ); done


