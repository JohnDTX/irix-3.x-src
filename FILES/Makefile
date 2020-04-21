# @(#)$Header: /d2/3.7/src/FILES/RCS/Makefile,v 1.1 89/03/27 13:49:38 root Exp $

include ${ROOT}/usr/include/make/commondefs

IDBTAG	=-idb "upd"
INSCMD	=${INSTALL} ${IDBTAG}

FRO	= sulog OLDsulog
FADM	= cronlog OLDcronlog
FETC	= wtmp
FROOT	= .cshrc .login .profile
FGUEST	= .cshrc .login .profile rhosts
FUSRLIB	= crontab
U3010	= mexrc tcpwsirisrc .wsirisrc

all clean clobber:

install:
	${INSCMD} -m 600 -g sys -u root -F /usr/adm "${FRO}"
	${INSCMD} -m 644 -g sys -u root -F /usr/lib "${FADM}"
	${INSCMD} -m 644 -g sys -u root -F /etc "${FETC}"
	cd root; ${INSCMD} -m 644 -g sys -u root -F / "${FROOT}"
	cd guest; ${INSCMD} -m 644 -g guest -u guest \
		-F /usr/people/guest  "${FGUEST}
	cd usrlib; ${INSCMD} -m 644 -g sys -u root -F /usr/lib "${FUSRLIB}"
