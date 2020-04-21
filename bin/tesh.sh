#! /bin/csh
# tesh - Terminal emulator shell for the 2300 and 3010.  It is started 
#	 by /bin/su from /etc/inittab.  It merely sets some environment 
#	 variables and exec-s the terminal emulator.

set t3279 = /usr/bin/t3279
if ( -c /dev/pxd && { ( cat /dev/null > /dev/pxd ) >& /dev/null } ) then
	set termemul = $t3279
else
	set termemul = /bin/wsiris
endif
setenv PATH :/usr/bin:/bin:/etc
setenv SHELL /bin/csh		# always, per doc. and guest passwd entry
if ( ! $?TERM )  setenv TERM wsiris
setenv HOME /			# wsiris config file is in /
cd /	
clear
# getty is never run, so set the bits which it would have, but leave the
# keyboard chars alone.
# 
stty ixon brkint ignpar tab3
if ( -x $termemul ) then
	exec $termemul $*
else
	echo $termemul not executable
	exec su
endif
