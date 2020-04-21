#! /bin/sh
#	@(#)chargefee.sh	1.2 of 3/31/82	#
#	"chargefee login-name number"
#	"emits tacct.h/ascii record to charge name $number"
cd /usr/adm
PATH=/usr/lib/acct:/bin:/usr/bin:/etc
if test $# -lt 2; then
	echo "usage: chargefee name number"
	exit
fi
_entry="`grep \^$1: /etc/passwd`"
if test -z "${_entry}"; then
	echo "can't find login name $1"
	exit
fi
case "$2"  in
-[0-9]*|[0-9]*);;
*)
	echo "charge invalid: $2"
	exit
esac

if test ! -r fee; then
	nulladm fee
fi
_userid=`echo "${_entry}" | cut -d: -f3`  # get the UID
echo  "${_userid} $1 0 0 0 0 0 0 0 0 0 0 $2"  >>fee
