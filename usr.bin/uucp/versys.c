/* @(#)versys.c	1.6 */
/* $Source: /d2/3.7/src/usr.bin/uucp/RCS/versys.c,v $*/
static	char	Sccsid[] = "@(#)$Revision: 1.1 $";
/* $Date: 89/03/27 18:31:08 $*/

#include "uucp.h"


/*
 * verify system name
 *	name	-> system name
 * returns:  
 *	0	-> success
 *	FAIL	-> failure
 */
versys(name)
char *name;
{
	register FILE *fp;
	register char *iptr;
	char line[300];
	char s1[SYSNSIZE + 1];
	char myname[SYSNSIZE + 1];
	char *strchr(), *strpbrk();

	sprintf(myname, "%.*s", SYSNSIZE, Myname);
	sprintf(s1, "%.*s", SYSNSIZE, name);
	if (strncmp(s1, myname, SYSNSIZE) == SAME)
		return(0);
	fp = fopen(SYSFILE, "r");
	if (fp == NULL)
		return(FAIL);
	
	while (fgets(line, sizeof(line) ,fp) != NULL) {
		if((line[0] == '#') || (line[0] == ' ') || (line[0] == '\t') || 
			(line[0] == '\n'))
			continue;
		if ((iptr=strpbrk(line, " \t")) == NULL) continue;
		*iptr = '\0';
		line[SYSNSIZE] = '\0';
		if (strncmp(s1, line, SYSNSIZE) == SAME) {
			fclose(fp);
			return(0);
		}
	}
	fclose(fp);
	return(FAIL);
}
