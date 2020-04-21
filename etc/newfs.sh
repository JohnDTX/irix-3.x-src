#! /bin/sh
#
# New arguments for filesystem name and volume name for labelit.
# usage: newfs dd#f [ -f filesystemname -v volname ]
#
# @(#)$Header: /d2/3.7/src/etc/RCS/newfs.sh,v 1.1 89/03/27 15:38:22 root Exp $
# $Log:	newfs.sh,v $
# Revision 1.1  89/03/27  15:38:22  root
# Initial check-in for 3.7
# 
# Revision 1.4  85/11/04  13:32:35  fong
# modified for compatibility with efs
# 
# Revision 1.3  85/04/11  17:44:32  chase
# Added labelit call for the file systems.
# Per SCR 600.
# 
dev=
filesystem=
volname=
MKFS=etc/mkfs.bell
if ( test ! -x /$MKFS ) then
	MKFS=etc/mkfs
fi

case $# in
  5) ;;
  1) ;;
  *) echo 'usage: newfs dd#f (eg: md1c) [ -f filesystemname -v volname ]'
     exit 1;;
esac
dev=$1
shift
while test $# -gt 0
do
	case $1 in
	-f)	shift; filesystem=$1 ;;
	-v)	shift; volname=$1 ;;
	*)	echo "newfs: unrecognized option $1." ; exit 1 ;;
	esac
	shift
done

xx=`sgilabel $dev`
case $xx in
  '') exit 1;;
  *) ;;
esac
set $xx
case $1 in
  0) echo $dev: 'Zero size file system'; exit 1;;
esac
case $dev in
  *md*) dilv=10;;
  *) dilv=2;;
esac
ilv=${3:-$dilv}
echo "  /$MKFS" /dev/r$dev $1 $ilv $2'; ok? \c'
read ok
case $ok in
  y|yes) /$MKFS /dev/r$dev $1 $ilv $2;;
  *) echo "  aborted";;
esac
# call labelit if necessary
if test "$filesystem"
then
	labelit /dev/$dev $filesystem $volname
fi
