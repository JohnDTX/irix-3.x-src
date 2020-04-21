

#
#
include ${ROOT}/usr/include/make/commondefs
#	text Development Support Library (DSL) tbl make file
#	SCCS: @(#)tbl.mk	1.12
# $Source: /d2/3.7/src/usr.bin/text/tbl.d/RCS/Makefile,v $
# @(#)$Revision: 1.1 $
# $Date: 89/03/27 18:29:15 $


SFILES = t..c t[0-9].c t[bcefgimrstuv].c
OFILES = t0.o t1.o t2.o t3.o t4.o t5.o t6.o t7.o t8.o t9.o tb.o tc.o \
	te.o tf.o tg.o ti.o tm.o tr.o ts.o tt.o tu.o tv.o

all:	tbl

install:all
	${INSTALL} -u bin -g bin -m 755 -F /usr/bin  \
	-idb "std.sw.unix mr" tbl

tbl:	$(OFILES) 
	$(CC) $(LDFLAGS) -o $@ $(OFILES)

clean:
	rm -f *.o

clobber: clean
	rm -f tbl
