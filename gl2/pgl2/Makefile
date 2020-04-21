# the next line must have nothing after the  = sign

include ${ROOT}/usr/include/make/commondefs

NULL=

DOTH =	\
	pdevice.h \
	pglconsts.h \
	pglmacs.h \
	pglprocs.h \
	pgltypes.h \
	$(NULL)

INCS = $(DOTH)

LIBPRIM = ../gl2/lib.prim
MUTABLES = mandefs.p pglprocs.h pglmacs.h

.SUFFIXES:
.SUFFIXES: .c .o .p .s

.p.o:
	pc -c $*.p

.s.o:
	pc -c $*.s

all: p2cstr.o $(MUTABLES)
#all: $(MUTABLES)

p2cstr.o:	p2cstr.s
	cc -c p2cstr.s

# the awk script that creates pglprocs.h and pglmacs.h
pglprocs.h pglmacs.h mandefs.p: plibgen.awk $(LIBPRIM)
	awk -f plibgen.awk < ../gl2/lib.prim

install: all
	${INSTALL} -g bin -u bin -m 755 -idb "pas" \
		-F /usr/lib p2cstr.o
	${INSTALL} -g bin -u bin -m 444 -idb "pas" \
		-F /usr/include/gl2 "${INCS}"
	cd ${ROOT}/usr/include/gl2 ; \
		for i in $(INCS) ; do rm -f ../$$i ; ln $$i ../$$i ; done 

clobber clean:
	rm -f p2cstr.o $(MUTABLES)

