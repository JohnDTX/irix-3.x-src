# Makefile for sccs
#
include ${ROOT}/usr/include/make/commondefs
# @(#)$Header: /d2/3.7/src/usr.bin/sccs/RCS/Makefile,v 1.1 89/03/27 18:23:35 root Exp $
# $Log:	Makefile,v $
# Revision 1.1  89/03/27  18:23:35  root
# Initial check-in for 3.7
# 
# Revision 1.8  87/08/25  22:26:48  andre
# use commondefs to cleanup
# 
# Revision 1.7  87/05/29  16:45:59  andre
# fixed minor install bug
# 
# Revision 1.6  87/05/29  16:26:52  andre
# fixed libraries
# 
# Revision 1.5  87/05/04  10:51:17  andre
# 3.6 Product, New 5.3 Style Installs Added, Native Installs done
# 
# Revision 1.4  87/05/04  00:36:13  andre
# 3.6 Product, New 5.3 Style Installs Added, Native Installs done
# 
# Revision 1.2  86/08/27  15:53:09  paulm
# Add Yellow Pages libraries to the load list to pick
# up YP versions of getpwent, getgrent and so forth.
# 
# Revision 1.1  86/08/27  15:50:49  paulm
# Initial revision
# 
# Revision 1.5  85/02/25  17:46:21  bob
# Changed to deal with rmchg.c being moved to rmdel.c since there shouldn't be
# any rmchg!
# 
SRCDIR	= src
HDRDIR	= hdr
LIBDIR	= lib
HELPDIR	= help.d
HELPSTUFF = ad bd cb cm cmds co de default ge he prs rc un ut vc

#	Public directory where help files are stored.
PUB_HELPLIB=/usr/lib/help
LIBES	= $(LIBDIR)/comobj.a /usr/lib/libPW.a
LIBS	= $(LIBES) -lsun -lbsd -ldbm

PROGS	= prs help cdc comb delta get bdiff admin rmdel sact sccsdiff unget \
	  val vc what

all:	prs help cdc comb delta get \
	  bdiff admin rmdel sact sccsdiff \
	  unget val vc what

admin:	$(SRCDIR)/admin.c $(LIBES)
	$(CCF) -I$(HDRDIR) $(LDFLAGS) -o $@ $(SRCDIR)/admin.c $(LIBS)

bdiff:	$(SRCDIR)/bdiff.c $(LIBES)
	$(CCF) -I$(HDRDIR) $(LDFLAGS) -o $@ $(SRCDIR)/bdiff.c $(LIBS)

cdc:	rmdel
	rm -f $@
	ln $? $@

comb:	$(SRCDIR)/comb.c $(LIBES)
	$(CCF) -I$(HDRDIR) $(LDFLAGS) -o $@ $(SRCDIR)/comb.c $(LIBS)

delta:	$(SRCDIR)/delta.c $(LIBES)
	$(CCF) -I$(HDRDIR) $(LDFLAGS) -o $@ $(SRCDIR)/delta.c $(LIBS)

get:	$(SRCDIR)/get.c $(LIBES)
	$(CCF) -I$(HDRDIR) $(LDFLAGS) -o $@ $(SRCDIR)/get.c $(LIBS)

help:	$(SRCDIR)/help.c $(LIBES)
	$(CCF) -I$(HDRDIR) $(LDFLAGS) -o $@ $(SRCDIR)/help.c $(LIBS)

prs:	$(SRCDIR)/prs.c $(LIBES)
	$(CCF) -I$(HDRDIR) $(LDFLAGS) -o $@ $(SRCDIR)/prs.c $(LIBS)

rmdel:	$(SRCDIR)/rmdel.c $(LIBES)
	$(CCF) -I$(HDRDIR) $(LDFLAGS) -o $@ $(SRCDIR)/rmdel.c $(LIBS)

sact:	unget
	rm -f $@
	ln unget $@

sccsdiff:$(SRCDIR)/sccsdiff.sh
	cp $(SRCDIR)/sccsdiff.sh $@ ; chmod 755 $@

unget:	$(SRCDIR)/unget.c $(LIBES)
	$(CCF) -I$(HDRDIR) $(LDFLAGS) -o $@ $(SRCDIR)/unget.c $(LIBS)

val:	$(SRCDIR)/val.c $(LIBES)
	$(CCF) -I$(HDRDIR) $(LDFLAGS) -o $@ $(SRCDIR)/val.c $(LIBS)

vc:	$(SRCDIR)/vc.c $(LIBES)
	$(CCF) -I$(HDRDIR) $(LDFLAGS) -o $@ $(SRCDIR)/vc.c $(LIBS)

what:	$(SRCDIR)/what.c $(LIBES)
	$(CCF) -I$(HDRDIR) $(LDFLAGS) -o $@ $(SRCDIR)/what.c $(LIBS)

$(LIBDIR)/comobj.a:$(LIBDIR)/*.c
	cd $(LIBDIR) ; $(MAKE) comobj.a

$(PUB_HELPLIB):
	${INSTALL} -dir -u bin -g bin -m 755 -F /usr/bin  \
	-idb "std.sw.unix mr" "${PUB_HELPDIR}"

install:all $(PUB_HELPLIB)
	${INSTALL} -u bin -g bin -m 755 -F /usr/bin  \
	-idb "std.sw.unix mr" "${PROGS}"
	${INSTALL} -ln /usr/bin/rmdel -F /usr/bin  \
	-idb "std.sw.unix mr" cdc 
	${INSTALL} -ln /usr/bin/unget -F /usr/bin  \
	-idb "std.sw.unix mr" sact 
	cd help.d ; ${INSTALL} -u bin -g bin -m 755 -F ${PUB_HELPLIB} \
			-idb "std.sw.unix mr" "${HELPSTUFF}"

clean:
	rm -f *.o
	cd $(LIBDIR) ; $(MAKE) $@

clobber:clean
	rm -f $(PROGS)
	cd $(LIBDIR) ; $(MAKE) $@
