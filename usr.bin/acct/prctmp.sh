#! /bin/sh
#	@(#)prctmp.sh	1.2 of 3/31/82	#
#	"print session record file (ctmp.h/ascii) with headings"
#	"prctmp file [heading]"
PATH=/usr/lib/acct:/bin:/usr/bin:/etc
(cat <<!; cat $*) | pr -h "SESSIONS, SORTED BY ENDING TIME"

		LOGIN	CONN.TIME(SECS)	START TIME	START TIME


!
