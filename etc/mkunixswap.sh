#! /bin/sh
#
# mkunixswap kernel
#
set -e

dev=
kernel=
size=

if test ! -f "$1"
then
	echo "  usage: mkunixswap kernel"
	exit 1
fi

kernel=$1

cp $kernel ${kernel}swap
set `devnm /`
dev=$1
#  echo "dev=$dev"
if test $dev = 'md0a'
then
	set `sgilabel md0b`
	size=$1
else
	set `sgilabel ip0b`
	size=$1
fi
size=`expr $size / 2`
size=`expr $size - 1000`

adb -w ${kernel}swap << EOF
?m c00400 cfffff
\$d
nswap?W $size
swplo?W 1000
\$q
EOF
