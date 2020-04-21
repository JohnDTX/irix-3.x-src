#! /bin/sh
# rename9x - rename man page files and their RCS files to have basenames of
# 	     9 characters or less
#
# Usage rename9x file ...
#
# Actually, only the mv commands to do the renaming are generated.  This way
# they can be reviewed before the renaming is done.
#
# $Revision: 1.1 $
# $Date: 89/03/27 16:39:47 $

for f
do
    /bin/echo -n `dirname $f` `basename $f` 
    nroff -Tlp - $f << \! | col -b | newform -i
.de TH
 \\$1 \\$2
..
.de SH
.nx
..
.de so	\" ignore files that are just .so's
.sp
..
.de EQ	\" gobble up stuff between .EQ/.EN pairs
.di xx
..
.de EN
.di
..
.nf
.na
.po 0
.pl 1v
!
done | awk 'NF > 2 {print substr($3,1,9) "." $4; print $2; print $1}' \
     | sed 'y/ABCDEFGHIJKLMNOPQRSTUVWXYZ/abcdefghijklmnopqrstuvwxyz/;N;N;p;d' \
     | paste - - - | awk '$1 != $2 { if ( $3 != "." ) \
					printf "( cd " $3 "; "; \
			 	   print "mv", $2, $1,")" }'
exit 0
