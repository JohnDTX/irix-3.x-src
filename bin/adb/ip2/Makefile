#
#
include ${ROOT}/usr/include/make/commondefs

BASE= ..
LCINCS= -I../ 
LCOPTS= -DIP2 -ZJ

TARGET= adb

OBJS=	access.o command.o expr.o format.o \
	input.o main.o message.o opset.o \
	output.o pcs.o print.o process.o \
	runpcs.o setup.o sym.o 

all: ${TARGET}

${TARGET}: ${OBJS}
	${CC} ${LDFLAGS} -o ${TARGET} ${OBJS} ${LIBS}

install: all
	${INSTALL} -g bin -u bin -m 755 -F /bin -idb "root upd" \
	    ${TARGET}

clean:
	rm -f a.out core ${OBJS}

clobber:	clean
	rm -f ${TARGET}

access.o: ${BASE}/access.c
	${CCF} -c ${BASE}/access.c

command.o: ${BASE}/command.c
	${CCF} -c ${BASE}/command.c

expr.o: ${BASE}/expr.c
	${CCF} -c ${BASE}/expr.c

format.o: ${BASE}/format.c
	${CCF} -c ${BASE}/format.c

input.o: ${BASE}/input.c
	${CCF} -c ${BASE}/input.c

main.o: ${BASE}/main.c
	${CCF} -c ${BASE}/main.c

message.o: ${BASE}/message.c
	${CCF} -c ${BASE}/message.c

opset.o: opset.c
	${CCF} -c opset.c

output.o: ${BASE}/output.c
	${CCF} -c ${BASE}/output.c

pcs.o: ${BASE}/pcs.c
	${CCF} -c ${BASE}/pcs.c

print.o: ${BASE}/print.c
	${CCF} -c ${BASE}/print.c

process.o: ${BASE}/process.c
	${CCF} -c ${BASE}/process.c

runpcs.o: ${BASE}/runpcs.c
	${CCF} -c ${BASE}/runpcs.c

setup.o: setup.c
	${CCF} -c setup.c

sym.o: ${BASE}/sym.c
	${CCF} -c ${BASE}/sym.c

