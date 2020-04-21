AR	= ar
#
include ${ROOT}/usr/include/make/commondefs

OBJ	= auxf.o chkid.o chksid.o cmrcheck.o date_ab.o date_ba.o del_ab.o \
	del_ba.o deltack.o dodelt.o dofile.o dohist.o doie.o dolist.o \
	eqsid.o error.o filehand.o flushto.o fmalloc.o fmterr.o getline.o gf.o \
	logname.o newstats.o permiss.o pf_ab.o putline.o rdmod.o setup.o \
	sid_ab.o sid_ba.o sidtoser.o sinit.o stats_ab.o xcreat.o xmsg.o

all:	comobj.a

install:all

clean:
	rm -f *.o

clobber:clean
	rm -f comobj.a

comobj.a:$(OBJ)
	rm -f $@
	$(AR) qv $@ $(OBJ)
	ranlib $@

.PRECIOUS:comobj.a

.c.o:
	$(CCF) -I../hdr -c $<
