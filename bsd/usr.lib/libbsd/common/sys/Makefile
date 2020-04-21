include ${ROOT}/usr/include/make/commondefs

LLDLIBS	=-lbsd
LCDEFS	=-Dsgi
LCINCS=-I${ROOT}/usr/include/bsd -I${ROOT}/usr/include
LCOPTS= -O

SRCS	=gettimeday.c dup2.c writev.c readv.c killpg.c
OBJS	=$(SRCS:.c=.o)

default:$(OBJS)
