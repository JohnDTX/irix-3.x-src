
#
#
include ${ROOT}/usr/include/make/commondefs

LIB	= libl.a
FILES	= allprint.o main.o reject.o yyless.o yywrap.o

all:	${LIB}

install:all 
	${INSTALL} -g bin -u bin -m 644 -F /usr/lib \
	-idb "std.sw.unix mr" ${LIB} 
	ranlib ${ROOT}/usr/lib/${LIB}
	${INSTALL} -g bin -u bin -m 755 -dir lex -F /usr/lib -idb "upd root " 
	${INSTALL} -g bin -u bin -m 644 -F /usr/lib/lex \
	-idb "std.sw.unix mr" "ncform nrform"

clean:
	rm -f ${FILES} 

clobber:
	rm -f ${FILES} 
	rm -f ${LIB}

${LIB}:	${FILES}
	rm -f ${LIB}
	${AR} rv ${LIB} ${FILES}
	ranlib $@

.c.o:
	${CC} ${CFLAGS} -c $< ; ${LD} -r -x $@ ; mv a.out $@
