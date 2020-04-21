#! /bin/sh
# Copyright 1986 Silicon Graphics, Inc. - All Rights Reserved
# Execute once per client on a host machine (i.e., the machine with the
# physical printer attached.)  This is for TCP only.


if test $# != 1 ; then
	echo usage: addclient client_machine_name
	exit 1
fi

cm=$1

set `id`
if test "$1" != "uid=0(root)" ; then
    echo "you must be logged in as root to run this program"
    exit 1
fi



# if there is no home dir for lp, give it one
if grep -s '^lp:.*:/:$' /etc/passwd ; 
then
ed - /etc/passwd << '__JUNK__'
/^lp:/s/:\/:/:\/usr\/spool\/lp:\/bin\/csh/
w
q
__JUNK__
fi

# get the current home directory of the lp user
lpusr=`grep '^lp:' /etc/passwd | cut -d: -f6`

if [ ! -r $lpusr/.rhosts ] ; then
touch $lpusr/.rhosts
chown lp $lpusr/.rhosts
chgrp lp $lpusr/.rhosts
fi

if grep -s $cm $lpusr/.rhosts ; then 
	echo "$cm is already a client"
else
	echo "$cm lp" >> $lpusr/.rhosts
	echo "$cm root" >> $lpusr/.rhosts
	echo Added \"$cm lp\" to "$lpusr/.rhosts"
fi
