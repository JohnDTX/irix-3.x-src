/* Copyright (c) 1982 Regents of the University of California */

static char rcsid[] = "$Header: /d2/3.7/src/usr.bin/dbx/RCS/mkdate.c,v 1.1 89/03/27 17:44:39 root Exp $";

#include <stdio.h>
#ifdef sgi
#   include <time.h>
#else
#   include <sys/time.h>
#endif

main()
{
    struct tm *t;
    long clock;
    char name[100];
    int namelen;

    printf("char *date = \"");
    clock = time(0);
    t = localtime(&clock);
    printf("%d/%d/%d ", t->tm_mon + 1, t->tm_mday, t->tm_year % 100);
    printf("%d:%02d", t->tm_hour, t->tm_min);
    /* GB (SCR0599) - change dbx's prompt to just give version number
       and date
    gethostname(name, &namelen);
    printf(" (%s)", name);
    */
    printf("\";\n");
    DoVersionNumber();
    exit(0);
}

DoVersionNumber()
{
    FILE *f;
    int version,subversion;

    f = fopen("version", "r");
    if (f == NULL) {
	version = 1;
	subversion = 0;
    } else {
	fscanf(f, "%d %d", &version, &subversion);
	subversion += 1;
	fclose(f);
    }
    f = fopen("version", "w");
    if (f != NULL) {
	fprintf(f, "%d %d\n", version, subversion);
	fclose(f);
    }
    printf("int versionNumber = %d, subversionNumber = %d;\n", version, 
			subversion);
}
