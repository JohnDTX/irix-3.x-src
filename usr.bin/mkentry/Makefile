#
#
include ${ROOT}/usr/include/make/commondefs


SRCS	= c2fsubs.c charmap.c f2csubs.c mkentry.y scan.c token.c \
	  extcentry.lex
HDRS	= lex.h tokens.h macros.h mkentry.h

all: mkc2f mkf2c extcentry

test:	test.o scan.o token.o charmap.o
	${CCF} -o $@ test.o scan.o token.o charmap.o

scan.o: scan.c lex.h tokens.h 

token.o: token.c lex.h tokens.h

charmap.o: charmap.c lex.h tokens.h

f2csubs.o: f2csubs.c tokens.h macros.h mkentry.h

c2fsubs.o: c2fsubs.c tokens.h macros.h mkentry.h

mkf2c:	mkentry.o f2csubs.o scan.o token.o charmap.o
	  ${CCF} $(LDFLAGS) -o mkf2c \
	  mkentry.o f2csubs.o scan.o token.o charmap.o

mkc2f: mkentry.o c2fsubs.o scan.o token.o charmap.o
	  ${CCF} $(LDFLAGS) \
	  -o mkc2f mkentry.o c2fsubs.o scan.o token.o charmap.o

extcentry: extcentry.c
	${CCF} $(LDFLAGS) -o extcentry \
	-O extcentry.c ${ROOT}/usr/lib/libl.a

extcentry.c: extcentry.lex
	lex extcentry.lex
	mv lex.yy.c extcentry.c

mkentry.c: mkentry.y
	yacc mkentry.y
	mv y.tab.c mkentry.c

install : all
	${INSTALL} -u bin -g bin -m 755 -F /usr/bin \
	-idb "std.sw.unix mr" "mkc2f mkf2c extcentry"

clobber clean:
	rm -f *.o mkc2f mkf2c mkentry.c extcentry extcentry.c

release:
	release $(SRCS) $(HDRS) Makefile

snapshot:
	snapshot $(SRCS) $(HDRS) Makefile

