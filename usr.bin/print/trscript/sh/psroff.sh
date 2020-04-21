#! /bin/sh
# transcript/sh/psroff.sysv
# Copyright (c) 1985 Adobe Systems Incorporated
# PostScript is a trademark of Adobe Systems Incorporated
# RCSID: $Header: /d2/3.7/src/usr.bin/print/trscript/sh/RCS/psroff.sh,v 1.1 89/03/27 18:20:25 root Exp $
#
# run ditroff in an System V environment to print on a PostScript printer
#
# pstroff - ditroff | psdit [| lp]
#

ditroff=troff
psdit=psdit
nospool= dopt= fil= spool= dit=
printer=-d${LPDEST-PostScript}
while test $# != 0
do	case "$1" in
	-t)	nospool=1 ;;
	-Tpsc)	;;
	-T*)	echo only -Tpsc is valid 1>&2 ; exit 2 ;;
	-n*|-m|-w|-s)	spool="$spool $1" ;;
	-r|-h)	spool="$spool -o$1" ;;
	-d*)	printer=$1 ;;
	-)	fil="$fil $1" ;;
	-*)	dopt="$dopt $1" ;;
	*)	fil="$fil $1" ; jobname=${jobname-$1} ;;
	esac
	shift
done
if test "$jobname" = "" ; then
	jobname="Troff"
fi
spool="lp $printer -t'$jobname' $spool"
if test "$fil" = "" ; then
	fil="-"
fi
dit="$ditroff -Tpsc $dopt $fil "

if test "$nospool" = "1" ; then
	$dit | $psdit
else
	$dit | $psdit | $spool
fi
