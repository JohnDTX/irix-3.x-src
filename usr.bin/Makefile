# makefile for workstation usr/bin directory
# $Header: /d2/3.7/src/usr.bin/RCS/Makefile,v 1.1 89/03/27 17:39:32 root Exp $

include ${ROOT}/usr/include/make/commondefs

DIRS	= acct awk bc bs calendar cxref dbx dc diction diff3 edge \
	  error lex lint lp lpr Mail m4 mkentry print prof ratfor rcs \
	  refer sccs sno spell text uucp yacc

PROGS	= 300 300s 4014 450 apply asa banner banner7 bcat bfs cal cb \
	  check checkcw checkeq clear col comm csplit ct ctags cut cw \
	  deroff devport diffmk dircmp egrep factor fgrep fold fsplit \
	  gclear getopt graph hp hyphen ipcrm join last logname lorder \
	  man newform news nl pack paste pcat ptx regcmp script sdiff see \
	  spline split ssp sort tabs tc tr tsort uniq units unpack uupick \
	  uuto whereis which whoami xargs 

# programs that need to be setuid to root
SUID	= at cu
# programs that need to be setgid to sys
GUID	= iostat ipcs lav w

INSCMD	= ${INSTALL} -idb "upd root" 


# Yellow Pages libraries and programs that require them
#
SUNLIBS	= -lsun -lbsd -ldbm
YPPROGS	= ct ipcs last logname news whoami

# ignored: xwrite systat xfsba xps zero


all default install clean clobber:ub_$$@ FRC
	-for i in ${DIRS}; do \
		cd $$i; echo ====usr.bin/$$i; ${MAKE} $@; cd ..; done

$(DIRS) DIRS:FRC
	cd $@; make

ub_all ub_default:$(PROGS) ${SUID} ${GUID}

# fortran is installed first and therfore not done here

ub_install: ub_all fortran FRC
	${INSCMD} -u bin -g bin -m 755 -F /usr/bin "${PROGS}"
	${INSCMD} -u root -g sys -m 4755 -F /usr/bin "${SUID}"
	${INSCMD} -u root -g sys -m 2755 -F /usr/bin "${GUID}"
	${INSCMD} -ln /usr/bin/w -F /usr/bin uptime

ub_clean: FRC
	rm -f *.o a.out core y.tab.c y.tab.h
	cd fortran10; make clean
	cd fortran20; make clean

ub_clobber:FRC
	rm -f *.o a.out core y.tab.c y.tab.h
	rm -f $(PROGS) 
	cd fortran10; make clobber
	cd fortran20; make clobber

FRC:

$(YPPROGS):$$@.c
	$(CCF) $? -o $@ $(SUNLIBS) $(LDFLAGS) 

bfs regcmp:$$@.c
	$(CCF) $? -o $@ -lPW $(LDFLAGS) 

egrep:	$$@.y
	yacc $?
	$(CCF) y.tab.c -o $@ $(LDFLAGS)
	rm -f y.tab.c

factor spline:$$@.c
	$(CCF) $? -o $@ -lm $(LDFLAGS) 

graph:	$$@.c
	$(CCF) $? -o $@ -lplot -lm $(LDFLAGS) 

devport gclear:$$@.c
	$(CC) -I$(ROOT)/usr/include/gl2 $(CFLAGS) -Dgl2 \
		$? -o $@ $(ROOT)/usr/lib/libgl2.a -lm $(LDFLAGS)

w:	$$@.c
	$(CCF) $@.c -o $@ $(LDFLAGS) 

clear ul:$$@.c
	$(CCF) $? -o $@ -ltermlib $(LDFLAGS)
