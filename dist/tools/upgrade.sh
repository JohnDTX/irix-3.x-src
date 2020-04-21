#! /bin/sh
# usage: upgrade root usr
# Root and usr are block device file names (path suffixes, not pathnames).
#
# This script expects to find hiqprom.r and rhqprom.r in /stand, the swaproot
# stand-alone directory.
#
root=$1
usr=$2
case $root in
ip0a)				# 2500s
	disk=ip0
	boot=ip0g
	patch=/ippatch
	;;
md0a)				# 2300s and 2400s
	disk=md0
	boot=md0g
	patch=/mdpatch
esac

#
# Boot partition size in sectors (4 cylinders on 2[45]00, 7 on 2300).
# Allocate a physical block's worth of inodes (512/sizeof(inode) is 8).
# The reasoning behind these numbers accounts for:
#	bell superblock
#	at least three inodes (for the following)
#	boot root directory
#	two prom images (each 64k or 128 sectors)
#	indirect blocks
# These numbers allow some room for future growth.
#
bootsize=476
bootinodes=8

#
# Compute new partition sizes from existing sizes (iff not already created).
#
set `sgilabel $boot`
if test $1 = 0
then
	set `sgilabel $disk | grep Swap`
	swapbase="`echo $2 | sed 's/(//'`"
	swapsize="`echo $4 | sed 's/(//'`"
	bootbase="`expr $swapbase + $swapsize - $bootsize`"
	swapsize="`expr $swapsize - $bootsize`"
	if test $swapsize -le 0
	then
		echo "cannot upgrade: swap partition too small" ; exit 1
	fi
	
	# create bootroot on "g"; patch running kernel to know about it
	sgilabel -p b:$swapbase,$swapsize \
		 -p g:$bootbase,$bootsize \
		 -r g $disk
	$patch g $bootbase $bootsize
fi

bootdev=/dev/$boot
mkfs.bell $bootdev $bootsize:$bootinodes
mkdir /boot
mount $bootdev /boot

cp /stand/hiqprom.r /boot/mon
cp /stand/rhqprom.r /boot/defaultboot
ln /boot/defaultboot /boot/vmunix

umount $bootdev
