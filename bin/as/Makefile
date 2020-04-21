#
#
include ${ROOT}/usr/include/make/commondefs

# Define VV if this is for a system V
# Define XAS if this is for a cross assembler, running on a vax

SRCS	= error.c init.c ins.c list.c ps.c rel.c \
	  scan.c sdi.c sym.c stab.c
HDRS	= inst.h mical.h scan.h

OBJECTS	= error.o init.o ins.o ps.o rel.o sdi.o sym.o scan.o list.o stab.o
NFLAG=	-n

all:	as10

install:all
	strip as10
	if test $(GLHDWR) = gl1 ; then \
		${INSTALL} -u bin -g bin -m 755 -F /bin \
			-idb "root upd" as10 ;\
		${INSTALL} -ln ${NB}/bin/as10 -g bin -u bin -m 755 \
			-idb "root upd" as ;\
	else \
		${INSTALL} -src as10 -u bin -g bin -m 755 -F /bin \
			-idb "root upd" oas; \
	fi

clean:
	rm -f *.o as10

clobber:clean
	rm -f as10

as10:	$(OBJECTS)
	$(CCF) $(NFLAG) -o $@ $(OBJECTS) $(LDFLAGS)

.c.o:
	$(CCF) $(DEFS) -DVV -Dm68010 -c $<

ins.o:	ins.c mical.h inst.h
rel.o:	rel.c mical.h
init.o:	init.c mical.h inst.h
sym.o:	sym.c mical.h
list.o:	list.c mical.h
ps.o:	ps.c mical.h
sdi.o:	sdi.c mical.h
scan.o:	scan.c mical.h
error.o:error.c mical.h
