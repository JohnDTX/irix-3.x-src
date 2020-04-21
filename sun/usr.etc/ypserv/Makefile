#
#
include ${ROOT}/usr/include/make/commondefs

LCDEFS= -O  
LCINCS= -I.. -I${ROOT}/usr/include/bsd -I${ROOT}/usr/include
LLDLIBS=  -lsun -lbsd -ldbm
OBJS= ypserv.o yps_proc.o ypdbname.o yps_map.o yps_ancil.o
SRCS= ypserv.c yps_proc.c ypdbname.c yps_map.c yps_ancil.c
HDRS= ypsym.h
TARGETS= ypserv

all: ${TARGETS}

ypserv: ${OBJS} 
	${CC} ${CFLAGS} -o ypserv ${OBJS} ${LDFLAGS}

install:	all
	${INSTALL} -u root -g sys -m 755 -F /usr/etc \
	-idb "std.sw.unix mr" "${TARGETS}"

clobber: clean
	rm -f ${TARGETS}
clean: 
	rm -f a.out core *.o
depend: 
	@rm -f makedep Makefile.BAK 
	@cp Makefile "#Makefile" 
	@for i in ${SRCS}; do \
		echo $$i; \
		cc -M ${CFLAGS} $$i | \
		sed -e 's:\.\./[^ ]*/\.\.:..:g' \
		    -e 's: ./: :' | \
		awk 'BEGIN { INDENT = "\t" } \
		     NF > 0 { \
			if ($$1 != lhs) { \
			    lhs = $$1; print p_line; p_line = $$1 " "; \
			    lim = 72; } \
			if (length(p_line) + length($$2) > lim) { \
			    print p_line "\\"; p_line = INDENT; lim = 63; \
			} \
			p_line = p_line $$2 " "; \
		     } \
		END { print p_line; }' >> makedep; \
	done 
	sed -e "/^# DO NOT DELETE THIS LINE -- make depend uses it/,/^# DO NOT DELETE THIS 2nd LINE -- make depend uses it/d" \
	    -e "/^# DO NOT DELETE THIS LINE -- make depend use it/"',$$d' \
	    < Makefile > Makefile.new 
	echo "# DO NOT DELETE THIS LINE -- make depend uses it" >> \
		Makefile.new 
	cat makedep >> Makefile.new 
	echo "# DO NOT DELETE THIS 2nd LINE -- make depend uses it" >> \
		Makefile.new 
	rm -f Makefile; mv Makefile.new Makefile 
	rm -f "#Makefile"
tags: 
	ctags ${SRCS}
fluff: ${SRCS} 
	lint -Dlint ${CC_DEFS} ${CC_INCL} ${SRCS} > fluff

# DO NOT DELETE THIS LINE -- make depend uses it 
# DO NOT DELETE THIS 2nd LINE -- make depend uses it
