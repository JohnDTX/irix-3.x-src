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
#	This awk script generates the command name table from
#	the output of strgen.awk.
#

BEGIN {
	print "static char *cmdnametab[] = {"
}

{   
	funcname = substr($5,1,length($5)-1);
	if (funcname == "NULL")
	    print "    NULL,"
	else
	    printf "    \"%s\",\n", funcname;
}

END {
	print "};"
}
