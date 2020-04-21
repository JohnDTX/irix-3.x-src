#
#
include ${ROOT}/usr/include/make/commondefs

OBJS =	jcode.o ../svslib.o
SRCS = 	j.code.1 j.code.2a j.code.2b j.code.3 j.code.4 j.code.5 \
	j.code.6 j.code.7 j.code.8 j.code.9 j.code.flt j.code.jun \
	j.code.max jcode.p

all:	jcode

install:	all
	${INSTALL} -src jcode -u bin -g bin -m 755 -F /usr/lib \
		-idb "std.sw.unix" code20
	if test $$machine = juniper;\
	then\
		${INSTALL} -ln /usr/lib/code20 -F /usr/lib \
			-idb "std.sw.unix" code; \
	fi

jcode:	$(OBJS)
	$(PC) $(PCFLAGS) -o jcode $(OBJS)

jcode.o:	$(SRCS)

.p.o:
	$(PC) $(PCFLAGS) -c $<

../svslib.o:
	cd ../lib; $(MAKE) -$(MAKEFLAGS) svslib

clean:
	rm -f *.o *.obj *.j

clobber:	clean
	rm -f jcode

