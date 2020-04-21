CFLAGS 	=	-O -N -V -DUC4

INCLUDES = 	-I/usr/include/gl2

CLIBS = -Zg

SRCS =	parse.c

OBJS =	parse.o

all: plotexample traceexample

parse.o: parsetab.h
	${CC} ${INCLUDES} ${CFLAGS} -c parse.c 

plotexample.o:
	${CC} ${INCLUDES} ${CFLAGS} -c plotexample.c 

traceexample.o:
	${CC} ${INCLUDES} ${CFLAGS} -c traceexample.c 

plotexample:	parse.o plotexample.o
	${CC} ${CFLAGS} -o plotexample plotexample.o parse.o $(CLIBS) \
			-lplot

traceexample:	parse.o traceexample.o
	${CC} ${CFLAGS} -o traceexample traceexample.o parse.o $(CLIBS)

clean:
	rm -f plotexample traceexample ${OBJS} *CKP *BAK *.b *.r *.br *.o *.v .emacs*

