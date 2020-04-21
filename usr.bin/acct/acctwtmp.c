char _Version_[] = "(C) Copyright 1983 UniSoft Corp. Version V.1.0";
char _Origin_[] = "System V";
/*	@(#)acctwtmp.c	1.2 of 3/31/82	*/
/*
 *	acctwtmp reason >> /etc/wtmp
 *	writes utmp.h record (with current time) to end of std. output
 *	acctwtmp `uname` >> /etc/wtmp as part of startup
 *	acctwtmp pm >> /etc/wtmp  (taken down for pm, for example)
 */
#include <stdio.h>
#include "acctdef.h"
#include <sys/types.h>
#include <utmp.h>

struct	utmp	wb;

main(argc, argv)
char **argv;
{
	if(argc < 2) {
		fprintf(stderr, "Usage: %s reason [ >> %s ]\n",
			argv[0], WTMP_FILE);
		exit(1);
	}

	strncpy(wb.ut_line, argv[1], LSZ);
	wb.ut_line[11] = NULL;
	wb.ut_type = ACCOUNTING;
	time(&wb.ut_time);
	fseek(stdout, 0L, 2);
	fwrite(&wb, sizeof(wb), 1, stdout);
}
