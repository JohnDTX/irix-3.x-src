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
#   This awk script does most of the work to generate the remote graphics
#   libraries (stubs).  See the Makefile in this directory
#   for how it it used and what is required to complete the stubs and
#   header file generation.
#
#   Also added now is generation of a set of function and subroutine
#   definitions (in tempmandefs.c) to be checked against the graphics
#   library manual.  RDG, Feb 85
#
#   The creation of rgl.h has been commented out.  We don't want to have
#   to change include file names when moving from workstation to remote 
#   host.  CSK 6/5/85
#
#   Note that its actions are bounded by search strings.  For instance,
#   only certain commands are executed on segments of the input file
#   (usually lib.prim) that are bounded by the strings #---------- ROUTINES
#   and #---------- END ROUTINES.  The number of dashes is important.
#	GEW 2/5/86
#
BEGIN {
    cmd = 0		# graphics function call number
    cl = "lib.c"
#   ch = "rgl.h"
    cd = "mandefs.cr"

# print headings for files

    print "#include \"decl.h\"" >cl
#   print "/*\n**      function declarations\n*/\n" >ch

    printf "\nDefinitions of 'c' graphics functions & subroutines " >cd
    print "to check against manual\n" >cd
    print "Notes:\n" >cd
    print "	(1) Arrays abc[x][y] in the manual may be presented here as" >cd
    print "	    abc[x*y] or abc[y*x]. In lib.c they are merely abc[].\n" >cd
    print "	(2) gflush() is missing, & an x prefix is added as follows:" >cd
    print "	    the following:  gexit, ginit, greset, setfas, setslo" >cd
    print "	    become:         xgexi, xgini, xgrese, xsetfa, xsetsl\n" >cd
    print "	(3) The notation <received> is for reference only\n" >cd
}

# DO FOR EVERY LINE BRACKETED BY "---------- TRANSFER TYPES"...."---------- END TRIPLETS"
/^#---------- TRANSFER TYPES/ , /^#---------- END TRIPLETS$/  {

    if ( $0 ~ /^#/ || $0 ~ /^$/ ) { next }    # ignore comments and blank lines

    typ[$1] = $2

}

# DO FOR EVERY LINE BRACKETED BY "---------- ROUTINES"...."---------- END ROUTINES"
/^#---------- ROUTINES/ , /^#---------- END ROUTINES$/ {
    if ( $0 ~ /^#/ || $0 ~ /^$/ ) { next }    # ignore comments and blank lines

# GENERATE LIB.C, ETC.

# functions that are special cases

	if ( $0 ~ /:gflush/ ) { next }
	if ( $0 ~ /:ginit/ ) { $1 = "VVv:xginit(" }
	if ( $0 ~ /:greset/ ) { $1 = "VVv:xgreset(" }
	if ( $0 ~ /:gexit/ ) { $1 = "VVv:xgexit(" }
	if ( $0 ~ /:setslowcom/ ) { $1 = "UOv:xsetslowcom(" }
	if ( $0 ~ /:setfastcom/ ) { $1 = "UOv:xsetfastcom(" }
	if ( $0 ~ /:bogus/ ) { cmd++; next }
	

# FOR EACH FUNCTION, check for number of fields and proper parens

	for ( nf=0 ; nf<NF && substr($(nf+1),1,1)!=")" ; nf++ ) { }
		# this line allows comments after the right paren in lib.prim
		# nf = number of parameter fields
	printf "\n" >cl
	printf "\n" >cd
	if ( substr($1,length($1))!="(" ) {
	    print "Missing \"(\" in "$1", "FILENAME" line",NR
	    next
	}
	if ( substr($(nf+1),1,1)!=")" ) {
	    print "Missing \")\" in "$1", "FILENAME" line",NR
	    next
	}

# find function name, return value, return value type, etc.

	h = split($1,fn,":")	# Split return type from function name
	if ( h != 2 ) {
	    print "\""$1"\" should have exactly one colon, "FILENAME" line",NR
	    next
	}
	retval = fn[1]		# Return value

	deftype[0] = typ[retval]	# type declaration
	if ( length( deftype[0] ) == 0 ) {
	    print "No type for \"" retval "\" found in \""$1"\", "FILENAME" line",NR
	    next
	}
	if ( deftype[0] ~ /[\[]/ ) {
	    print "Return value should not be an array in \""$1"\", "FILENAME" line",NR
	    next
	}
	k = length(fn[2]) - 1
	fnname = substr(fn[2],1,k) 

# print comment, then function or subroutine declaration line

	print "/*\n** prim " $0 "\n*/" >cl
	print $0 "\n" >cd
	printf deftype[0] " " >cl
#	printf "extern " deftype[0] "\t" >ch
	printf "\t\t" deftype[0] " " >cd
	recsval[0] = 0			# assume void returned
	recsvals = 0			# assume no values received
		# recsval[n] & recsvals are used as Booleans
	if ( deftype[0] != "void" ) {
	    recsval[0] = 1		# nope, it returns a value
	    recsvals = 1		# at least 1 value received
	}

# print function or subroutine name
	# Note:  to support pointer return values, add printing of '*' here.

	printf fnname "(" >cl
	printf fnname "(" >cd
#	print fnname "();" >ch

# print parens and args on function or subroutine declaration line

	for ( i=1 ; i<nf ; i++ ) {
	    printf " a" i >cl
	    printf " a" i >cd
	    if ( i<nf-1 ) {
		printf "," >cl
		printf "," >cd
	    }
	}
	print " )" >cl
	print " )" >cd

# FOR EACH PARAMETER i, print declarations

	for ( i=1 ; i<nf ; i++ ) {
	    hasszfld[i] = split($(i+1),p,":") - 1
		# Boolean hasszfld[i] is true if arg i has an array size field
	    deffld[i] = p[1]			       # def'n field of param i
	    if ( hasszfld[i] ) { szfield[i] = p[2] }   # if is array, size field
	    if ( hasszfld[i] > 1 ) {
		printf "Too many colons in \"" $(i+1) "\", function \""
		print fnname "\", "FILENAME" line",NR
		next
	    }

	    hasbrackets = split(typ[deffld[i]],p,"[") - 1
		# Boolean hasbrackets is true if defined type has '[]' appended
	    deftype[i] = p[1]		# defined type without the '[]'
	    if ( length( deftype[i] ) == 0 ) {
		print "No type for \"" deffld[i] "\" found in \""$1"\", "FILENAME" line",NR
		next
	    }

	    if ( hasszfld[i] && ! hasbrackets ) {
		printf "Size field unused in \"" $(i+1) "\", function \""
		print fnname "\", "FILENAME" line",NR
		hasszfld[i] = 0
	    }
	    if ( ! hasszfld[i] && hasbrackets ) {
		printf "Size field missing in \"" $(i+1) "\", function \""
		print fnname "\", "FILENAME" line",NR
		next
	    }
	    recsval[i] = 0	# assume this param sent, not received
	    if ( (xfrtyp[i] = substr(deffld[i],2,1)) >="A" && xfrtyp[i]<="Z" ) {
		recsval[i] = 1	# nope, this param is received
		recsvals = 1	# If param is received, flag echo sw needed
	    }

# print the type of the parameter

	    printf deftype[i] " " >cl
	    printf "\t\t" deftype[i] " " >cd
	    mode[i] = substr(deffld[i],3,1)
	    if ( mode[i] == "V" ) { mode[i] = "v" }
	    if ( mode[i] == "R" ) { mode[i] = "r" }
	    if ( mode[i] == "A" ) { mode[i] = "a" }
	    if ( mode[i] == "r" && ! hasszfld[i] ) {
				# if passed by ref & has no array size field
		printf "*" >cl
		printf "*" >cd
	    }
	    printf "a" i >cl
	    printf "a" i >cd
		# declaration is complete unless the arg's xfr mode is array

	    if ( mode[i] == "a" ) {		# if xfr mode is array
		printf "[]" >cl
		printf "[" szfield[i] "]" >cd
	    }
	    printf ";" >cd
	    if ( recsval[i] ) {
		printf "  <received>" >cd
	    }
	    print ";" >cl
	    print "" >cd

	}	# end of argument declarations

# print declaration of value returned, if a value returned

	print "{" >cl
	if ( ! recsval[0] ) {	    # if a void returned, do nothing
	} else if ( (xfrtyp[0] = substr(retval,2,1)) ~ /[FLO]/ ) {
				# if a float, long, or Boolean is returned
	    print "\t" deftype[0] " retval;\n" >cl
	} else {
	    print "Invalid return value type for \""$1"\", "FILENAME" line",NR
	    next
	}

# CALL GRAPHICS PRIMITIVE

	if ( recsvals ) {
	    print "\techoff();" >cl
	}
	print "\tgcmd(" cmd ");" >cl
	cmd++

# FOR EACH PARAMETER TO BE SENT

	for ( i=1 ; i<nf ; i++ ) {
	    if ( ! recsval[i] ) {		# if this parameter is sent
		printf "\tsend" xfrtyp[i] >cl

		if ( hasszfld[i] ) {		# if an array
	# Note: this also works if a single typedef array is sent by reference
		    print "s(a" i "," szfield[i] ");" >cl
		} else {
		    print "(a" i ");" >cl
		}
	    }
	}

# FOR EACH VALUE TO BE RECEIVED

	if ( recsvals ) {			# if any vals received
	    print "\tflushg();" >cl
	    if ( recsval[0] ) {			# if a value is returned
		print "\tretval = rec" xfrtyp[0] "();" >cl
	    }

	    for ( i=1 ; i<nf ; i++ ) {		# for each parameter
		if ( recsval[i] ) {		# if this param is received
		    if ( mode[i] == "v" ) {
			printf "Params cannot be returned by value in \"" $(i+1)
			print "\", function \"" fnname "\", "FILENAME" line",NR
			next
		    }
		    if ( hasszfld[i] ) {		# if an array
	# Note: this also works if a single typedef array is sent by reference
			print "\trec" xfrtyp[i] "s(a" i ");" >cl
		    } else {			# if not an array
			printf "\t" >cl
			if ( mode[i] == "r" ) {	# if passed by reference
			    printf "*" >cl
			}
			print "a" i " = rec" xfrtyp[i] "();" >cl
		    }
		}
	    }
	    print "\treccr();\n\techoon();" >cl
	}

	print "\tendprim();" >cl
	if ( recsval[0] ) {		# if a value is returned
	    print "\treturn retval;" >cl
	}
	print "}" >cl
}
