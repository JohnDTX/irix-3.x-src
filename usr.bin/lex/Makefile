#	makefile for lex program {lex libraries are in usr.lib/lex}

#
#

include ${ROOT}/usr/include/make/commondefs

# $Source: /d2/3.7/src/usr.bin/lex/RCS/Makefile,v $
# @{#}$Revision: 1.1 $
# $Date: 89/03/27 18:03:40 $

OBJS	= main.o parser.o sub1.o sub2.o header.o

LEX	= lex

all:	${LEX}

install :all
	${INSTALL} -u bin -g bin -m 755 -F /usr/bin -idb "std.sw.unix" "${LEX}"

lex:	${OBJS}
	${CCF} ${OBJS} -o $@ ${LDFLAGS} 

smallex:main.c parser.c sub1.c sub2.c header.c
	${CCF} -o $@ -DSMALL main.c parser.c sub1.c sub2.c header.c ${LDFLAGS} 

parser.c:parser.y
	yacc parser.y ; mv y.tab.c parser.c

nmain.o:nmain.c ldefs.c once.c
	${CCF} -c nmain.c

main.o: main.c ldefs.c once.c
	${CCF} -c main.c

sub1.o: sub1.c ldefs.c
	${CCF} -c sub1.c

sub2.o: sub2.c ldefs.c
	${CCF} -c sub2.c

header.o: header.c ldefs.c
	${CCF} -c header.c

clean:
	rm -f *.o y.tab.c parser.c

clobber:clean
	rm -f lex smallex
