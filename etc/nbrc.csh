#! /bin/csh
#
# This script "probes" disk devices to determine which controllers are
# configured.
#
#	Figure out which controllers are attached
#	Build checklist
#	Make generic device links
#
alias	canwrite	'( sleep 0 > \!* ) >& /dev/null'
alias	configured	'( test -b \!* -o -c \!* )'
set	dev = ./dev
set	disklist = ( )

foreach disk ( md0 ip0 si0 )
	set root = $dev/${disk}a
	if ( { configured $root } && { canwrite $root } ) then
		set disklist = ( $disklist $disk )
	endif
end
echo $disklist
foreach disk ( $disklist )
end
#	set usr = $dev/${disk}c
#	if ( -b $usr && { > $usr } >& /dev/null ) then
#		echo $dev/r${disk}c >> $checklist
#	endif

#
# Depending on the model we decide if the sky floating point board
# should be downline loaded.  We must also make generic device links.
#
