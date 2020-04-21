# @(#) makefile 1.6@(#)

#
#
include ${ROOT}/usr/include/make/commondefs
# reorder gives an editor command for most common terminals
# (in reverse order from n'th to 1'st most commonly used)
# to move them to the front of termcap

#VGRIND	= csh /usr/ucb/vgrind
VGRIND	= pr
TS	= tabset/3101 tabset/aa tabset/beehive tabset/diablo \
	  tabset/dtc tabset/owl tabset/std tabset/stdcrt tabset/teleray \
	  tabset/v50 tabset/vt100 tabset/xerox1720
T	= /etc/tabset
IDBTAGS = -idb "root upd"

#all:	termcap.obj
all:	termcap

install:all 
	${INSTALL} -dir $T -u bin -g bin -m 755 ${IDBTAGS} 
	${INSTALL} -u bin -g bin -m 644 -F /etc ${IDBTAGS} termcap
	${INSTALL} -u bin -g bin -m 644 -F /etc ${IDBTAGS} "${TS}"

clean:

clobber:clean
	rm -f termcap.obj 

termcap.obj:termcap.src reorder
	ed - termcap.src < reorder
vgrind:
	$(VGRIND) -h "Terminal Capibility Data Base" \
	termcap.src reorder makefile
