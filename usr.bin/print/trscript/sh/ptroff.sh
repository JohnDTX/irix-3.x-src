#! /bin/sh
# sh/ptroff.sysv
# Copyright (c) 1985 Adobe Systems Incorporated
# RCSID: $Header: /d2/3.7/src/usr.bin/print/trscript/sh/RCS/ptroff.sh,v 1.1 89/03/27 18:20:27 root Exp $
#
# run old troff in a System V environment to print on a PostScript printer
#
# ptroff - otroff | pscat [| lp]

opt= spool= 
family=Times
offset="-y -31"
printer=-d${LPDEST-PostScript}
while test "$1" != ""
do	case "$1" in
	-F)	if test "$#" -lt 2 ; then
			echo '-F takes following font family name' 1>&2
			exit 1 
		fi
		family=$2 ; shift ;;
	-F*)	echo 'use -F familyname' 1>&2 ;
		exit 1 ;;
	-t)	nospool=1 ;;
	-n*|-m|-w|-s)	spool="$spool $1" ;;
	-r|-h)	spool="$spool -o$1" ;;
	-d*)	printer=$1 ;;
	-)	fil="$fil $1" ;;
	-*)	opt="$opt $1" ;;

	*)	fil="$fil $1" ; jobname=${jobname-$1} ;;
	esac
	shift
done
if test "$jobname" = "" ; then
	jobname="otroff"
fi
spool="$printer -t'$jobname' $spool"
if test "$fil" = "" ; then
	fil="-"
fi
troff="otroff -t -Tps $opt /usr/lib/font/ps/${family}.head $fil "
pscat="pscat $offset -F/usr/lib/font/ps/${family}.ct "

if test "$nospool" = "1" ; then
	$troff | $pscat
else
	$troff | $pscat | lp $spool
fi
