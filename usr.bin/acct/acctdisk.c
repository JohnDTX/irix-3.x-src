char _Version_[] = "(C) Copyright 1983 UniSoft Corp. Version V.1.0";
char _Origin_[] = "System V";
/*	@(#)acctdisk.c	1.3 of 3/31/82	*/
/*
 *	acctdisk <dtmp >dtacct
 *	reads std.input & converts to tacct.h format, writes to output
 *	input:
 *	uid	name	#blocks
 */

#include <sys/types.h>
#include "acctdef.h"
#include <stdio.h>
#include "tacct.h"

struct	tacct	tb;
char	ntmp[NSZ+1];

main(argc, argv)
char **argv;
{
	tb.ta_dc = 1;
	while(scanf("%hu\t%s\t%f",
		&tb.ta_uid,
		ntmp,
		&tb.ta_du) != EOF) {

		CPYN(tb.ta_name, ntmp);
		fwrite(&tb, sizeof(tb), 1, stdout);
	}
}
