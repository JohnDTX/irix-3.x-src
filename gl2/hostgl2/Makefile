DEMOS = arch flight flow jet
DEMOSGET = $(DEMOSDIR)/arch/arch.dsk $(DEMOSDIR)/flight/flight.dsk \
	$(DEMOSDIR)/flow/flow.dsk $(DEMOSDIR)/jet/jet.dsk
# The following line assumes that you have chrooted to the top of the
# source tree.
DEMOSDIR = /src/demos.gl2/src

commands = dliris iftp irsh

CDIR = remotegl2/c
FDIR = remotegl2/f77
REM = remotegl2/boot remotegl2/man remotegl2/man/man3
libdir = $(DEST)/usr/lib

all:	$(commands)

dliris: dliris.c boot.h remprom.h irisboot.h
	$(CC) $(CFLAGS) -o dliris dliris.c

iftp: iftp.c $(libdir)/librgl2.a
	$(CC) $(CFLAGS) -o $@ -DMC68000 -DSYSTEM5 iftp.c \
		$(DEST)/usr/lib/librgl2.a

irsh: irsh.c $(libdir)/librgl2.a
	$(CC) $(CFLAGS) -o $@ -DMC68000 -DSYSTEM5 irsh.c \
		$(DEST)/usr/lib/librgl2.a

install: all dirs cterm f77term $(DEMOS) boot man
	cp $(commands) $(DEST)/usr/bin
	cd $(DEST)/usr/bin ; \
		chown bin $(commands) ; \
		chgrp bin $(commands) ; \
		chmod 755 $(commands)
	cp -r remotegl2 hostxns ; mkdir hosttcp
	cd hostxns/f77 ; rm -f *.f
	find hostxns -print | xargs chown bin
	find hostxns -print | xargs chgrp bin
	cd hostxns ; find . -print | cpio -pdaum ../hosttcp
	rm hostxns/boot/tcpiris ; rm hosttcp/boot/iris
	rm -r hosttcp/c/xns
	mv hosttcp/boot/tcpiris hosttcp/boot/iris

dirs:
	for d in remotegl2 $(REM) $(CDIR) $(CDIR)/xns $(FDIR) ;\
	do if test ! -d $$d ; then mkdir $$d ; fi ;\
	done

boot:
	if test $(machine) = iris ; \
	then	cp ../tpe2/term/iris remotegl2/boot ;\
		cp ../tpe2/term/tcpiris remotegl2/boot ;\
		for f in $(DEMOS) ; \
		do	cp $(DEMOSDIR)/$$f/$$f.dsk remotegl2/boot/$$f ;\
			cp $(DEMOSDIR)/$$f/$$f.dsk $(DEST)/usr/boot/$$f ;\
		done ; \
	fi

cterm:
	cp cMakefile  $(CDIR)/Makefile
	cp $(GLROOT)/include/gl.h $(CDIR)
	cp $(GLROOT)/include/device.h $(CDIR)
	cp $(GLROOT)/include/get.h $(CDIR)
	cp $(GLROOT)/include/gl2/rpc.h $(CDIR)
	cp ../rgl2/lib.c $(CDIR)
	cp ../rgl2/io.c $(CDIR)
	cp ../rgl2/decl.h $(CDIR)
	cp dliris.c $(CDIR)
	cp iftp.c $(CDIR)
	cp irsh.c $(CDIR)
	cp VAXboot.h $(CDIR)/boot.h
	cp irisboot.h $(CDIR)
	cp remprom.h $(CDIR)
	cp sqiral.c drawing.c popup.c textinit.c $(CDIR)
	cp iris.termcap $(CDIR)
	cp $(GLROOT)/include/xns/Xnsioctl.h $(CDIR)/xns/Xnsioctl.h
# wsirisrc should come from the same place that /.login does
	cp $(DTERM)/.wsirisrc $(CDIR)/wsirisrc
	cp $(DTERM)/mexrc $(CDIR)
# the hosts files will be part of the IP/TCP option distribution
#	echo 89.0.0.1 IRIS2300 > $(CDIR)/hosts
#	echo IRIS2300 > $(CDIR)/hosts.equiv
	echo IRIS2300 > $(CDIR)/sys_id
	cp $(DEST)/etc/TZ $(CDIR)
	cp rc.getdate $(CDIR)

#
# This is for 4.2BSD FORTRAN only.  If an IRIS is to be the host,
# the FORTRAN interface library libfgl.a is used with librgl.a
# when linking.  dliris.f does exist, but it is intentionally not included,
# as the existence of a C compiler is assumed.
#
f77term:
	cp fMakefile  $(FDIR)/Makefile
	cp ../fgl2/fgl.h $(FDIR)
	cp ../fgl2/fdevice.h $(FDIR)
	cp ../fgl2/fget.h $(FDIR)
	cp ../frgl2/frpc.h $(FDIR)
	cp ../frgl2/lib.f $(FDIR)
	cp ../frgl2/io.f $(FDIR)
	cp fio.h $(FDIR)
	cp fsys.c $(FDIR)
	cp sqiral.[fF] drawing.[fF] popup.[fF] textinit.[fF] $(FDIR)

man:
	p=`pwd`/remotegl2/man/man3 ; \
	cd ../../man/u_man/man3/gl2 ;\
	find . -print | cpio -padum $$p

arch:
	cd $(DEMOSDIR)/arch ; make arch.dsk
flight:
	cd $(DEMOSDIR)/flight ; make flight.dsk
flow:
	cd $(DEMOSDIR)/flow ; make flow.dsk
jet:
	cd $(DEMOSDIR)/jet ; make jet.dsk

clobber clean:
	rm -rf dliris iftp irsh remotegl2 hostxns hosttcp
