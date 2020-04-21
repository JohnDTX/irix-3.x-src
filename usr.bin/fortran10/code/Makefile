#
#
include ${ROOT}/usr/include/make/commondefs

.SUFFIXES: .o .p

OBJS = code10.o ../svslib.o
SRCS = code10.p h.code.1 h.code.2 h.code.3 h.code.4 h.code.5\
	   h.code.6 h.code.7 m.code.8 h.code.9 h.code.max

all:	code10

install:	all
	${INSTALL} -u bin -g bin -m 755 -F /usr/lib \
	-idb "std.sw.unix mr" code10
	if test $$machine = iris;\
	then \
		${INSTALL} -ln /usr/lib/code10 -F /usr/lib \
		-idb "std.sw.unix mr" code ;\
	fi

code10:	$(OBJS)
	$(PC) $(PCFLAGS) -o code10 $(OBJS)

code10.o: $(SRCS)

.p.o:
	$(PC) $(PCFLAGS) -c $<

../svslib.o:
	cd ..; $(MAKE) svslib

clean:
	rm -f code10.o *.obj *.j

clobber:	clean
	rm -f code10
