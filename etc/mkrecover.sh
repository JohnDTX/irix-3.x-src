#! /bin/sh
#
# mkrecover - make a kernel which will run off of md0c.  It's named 'recover'
#
# Usage: mkrecover kernel
#
unixc=recover
cp $1 $unixc
case `/bin/uname -t` in
2300)
	KBASE=c00400 KLIMIT=cfffff
	;;
2300T|3010)
	KBASE=0x20000000 KLIMIT=0x2fffffff
	;;
esac

adb -w $unixc << EOF
?m $KBASE $KLIMIT
rootfs?w 2
nswap?W 0d12240
$q
EOF
