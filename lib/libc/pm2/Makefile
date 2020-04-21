#

#
#
include ${ROOT}/usr/include/make/commondefs

# Makefile for pm2 libc targets:
#	libc.a, libc_p.a, crt0.o, mcrt0.o
#

LIBOBJS= crt0.o mcrt0.o
OBJS=	 $(LIBOBJS)
TARGETS= libc.a libc_p.a $(OBJS)

# MAKEPATH for crt0.o and mcrt0.o
MAKEPATH=../m68k/csu

all: check_machine the_works

clobber clean:
	rm -rf objs pobjs ${TARGETS}

check_machine:
	@if test "$$machine"x != "iris"x; then \
		echo "machine != iris"; \
		exit 1; \
	fi

the_works: objs pobjs crt0.o mcrt0.o
	cd objs; make -f ../Make.gen PREF= M4PROF= CPROF=
	cd pobjs; make -f ../Make.gen PREF=_p M4PROF=-DPROF CPROF=-p

.s.o:
	/lib/cpp -P $< | m4 | ${AS} -i -o $*.o

objs:
	mkdir objs
pobjs:
	mkdir pobjs

install: all
	${INSTALL} -u bin -g bin -m 644 -F /lib  \
	-idb "std.sw.unix mr" libc.a
	ranlib  ${ROOT}/lib/libc.a
	${INSTALL} -u bin -g bin -m 644 -F /usr/lib  \
	-idb "std.sw.unix mr" libc_p.a
	ranlib  ${ROOT}/usr/lib/libc_p.a
	${INSTALL} -u bin -g bin -m 644 -F /lib  \
	-idb "std.sw.unix mr" "${LIBOBJS}"
