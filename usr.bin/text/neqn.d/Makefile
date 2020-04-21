#
#
include ${ROOT}/usr/include/make/commondefs

LIBS	= /usr/lib/liby.a


SOURCE	= e.y e.h diacrit.c eqnbox.c font.c fromto.c funny.c glob.c integral.c \
	 io.c lex.c lookup.c mark.c matrix.c move.c over.c paren.c \
	 pile.c shift.c size.c sqrt.c text.c
OBJ	= diacrit.o eqnbox.o font.o fromto.o funny.o glob.o integral.o \
	 io.o lex.o lookup.o mark.o matrix.o move.o over.o paren.o \
	 pile.o shift.o size.o sqrt.o text.o e.o

all:	neqn

install:all
	${INSTALL} -u bin -g bin -m 755 -F /usr/bin  \
	-idb "std.sw.unix mr" neqn

clean:
	rm -f *.o y.tab.h y.tab.c e.def e.c

clobber:clean
	rm -f neqn

.c.o:
	$(CC) -c $(CFLAGS) -DNEQN $<

neqn:	$(OBJ)
	$(CC) $(LDFLAGS) -o $@ $(OBJ) $(LIBS)

$(OBJ):	e.h e.def

e.o:	e.c

e.c e.def:e.y
	  yacc -d e.y
	  mv y.tab.c e.c
	  mv y.tab.h e.def
