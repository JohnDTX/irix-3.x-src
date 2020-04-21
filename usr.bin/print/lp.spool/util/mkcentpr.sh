#! /bin/sh
# Copyright 1986 Silicon Graphics, Inc. - All Rights Reserved
# usage: mkcentpr printer-type PRINTER

# mkcentpr takes PRINTER printer-type
# 	where PRINTER is the local name you want for the printer
#	printer-type is currently one of: mits5 (for mitsubishi G500),
#	tek (for tektronics 4692) or vers (for versatec)


if [ $# != 2 ] ; then
	echo "usage: mkcentpr printer-type PRINTER"
	echo "  printer-type is one of mits5, tek, vers, or seiko"
	echo "  where mits5 denotes the Mitsubishi G500"
	echo "  where tek denotes the Tektronix 4692"
	echo "  where vers denotes the Versatec ECP-42"
	echo "  and where seiko denotes the Seiko CH-5312"
	exit 1
fi

PRINTER=$2
MODEL=$1

set `id`
if test "$1" != "uid=0(root)" ; then
    echo "you must be logged in as root to run this program"
    exit 1
fi

SPOOLUSER=lp
SPOOLGROUP=bin

# SPOOLDIR is the top-level spooling directory

SPOOLDIR=/usr/spool/lp
PRINTLIB=/usr/lib/print

PCLASS="Parallel"
printdev=/dev/cent
driver=

case $MODEL in
  mits5)		driver=mprint;;
  vers)		driver=vprint; printdev=/dev/vp0;;
  tek)		driver=tprint;;
  seiko)	driver=sprint;;
  *)		echo "bad model specification: $MODEL"
		echo "choose from: mits5, vers, tek, seiko" ; exit 1;;
esac

if [ ! -x $PRINTLIB/$driver ] ; then
   echo "$PRINTLIB/$driver not executable"
   exit 1
fi

if [ ! -w $printdev ] ; then
   echo "$printdev not writeable"
   exit 1
fi

# set up the device itself
rm -f /dev/${PRINTER}
chmod o+w /dev/null
ln $printdev /dev/${PRINTER}

# create directory log files
PDIR=${SPOOLDIR}/etc/log

if test ! -d $PDIR ; then
	mkdir $PDIR
fi

cp /dev/null ${PDIR}/${PRINTER}-log
chown $SPOOLUSER $PDIR $PDIR/$PRINTER-log
chgrp $SPOOLGROUP $PDIR $PDIR/$PRINTER-log
chmod 775 $PDIR
chmod 664 $PDIR/$PRINTER-log

# create the interface spec for this printer
sed 	-e s,XSENDX,$PRINTLIB/$driver,g \
	-e s,XLOGX,$PDIR/$PRINTER-log,g \
	../lib/centface >centface

/usr/lib/lpshut
if [ -r ${SPOOLDIR}/request/$PRINTER ] ; then
	/usr/lib/lpadmin -x$PRINTER
fi
/usr/lib/lpadmin -p$PRINTER -c${PCLASS} -h -icentface -v${PDIR}/${PRINTER}-log

chown $SPOOLUSER ${SPOOLDIR}/request/${PCLASS} ${SPOOLDIR}/request/$PRINTER
/usr/lib/lpsched
/usr/lib/accept $PRINTER ${PCLASS}
enable $PRINTER

rm -f centface

# report what we have done
echo "This is what you just created for ${PRINTER}:"
ls -lF /dev/null /dev/${PRINTER}
ls -ldF ${PDIR}
ls -lF ${PDIR}/${PRINTER}-log
lpstat -t

exit 0
