#! /bin/sh
#	@(#)startup.sh	1.2 of 3/31/82	#
#	"startup (acct) - should be called from /etc/rc"
#	"whenever system is brought up"
PATH=/usr/lib/acct:/bin:/usr/bin:/etc
acctwtmp "acctg on" >>/etc/wtmp
turnacct on
#	"clean up yesterdays accounting files"
remove
