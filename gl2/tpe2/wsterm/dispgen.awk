##########################################################################
#									 #
# 		 Copyright (C) 1985, Silicon Graphics, Inc.		 #
#									 #
#  These coded instructions, statements, and computer programs  contain  #
#  unpublished  proprietary  information of Silicon Graphics, Inc., and  #
#  are protected by Federal copyright law.  They  may  not be disclosed  #
#  to  third  parties  or copied or duplicated in any form, in whole or  #
#  in part, without the prior written consent of Silicon Graphics, Inc.  #
#									 #
##########################################################################

#
#	This awk script is used to generate of declaration section of
#	dispatch.h.
#

BEGIN {
	print "/*\n**\tDispatch table\n*/\n"
	print "#define NULL 0\n"
	print "extern int bogus();"
	print "extern int spl_feedback();"
	print "extern int spl_pick();"
	print "extern int spl_select();"
}

{
	funcname = substr($5,1,length($5)-1);
	if (funcname != "NULL" && funcname != "" && funcname != "bogus")
	    print "extern int",funcname "();"
}
END {
	print "\ndispatchEntry dispatch[] = {"
}
