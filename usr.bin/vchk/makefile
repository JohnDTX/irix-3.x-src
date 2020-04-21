# %W%

CC = cc
CFLAGS = -O
U = $(DEST)/bin

R = .
CFILES = $R/build.c $R/chkit.c $R/chklink.c $R/ctrl.c $R/dsrch.c $R/entry.c \
	$R/file.c $R/macro.c $R/passwd.c $R/path.c $R/specfnc.c $R/spectype.c \
	$R/strfnc.c $R/vchk.c $R/version.c $R/lookup.c

OFILES = vchk.o build.o chkit.o chklink.o ctrl.o dsrch.o entry.o file.o\
	macro.o passwd.o path.o specfnc.o spectype.o strfnc.o version.o\
	lookup.o

vchk: $(OFILES) libucsc.o
	$(CC) $(LFLAGS) $(OFILES) libucsc.o -o vchk

install: vchk
	cp vchk /usr/bin

$(OFILES): $R/vchk.h $R/v3.h

tags:	$(CFILES) $R/libucsc.c
	-rm -f tags
	ctags $(CFILES) $R/libucsc.c

COMPILE =  $(CC) -c $(CFLAGS)

libucsc.o:	$R/libucsc.c
	$(COMPILE) $R/libucsc.c
vchk.o:		$R/vchk.c
	$(COMPILE) $R/vchk.c
build.o:	$R/build.c
	$(COMPILE) $R/build.c
chkit.o:	$R/chkit.c
	$(COMPILE) $R/chkit.c
chklink.o:	$R/chklink.c
	$(COMPILE) $R/chklink.c
ctrl.o:		$R/ctrl.c
	$(COMPILE) $R/ctrl.c
dsrch.o:	$R/dsrch.c
	$(COMPILE) $R/dsrch.c
entry.o:	$R/entry.c
	$(COMPILE) $R/entry.c
file.o:		$R/file.c
	$(COMPILE) $R/file.c
macro.o:	$R/macro.c
	$(COMPILE) $R/macro.c
passwd.o:	$R/passwd.c
	$(COMPILE) $R/passwd.c
path.o:		$R/path.c
	$(COMPILE) $R/path.c
specfnc.o:	$R/specfnc.c
	$(COMPILE) $R/specfnc.c
spectype.o:	$R/spectype.c
	$(COMPILE) $R/spectype.c
strfnc.o:	$R/strfnc.c
	$(COMPILE) $R/strfnc.c
version.o:	$R/version.c
	$(COMPILE) $R/version.c
lookup.o:	$R/lookup.c
	$(COMPILE) $R/lookup.c
# .c.o:
#	rm -i $@
#.o.o:
#	$(CC) -c $(CFLAGS) $R/$*.c
# all:
# 	make -k $U/vchk 'RELOC=$(RELOC)' 'INCLUDE=$(INCLUDE)' 'DOFF=$(DOFF)'\
# 	  'WHO=$(WHO)' 'DEST=$(DEST)' 'SHARED=$(SHARED)' 'SHAREDV=$(SHAREDV)'
# 
