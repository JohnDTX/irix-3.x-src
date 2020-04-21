#ident	"$Header: /d2/3.7/src/bin/newsh/RCS/sgish.mk,v 1.1 89/03/27 14:55:04 root Exp $"

include	$(ROOT)/usr/include/make/commondefs

# Compile Time Options
#
#	-DNICE		if defined then a backround process will be
#			niced down to NICEVAL.  Note if defined, NICEVAL
#			must be defined.
#	-DNICEVAL=XX	The value to nice backround processes to
#	-DACCT		if defined, then shell accounting will be done.
#			Note that the rule line for service.o needs to
#			be modified if this is included.
#	-DTIME_OUT	if defined the shell will timeout and log you off
#			if no input is typed after TIMEOUT seconds. Currently
#			0, and not defined.
#

SRCS=	setbrk.c blok.c stak.c cmd.c fault.c main.c word.c string.c\
	name.c args.c xec.c service.c error.c io.c print.c macro.c expand.c\
	ctype.c msg.c test.c defs.c echo.c hash.c hashserv.c pwd.c func.c
OBJS=$(SRCS:.c=.o)

LCFLAGS = -DNICE -DNICEVAL=4

# We make this a pure executable
# NOTE	currently not supported by the hardware
#LLDFLAGS = -n

IDB_TAG = -idb "std.sw.unix mr"


default:sh

clean:
	rm -f $(OBJS) a.out

clobber:clean
	rm -f sh

FRC:

install: default
	$(INSTALL) -F /bin $(IDB_TAG) -o sh
	$(INSTALL) -o -ln /bin/sh -F /bin $(IDB_TAG) rsh

depend:
	$(MKDEPEND) sh.mk $(SRCS)

sh:	$(OBJS)
	$(CCF) $(OBJS) -o $@ $(LDFLAGS)

# If ACCT is defined then uncomment the next line
#	$(CCF) -I$(ROOT)/usr/src/cmd/acct -c service.c
