#! /bin/sh
# bring system up from single-user mode to multi-user mode

#
# Use run-level 3 for tcp systems, and run-level 2 for other systems.
#
if test -x /etc/havetcp && /etc/havetcp; then
	telinit 3
else
	telinit 2
fi

# prevent the shell from displaying a prompt
sleep 5
