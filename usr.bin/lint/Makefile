#
# @(#)$Header: /d2/3.7/src/usr.bin/lint/RCS/Makefile,v 1.1 89/03/27 18:03:54 root Exp $

include ${ROOT}/usr/include/make/commondefs

pass1	= /usr/lib/lint/lint1
IDBTAG  = -idb "std.sw.unix"
GRPOWN  = -g bin -u bin
INST	= ${INSTALL} ${IDBTAG} ${GRPOWN}

M	= ../../lib/ccom
LOAD1	= 
LOAD2	= 
DEFINES	= -DLINT -DIEEE -DHOST_FULL_IEEE -DDOUBLES32BITS -DNATIVE \
	  -DSGI_REGS -DSGI_FIELDS -DSTABDOT -DFLEXNAMES  -DOLD_SGI_DEFAULT
CFLAGS2	= $(DEFINES) -I$M -I.
LINTLIBS= llib-port.ln llib-lc.ln llib-lm.ln llib-lcurses.ln llib-lm.ln

LIFLAGS= -D$C -DSYS5

TARGETS=	lpass1 lpass2 $(LINTLIBS) llib-l$(GLHDWR).ln

all:	$(TARGETS)

#boot:	lpass1
#	cp lpass1 $(ROOT)/usr/lib/lint/lint1

.c.o:
	$(CCF) -c $(CFLAGS2) $<

lpass1:	cgram.o xdefs.o scan.o common.o pftn.o trees.o optim.o lint.o \
	  hash.o sgi.o
	$(CCF) cgram.o xdefs.o scan.o common.o pftn.o trees.o optim.o \
	  lint.o hash.o sgi.o -o $@ $(LDFLAGS)

trees.o:$M/manifest.h $M/macdefs.h $M/mfile1.h $M/trees.c
	$(CCF) -c $(CFLAGS2) -ULINT $M/trees.c

optim.o:$M/manifest.h $M/macdefs.h $M/mfile1.h $M/optim.c
	$(CCF) -c $(CFLAGS2) $M/optim.c

pftn.o:	$M/manifest.h $M/macdefs.h $M/mfile1.h $M/pftn.c
	$(CCF) -c $(CFLAGS2) $M/pftn.c

lint.o:	$M/manifest.h $M/macdefs.h $M/mfile1.h lmanifest.h
	$(CCF) -c $(CFLAGS2) lint.c

scan.o:	$M/manifest.h $M/macdefs.h $M/mfile1.h $M/scan.c
	$(CCF) -c $(CFLAGS2) $M/scan.c

xdefs.o:$M/manifest.h $M/mfile1.h $M/macdefs.h $M/xdefs.c
	$(CCF) -c $(CFLAGS2) $M/xdefs.c

cgram.o:$M/manifest.h $M/mfile1.h $M/macdefs.h cgram.c
	$(CCF) -c $(CFLAGS2) cgram.c

cgram.c:$M/cgram.y
	$(YACC) $(YFLAGS) $M/cgram.y
	mv y.tab.c cgram.c

common.o:$M/manifest.h $M/mfile1.h $M/common.c $M/macdefs.h 
	$(CCF) -c $(CFLAGS2) $M/common.c

llib-port.ln:llib-port lpass1
	(/lib/cpp -I${ROOT}/usr/include \
		  -C -Dlint llib-port | $(pass1) -puv > llib-port.ln)

llib-lm.ln:llib-lm lpass1
	(/lib/cpp -I${ROOT}/usr/include \
		  -C -Dlint llib-lm | $(pass1) -puv > llib-lm.ln)

#llib-lmp.ln:llib-lmp lpass1
#	(/lib/cpp -I${ROOT}/usr/include \
		  -C -Dlint llib-lmp | $(pass1) -puv > llib-lmp.ln)

llib-lc.ln:llib-lc lpass1
	(/lib/cpp -I${ROOT}/usr/include \
		  -C -Dlint llib-lc | $(pass1) -v > llib-lc.ln)

llib-lcurses.ln:llib-lcurses lpass1
	(/lib/cpp -I${ROOT}/usr/include \
		  -C -Dlint llib-lcurses | $(pass1) -v > llib-lcurses.ln)

llib-lgl1.ln:llib-lgl1 lpass1
	(/lib/cpp -I${ROOT}/usr/include \
		  -C -Dlint $(LIFLAGS) llib-lgl1 | $(pass1) -v > llib-lgl1.ln)

llib-lgl2.ln:llib-lgl2 lpass1
	(/lib/cpp -I${ROOT}/usr/include \
		  -C -Dlint $(LIFLAGS) llib-lgl2 | $(pass1) -v > llib-lgl2.ln)

lpass2:	lpass2.o hash.o
	$(CCF) lpass2.o hash.o -o lpass2

lpass2.o:$M/manifest.h lmanifest.h
	$(CCF) $(CFLAGS2) -c -I$M -I. lpass2.c
	
lintall:
	lint -hpv -I. -I$M  $M/cgram.c $M/xdefs.c $M/scan.c \
	  $M/pftn.c $M/trees.c $M/optim.c lint.c


L	= /usr/lib/lint
install:all SHELL 
	${INST} -m 644 -F $L "`echo $(LINTLIBS) | sed 's:\.ln::g'` \
			llib-l$(GLHDWR)"
	${INST} -m 644 -F $L "${LINTLIBS} llib-l${GLHDWR}.ln"
	${INST} -ln ${L}/llib-l$(GLHDWR) -F $L llib-lgl
	${INST} -ln ${L}/llib-l$(GLHDWR).ln  -F $L llib-lgl.ln
	${INST} -ln ${L}/llib-lm -F $L llib-lmsky
	${INST} -ln ${L}/llib-lm.ln -F $L llib-lmsky.ln
	${INST} -m 755 -src SHELL -F /usr/bin lint
	${INST} -m 755 -src lpass1 -F $L lint1
	${INST} -m 755 -src lpass1 -F $L lint2

shrink:
	rm -f *.o 

clobber clean:shrink
	rm -f cgram.c $(TARGETS)
