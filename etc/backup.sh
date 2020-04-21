#! /bin/csh -f
#
# Interactive disk BACKUP program
#
# Run once every weekday in single-user mode with no other processes running.
#
# Once a week it does:
#	1. Runs FSCK on both file systems.
#	2. Does a full backup of each file system onto a tape via TAR, using
#	   alternate tapes on alternate weeks in case the system fails during
#	   backup.
# Daily (except on the day that the weekly backup is done) it does:
#	1. Does an incremental backup of both file systems onto a single tape
#	   via TAR, using alternate tapes on alternate days in case the system
#	   fails during backup. The backup is of all files that have changed
#	   since the full backup.
# $Source: /d2/3.7/src/etc/RCS/backup.sh,v $
# @(#)$Revision: 1.1 $
# $Date: 89/03/27 15:37:29 $
set	usr=/usr
set	usrdev=/dev/md0c
set x=(`id`)
if ( $x[1] != "uid=0(root)" ) then
	(echo -n "OOnnllyy r_o_o_t_ ";\
	  echo "can make system backups\!"|sed 's/[^ ]/&&/g')|ul|more
	echo 'BACKUP FAILED\!\!\!'|sed 's/[^ ]/&_/g'|ul|more
	echo -n 
	exit 1
endif
echo -n 'Are you in single-user mode with no cron, xnsd, etc.? (yes or no) '
set x="$<"
if ( "$x" != yes && "$x" != Yes && "$x" != y && "$x" != y ) then
	echo "System must be in single-user mode."
	echo 'BACKUP FAILED\!\!\!'|sed 's/[^ ]/&_/g'|ul|more
	echo -n 
	exit 1
endif
echo ""
set t1=/tmp/backa$$
set t2=/tmp/backb$$
ps -ef > $t1
(grep cron $t1;grep xnsd $t1;grep update $t1;grep vi $t1;grep cc $t1) > $t2
set x=(`wc -l $t2`)
if ( "$x[1]" != 0 ) then
	echo "These interfering processes are running:"
	cat $t2
	/bin/rm -f $t1 $t2
	echo "Try:"
	echo "	telinit [kK]"
	echo 'BACKUP FAILED\!\!\!'|sed 's/[^ ]/&_/g'|ul|more
	echo -n 
	exit 1
endif
/bin/rm -f $t1 $t2
echo "I am assuming that you use $usrdev for your $usr file system"
echo 'and that you have no other mountable file systems.'
echo -n 'Is this Correct? (yes or no) '
set x="$<"
if ( "$x" != yes && "$x" != Yes && "$x" != y && "$x" != y ) then
	echo "Get help\!"
	echo 'BACKUP FAILED\!\!\!'|sed 's/[^ ]/&_/g'|ul|more
	echo -n 
	exit 1
endif
echo ""
set x=(`ls -di $usr`)
set uinode=$x[1]
set x="`find /etc -name backfull.last -mtime -7 -print`"
if ( "$x" != "/etc/backfull.last" ) then
	if ( $uinode == 2 ) then
		/etc/umount $usrdev
		if ( $status != 0) then
			echo "Can't unmount $usr - someone is using it. "
			echo -n "Get help\!"
			echo 'BACKUP FAILED\!\!\!'|sed 's/[^ ]/&_/g'|ul|more
			echo -n 
			exit 1
		endif
	endif
	set was=2
	if ( -s /etc/backfull.last ) then
		set was=`cat /etc/backfull.last`
	endif
	@ is = 3 - $was
	echo "It's time for the WEEKLY FSCK"|sed 's/[^ ]/&&/g'|ul|more
	echo "Please respond appropriately..."
	fsck
	echo -n 
	echo "File System Check done"|sed 's/[^ ]/&_/g'|ul|more
	echo ""
	echo "It's time for the WEEKLY FULL BACKUP"|sed 's/[^ ]/&&/g'|ul|more
	echo "Insert tape for doing WEEKLY backup of ROOT #$is file system and "|sed 's/[WEKLYROT#12]/&_/g'|ul|more
	echo -n "hit RETURN:"
	set x="$<"
	echo -n "Weekly Full Backup of	ROOT	#$is " >> /etc/back.log
	date '+%D %H%M %a' >> /etc/back.log
	chmod 644 /etc/back.log
	tar -cv -C /etc back.log -C / .
	echo "Backup of ROOT file system done."|sed 's/[^ ]/&_/g'|ul|more
	echo -n 
	echo -n "Remove tape and hit RETURN:"
	set x="$<"
	echo ""
	/etc/mount $usrdev $usr
	if ( $status != 0 ) then
		echo "Can't mount $usr. Get help\!"
		echo 'BACKUP FAILED\!\!\!'|sed 's/[^ ]/&_/g'|ul|more
		echo -n 
		exit 1
	endif
	echo "Insert tape for doing WEEKLY backup of USR #$is file system and "|sed 's/[WEKLYUSR#12]/&_/g'|ul|more
	echo -n "hit RETURN:"
	set x="$<"
	echo -n "Weekly Full Backup of	$usr	#$is " >> /etc/back.log
	date '+%D %H%M %a' >> /etc/back.log
	chmod 644 /etc/back.log
	tar -cv -C /etc back.log -C $usr .
	echo "Backup of USR file system done."|sed 's/[^ ]/&_/g'|ul|more
	echo -n 
	echo -n "Remove tape and hit RETURN:"
	set x="$<"
	echo ""
	if ( $uinode != 2 ) then
		/etc/umount $usrdev
		if ( $status != 0 ) then
			echo "Can't unmount $usr. Get help\!"
			echo 'BACKUP FAILED\!\!\!'|sed 's/[^ ]/&_/g'|ul|more
			echo -n 
			exit 1
		endif
	endif
	echo $is > /etc/backfull.last
	chmod 644 /etc/backfull.last
	echo "The WEEKLY FULL BACKUP is done"|sed 's/[^ ]/&_/g'|ul|more
	exit 0
endif
set day="`date +%w`"
if ( $uinode != 2 ) then
	/etc/mount $usrdev $usr
	if ( $status != 0) then
		echo "Can't mount $usr - someone is using it. "
		echo -n "Get help\!"
		echo 'BACKUP FAILED\!\!\!'|sed 's/[^ ]/&_/g'|ul|more
		echo -n 
		exit 1
	endif
endif
echo "It's time for the DAILY INCREMENTAL BACKUP"|sed 's/[^ ]/&&/g'|ul|more
if ( $day == 2 || $day == 4 || $day == 6 || $day == 7 ) then
	set odd="TUESDAY/THURSDAY/SATSUN"
	set oddnum=E
else
	set odd="MONDAY/WEDNESDAY/FRIDAY"
	set oddnum=O
endif
(echo -n "Insert tape for doing ";echo "$odd DAILY backup and "|sed 's/[A-Z/]/&_/g')|ul|more
echo -n "hit RETURN:"
set x="$<"
echo -n "$odd	Incremental backup of / and $usr " >> /etc/back.log
date '+%D %H%M %a' >> /etc/back.log
chmod 644 /etc/back.log
cd / ; find . -newer /etc/backfull.last ! -type d -print | tar -cv -C /etc back.log -C / -
echo "$odd Backup of both file systems done."|sed 's/[^ ]/&_/g'|ul|more
echo -n 
echo -n "Remove tape and hit RETURN:"
set x="$<"
echo ""
if ( $uinode != 2 ) then
	/etc/umount $usrdev
	if ( $status != 0 ) then
		echo "Can't unmount $usr. Get help\!"
		echo 'BACKUP FAILED\!\!\!'|sed 's/[^ ]/&_/g'|ul|more
		echo -n 
		exit 1
	endif
endif
touch /etc/backincr${oddnum}.last
chmod 600 /etc/backincr${oddnum}.last
echo "The DAILY INCREMENTAL BACKUP is done"|sed 's/[^ ]/&_/g'|ul|more
exit 0
