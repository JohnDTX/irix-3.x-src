include	$(ROOT)/usr/include/make/commondefs
CC_OPTS0= -g 
CC_DEFS0= 
CC_INCL0= -I$(ROOT)/usr/include/gl -I../h
CC_FLGS0= ${CC_OPTS0} ${CC_DEFS0} ${CC_INCL0} $(CFLAGS)
LIBS0=  -Zg ../libwin/ip2/libwin.a -lbsd
OBJS0= bar.o font.o keyboard.o main.o makeshell.o menu.o misc.o shelltool.o \
       softq.o textport.o winlib.o butdata.o wintty.o buttons.o shm.o srcwin.o\
	scanner.o my_bcopy.o cmap.o speed_bar.o bogus.o var_disp.o
SRCS0= ../common/bar.c ../common/font.c ../common/keyboard.c \
       ../common/main.c ../common/makeshell.c ../common/menu.c \
       ../common/misc.c ../common/shelltool.c ../common/softq.c \
       ../common/textport.c ../common/winlib.c ../common/butdata.c \
       ../common/wintty.c ../common/buttons.c ../common/shm.c \
	../common/srcwin.c ../common/scanner.c my_bcopy.c cmap.c speed_bar.c \
	../common/bogus.c var_disp.c
HDRS0= ../common/window.h ../common/kb.h ../common/gsh.h ../common/manage.h 
TARGETS= edge
all: ${TARGETS}
clobber: 
	rm -f ${TARGETS} a.out core
clean: 
	rm -f a.out core ${OBJS0}
depend: 
	@rm -f makedep Makefile.BAK 
	@cp Makefile "#Makefile" 
	@for i in ${SRCS0}; do \
		echo $$i; \
		cc -M ${CC_FLGS0} $$i | \
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
edge: ${OBJS0} 
	${CC} $(LDFLAGS) ${CC_FLGS0} -o edge ${OBJS0} ${LIBS0}
bar.o: ../common/bar.c 
	${CC} ${CC_FLGS0} -c ../common/bar.c
font.o: ../common/font.c 
	${CC} ${CC_FLGS0} -c ../common/font.c
keyboard.o: ../common/keyboard.c 
	${CC} ${CC_FLGS0} -c ../common/keyboard.c
main.o: ../common/main.c 
	${CC} ${CC_FLGS0} -c ../common/main.c
twins.o: ../common/twins.c 
	${CC} ${CC_FLGS0} -c ../common/twins.c
makeshell.o: ../common/makeshell.c 
	${CC} ${CC_FLGS0} -c ../common/makeshell.c
menu.o: ../common/menu.c 
	${CC} ${CC_FLGS0} -c ../common/menu.c
misc.o: ../common/misc.c 
	${CC} ${CC_FLGS0} -c ../common/misc.c
shelltool.o: ../common/shelltool.c 
	${CC} ${CC_FLGS0} -c ../common/shelltool.c
softq.o: ../common/softq.c 
	${CC} ${CC_FLGS0} -c ../common/softq.c
textport.o: ../common/textport.c 
	${CC} ${CC_FLGS0} -c ../common/textport.c
winlib.o: ../common/winlib.c 
	${CC} ${CC_FLGS0} -c ../common/winlib.c
butdata.o: ../common/butdata.c 
	${CC} ${CC_FLGS0} -c ../common/butdata.c
wintty.o: ../common/wintty.c 
	${CC} ${CC_FLGS0} -c ../common/wintty.c
buttons.o: ../common/buttons.c 
	${CC} ${CC_FLGS0} -c ../common/buttons.c
shm.o: ../common/shm.c 
	${CC} ${CC_FLGS0} -c ../common/shm.c
srcwin.o: ../common/srcwin.c 
	${CC} ${CC_FLGS0} -c ../common/srcwin.c
scanner.o: ../common/scanner.c 
	${CC} ${CC_FLGS0} -c ../common/scanner.c
my_bcopy.o: ../common/my_bcopy.c 
	${CC} ${CC_FLGS0} -c ../common/my_bcopy.c
cmap.o: ../common/cmap.c 
	${CC} ${CC_FLGS0} -c ../common/cmap.c
speed_bar.o: ../common/speed_bar.c 
	${CC} ${CC_FLGS0} -c ../common/speed_bar.c
bogus.o: ../common/bogus.c 
	${CC} ${CC_FLGS0} -c ../common/bogus.c
var_disp.o: ../common/var_disp.c 
	${CC} ${CC_FLGS0} -c ../common/var_disp.c


# DO NOT DELETE THIS LINE -- make depend uses it 
# DO NOT DELETE THIS 2nd LINE -- make depend uses it
