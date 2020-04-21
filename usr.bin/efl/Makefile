# makefile for usr.bin/efl

#
#
include ${ROOT}/usr/include/make/commondefs

# $Source: /d2/3.7/src/usr.bin/efl/RCS/Makefile,v $
# @(#)$Revision: 1.1 $
# $Date: 89/03/27 17:48:52 $


FILES1	= makefile efl.1 defs tokens main.c init.c tailor.c \
	  gram.head gram.dcl gram.expr gram.exec \
	  lex.l fixuplex symtab.c \
	  dcl.c addr.c struct.c field.c misc.c alloc.c mk.c
FILES2	= exec.c blklab.c simple.c print.c temp.c io.c error.c \
	  pass2.c icfile.c free.c dclgen.c namgen.c
FILES	= $(FILES1) $(FILES2)
P	= detab 0 6 12 18 24 30 36 42 48 54 60 66 72 | und -3 | opr -r2

OBJECTS	= main.o init.o tailor.o lex.o symtab.o \
	  dcl.o addr.o struct.o field.o blklab.o \
	  misc.o alloc.o print.o simple.o exec.o temp.o error.o \
	  pass2.o icfile.o free.o dclgen.o namgen.o mk.o io.o gram.o

all:	efl

install:all
	${INSTALL} -u bin -g bin -m 755 -F /bin \
	-idb "std.sw.unix mr" ef1

efl:	$(OBJECTS)
	$(CCF) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CCF) $(CFLAGS) -Dunix -c $<

$(OBJECTS):defs
lex.o init.o:tokdefs

# gram.c can no longer be made on a pdp11 because of yacc limits
gram.c:	gram.head gram.dcl gram.expr gram.exec tokdefs
	( sed < tokdefs "s/#define/%token/" ; \
	cat gram.head gram.dcl gram.expr gram.exec) > gram.in
	$(YACC) $(YFLAGS) gram.in
	(echo "#define YYDEBUG"; cat y.tab.c) > gram.c
	rm -f gram.in y.tab.c

lex.c:	fixuplex
	lex lex.l
	fixuplex
	mv lex.yy.c lex.c

tokdefs:tokens
	rm -f $@
	grep -n . < tokens | sed "s/\([^:	]*\):\(.*\)/#define \2 \1/" > tokdefs

clean:	
	rm -f *.o gram.c lex.c tokdefs 

clobber:clean
	rm -f a.out efl

printall:	
	-pr $(FILES1) | $P
	-pr $(FILES2) | $P
	touch print

print:	$(FILES)
	-pr $? | $P
	touch $@

lint:	a.out
	lint -p *.c -lS

# par example
test:	/usr/bin/efl
	/usr/bin/efl "system	= gcos" efltest/Hard.e >z1
	cmp z1 efltest/Hard.out
	/usr/bin/efl "system	= gcos" efltest/Band.e >z1
	cmp z1 efltest/Band.out
	/usr/bin/efl "system	= gcos" efltest/Buram.e >z1
	cmp z1 efltest/Buram.out
	/usr/bin/efl "system	= gcos" efltest/Dgl.e >z1
	cmp z1 efltest/Dgl.out
	rm z1 z2
	@echo TEST OK

test1:	
	efl "system = gcos" efltest/Hard.e >z1 2>z2
	cmp z1 efltest/Hard.out
	efl "system = gcos" efltest/Band.e >z1 2>z3
	cmp z1 efltest/Band.out
	efl "system = gcos" efltest/Buram.e >z1 2>z4
	cmp z1 efltest/Buram.out
	efl "system = gcos" efltest/Dgl.e >z1 2>z5
	cmp z1 efltest/Dgl.out
#	rm z1 z2 z3 z4 z5
	@echo TEST OK
