# Makefile for bin/sh
# $Header: /d2/3.7/src/bin/sh/RCS/Makefile,v 1.1 89/03/27 14:55:26 root Exp $
#
# -DRESTRICTED_SH to enable "restricted" shells (which breaks sh currently)
#

include	$(ROOT)/usr/include/make/commondefs
IDB_TAG = -idb "root upd"

CFILES	= setbrk.c builtin.c blok.c stak.c cmd.c \
	fault.c main.c word.c string.c name.c args.c \
	xec.c service.c error.c io.c print.c macro.c \
	expand.c ctype.c msg.c test.c defs.c

TARGETS = sh

all default:	${TARGETS}

install: default
	$(INSTALL) -F /bin $(IDB_TAG) -o "${TARGETS}"

include	$(ROOT)/usr/include/make/commonrules

sh:	$(OBJECTS)
	$(CCF) -o $@ $(OBJECTS) $(LDFLAGS) 
