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
#	This awk script documents the two character escape sequences 
#	transmited for each graphics primitive.
#
#

BEGIN {
	cmd = 0;
 	table[0] =  " ";
 	table[1] =  "!";
 	table[2] =  "\"";
 	table[3] =  "#";
 	table[4] =  "$";
 	table[5] =  "%";
 	table[6] =  "&";
 	table[7] =  "'";
 	table[8] =  "(";
 	table[9] =  ")";
 	table[10] =  "*";
 	table[11] =  "+";
 	table[12] =  ",";
 	table[13] =  "-";
 	table[14] =  ".";
 	table[15] =  "/";
 	table[16] =  "0";
 	table[17] =  "1";
 	table[18] =  "2";
 	table[19] =  "3";
 	table[20] =  "4";
 	table[21] =  "5";
 	table[22] =  "6";
 	table[23] =  "7";
 	table[24] =  "8";
 	table[25] =  "9";
 	table[26] =  ":";
 	table[27] =  ";";
 	table[28] =  "<";
 	table[29] =  "=";
 	table[30] =  ">";
 	table[31] =  "?";
 	table[32] =  "@";
 	table[33] =  "A";
 	table[34] =  "B";
 	table[35] =  "C";
 	table[36] =  "D";
 	table[37] =  "E";
 	table[38] =  "F";
 	table[39] =  "G";
 	table[40] =  "H";
 	table[41] =  "I";
 	table[42] =  "J";
 	table[43] =  "K";
 	table[44] =  "L";
 	table[45] =  "M";
 	table[46] =  "N";
 	table[47] =  "O";
 	table[48] =  "P";
 	table[49] =  "Q";
 	table[50] =  "R";
 	table[51] =  "S";
 	table[52] =  "T";
 	table[53] =  "U";
 	table[54] =  "V";
 	table[55] =  "W";
 	table[56] =  "X";
 	table[57] =  "Y";
 	table[58] =  "Z";
 	table[59] =  "[";
 	table[60] =  "\\";
 	table[61] =  "]";
 	table[62] =  "^";
 	table[63] =  "_";
}

# special hacks
/:gflush\(/ { next }
/:ginit\(/ { $1 = "VVv:xginit(" }
/:greset\(/ { $1 = "VVv:xgreset(" }
/:gbegin\(/ { $1 = "VVv:xgbegin(" }
/:getport\(/ { $1 = "VVv:xgetport(" }
/:getpor\(/ { $1 = "VVv:xgetpor(" }
/:gexit\(/ { $1 = "VVv:xgexit(" }
/:setslowcom\(/ { $1 = "VVv:xsetslowcom(" }
/:setfastcom\(/ { $1 = "VVv:xsetfastcom(" }

# DO FOR EVERY LINE BRACKETED BY "---------- ROUTINES"...."---------- END ROUTINES"
/^#---------- ROUTINES/ , /^#---------- END ROUTINES$/ {
    if ( $0 ~ /^#/ || $0 ~ /^$/ ) { next }    # ignore comments and blank lines
	h = split($1,p,":");
	number = cmd;
	high = int(number / 64);
	low = number - (high*64);
	print cmd "\t'" table[low] "'  '" table[high] "'    " substr(p[h],1,length(p[h])-1)
	cmd++
}
