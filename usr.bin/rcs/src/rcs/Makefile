#$Header: /d2/3.7/src/usr.bin/rcs/src/rcs/RCS/Makefile,v 1.1 89/03/27 18:21:34 root Exp $

include ${ROOT}/usr/include/make/commondefs

# Copyright (C) 1982 by Walter F. Tichy
#                       Purdue University
#                       Computer Science Department
#                       West Lafayette, IN 47907
#
# All rights reserved. No part of this software may be sold or distributed
# in any form or by any means without the prior written permission of the
# author.
# Report problems and direct all inquiries to
# tichy@purdue, tichy@purdue-tcp, decvax!pur-ee!purdue!tichy


#               INSTRUCTIONS
#	= ===========
#
# 0.  Do the instructions in rcsbase.h
#
# 1.  Figure out where to put the modified diff and diff3 commands, and
#     install them. (Read the instructions in the Make-files).
#     Define the macros DIFF and DIFF3 accordingly (below).
DIFF	= /usr/lib/rdiff
DIFF3	= /usr/lib/rdiff3

# 2.  Implant the macro DIFF3 as modified by you into the file merge.
#     (That's a shell file.)

# 3.  Figure out where to put the RCS commands; define TARGETDIR
#     and merge accordingly.
TARGETDIR= /usr/bin
MERGE	= $(TARGETDIR)/merge

# 6.  RCS can maintain a log of the use of the co and ci commands. If you
#     want the logging, you must
#     a) define SNOOP (that's where the process that writes the log goes),
#     b) define SNOOPFILE (that's the file where SNOOP writes the log; it must
#        be owned by the same user that owns SNOOP),
#     c) include SNOOP and SNOOPFILE in the CFLAGS macro.
#     If you don't want the logging of command usage, omit the SNOOP and
#     SNOOPFILE definitions from CFLAGS.
#     The two variants of the CFLAG macros are given below.
SNOOPFILE= /usr/wft/RCSLOG/log
SNOOP	= /usr/wft/BIN/snoop
#CFLAGS2= -DCOMPAT2 -DDIFF='"$(DIFF)"' -DMERGE='"$(MERGE)"' \
#	  -DTARGETDIR='"$(TARGETDIR)"' -DSNOOPFILE='"$(SNOOPFILE)"' \
#	  -DSNOOP='"$(SNOOP)"'
#CFLAGS2= -DDIFF='"$(DIFF)"' -DMERGE='"$(MERGE)"' -DTARGETDIR='"$(TARGETDIR)"' \
#	  -DV4_2BSD
CFLAGS2	= -DDIFF='"$(DIFF)"' -DMERGE='"$(MERGE)"' -DTARGETDIR='"$(TARGETDIR)"' \
	  -DSYS5

# 7.  If you don't have release 2 RCS files around (e.g., if you don't have
#     an old release of RCS), remove the word -DCOMPAT2 from whatever
#     variant of CFLAGS you selected. -DCOMPAT2 assures that release 2
#     RCS files can still be read. (After all release 2 RCS files have
#     been updated with release 3 ci or rcs, you can remake RCS without
#     -DCOMPAT2.) Also, make sure that the co commands at the end of this
#     file are commented out; otherwise, Make will complain.
#
# 8.  Now you are ready. Try to make all. If all went well, make
#     INSTALLW if you want the logging of command usage, INSTALLNO otherwise.
#
# 9.  IMPORTANT! When you have RCS installed, rename old RCS files as
#     follows (if you have any):
#     If the working file was "f.c" and the RCS file "f.c.v", rename the
#     RCS file to "f.c,v". If the working file was "f.c" and the RCS file
#     "f.v", rename the RCS file "f.c,v". Thus, suffixes are no longer
#     dropped and RCS files end in ",v" rather than ".v".
#
# 10. If you want to maintain RCS with itself, be sure you preserve the
#     original revision numbers, dates, etc. This is done by checking the
#     files in with the -k option. When done, remove the comments from the
#     co-rules at the end of this file.

OBJECTS	= rcslex.o rcssyn.o rcsrev.o rcsgen.o rcssynTST.o rcsrevTST.o \
	  rcsedit.o rcsutil.o rcsfnms.o co.o ci.o ident.o rlog.o rcs.o \
	  rcsdiff.o rcsmerge.o rcskeep.o rcsfcmp.o snoop.o
RCSCOMMANDS= ci ident rcs rcsdiff rcsmerge rlog co
CI	= ci.o rcslex.o rcssyn.o rcsgen.o rcsedit.o rcsrev.o rcsutil.o \
	  rcsfnms.o rcskeep.o rcsfcmp.o
#
# The following used to be called 'CO', but that name is used by the
# SGI default rules as a macro name for the 'co' command.
#
COOBJS	= co.o rcslex.o rcssyn.o rcsgen.o rcsedit.o rcsrev.o rcsutil.o \
	  rcsfnms.o partime.o maketime.o
RLOG	= rlog.o rcslex.o rcssyn.o rcsrev.o rcsutil.o partime.o maketime.o \
	  rcsfnms.o
RCS	= rcs.o rcslex.o rcssyn.o rcsrev.o rcsutil.o rcsgen.o rcsedit.o \
	  rcsfnms.o
RCSDIFF	= rcsdiff.o rcsutil.o rcsfnms.o rcsrev.o rcssyn.o rcslex.o
RCSMERGE= rcsmerge.o rcsutil.o rcsfnms.o rcsrev.o rcssyn.o rcslex.o
PAIRTEST= rcsfnmsTST.o rcssyn.o rcslex.o
REVTEST	= rcsrevTST.o rcssyn.o rcslex.o
SYNTEST	= rcslex.o rcssynTST.o rcsrev.o

LLDLIBS	= -lsun -lbsd -ldbm

all:		merge $(RCSCOMMANDS)

INSTALLW:	INSTALLSNOOP INSTALLNO

install:	INSTALLNO

INSTALLNO:	merge $(RCSCOMMANDS)
		${INSTALL} -u bin -g bin -m 755 -F "${TARGETDIR}"  \
			-idb "upd root" "${RCSCOMMANDS}"

INSTALLSNOOP:	snoop
		${INSTALL} -u root -g sys -m 4755 -F "${SNOOP}"  \
			-idb "std.sw.unix mr" snoop
		chmod 4755 $(ROOT)$(SNOOP)
		chmod 644 $(ROOT)$(SNOOPFILE)
		chown root $(ROOT)$(SNOOP) $(ROOT)$(SNOOPFILE)
		chgrp sys $(ROOT)$(SNOOP) $(ROOT)$(SNOOPFILE)
# CAUTION: You may have to do a chown of SNOOP and SNOOPFILE.

clean:
		rm -f *.o

clobber:clean
		rm -f $(RCSCOMMANDS) snoop merge

depend:

$(OBJECTS):	rcsbase.h

merge:	merge.sh
		grep -v '^# ' merge.sh > merge
		chmod 755 merge

partime.o maketime.o co.o ci.o rlog.o rcsutil.o:	time.h

ci:		$(CI)
		$(CCF) -o $@ $(CI) $(LDFLAGS)

co:		$(COOBJS)
		$(CCF) -o $@ $(COOBJS) $(LDFLAGS)

rlog:		$(RLOG)
		$(CCF) -o $@ $(RLOG) $(LDFLAGS)

rcs:		$(RCS)
		$(CCF) -o $@ $(RCS) $(LDFLAGS)

rcsdiff:	$(RCSDIFF)
		$(CCF) -o $@ $(RCSDIFF) $(LDFLAGS)

rcsmerge:	$(RCSMERGE)
		$(CCF) -o $@ $(RCSMERGE) $(LDFLAGS)

pairtest:	$(PAIRTEST)
		$(CCF) -o $@ $(PAIRTEST) $(LDFLAGS)

rcsfnmsTST.o:   rcsfnms.c
		$(CCF) $(CFLAGS2) -c -DPAIRTEST rcsfnms.c
		mv rcsfnms.o rcsfnmsTST.o

revtest:	$(REVTEST)
		cc $(REVTEST)
		echo "a.out ../TEST/rev.test.v <../TEST/rev.test.in >&! tmp" \
		  | csh
		diff tmp ../TEST/rev.test.out
		rm -f tmp
		mv a.out revtest

rcsrevTST.o:	rcsrev.c
		$(CCF) $(CFLAGS2) -c -DREVTEST rcsrev.c
		mv rcsrev.o rcsrevTST.o

syntest:	$(SYNTEST)
		cc $(SYNTEST)
		echo "a.out ../TEST/syn.test >&! tmp" | csh
		diff tmp ../TEST/syn.result
		rm -f tmp
		mv a.out syntest

rcssynTST.o:	rcssyn.c
		$(CCF) $(CFLAGS2) -c -DSYNDB -DCOMPAT2 -DSYNTEST \
		  rcssyn.c
		mv rcssyn.o rcssynTST.o

lextest:	rcslex.c
		cc -DLEXDB rcslex.c
		echo "a.out ../TEST/lex.test >&! tmp" | csh
		diff tmp ../TEST/lex.result
		rm -f tmp
		mv a.out lextest

.c.o:
	$(CCF) $(CFLAGS2) -c $<

.c:
	$(CCF) $(CFLAGS2) $< -o $@ $(LDFLAGS)
