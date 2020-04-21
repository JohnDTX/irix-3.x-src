#! /bin/sh
#	@(#)prdaily.sh	1.3 of 3/31/82	#
#	"prdaily	prints daily report"
#	"last command executed in runacct"
#	"if given a date mmdd, will print that report"
PATH=/usr/lib/acct:/bin:/usr/bin:/etc

if [ `expr "$1" : [01][0-9][0-3][0-9]` -eq 4 ]; then
	cat /usr/adm/acct/sum/rprt$1
	exit
fi

_sysname="`uname`"
_nite=/usr/adm/acct/nite

cd ${_nite}
(cat reboots; echo ""; cat lineuse) | pr -h "DAILY REPORT FOR ${_sysname}"  

prtacct daytacct "DAILY USAGE REPORT FOR ${_sysname}"  
pr -h "DAILY COMMAND SUMMARY" daycms
pr -h "MONTHLY TOTAL COMMAND SUMMARY" cms 
pr -h "LAST LOGIN" -3 ../sum/loginlog  
