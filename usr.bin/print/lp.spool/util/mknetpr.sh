#!/bin/sh
# usage: mknetpr PRINTER NETADDR NETPRINTER

# mknetpr takes PRINTER NETADDR NETPRINTER
# 	where PRINTER is the local name you want for a remote printer
#	NETADDR is the machine the real printer is on; NETPRINTER
#	is the name of the printer on that machine.

if [ $# != 3 ] ; then
	echo "usage: mknetpr PRINTER NETADDR NETPRINTER"
	exit 1
fi

SPOOLUSER=lp
SPOOLGROUP=bin

# SPOOLDIR is the top-level spooling directory

SPOOLDIR=/usr/spool/lp


PCLASS="NetClass"
PRINTER=$1
NETADDR=$2
NETPRINTER=$3
#export PRINTER NETADDR NETPRINTER

set `id`
if test "$1" != "uid=0(root)" ; then 
    echo "you must be logged in as root to run this program"
    exit 1
fi

xPRIMARY_RCMDx=rsh
xPRIMARY_RCPYx=rcp
xPRIM_TCP_USERx="-l lp"
xPRIMARY_RM_HOSTx="lp@$NETADDR"

xSECOND_RCMDx=xx
xSECOND_RCPYx=xcp
xSECOND_TCP_USERx=
xSECOND_RM_HOSTx="$NETADDR"

# see if the remote host can listen before we get into the installation
host=$NETADDR

# find out if we're TCP or XNS
rcmd="rsh"
rcpy="rcp"
tcpuser="-l lp"

if test -x /etc/havetcp \
    && /etc/havetcp ; then
   rmhost="lp@$host"
   protocol="TCP"
   altprotocol="XNS"
else
   echo "TCP is not operational on this machine, trying XNS."
   rcmd="xx"
   rcpy="xcp"
   tcpuser=
   rmhost="$host"
   protocol="XNS"
   altprotocol="TCP"
fi

echo ""
echo "	Testing connection to print server using $protocol."

response=`$rcmd $host $tcpuser hostname`
val=$?
if [ $val -ne 0 ] ; then
    if [ $rcmd="rsh" ] ; then
	echo ""
	echo "Failed to connect using TCP, trying XNS."
	rcmd="xx" ; rcpy="xcp" ; tcpuser= ; rmhost="$host"
	response=`$rcmd $host $tcpuser hostname`

	if [ $? -ne 0 ] ; then
	    echo "Host \"$host\" does not respond using XNS nor TCP at this time."
	    echo "Make sure \"$host\" can respond."
	    exit 1
	fi

	echo "Succeeded with XNS, proceeding."
	echo ""
	protocol="XNS"
	altprotocol="TCP"
	xPRIMARY_RCMDx=xx
	xPRIMARY_RCPYx=xcp
	xPRIM_TCP_USERx=
	xPRIMARY_RM_HOSTx=$NETADDR

	xSECOND_RCMDx=rsh
	xSECOND_RCPYx=rcp
	xSECOND_TCP_USERx="-l lp"
	xSECOND_RM_HOSTx=lp@$NETADDR
    else
	echo "Host \"$host\" does not respond using XNS at this time."
	echo "TCP is not operational"
    fi

fi


# set up the device itself
rm -f /dev/${PRINTER}
chmod o+w /dev/null
ln /dev/null /dev/${PRINTER}

# create NETWORK directory log files
PDIR=${SPOOLDIR}/etc/log
export PDIR
if test ! -d $PDIR ; then
	mkdir $PDIR
fi

cp /dev/null ${PDIR}/${PRINTER}-log
chown $SPOOLUSER $PDIR $PDIR/$PRINTER-log
chgrp $SPOOLGROUP $PDIR $PDIR/$PRINTER-log
chmod 775 $PDIR
chmod 664 $PDIR/$PRINTER-log

# create the interface spec for this printer
sed 	-e s,XHOSTX,$NETADDR,g \
	-e s,XPRINTERX,$NETPRINTER,g \
	-e s,XCLASSX,$PCLASS,g \
	-e s,XLOGX,$PDIR/$PRINTER-log,g \
	-e s,XLOCALPRINTERX,$PRINTER,g \
	-e s,xPRIMARY_RCMDx,$xPRIMARY_RCMDx,g \
	-e s,xPRIMARY_RCPYx,$xPRIMARY_RCPYx,g \
	-e s,xPRIM_TCP_USERx,"$xPRIM_TCP_USERx",g \
	-e s,xPRIMARY_RM_HOSTx,"$xPRIMARY_RM_HOSTx",g \
	-e s,xSECOND_RCMDx,$xSECOND_RCMDx,g \
	-e s,xSECOND_RCPYx,$xSECOND_RCPYx,g \
	-e s,xSECOND_TCP_USERx,"$xSECOND_TCP_USERx",g \
	-e s,xSECOND_RM_HOSTx,"$xSECOND_RM_HOSTx",g \
	-e s,XPROTOCOLX,$protocol,g \
	-e s,XALTPROTOCOLX,$altprotocol,g \
	../lib/netface >netface

/usr/lib/lpshut
if [ -r ${SPOOLDIR}/request/$PRINTER ] ; then
	/usr/lib/lpadmin -x$PRINTER
fi
/usr/lib/lpadmin -p$PRINTER -c${PCLASS} -h -inetface -v${PDIR}/${PRINTER}-log
rm -rf netface
chown $SPOOLUSER ${SPOOLDIR}/request/${PCLASS} ${SPOOLDIR}/request/$PRINTER
chgrp $SPOOLGROUP ${SPOOLDIR}/request/${PCLASS} ${SPOOLDIR}/request/$PRINTER
chmod 755 ${SPOOLDIR}/request/${PCLASS} ${SPOOLDIR}/request/$PRINTER
/usr/lib/lpsched
/usr/lib/accept $PRINTER ${PCLASS}
enable $PRINTER

# report what we have done
echo "This is what you just created for ${PRINTER}:"
ls -lF /dev/null /dev/${PRINTER}
ls -ldF ${PDIR}
ls -lF ${PDIR}/${PRINTER}-log
lpstat -t

exit 0
