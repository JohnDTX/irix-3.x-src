#
#ident	"\%W\%"
#
# Notes:

#
# Specific Target/Rules follow
#
# transcript/src/Makefile.sysv
#
# Copyright (C) 1985 Adobe Systems Incorporated
#
# RCSID: $Header: /d2/3.7/src/usr.bin/print/trscript/src/RCS/src.mk,v 1.1 89/03/27 18:20:59 root Exp $
#

#
# Common Definitions
#
include	$(ROOT)/usr/include/make/commondefs

#

# Compile Time Options
#
#LIBS = -lyp -lrpc -lbsd -ldbm

HCC=	ROOTDIR=/usr /bin/cc
LLDFLGS = -lyp -lrpc -lbsd -ldbm

LLDLIBS = -lsun -lbsd

LCFLAGS = -DSYSV
# Local Definitions
#
I_FLAGS	=-idb "trans.sw.trans"
# configuration files that should be discarded if user has one already
NOUPDATE=-idb "trans.sw.trans config(noupdate)"
# configuration files that should override, but save, existing version
UPDATE	=-idb "trans.sw.trans config(update)"
SU_FLAGS=-m 04555 -u root -g sys $(I_FLAGS)
GU_FLAGS=-m 755 -u bin -g bin $(UPDATE)
DAT_FLAGS=-m 644 -u bin -g bin $(UPDATE)

LPROGS = map psbanner pscomm psrv pstext
#BPROGS = enscript ps4014 ps630 pscat pscatmap psdit psplot psrev
BPROGS = enscript ps4014 ps630 psdit psplot psrev pscat pscatmap

OBJECTS = map.o psbanner.o pscomm.o psrv.o pstext.o \
	enscript.o ps4014.o ps630.o pscat.o pscatmap.o \
	psdit.o psplot.o psrev.o \
	mapname.o psutil.o


#

HCMDS= map_host

#
# Targets/Rules
#

default programs all:	${LPROGS} ${BPROGS} ${HCMDS}

clean clobber:
	rm -f - *.o config.c ${BPROGS} ${LPROGS} *BAK *CKP .emacs_[0-9]*
	rm -f map_host
FRC:


boot:	map
	cp map $(DEST)/usr/bin

enscript: enscript.o psutil.o mapname.o config.o
	$(CC) ${CFLAGS}  enscript.o psutil.o mapname.o config.o $(LDFLAGS) -o enscript

pscat: pscat.o psutil.o config.o 
	$(CC) ${CFLAGS}  pscat.o psutil.o config.o $(LDFLAGS) -o pscat

psbanner: psbanner.o psutil.o config.o
	$(CC) ${CFLAGS}  psbanner.o psutil.o config.o $(LDFLAGS) -o psbanner

pscomm: pscomm.o psutil.o config.o
	$(CC) ${CFLAGS}  pscomm.o psutil.o config.o $(LDFLAGS) -o pscomm

pstext: pstext.o psutil.o config.o
	$(CC) ${CFLAGS}  pstext.o psutil.o config.o $(LDFLAGS) -o pstext

psplot: psplot.o psutil.o config.o
	$(CC) ${CFLAGS}  psplot.o psutil.o config.o $(LDFLAGS) -o psplot

psrv: psrv.o psutil.o config.o
	$(CC) ${CFLAGS}  psrv.o psutil.o config.o $(LDFLAGS) -o psrv

psrev: psrev.o psutil.o config.o
	$(CC) ${CFLAGS}  psrev.o psutil.o config.o $(LDFLAGS) -o psrev

ps630: ps630.o psutil.o config.o
	$(CC) ${CFLAGS}  ps630.o psutil.o config.o $(LDFLAGS) -o ps630

ps4014: ps4014.o psutil.o config.o
	$(CC) ${CFLAGS}  ps4014.o psutil.o config.o $(LDFLAGS) -o ps4014

pscatmap: pscatmap.o psutil.o config.o mapname.o
	$(CC) ${CFLAGS}  pscatmap.o psutil.o config.o mapname.o $(LDFLAGS) -o pscatmap

psdit: psdit.o psutil.o config.o
	$(CC) ${CFLAGS}  psdit.o psutil.o config.o  $(LDFLAGS) -o psdit

map: map.o mapname.o psutil.o config.o
	$(CC) ${CFLAGS}  map.o mapname.o psutil.o config.o $(LDFLAGS) -o map

${OBJECTS}: transcript.h
pscomm.o pscat.o psrv.o: psspool.h
pscat.o pscatmap.o: action.h 
psdit.o: dev.h

config.c: config.proto ../config
	-rm -f - config.c
	sed	-e s,PSLIBDIR,$$PSLIBDIR,g \
		-e s,TROFFFONTDIR,$$TROFFFONTDIR,g \
		-e s,DITDIR,$$DITDIR,g \
		-e s,PSTEMPDIR,$$PSTEMPDIR,g \
		config.proto >config.c

install:    default
	$(INSTALL) $(I_FLAGS) -F $$BINDIR  "$(BPROGS)"
	$(INSTALL) $(I_FLAGS) -F $$PSLIBDEST  "$(LPROGS)"
	for i in $(BPROGS); do $(INSTALL) $(I_FLAGS) -ln $$BINDIR/$$i -F $$PSLIBDEST $$i ; done

reset:
# delete the target-machine versions of these files so we can build them
# for the host machine.
	touch map.c mapname.c psutil.c config.c

# This is a program that has to help with the build, so has to run on the host.
map_host:
	if [ -n "$$HOSTENV" ] ; \
	then \
	$(HCC) -c -Dsgi -I/usr/include -DBSD map.c ; \
	$(HCC) -c -Dsgi -I/usr/include -DBSD mapname.c ; \
	$(HCC) -c -Dsgi -I/usr/include -DBSD psutil.c ; \
	$(HCC) -c -Dsgi -I/usr/include -DBSD config.c ; \
	$(HCC) -DBSD  map.o mapname.o psutil.o \
	     config.o $(LDFLAGS) -o map_host; \
	else \
	$(CC) -c ${CFLAGS}  map.c ; \
	$(CC) -c ${CFLAGS}  mapname.c ; \
	$(CC) -c ${CFLAGS}  psutil.c ; \
	$(CC) -c ${CFLAGS}  config.c ; \
	$(CC) ${CFLAGS}  map.o mapname.o psutil.o \
	    config.o $(LDFLAGS) -o map_host; \
	fi
