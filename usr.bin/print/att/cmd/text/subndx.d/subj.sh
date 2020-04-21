#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#		Copyright (c) 1984 AT&T
#		  All Rights Reserved
#     THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#   The copyright notice above does not evidence any actual
#   or intended publication of such source code.
trap  'rm -f /tmp/sbj*$$; exit' 1 2 3 15
UL=/usr/lib/dwb
d=d
if test "$1" = "-e"; then
	shift
	d=
fi
if test $# -eq 0 -o "$1" = "-"; then
	arg=/tmp/subj0$$
	cat - >$arg
else
	arg=$*
fi
COUNT=0
for i in $arg
do
	if test ! \( -r $i \); then
        	echo "subj: subj aborted, cannot read the file  $i" >&2
        	echo "Please check to see that $i exists and is readable" >&2
        	exit 1
	fi
	TEXT=/tmp/sbj$$
	sed -e "s/[\][*]F//g" -e "s/[\]f(..//g" -e "s/[\]f.//g" -e "s/[\]s[+-][0-9]//g" \
	 -e "s/[\]s[0-9]\{1,2\}//g" $i > $TEXT
	
	HFILE=/tmp/sbj1$$
	SFILE=/tmp/sbj2$$
	CFILE=/tmp/sbj3$$

	$UL/sbj1 $TEXT $HFILE $SFILE $CFILE $d > /dev/null
	sed  "/[.][ 	]*is[.]$/s// is./" $HFILE |
		$UL/parts -o    | $UL/sbj2 | sort -udf -o $HFILE
	$UL/parts -o  $SFILE | $UL/sbj3 | sort -udf -o $SFILE
	
	FINAL=/tmp/sbj8$COUNT$$
	
	sort -udf  $CFILE -o  $CFILE
	
	cat -s  $HFILE $SFILE $CFILE | sort -udf > $FINAL
	rm -f /tmp/sbj[1-3]$$
	COUNT=`expr "$COUNT" + 1`
done
cat /tmp/sbj8*$$ |
	sort -udf -o /tmp/sbj0$$
cat /tmp/sbj0$$
rm -f /tmp/sbj*$$
echo 'subj completed' >&2
