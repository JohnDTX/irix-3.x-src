#! /bin/sh
#	@(#)dodisk.sh	1.2 of 3/31/82	#
# 'perform disk accounting'
PATH=:/bin:/usr/lib/acct/bin:/usr/bin:/etc
export PATH
_dir=/usr/adm
_pickup=acct/nite
PATH=/usr/lib/acct:/bin:/usr/bin:/etc
cd ${_dir}
date
find / -print | acctdusg >dtmp
date
sort +0n +1 -o dtmp dtmp
acctdisk <dtmp >disktmp
chmod 644 disktmp
chown adm disktmp
mv disktmp ${_pickup}/disktacct
