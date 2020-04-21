#! /bin/sh
#
# usage: mktape [ rootdir ]
#
# Where "rootdir" is the pathname to a mounted root file system, if
# something other than the current root should be used.

stand=stand
root=/
size=
bpc=

if test $# -gt 0 
then
	if test ! -d $1 ; then echo $1: bad root directory ; exit 1 ; fi
	root=$1
else
	root=/
fi

cd $root

smt rewind

echo "mktape: file 1 = cpio of $stand"
cd $stand ; cpio -oah2 mdfex ipfex stfex

echo "mktape: file 2 = dd of /stand/swapfs_dd"
dd if=swapfs_dd of=/dev/rmt2 bs=250k count=4

echo "mktape: file 3 = cpio of the root file system"
rootlist=`ls -a . | egrep -v '^(usr|\.+)$'`
find $rootlist -print | cpio -oah2

echo "mktape: file 4 = cpio of /usr"
cd $d ; find * -print | cpio -oah2

smt rewind
echo "mktape: complete."
