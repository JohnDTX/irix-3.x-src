#! /bin/sh
# Delete all memories of printers and return the lp system to a pristine state
# Copyright 1987 Silicon Graphics, Inc. - All Rights Reserved


lp=/usr/spool/lp

set `id`
if test "$1" != "uid=0(root)" ; then
    echo "you must be logged in as root to run this program"
    exit 1
fi

/usr/lib/lpshut
cat /dev/null > $lp/pstatus
cat /dev/null > $lp/qstatus
cat /dev/null > $lp/outputq
rm -rf $lp/member
rm -rf $lp/request
rm -rf $lp/interface
rm -rf $lp/class
rm -rf $lp/etc/log
mkdir $lp/member
mkdir $lp/request
mkdir $lp/interface
mkdir $lp/class
mkdir $lp/etc/log
chown lp $lp/member $lp/request $lp/interface $lp/class $lp/etc/log
chgrp lp $lp/member $lp/request $lp/interface $lp/class $lp/etc/log
chmod 755 $lp/member $lp/request $lp/interface $lp/class $lp/etc/log
rm -rf $lp/transcript/*log
/usr/lib/lpadmin -d""
/usr/lib/lpsched

echo "All is well."
