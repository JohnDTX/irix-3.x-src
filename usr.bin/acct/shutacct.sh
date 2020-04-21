#! /bin/sh
#	@(#)shutacct.sh	1.2 of 3/31/82	#
#	"shutacct [arg] - shuts down acct, called from /etc/shutdown"
#	"whenever system taken down"
#	"arg	added to /etc/wtmp to record reason, defaults to shutdown"
PATH=/usr/lib/acct:/bin:/usr/bin:/etc
_reason=${1-"acctg off"}
acctwtmp  "${_reason}"  >>/etc/wtmp
turnacct off
