#! /bin/sh -xe
# flpuxfer - transfer the UNIX on the floppy to md0a or md0c
#
# Usage: flpuxfer {md0a|md0c}
#
case $1 in
	md0a) 	dev=100
		;;
	md0c)	dev=102
		;;	
	*)	set +x 
		echo Usage: flpuxfer '{md0a|md0c}'
		exit 1
		;;
esac
PATH=:/bin:/etc
export PATH
mkfs.bell /dev/$1 8364
mount /dev/$1 /mnt
tar cfb - 80 bin dev etc tmp | ( cd /mnt; tar xvfpb - 80)
cp vmunix /mnt
ln /mnt/vmunix /mnt/defaultboot
flppatch /mnt/vmunix $dev
