# build Sendmail databases for the IRIS
#
# "$Header: /d2/3.7/src/bsd/usr.lib/sendmail/lib/RCS/Makefile,v 1.1 89/03/27 15:02:34 root Exp $"


include ${ROOT}/usr/include/make/commondefs
IDBTAG=-idb "std.sw.unix"
FILES = sendmail.hf aliases sendmail.cf

default all clean clobber:

install: aliases sendmail.cf sendmail.hf
	${INSTALL} $(IDBTAG) -m 644 -F /usr/lib "${FILES}"
