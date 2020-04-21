# makefile for usr.bin/text/eqn.d
# $Source: /d2/3.7/src/usr.bin/text/eqn.d/RCS/Makefile,v $
# @(#)$Revision: 1.1 $
# $Date: 89/03/27 18:25:41 $

include ${ROOT}/usr/include/make/commondefs

SOURCE	= e.y e.h diacrit.c eqnbox.c font.c fromto.c funny.c glob.c integral.c \
	io.c lex.c lookup.c mark.c matrix.c move.c over.c paren.c \
	pile.c shift.c size.c sqrt.c text.c

OBJ	= diacrit.o eqnbox.o font.o fromto.o funny.o glob.o integral.o \
	io.o lex.o lookup.o mark.o matrix.o move.o over.o paren.o \
	pile.o shift.o size.o sqrt.o text.o e.o

all:	eqn

install:all
	${INSTALL} -m 755 -g bin -u bin -idb "std.unix.sw" -F /usr/bin eqn

clean:
	rm -f *.o e.c e.def y.tab.c y.tab.h
	
clobber:clean
	rm -f eqn

eqn:	e.h e.def $(OBJ)
	$(CC) $(LDFLAGS) $(OBJ) -o $@

e.c e.def:e.y
	$(YACC) $(YFLAGS) -d $?
	mv y.tab.c e.c
	mv y.tab.h e.def

e.o:	e.c

diacrit.o:e.h e.def diacrit.c
	$(CC) $(CFLAGS) -c $*.c

eqnbox.o:e.h e.def eqnbox.c
	$(CC) $(CFLAGS) -c $*.c

font.o:	e.h e.def font.c
	$(CC) $(CFLAGS) -c $*.c

fromto.o:e.h e.def fromto.c
	$(CC) $(CFLAGS) -c $*.c

funny.o:e.h e.def funny.c
	$(CC) $(CFLAGS) -c $*.c

glob.o:	e.h e.def glob.c
	$(CC) $(CFLAGS) -c $*.c

integral.o:e.h e.def integral.c
	$(CC) $(CFLAGS) -c $*.c

io.o:	e.h e.def io.c
	$(CC) $(CFLAGS) -c $*.c

lex.o:	e.h e.def lex.c
	$(CC) $(CFLAGS) -c $*.c

lookup.o:e.h e.def lookup.c
	$(CC) $(CFLAGS) -c $*.c

mark.o:	e.h e.def mark.c
	$(CC) $(CFLAGS) -c $*.c

matrix.o:e.h e.def matrix.c
	$(CC) $(CFLAGS) -c $*.c

move.o:	e.h e.def move.c
	$(CC) $(CFLAGS) -c $*.c

over.o:	e.h e.def over.c
	$(CC) $(CFLAGS) -c $*.c

paren.o:e.h e.def paren.c
	$(CC) $(CFLAGS) -c $*.c

pile.o:	e.h e.def pile.c
	$(CC) $(CFLAGS) -c $*.c

shift.o:e.h e.def shift.c
	$(CC) $(CFLAGS) -c $*.c

size.o:	e.h e.def size.c
	$(CC) $(CFLAGS) -c $*.c

sqrt.o:	e.h e.def sqrt.c
	$(CC) $(CFLAGS) -c $*.c

text.o:	e.h e.def text.c
	$(CC) $(CFLAGS) -c $*.c

e.o:	e.h e.def e.c
	$(CC) $(CFLAGS) -c $*.c

list:
	pr TODO $(SOURCE) makefile

lint:	e.c
	lint -spb *.c
