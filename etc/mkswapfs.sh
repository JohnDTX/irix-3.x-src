#! /bin/sh
#
# usage: mkswapfs [filesystem] [-10MB]
#
#
# This script will build the file system on the swap partition,
# if the default is used,
# and copy all the necessary goo to the file system.
# Some very important assumptions are made:
#	1. The kernel running supports swplo with swplo at 1000.
#	2. A directory called /mnt is present.
#

BINFILES="cpio echo login ls mkdir pwd sgilabel sh smt stty su sync tset"
ETCFILES="fsck getty gettydefs group init inittab ioctl.syscon labelit mkfs
	  mnttab mount newfs passwd rc rc.fs rc.gpib rc.tcp rc.xns
	  reboot ttytype umount mktape mkunixswap mkswapfs tabset/*"
tandon=0

set -e
dev=

cd /

#  echo "args=$#"
#  check the usage
#
if test $# -gt 2
then
	echo "mkswapfs: usage mkswapfs [filesystem] [-10MB]"
	exit 1
fi

# calculate the file system to build the swap fs on.
#
while test $# -gt 0
do
	case $1 in
		-10MB) tandon=1;;
		*) dev=$1
	esac
	shift
done

if test -z "$dev"
then
	cd /
	set `devnm /`
	dev=$1
	case $dev in
		*md*) dev='md0b';;
		*ip*) dev='ip0b';;
		*) echo "mkswapfs: unrecognized device $dev"; exit 1;
	esac
fi

#  echo "dev=$dev"
#  make sure that the device is a block device
#
if test -b /dev/$dev
then
	echo "mkswapfs: building root file system on /dev/$dev"
else
	echo "mkswapfs: /dev/$dev is not a block device"
	exit 1
fi

#  setting some variables for the mkfs command
#
m=2
n=
set `sgilabel $dev`
n=$2

dev='/dev/'$dev

#  now do the real work
#
#
sleep 5
if test $tandon -eq 0
then
	mkfs $dev 2000 $m $n
	labelit $dev mnt swapfs
	fsck $dev
else
	m=10
	mkfs $dev 4000 $m $n
	labelit $dev root tandon
	fsck $dev
fi

mount $dev /mnt

cd /mnt
mkdir bin etc stand usr mnt dev tmp lib lib/tabset

cd /bin ; cp $BINFILES /mnt/bin
cd /etc ; cp $ETCFILES /mnt/etc
cd /etc/swap ; cp termcap passwd /mnt/etc ; cp .profile /mnt/.profile
cp /dev/MAKEDEV /mnt/dev
cd /mnt/dev ; MAKEDEV -s
cd /mnt

#  just the special build for the 10MB disk
#
if test $tandon -eq 1
then
	cp /bin/xx /mnt/bin
	cp /bin/xcp /mnt/bin
	cp /bin/hostname /mnt/bin
	cp /etc/xnsd /mnt/etc
fi

#  now do the kernels
#
cp /vmunix /mnt/vmunix

cd /mnt

if test $tandon -eq 1
then
	echo "mkswapfs: tandon root file system is built"
else
	ln vmunix swapunix_md
	adb -w swapunix_md << 'EOF'
	?m c00400 cfffff
	rootfs?w 1
	swplo?W 0d1000
	nswap?W 0d7865
	$q
EOF
	cp vmunix swapunix_ip
	adb -w swapunix_ip << 'EOF'
	?m c00400 cfffff
	rootfs?w 1
	swplo?W 0d1000
	nswap?W 0d20120
	$q
EOF
fi

cp vmunix defaultboot
cd /
umount $dev
fsck $dev

dd if=/dev/r$dev of=/stand/swapfs_dd bs=250k count=4
