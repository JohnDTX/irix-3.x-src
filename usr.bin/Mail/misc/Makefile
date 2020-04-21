#
#
include ${ROOT}/usr/include/make/commondefs

all:


install:	all
	${INSTALL} -u bin -g bin -m 644 -F /usr/lib \
	-idb "mail usr" "Mail.help Mail.rc"
	${INSTALL} -src Mail.Thelp -u bin -g bin -m 644 -F /usr/lib \
	-idb "mail usr" Mail.help.~
