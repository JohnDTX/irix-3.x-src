#! /bin/sh
#
# This script "probes" disk devices to determine which controllers are
# configured.  It constructs fsck's filesystem checklist (/etc/checklist)
# and creates generic device links for the floppy and tape devices.
#

for disk in md0 ip0 si0
do
	root = /dev/${disk}a
	usr = /dev/${disk}c
	if test -b $root && ( > $root ) >& /dev/null )
	then
		echo $root >> $checklist
	fi
	if test -b $usr && { > $usr } >& /dev/null )
	then
		echo /dev/r${disk}c >> $checklist
	fi
done

#
# Depending on the model we decide if the sky floating point board
# should be downline loaded.  We must also make generic device links.
#
