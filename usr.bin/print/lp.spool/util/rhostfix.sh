#! /bin/sh
# Add a line that has root as the user for each line with lp found
# They should all be lp to start.
# Copyright 1987 Silicon Graphics, Inc. - All Rights Reserved


set `id`
if test "$1" != "uid=0(root)" ; then
    echo "you must be logged in as root to run this program"
    exit 1
fi

if [ -n "$debug" ] ; then
	file=rhosts
else
	file=/usr/spool/lp/.rhosts
fi

if [ -n "$debug" ] ; then
	echo "Working on file $file"
fi

if [ ! -r $file ] ;then
	echo "All is well."
	exit 0
fi

cnt=`grep -c root $file`

if [ $cnt != 0 ] ; then
    echo "This program should only be run once, and has already been run."
    exit 1
fi


ed - $file << '__JUNK__'
$ka
1,$t$
'a
+
.,$s/lp$/root/g
w
q
__JUNK__

	echo "All is well."
