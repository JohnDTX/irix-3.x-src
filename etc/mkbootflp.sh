#! /bin/sh
# NAME
#	mkbootflp - make a bootable UNIX floppy
# SYNOPSIS
#	mkbootflp [rootdir]
#
# DESCRIPTION
#	make a blank file system on the floppy,
#	including
#		empty /tmp/ and /mnt/
#		bare-bones /etc/ and /bin/
#		subset /dev
#		small /vmunix, rooting on /dev/mf0a, swapping on /dev/md0b
#
#	there are kernel version dependencies.
#
#	NOTE: intended to be self-hosted, i.e. run on the terminal for which
#	the bootable floppy is being made.
#
root=${1-/}

case `uname -t` in
3*|2*T)
	FLPKERNEL=/kernels/3000.flp
	KBASE=20000000 KLIMIT=2fffffff
	;;
*)
	FLPKERNEL=/kernels/2400.flp
	KBASE=c00400 KLIMIT=cfffff
	;;
esac
MNT=/mnt
DISK=/dev/mf0a
ROOTDEV=0x200
SWAPDEV=0x101
NSWAP=0d12240
MKFS=etc/mkfs.bell MKFSARGS="$DISK 1280 16 4"
if ( test ! -x /$MKFS ) then
	MKFS=etc/mkfs MKFSARGS="-C 1280:64 -G 1 1 $DISK 1280"
fi

cd $root
trap "cd /; umount $DISK 2>/dev/null; exit" 0 2 3

find dev \( -type b -o -type c \) -print > /tmp/devlist
ex - << ! /tmp/devlist
g:dev/EXOS:d
g:dev/ttyn:d
g:dev/ttyp:d
g:dev/pty:d
g:dev/ib:d
g:dev/ttyw[1-9]:d
g:dev/tek:d
g:dev/pxd:d
g:dev/ip.[a-h]:d
g:dev/rip.[a-h]:d
g:dev/md.[bd-g]:d
g:dev/rmd.[bd-g]:d
g:dev/si.[a-h]:d
g:dev/rsi.[a-h]:d
g:dev/mt[34]:d
g:dev/rmt[34]:d
g:dev/ttyd[23]:d
w
q
!
devlist="`cat /tmp/devlist`"
rm -f /tmp/devlist

/$MKFS $MKFSARGS
mount $DISK $MNT
tar cfb - 80 - << ! | (cd $MNT; tar xvfpb - 80)
$devlist
etc/init
etc/mkfs
etc/mkfs.bell
etc/mount
etc/reboot
etc/flppatch
etc/flpuxfer
bin/cp
bin/echo
bin/ln
bin/mkdir
bin/mv
bin/pwd
bin/sgilabel
bin/sh
bin/sync
bin/tar
!
mkdir $MNT/tmp $MNT/mnt
cp $FLPKERNEL $MNT/vmunix

adb -w $MNT/vmunix << !
?m $KBASE $KLIMIT
rootdev?w $ROOTDEV
swapdev?w $SWAPDEV
nswap?W $NSWAP
!
ln $MNT/vmunix $MNT/defaultboot
