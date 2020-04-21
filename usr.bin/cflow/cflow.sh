#! /bin/sh
#	@(#)cflow.sh	1.3
#	3.0 SID #	1.2
# @(#)$Header: /d2/3.7/src/usr.bin/cflow/RCS/cflow.sh,v 1.1 89/03/27 17:43:43 root Exp $
INVFLG=
DFLAG=
IFLAG=
DIR=/usr/lib
LINT1=/usr/lib/lint/lint1
TMP=/usr/tmp/cf.$$
TMPG=$TMP.g
trap "rm -f $TMP.?; kill $$" 1 2 3
echo "" >$TMP.g
while [ "$1" != "" ]
do
	case "$1" in
	-r)
		INVFLG=1
		;;
	-d*)
		DFLAG=$1
		;;
	-i*)
		IFLAG="$IFLAG $1"
		;;
	-f)
		cat $2 </dev/null >>$TMPG
		shift
		;;
	-g)
		TMPG=$2
		if [ "$TMPG" = "" ]
		then
			TMPG=$TMP.g
		fi
		shift
		;;
	-[IDU]*)
		o="$o $1"
		;;
	*.y)
		yacc $1
		sed -e "/^# line/d" y.tab.c > $1.c
		/lib/cpp $o $1.c | $LINT1 -H$TMP.j 2>/dev/null \
			| $DIR/lpfx $IFLAG >>$TMPG
		rm y.tab.c $1.c
		;;
	*.l)
		lex $1
		sed -e "/^# line/d" lex.yy.c > $1.c
		/lib/cpp $o $1.c | $LINT1 -H$TMP.j 2>/dev/null \
			| $DIR/lpfx $IFLAG >>$TMPG
		rm lex.yy.c $1.c
		;;
	*.c)
		/lib/cpp $o $1 | $LINT1 -H$TMP.j 2>/dev/null \
			| $DIR/lpfx $IFLAG >>$TMPG
		;;
	*.i)
		$LINT1 -H$TMP.j 2>/dev/null | $DIR/lpfx >>$TMPG
		;;
	*.s)
		a=`basename $1 .s`
		as -o $TMP.o $1
		nm -pg $TMP.o | $DIR/nmf $a $1 >>$TMPG
		;;
	*.o)
		a=`basename $1 .o`
		nm -pg $1 | $DIR/nmf $a $1 >>$TMPG
		;;
	*)
		echo $1 "-- cflow can't process - file skipped"
		;;
	esac
	shift
done
if [ "$INVFLG" != "" ]
then
	grep "=" $TMPG >$TMP.q
	grep ":" $TMPG | $DIR/flip >>$TMP.q
	sort <$TMP.q >$TMPG
	rm $TMP.q
fi
$DIR/dag $DFLAG <$TMPG
rm -f $TMP.?
