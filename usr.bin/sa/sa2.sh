#! /bin/sh
#	sa2.sh 1.1 of 4/7/82
#	@(#)sa2.sh	1.1
DATE=`date +%d`
RPT=/usr/adm/sa/sar$DATE
DFILE=/usr/adm/sa/sa$DATE
ENDIR=/usr/bin
cd $ENDIR
$ENDIR/sar $* -f $DFILE > $RPT
find /usr/adm/sa \( -name 'sar*' -o -name 'sa*' \) -mtime +7 -exec rm {} \;
