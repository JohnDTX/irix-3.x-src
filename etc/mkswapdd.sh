#! /bin/sh
#
#
# This will create the swapfs_dd file in /stand
#

dev=
set `devnm /`
dev=$1

if test $dev = 'md0a'
then
	echo "  swapfs: ST-506 Disk Drives"
	echo "  swapfs: dd if=/dev/rmd0b of=/stand/swapfs_dd bs=250k count=4"
	dd if=/dev/rmd0b of=/stand/swapfs_dd bs=250k count=4
elif test $dev = 'ip0a'
then
	echo "  swapfs: SMD Disk Drives"
	echo "  swapfs: dd if=/dev/rip0b of=/stand/swapfs_dd bs=250k count=4"
	dd if=/dev/rip0b of=/stand/swapfs_dd bs=250k count=4
else
	echo "  swapfs: Unrecognized root file system"
	exit 1
fi
