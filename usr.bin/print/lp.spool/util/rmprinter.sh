#! /bin/sh
# Copyright 1986 Silicon Graphics, Inc. - All Rights Reserved

# usage: rmprinter PRINTER

set -u
if test $# != 1 ; then
	echo usage: rmprinter PRINTER
	exit 1
fi

PRINTER=$1
export PRINTER

set `id`
if test "$1" != "uid=0(root)" ; then
    echo "you must be logged in as root to run this program"
    exit 1
fi


# shut down the printer
/usr/lib/reject -r"printer being deleted" $PRINTER
disable -c -r"printer being deleted" $PRINTER
cancel $PRINTER
/usr/lib/lpshut
/usr/lib/lpadmin -x$PRINTER
/usr/lib/lpsched

# remove the device
rm -f /dev/${PRINTER}

# remove log and options files for postscript and other printers
PDIR=/usr/spool/lp/transcript
SDIR=/usr/spool/lp/etc/log
export PDIR
rm -rf ${PDIR}/${PRINTER}-log* ${PDIR}/${PRINTER}.opt ${SDIR}/${PRINTER}-log*

# report what we have done
echo $PRINTER removed - lp status is now:
lpstat -t

exit 0
