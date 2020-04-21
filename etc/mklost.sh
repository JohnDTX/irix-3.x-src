#! /bin/csh
# $Source: /d2/3.7/src/etc/RCS/mklost.sh,v $
# @(#)$Revision: 1.1 $
# $Date: 89/03/27 15:38:10 $
set x=`pwd`
if ( $x:t == lost+found ) then
	echo Do "cd .." first.
	exit 1
endif
mkdir lost+found
chmod 700 lost+found
cd lost+found
echo creating slots...
touch {a,b,c,d,e,f,g,h,i,j}{0,1,2,3,4,5,6,7,8,9}
echo removing dummy files...
/bin/rm -f {a,b,c,d,e,f,g,h,i,j}{0,1,2,3,4,5,6,7,8,9}
/bin/ls -ld $x/lost+found
echo done
