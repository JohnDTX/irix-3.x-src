#! /bin/sh
#
# Copyright (c) 1985 Adobe Systems Incorporated
# Modified by Glen Williams for SGI July 25, 1986
# 	Copyright 1986 Silicon Graphics, Inc. - All Rights Reserved

# usage: mkPS PRINTER TTY

# mkPS takes PRINTER TTY
# 	where PRINTER is the name by which you want a printer and 
#	TTY is the actual device (/dev/TTY) it is hooked up to

if test $# != 2 ; then
	echo "usage: mkPS PRINTER TTY (e.g., TTY can be ttyd2)"
	exit 1
fi

PRINTER=$1
TTY=$2
set `id`
if test "$1" != "uid=0(root)" ; then
    echo "you must be logged in as root to run this program"
    exit 1
fi

export PRINTER TTY

if test -r ../config ; then
	. ../config
else
	echo "../config file missing!"
	exit 1
fi	

if test -r ../printer ; then
	. ../printer
else
	echo "printer file missing!"
	exit 1
fi

echo "Here is the /etc/inittab line for $TTY"
echo "You may need to edit it"
fgrep "${TTY}" /etc/inittab

# set up the device itself
rm -f /dev/${PRINTER}
ln /dev/${TTY} /dev/${PRINTER}
chown ${SPOOLUSER} /dev/${PRINTER}
chgrp ${SPOOLGROUP} /dev/${PRINTER}
chmod 660 /dev/${PRINTER}

# find out what stty string to use
CODE=`(stty cs8 9600 cread -clocal -ignbrk brkint -parmrk \
	inpck -istrip -inlcr -igncr -icrnl -iuclc ixon -ixany ixoff \
	-opost -isig -icanon -xcase \
	-echo -echoe -echok -echonl min \^a time \^d 
	stty -g ) </dev/${PRINTER}`
export CODE

# create a transcript directory log files
PDIR=${SPOOLDIR}/transcript
export PDIR
if test ! -d $PDIR ; then
	mkdir $PDIR
fi
rm -rf ${PDIR}/${PRINTER}-log*

cp /dev/null ${PDIR}/${PRINTER}-log
cp /dev/null ${PDIR}/${PRINTER}.opt
chown $SPOOLUSER $PDIR $PDIR/$PRINTER-log ${PDIR}/${PRINTER}.opt
chgrp $SPOOLGROUP $PDIR $PDIR/$PRINTER-log ${PDIR}/${PRINTER}.opt
chmod 775 $PDIR
chmod 664 $PDIR/$PRINTER-log ${PDIR}/${PRINTER}.opt

# create the interface spec for this printer
sed 	-e s,XPSLIBDIRX,${PSLIBDIR},g \
	-e s,XPSTEMPDIRX,${PSTEMPDIR},g \
	-e s,XCODEX,${CODE},g \
	../lib/psinterface >psinterface

/usr/lib/lpshut
/usr/lib/lpadmin -x$PRINTER
/usr/lib/lpadmin -p$PRINTER -cPostScript -h -ipsinterface -v${PDIR}/${PRINTER}-log
/usr/lib/lpsched
/usr/lib/accept $PRINTER PostScript
enable $PRINTER

# report what we have done
echo "Here are the goods on ${PRINTER}:"
ls -lF /dev/${TTY} /dev/${PRINTER}
ls -ldF ${PDIR}
ls -lF ${PDIR}/${PRINTER}-log ${PDIR}/${PRINTER}.opt
lpstat -t

exit 0
