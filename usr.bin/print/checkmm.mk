#
#ident	"\%W\%"
#
# Notes:
#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	checkmm make file
#
# for DSL 2

#
# Common Definitions
#
include	$(ROOT)/usr/include/make/commondefs

#
# Compile Time Options
#
#
#LCFLAGS = -O $(FFLAG) $(B10)
LCFLAGS =
IFLAG = -n
#
# Local Definitions
#
OL = $(ROOT)/
INS = :
INSDIR = $(OL)usr/bin
B10 =
SOURCE = chekl.l  chekmain.c chekrout.c
FILES = chekl.o chekmain.o chekrout.o
SOURCE1 = chekl1.l chekmain1.c chekrout1.c
FILES1 = chekl1.o chekmain1.o chekrout1.o
MAKE = make

#
# Targets/Rules
#

default:    stamp all

clean:
	  rm -f $(FILES) $(FILES1)

clobber:  clean
	  rm -f checkmm checkmm1

FRC:

#
# Specific Target/Rules follow
#
install: default

stamp:
	pwd

compile all: checkmm1 checkmm
	:

checkmm1:	$(FILES1)
	 $(CC) -s $(B10) $(IFLAG) -o checkmm1 $(FILES1) -ll -lPW
checkmm:	$(FILES)
	$(CC) -s  $(B10) $(IFLAG) -o checkmm $(FILES) -ll -lPW
	$(INS) checkmm $(INSDIR)
	- $(CH) cd $(INSDIR); \
	chmod 755 checkmm; chgrp bin checkmm; chown bin checkmm
	$(INS) checkmm1 $(INSDIR)
	- $(CH) cd $(INSDIR); \
	chmod 755 checkmm1; chgrp bin checkmm1; chown bin checkmm1

#install:
#	$(MAKE) -f checkmm.mk INS=cp ROOT=$(ROOT) ch=$(CH)

