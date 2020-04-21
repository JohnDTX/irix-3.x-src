# make IRIS ftp command
#
# "$Header: /d2/3.7/src/bsd/ucb/ftp/RCS/Makefile,v 1.1 89/03/27 15:00:34 root Exp $"
#
include ${ROOT}/usr/include/make/commondefs
ALL=	ftp
LCINCS=-I${ROOT}/usr/include/bsd -I${ROOT}/usr/include 
LCOPTS = -O
LLDLIBS = -lsun -lbsd -ldbm
OBJS=	cmds.o cmdtab.o ftp.o getpass.o glob.o main.o pclose.o ruserpass.o \
	    domacro.o

all:	${ALL}

ftp:	${OBJS}
	${CCF} -o ftp ${OBJS} ${LDFLAGS} 

ftp.o:	ftp_var.h
cmds.o:	ftp_var.h
cmdtab.o: ftp_var.h
ftp.o:	ftp_var.h
main.o: ftp_var.h

install:all
	${INSTALL} -u bin -g bin -m 755 -F /usr/bin \
		-idb "upd usr" "${ALL}"

clean:
	rm -f *.o *.s a.out

clobber:clean
	rm -f ${ALL}
