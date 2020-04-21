#
#
include ${ROOT}/usr/include/make/commondefs
SRCS= 	main.c op[1-9].c optab.c opk.c 
HDRS=	opt.h
OBJS=	main.o op1.o op2.o op3.o op4.o op5.o op6.o \
	op7.o op9.o optab.o opk.o

all:	c2

c2:	${OBJS}
	${CCF} -n -o c2 ${OBJS}

clobber clean:
	rm -f *.o c2


install: all
	strip c2
	${INSTALL} -u bin -g bin -m 755 -F /lib \
	-idb "std.sw.unix mr" c2

release :
	release $(SRCS) $(HDRS) Makefile

snapshot :
	snapshot $(SRCS) $(HDRS) Makefile
