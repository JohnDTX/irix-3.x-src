#
#
include ${ROOT}/usr/include/make/commondefs


.SUFFIXES: .o .p

SRCS = p.body.1 p.body.2 p.body.3 p.body.4 p.body.5 p.decl.1 \
	   p.decl.2 p.decl.3 p.dump.1 p.dump.2 p.init p.misc.0 \
	   p.misc.1 p.misc.2 p.opt pascal.p

OBJS = pascal.o ../svslib.o

all:	pascal10

install:	all
	${INSTALL} -u bin -g bin -m 755 -F /usr/lib \
	-idb "std.sw.unix mr" pascal10
	if test $$machine = iris;\
	then \
		${INSTALL} -ln /usr/lib/pascal10 -F /usr/lib \
		-idb "std.sw.unix mr" pascal ;\
	fi

pascal10:	$(OBJS)
	$(PC) $(PCFLAGS) -o pascal10 $(OBJS)

pascal.o: $(SRCS)

.p.o:
	$(PC) $(PCFLAGS) -c $<

../svslib.o:
	cd ..; $(MAKE) svslib

clean:
	rm -f pascal.o *.obj *.j

clobber:	clean
	rm -f pascal10
