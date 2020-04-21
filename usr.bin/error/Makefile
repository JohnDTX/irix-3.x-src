#
#	@(#)Makefile	1.4 (Berkeley) 5/4/82
#
#
#
include ${ROOT}/usr/include/make/commondefs

# @(#)$Header: /d2/3.7/src/usr.bin/error/RCS/Makefile,v 1.1 89/03/27 17:49:36 root Exp $
# $Log:	Makefile,v $
# Revision 1.1  89/03/27  17:49:36  root
# Initial check-in for 3.7
# 
# Revision 1.8  87/05/20  10:10:14  andre
# fixed yp libs to sun libs
# 
# Revision 1.7  87/05/04  10:43:08  andre
# 3.6 Product, New 5.3 Style Installs Added, Native Installs done
# 
# Revision 1.6  87/05/04  00:32:06  andre
# 3.6 Product, New 5.3 Style Installs Added, Native Installs done
# 
# Revision 1.4  86/08/27  14:39:53  paulm
# Add Yellow Pages libraries to load list to pick up YP
# versions of getpwent, getgrent and so forth.
# 
# Revision 1.3  86/03/07  10:58:24  glen
# minor fix
# 
# Revision 1.3  85/01/30  01:29:52  bob
# 1. Uses $(LDFLAGS).
# 2. Copies, rather than moves, during installation.
# 
# Revision 1.2  85/01/29  19:52:35  bob
# Tweeked.
# 
#
#	error.h			error header file
#	errorinput.c		does all of the input, and canonicalizes errors
#	errorpi.c		canonicalizes pi errors
#	errormain.c		main
#	errorsubr.c		does grotty subroutine work
#	errorfilter.c		"greps" out unwanted messages
#	errortouch.c		touches all mentioned files
#

HDRS = error.h
PHDRS = $(HDRS)
SRCS = errormain.c errorinput.c errorpi.c errorsubr.c errorfilter.c errortouch.c
PSRCS = $(SRCS)
OBJS = errormain.o errorinput.o errorpi.o errorsubr.o errorfilter.o errortouch.o

YPLIBS	= -lsun -lbsd -ldbm

DFLAGS = -DDEBUG

LINTFLAGS = $(DFLAGS)

all:	error

error:	$(OBJS)
	$(CCF) $(OBJS) -o $@ $(LDFLAGS) $(YPLIBS)

#$(OBJS): $(HDRS)

install:all
	${INSTALL} -u bin -g bin -m 755 -F /usr/bin \
	-idb "std.sw.unix mr" error

clean:
	rm -f error $(OBJS)

clobber:clean

lint:	
	lint $(LINTFLAGS) $(SRCS)

psrcs:
	echo $(PHDRS) $(PSRCS)

print:
	print $(PHDRS) $(PSRCS)
