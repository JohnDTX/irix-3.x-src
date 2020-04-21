#
include ${ROOT}/usr/include/make/commondefs
#
INSCMD=${INSTALL} -g bin -u bin -idb "2000 root upd" 
CURRENT_MICROCODE = ffp325.ram

# avoid doing anything, except for 2400's
all install:h$$@_$(machine)
hall_ hall_juniper:
hinstall_ hinstall_juniper:

default:all
 
hall_iris: fload binary

fload: fload.c ffload.s atobin
	${CCF} -o fload fload.c ffload.s
	strip fload 

atobin: atobin.c
	${CCF} -o atobin atobin.c ${LDFLAGS}

binary:atobin ${CURRENT_MICROCODE}
	cp ${CURRENT_MICROCODE} ascii
	atobin 

hinstall_iris: all
	${INSCMD} -m 755 -src binary -F /lib skyffp.fas
	${INSCMD} -m 444 -F /etc fload

clean:
	rm -f atobin fload *.o ascii

clobber: clean
	rm -f fload binary

release:
	release fload.c ffload.s atobin.c $(CURRENT_MICROCODE)
