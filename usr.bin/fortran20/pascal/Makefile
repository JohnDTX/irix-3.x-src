#
#
include ${ROOT}/usr/include/make/commondefs
OBJS=pascal.o ../svslib.o

SRCS = p.body.1 p.body.2 p.body.3 p.body.4 p.body.5 p.decl.1 \
	   p.decl.2 p.decl.3 p.dump.1 p.dump.2 p.init p.misc.0 p.misc.1 \
	   p.misc.2 p.opt pascal.p

LPCDEFS = -Uident

all:	pascal
	
install:	all
	${INSTALL} -src pascal -u bin -g bin -m 755 -F /usr/lib \
		-idb "std.sw.unix mr" pascal20
	if test $$machine = juniper;\
	then \
		${INSTALL} -ln /usr/lib/pascal20 -F /usr/lib \
			-idb "std.sw.unix mr" pascal;\
	fi

pascal:	$(OBJS)
	$(PC) -o pascal $(PCFLAGS) $(OBJS)


../svslib.o:
	cd ../lib; $(MAKE) -$(MAKEFLAGS) svslib

pascal.o:	$(SRCS)

.p.o:
	$(PC) $(PCFLAGS) -c $<

clean:
	rm -f *.o *.obj *.j

clobber:	clean
	rm -f pascal
