# $Header: /d2/3.7/src/etc/RCS/Makefile,v 1.1 89/03/27 15:37:26 root Exp $"
#
# Makefile for IRIS 2-3K /etc directory
#
include ${ROOT}/usr/include/make/commondefs

LIBFSTOOLSDIR= toyfs
LIBFSTOOLS= $(LIBFSTOOLSDIR)/libtoyfs.a
DIRS= $(LIBFSTOOLSDIR) fsck fsdb mkfs fload

# don't make rc.getdate executable
FILES	= TZ cshrc gettydefs group inittab ioctl.syscon hosts.equiv \
	  hosts motd profile passwd ttytype rc.getdate syslog.conf sys_id 

PROGS	= backup bcopy chroot clri devnm dfsck grpck killall \
	     ./install labelit link mkboot mklost+found mknod mvdir multi \
	     ncheck pwck reboot setmnt single shutdown unlink wall whodo  \
	     kernel labelit gethostident ./Install

BUSYFILES = cron init update getty 

IPROGS	= newfs 

SPROGS = vmstat 

# don't make rc.getdate executable
SFILES =  bcheckrc brc powerfail rc rebootrc rc.s0 rc.tcp rc.xns rc.nfs rc.mail

U3010	= ibtab.2300 inittab.2300 termcap.2300 \
	  mkbootflp mkrecover rootcopy flpuxfer flppatch

GRPOWN	= -g sys -u root
IDBTAG	= -idb "root upd"
INSCMD	=${INSTALL} ${IDBTAG}

# Yellow Pages library list
YPLIBS	= -lsun -lbsd -ldbm
# Programs that need YPLIBS
YPPROGS	= grpck wall


all default install clean clobber:etc_$$@ FRC
	-for i in ${DIRS}; do cd $$i; echo ====etc/$$i; ${MAKE} $@; cd ..; done

$(DIRS):FRC
	cd $@; make

etc_all etc_default:$(PROGS) $(FILES) ${SFILES} ${SPROGS} ${IPROGS} \
		     ${BUSYFILES} FRC

etc_install:etc_all hinstall$(machine) FRC
# install data files
	${INSCMD} ${GRPOWN} -m 755 -F /etc "${PROGS} ${YPPROGS}"
	${INSCMD} ${GRPOWN} -O -m 755 -F /etc "${BUSYFILES}"
	${INSCMD} -ln /etc/init -F /etc telinit
	${INSCMD} ${GRPOWN} -m 2755 -F /etc "${SPROGS}"
	${INSCMD} ${GRPOWN} -o -m 755 -F /etc "${SFILES}"
	${INSCMD} ${GRPOWN} -o -m 644 -F /etc "${FILES}"

hinstalljuniper hinstall:
	${INSCMD} ${GRPOWN} -F /etc "${IPROGS}"
hinstalliris:

install3010: $(U3010)
	${INSCMD} ${GRPOWN} -src ibtab.2300  -F /etc  ibtab
	${INSCMD} ${GRPOWN} -src inittab.2300 -F /etc inittab
	${INSCMD} ${GRPOWN} -src termcap.2300 -F /etc termcap
	${INSCMD} ${GRPOWN} -F /etc "mkboot flp mkrecover rootcopy \
			flpuxfer flppatch"

etc_clean:
	rm -f *.o a.out core y.tab.c y.tab.h

etc_clobber: 
	rm -f brc
	rm -f *.o a.out core y.tab.c y.tab.h
	rm -f $(PROGS) mkbootflp mkrecover rootcopy flpuxfer \
	      flppatch ${BUSYFILES}

.sh:
	cp $< $@ ; chmod 0555 $@

ncheck: ncheck.c $(LIBFSTOOLS)
	$(CCF) -I$(LIBFSTOOLSDIR) -ZN -o $@ $@.c $(LDFLAGS) $(LIBFSTOOLS)

$(LIBFSTOOLS):
	cd $(LIBFSTOOLSDIR); make

$(YPPROGS):	$$@.c
	$(CCF) -o $@ $? $(LDFLAGS) $(YPLIBS)

clri:	clri.c $(LIBFSTOOLS)
	$(CCF) clri.c $(LDFLAGS) $(LIBFSTOOLS) -o $@

labelit: labelit.c $(LIBFSTOOLS)
	$(CCF) -DEFS -ZN labelit.c $(LIBFSTOOLS) -o $@

mklost+found: mklost.sh
	cp $? $@; chmod 755 $@

gethostident: gethostid.c
	$(CCF) gethostid.c $(LDFLAGS) -o gethostident

./Install: $$@.sh
	$(M4) -DSYSTEM=$(SYSTEM) $@.sh | tr -s '\012' > $@

FRC:
