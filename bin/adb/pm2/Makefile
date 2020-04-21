#
#
include ${ROOT}/usr/include/make/commondefs
#
BASE= ../
TARGET= adb
INCL = -I${BASE}

OBJS=	access.o command.o expr.o format.o \
	input.o main.o message.o opset.o \
	output.o pcs.o print.o process.o \
	runpcs.o setup.o sym.o 

all: ${TARGET}

${TARGET}: ${OBJS}
	${CC} ${CFLAGS} ${INCL} ${LDFLAGS} -o ${TARGET} ${OBJS} ${LIBS}

install: all
	${INSTALL} -g bin -u bin -m 755 -F /bin -idb "root upd" \
		${TARGET}
clean:
	rm -f a.out core ${OBJS}

clobber:	clean
	rm -f ${TARGET}

access.o: ${BASE}/access.c
	${CC} ${CFLAGS} ${INCL} -c ${BASE}/access.c

command.o: ${BASE}/command.c
	${CC} ${CFLAGS} ${INCL} -c ${BASE}/command.c

expr.o: ${BASE}/expr.c
	${CC} ${CFLAGS} ${INCL} -c ${BASE}/expr.c

format.o: ${BASE}/format.c
	${CC} ${CFLAGS} ${INCL} -c ${BASE}/format.c

input.o: ${BASE}/input.c
	${CC} ${CFLAGS} ${INCL} -c ${BASE}/input.c

main.o: ${BASE}/main.c
	${CC} ${CFLAGS} ${INCL} -c ${BASE}/main.c

message.o: ${BASE}/message.c
	${CC} ${CFLAGS} ${INCL} -c ${BASE}/message.c

opset.o: opset.c
	${CC} ${CFLAGS} ${INCL} -c opset.c

output.o: ${BASE}/output.c
	${CC} ${CFLAGS} ${INCL} -c ${BASE}/output.c

pcs.o: ${BASE}/pcs.c
	${CC} ${CFLAGS} ${INCL} -c ${BASE}/pcs.c

print.o: ${BASE}/print.c
	${CC} ${CFLAGS} ${INCL} -c ${BASE}/print.c

process.o: ${BASE}/process.c
	${CC} ${CFLAGS} ${INCL} -c ${BASE}/process.c

runpcs.o: ${BASE}/runpcs.c
	${CC} ${CFLAGS} ${INCL} -c ${BASE}/runpcs.c

setup.o: setup.c
	${CC} ${CFLAGS} ${INCL} -c setup.c

sym.o: ${BASE}/sym.c
	${CC} ${CFLAGS} ${INCL} -c ${BASE}/sym.c

