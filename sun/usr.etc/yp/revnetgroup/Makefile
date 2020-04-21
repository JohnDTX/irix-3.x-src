#
#
include ${ROOT}/usr/include/make/commondefs
#

#  @(#)Makefile 1.1 86/02/05 Copyright (c) 1985 by Sun Microsystems, Inc.
#  @(#)Makefile	2.1 86/04/16 NFSSRC
#

HDRS = getgroup.h table.h util.h
SRCS = revnetgrp.c getgroup.c table.c util.c
OBJS = ${SRCS:.c=.o}

LCOPTS= -O 
LCINCS= -I${ROOT}/usr/include/bsd
LLDLIBS= -lsun -lbsd -ldbm
IDBTAGS=-idb "std.sw.unix mr"

revnetgroup: $(OBJS)
	$(CCF) $(OBJS) -o revnetgroup $(LDFLAGS)

lint:
	lint $(SRCS)

clean clobber:
	rm -f revnetgroup  $(OBJS) a.out core

install: revnetgroup
	${INSTALL} -F /usr/etc/yp ${IDBTAGS}  revnetgroup

depend:
	cp /dev/null x.c
	for i in $(SRCS) ; do \
		(/bin/grep '^#[     ]*include' x.c $$i | sed \
			-e 's,<\(.*\)>,"${ROOT}/usr/include/\1",' \
			-e 's/:[^"]*"\([^"]*\)".*/: \1/' \
			-e 's/\.c/\.o/' >>makedep); done
	echo '/^# DO NOT DELETE THIS LINE/+2,$$d' >eddep
	echo '$$r makedep' >>eddep
	echo 'w' >>eddep
	cp Makefile Makefile.bak
	ed - Makefile < eddep
	rm eddep makedep x.c
	echo '# DEPENDENCIES MUST END AT END OF FILE' >> Makefile
	echo '# IF YOU PUT STUFF HERE IT WILL GO AWAY' >> Makefile
	echo '# see make depend above' >> Makefile
 
# DO NOT DELETE THIS LINE -- make depend uses it
# DEPENDENCIES MUST END AT END OF FILE
revnetgrp.o: ${ROOT}/usr/include/stdio.h
revnetgrp.o: ${ROOT}/usr/include/ctype.h
revnetgrp.o: ${ROOT}/usr/include/strings.h
revnetgrp.o: ${ROOT}/usr/include/pwd.h
revnetgrp.o: ${ROOT}/usr/include/rpcsvc/ypclnt.h
revnetgrp.o: util.h
revnetgrp.o: table.h
revnetgrp.o: getgroup.h
getgroup.o: ${ROOT}/usr/include/stdio.h
getgroup.o: ${ROOT}/usr/include/ctype.h
getgroup.o: ${ROOT}/usr/include/strings.h
getgroup.o: table.h
getgroup.o: util.h
getgroup.o: getgroup.h
table.o: ${ROOT}/usr/include/ctype.h
table.o: util.h
table.o: table.h
util.o: ${ROOT}/usr/include/stdio.h
util.o: util.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
