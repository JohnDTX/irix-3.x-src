#	@(#)Makefile	4.4	(Berkeley)	83/06/22
#
# $Source: /d2/3.7/src/usr.bin/refer/RCS/Makefile,v $
# @(#)$Revision: 1.1 $
# $Date: 89/03/27 18:22:14 $

include ${ROOT}/usr/include/make/commondefs
SHELL	= /bin/sh


PROGS	= addbib hunt inv lookbib mkey refer sortbib roffbib indxbib

all:	$(PROGS)

mkey:	mkey1.o mkey2.o mkey3.o deliv2.o
	$(CC) $(LDFLAGS) mkey?.o deliv2.o -o $@

inv:	inv1.o inv2.o inv3.o inv5.o inv6.o deliv2.o
	$(CC) $(LDFLAGS) inv?.o deliv2.o -o $@

hunt:	hunt1.o hunt2.o hunt3.o hunt5.o hunt6.o hunt7.o glue5.o \
	  refer3.o hunt9.o shell.o deliv2.o hunt8.o glue4.o tick.o
	$(CC) $(LDFLAGS) hunt?.o refer3.o glue5.o glue4.o shell.o deliv2.o \
	  tick.o -o $@

glue3.o: refer..c glue3.c
hunt2.o: refer..c hunt2.c
hunt3.o: refer..c hunt3.c
refer0.o:refer..c refer0.c
refer1.o:refer..c refer1.c
refer2.o:refer..c refer2.c
refer3.o:refer..c refer3.c
refer4.o:refer..c refer4.c
refer6.o:refer..c refer6.c
refer5.o:refer..c refer5.c

refer:	glue1.o refer1.o refer2.o refer4.o refer5.o refer6.o mkey3.o \
	  refer7.o refer8.o hunt2.o hunt3.o deliv2.o hunt5.o hunt6.o hunt8.o \
	  glue3.o hunt7.o hunt9.o glue2.o glue4.o glue5.o refer0.o shell.o
	$(CC) $(LDFLAGS) $(NFLAG) glue?.o refer[01245678].o hunt[2356789].o \
	  mkey3.o shell.o deliv2.o -o $@

install:all
	${INSTALL} -u bin -g bin -F /usr/lib/refer  \
	-idb "std.sw.unix mr" "mkey inv hunt"
	${INSTALL} -u bin -g bin -F /usr/bin \
	-idb "std.sw.unix mr" "refer addbib indxbib roffbib sortbib lookbib "
	${INSTALL} -u bin -g bin -m 644 -F /usr/lib/tmac  \
	-idb "std.sw.unix mr" tmac.bib
clean:
	rm -f *.o

clobber:clean
	rm -f $(PROGS)

whatabout:what1.o what2.o what3.o what4.o shell.o mkey3.o
	$(CC) $(LDFLAGS) what?.o shell.o mkey3.o -o $@

deliv:	deliv1.o deliv2.o
	$(CC) $(LDFLAGS) deliv?.o -o $@

refpart:refer0.o refer1.o refer2.o refer3.o refer4.o refer5.o \
	  refer6.o refer7.o refer8.o deliv2.o glue4.o
	$(CC) $(LDFLAGS) refer?.o deliv2.o glue4.o -o $@
