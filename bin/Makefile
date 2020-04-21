#IRIS /bin Makefile
# "$Header: /d2/3.7/src/bin/RCS/Makefile,v 1.1 89/03/27 14:50:06 root Exp $"

include ${ROOT}/usr/include/make/commondefs

STRIP	  = strip
INST	  = Install -idb "root upd"

DIRS	  = adb as as20 csh diff ed file make more sed tset vi sh 

PROGSCC	  = ar ld nm ranlib size strip 
PROGS	  = basename cat chgrp chmod chown cmp cpio date dd df \
	    dirname du echo env expr false find flp freq grep head id kill \
	    line login ls mesg mkdir mt mv \
	    newgrp nice nohup od passwd pr printenv ps pwd \
	    reset rm rmail rmdir sgilabel sleep strings stty sum sync \
	    tail tar tee test time touch true tty \
	    ul wc who write uname
MAILS	  = mail 
TRUES	  = m68000 m68k
FALSES	  = mips pdp11 vax u3b u3b2 u3b5 u3b15 4d60
CPS	  = ln cp
CCX	  = cc.x cc.gl1
SUIDS	  = df login newgrp passwd ps uname su
TARGETS	  = ${MAILS} ${PROGS} ${PROGSCC} ${CCX} ${SUIDS} \
		${CPS} ${TRUES} ${FALSES}

# Library list for programs that need yellow pages getpwent and getgrent
#
SUNLIBS	  = -lsun -lbsd -ldbm

CLS	  = ${ROOT}/usr/lib/clstat.o
TDIR      = /bin


all default install clean clobber:bin_$$@ FRC
	-for i in ${DIRS}; do cd $$i; echo ====bin/$$i; ${MAKE} $@; cd ..; done

${DIRS}:FRC
	 cd $@; make all

bin_all bin_default: ${TARGETS} FRC
	rm -f '[' ; ln test '['		# ugh


# Compile as needed and install.
# Special Notes:
bin_install: bin_all FRC
	${INST} -F ${TDIR} "${PROGS} ${PROGSCC}"
	${INST} -m 4755 -g sys -u root -F ${TDIR} "${SUIDS}"
	${INST} -m 2755 -g mail -F ${TDIR} "${MAILS}"
	${INST} -ln ${TDIR}/test -F ${TDIR} '['
# 	link copy of tar into /etc for emergency recovery by init
	${INST} -ln ${TDIR}/tar -F /etc tar
	${INST} -o -F ${TDIR} mv
	${INST} -ln ${TDIR}/mv -F ${TDIR} "${CPS}"
	${INST} -ln ${TDIR}/true -F ${TDIR} "${TRUES}"
	${INST} -ln ${TDIR}/false -F ${TDIR} "${FALSES}"
	${INST} -ln ${TDIR}/m68k -F ${TDIR} m68000
	if test "${GLHDWR}" = gl1 ; then \
		${INST} -src cc.gl1 -F ${TDIR} cc ;\
	else \
		${INST} -src cc.x -F ${TDIR} cc ;\
	fi
	${INST} -ln ${TDIR}/cc -F ${TDIR} f77
	${INST} -ln ${TDIR}/cc -F ${TDIR} pc 


bin_clean: 
	rm -f *.o a.out core y.tab.c y.tab.h

bin_clobber:
	rm -f *.o a.out core y.tab.c y.tab.h ${TARGETS} '['

rm rmdir tar du:$$@.c
	${CCF} -o $@ $@.c ${CLS} ${LDFLAGS} 

${CPS}:	mv
	rm -rf $@; ln mv $@

${TRUES}:true
	rm -rf $@; ln true $@

${FALSES}:false
	rm -rf $@; ln false $@

cpio find ls:$$@.c
	${CCF} -o $@ $@.c ${CLS} ${SUNLIBS} ${LDFLAGS} 

date rmail:$$@.c
	${CC} -o $@ -I${ROOT}/usr/include/bsd ${CFLAGS} $@.c \
		${SUNLIBS} ${LDFLAGS} 

chgrp chown id mail who write:$$@.c
	${CCF} -o $@ $@.c ${SUNLIBS} ${LDFLAGS} 

ps:	$$@.c
	${CCF} $@.c -o $@ -lm ${SUNLIBS} ${LDFLAGS} 

# passwd MUST NOT link to Yellow Pages libraries
passwd:	$$@.c
	${CCF} -o $@ $? -lcrypt ${LDFLAGS} 

login su newgrp:$$@.c
	${CCF} -o $@ $? -lcrypt ${LDFLAGS} ${SUNLIBS} 

cc.x:	cc.o 
	${CC} -o cc.x cc.o ${SYSLIBS}
	${STRIP} cc.x

cc.gl1:	cc.c
	${CCF} -o cc.gl1 -DENV_CFLAGS -DECHOCMDS -DGL1 cc.c ${SYSLIBS}
	${STRIP} cc.gl1

cc.o: cc.c
	${CC} -c -O ${TOOLFLAGS} -DECHOCMDS -DALL_AS20 cc.c

ul:	$$@.c
	${CCF} $? -o $@ -ltermlib ${LDFLAGS} 

ar:	$$@.c
	${CCF} -DSYSTEM5 $? -o $@ ${LDFLAGS} 

.c:
	${CCF} -o $@ $< ${LDFLAGS}

.sh:
	cp $< $@ ; chmod ugo+x $@

FRC:
