#! /bin/csh -f
# fixref - fix incorrecly formatted man page cross-references.
#
# Usage: fixref > fixref.out
#
# See fixref.sed for what things are fixed.  The fixed pages are put into
# a subdirectory `fixref' of each man[1-8] directory.  Diffs of each man
# page changed are sent to stdout.  This output should be reviewed before
# moving the pages from the fixref subdirectories.
#
# NOTE: fixref must be run from this directory.
# 
# Wed Apr 16 11:49:01 1986  Charles (Herb) Kuta at SGI  (olympus!kuta)
# 
# $Revision: 1.1 $
# $Date: 89/03/27 16:39:26 $

set tmp1=/tmp/fixref.$$a
set tmp2=/tmp/fixref.$$b
set here=`pwd`
set newdirname=fixref
onintr INTR

# System V sed doesn't like comments
sed '/^#/d' fixref.sed > $tmp2

foreach dir ( ../[ua]_man/man? ../u_man/man1/gl2 ../troff/[au]_man/man? )
  cd $dir
    set cwd=`echo $dir | sed 's/^...//'`
    rm -rf $newdirname
    mkdir $newdirname
    set filecnt = 0
    foreach f ( *.[1-8] *.[1-8][a-z] )
	# ignore pages that have things which are changed but shouldn't be
	set ok=1
	switch ( $f )
	case bs.1:
	case yacc.1:
	case dbtext.3g:
	case mkstr.1:
        case getcwd.3c:
	    set ok=0
	    breaksw
	endsw
	if ($ok) then
	    sed -f $tmp2 $f > $newdirname/$f
	    diff $f $newdirname/$f > $tmp1	
	    if ( -z $tmp1 ) then
		rm $newdirname/$f
	    else
		echo "*** $cwd/$f ***"
		cat $tmp1
		echo " "
		@ filecnt ++
	    endif
	endif
    end
    if ( $filecnt == 0 ) rmdir $newdirname
    cd $here
end
rm -f $tmp1 $tmp2
exit(0)

INTR:
rm -f $tmp1 $tmp2
exit(1)

