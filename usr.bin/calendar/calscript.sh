#! /bin/sh
# $Source: /d2/3.7/src/usr.bin/calendar/RCS/calscript.sh,v $
# @(#)$Revision: 1.1 $
# $Date: 89/03/27 17:43:38 $
rm -f $2 $3
/lib/cpp -P -Uunix -Um68000 -Uvax $4/calendar > $2
cat $1 | while read p
do
	egrep -e "$p" $2 >> $3
done
