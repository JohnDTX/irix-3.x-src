# make IP2 version of libbsd*.a
# "$Header: /d2/3.7/src/bsd/usr.lib/libbsd/pm2/RCS/Makefile,v 1.1 89/03/27 15:02:17 root Exp $"

include ${ROOT}/usr/include/make/commondefs

TARGETS= libbsd.a libbsd_p.a

default all: $(TARGETS)

clean:
	rm -rf objs pobjs
clobber:clean
	rm -rf ${TARGETS}

libbsd.a:objs _force
	cd objs; make -f ../Make.gen PREF= M4PROF= CPROF=

libbsd_p.a:pobjs _force
	cd pobjs; make -f ../Make.gen PREF=_p M4PROF=-DPROF CPROF=-p

objs pobjs:
	mkdir $@

install: all
	${INSTALL} -u bin -g bin -m 644 -F /usr/lib \
		-idb "std.sw.cc" "$(TARGETS)"
	cd ${ROOT}/usr/lib; ranlib -t libbsd.a libbsd_p.a

_force:
