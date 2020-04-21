#! /bin/sh
#	spell program
# SCCS:		@(#)spell.sh	1.2
# B flags, D_SPELL dictionary, F files, H_SPELL history, S_SPELL stop, V data for -v
H_SPELL=${H_SPELL-/dev/null}
V=/dev/null
F= B=
L="sed -e \"/^[.'].*[.'][ 	]*nx[ 	]*\/usr\/lib/d\" -e \"/^[.'].*[.'][ 	]*so[ 	]*\/usr\/lib/d\" -e \"/^[.'][ 	]*so[ 	]*\/usr\/lib/d\" -e \"/^[.'][ 	]*nx[ 	]*\/usr\/lib/d\" "
trap "rm -f /tmp/spell.$$; exit" 0 1 2 13 15
for A in $*
do
	case $A in
	-v)	if /bin/pdp11
			then	echo -v option not supported on pdp11
				exit
		else	B="$B -v"
			V=/tmp/spell.$$
		fi ;;
	-a)	: ;;
	-b) 	D_SPELL=${D_SPELL-/usr/lib/spell/hlistb}
		B="$B -b" ;;
	-x)	B="$B -x" ;;
	-l)	L="cat" ;;
	+*)	if  LOCAL=`expr $A : '+\(.*\)' 2>/dev/null`;
		then if test ! -r $LOCAL;
			then echo "spell cannot read $LOCAL"; exit;
		     fi
		else echo "spell cannot identify local spell file"; exit;
		fi;;
	*)	F="$F $A"
	esac
	done
case $H_SPELL in
/dev/null)
	cat $F | eval $L | deroff -w | sort -u +0f +0 |
	/usr/lib/spell/spellprog ${S_SPELL-/usr/lib/spell/hstop} 1 |
	/usr/lib/spell/spellprog ${D_SPELL-/usr/lib/spell/hlista} $V $B |
	comm -23 - ${LOCAL-/dev/null}
	;;
*)
	cat $F | eval $L | deroff -w | sort -u +0f +0 |
	/usr/lib/spell/spellprog ${S_SPELL-/usr/lib/spell/hstop} 1 |
	/usr/lib/spell/spellprog ${D_SPELL-/usr/lib/spell/hlista} $V $B |
	comm -23 - ${LOCAL-/dev/null} |
	tee -a $H_SPELL
	who am i >>$H_SPELL 2>/dev/null
esac
case $V in
/dev/null)
	;;
*)	
	sed '/^\./d' $V | sort -u +1f +0
esac
