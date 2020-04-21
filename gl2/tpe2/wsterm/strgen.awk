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
#	This awk script is used to generate dispatch.h.
#
#   Note that its actions are bounded by search strings.  For instance,
#   only certain commands are executed on segments of the input file
#   (usually lib.prim) that are bounded by the strings #---------- ROUTINES
#   and #---------- END ROUTINES.  The number of dashes is important.
#	GEW 1/2/86
#

BEGIN	{
	gcmd = 0;
}
# special hacks
/:gflush\(/ { next }

# DO FOR EVERY LINE BRACKETED BY "---------- ROUTINES"...."---------- END ROUTINES"
/^#---------- ROUTINES/ , /^#---------- END ROUTINES$/ {
    if ( $0 ~ /^#/ || $0 ~ /^$/ ) { next }    # ignore comments and blank lines
	for ( nf=1 ; nf<=NF && substr($nf,1,1)!=")" ; nf++ ) { }
		# this line allows comments after the right paren in lib.prim

	if ( substr($1,length($1))!="(" ) {
		print "Missing \"(\" in "$1", "FILENAME" line",NR
		next
	}
	if ( substr($nf,1,1)!=")" ) {
		print "Missing \")\" in "$1", "FILENAME" line",NR
		next
	}

	printf "/* %3d */", gcmd
	gcmd++	

	if( substr($1,4,length($1)) == ":feedback(" ) {
		print "\t{ spl_feedback, \"V\", 0, 0, 0, 0 },"
		next
	}

	if( substr($1,4,length($1)) == ":pick(" ) {
		print "\t{ spl_pick, \"V\", 0, 0, 0, 0 },"
		next
	}

	if( substr($1,4,length($1)) == ":select(" ) {
		print "\t{ spl_select, \"V\", 0, 0, 0, 0 },"
		next
	}

  	returnsarray = 0;
  	retsvals = 0;

	for ( i=2 ; i<nf ; i++ ) {
		k = split($i,q,":")
		phys = substr($i,2,1);
		if( k==2 ) {
			if(phys>="A" && phys<="Z") {
			    returnsarray = 1;
			}
		} else if (phys>="A" && phys<="Z")
		     retsvals = 1;
	}
	h = split($1,p,":")
	if(substr(p[1],2,1) != "V")
	    retsvals = 1;
	printf "\t{ " 
	if(returnsarray) {
	    printf "ret_"	
	    retsvals = 0;
	}
	printf substr(p[h],1,length(p[h])-1)
	printf ", \""

	framesize = 0;
	if(returnsarray) {
		printf "V"
	} else {
		printf substr($1,2,1);
		for ( i=2 ; i<nf ; i++ ) {
			framesize += 4;
			k = split($i,q,":")
			if( k==1 ) {
				printf substr($i,2,1);
			} else {
			    	printf "a";
			}
		}
	}

	printf "\", "
	printf "0, 0"
	print ", " framesize ", " retsvals " },"
}
END {
	print "/* xxx */\t{ NULL, \"V\", 0, 0, 0, 0 }, "
	print "};"
}
