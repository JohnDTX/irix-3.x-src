/* @(#)gnsys.c	1.5 */
static	char	*Sccsid = "@(#)$Header: /d2/3.7/src/usr.bin/uucp/RCS/gnsys.c,v 1.1 89/03/27 18:30:25 root Exp $";
/*
 * $Log:	gnsys.c,v $
 * Revision 1.1  89/03/27  18:30:25  root
 * Initial check-in for 3.7
 * 
 * Revision 1.3  86/07/25  17:01:22  paulm
 * Use opendir/readdir to access directories.
 * 
 * Revision 1.2  85/10/21  17:44:37  jym
 * SCR #1200- uucp does not work with < 6 char. names,
 * now it does
 * 
 * Revision 1.2  85/02/07  21:35:07  bob
 * Fixed 8 char sys name bugs
 * 
 */
#include <sys/types.h>
#include <dirent.h>
#include "uucp.h"

#define LSIZE 30	/* number of systems to store */
#define	PRESIZE 2	/* size of prefix */
#define	SUFSIZE 6	/* size of suffix */
	/* max size of truncated file name:MAXBASENAME - PRESIZE - SUFSIZE */
#define	TRUNCSIZE (MAXBASENAME - PRESIZE - SUFSIZE)
char *strcpy();

/*
 * return the next system -name which has
 * work to be done.
 * pre is the prefix for work files.
 * dir is the directory to search.
 * sname is a string of size MAXBASENAME - WSUFSIZE.
 * to return name of called system
 * returns:
 *	0	-> no more names
 *	1	-> name returned in sname
 *	FAIL 	-> bad directory
 */
int	rotindx;
char	*list[LSIZE];
int	nitem=0, n=0;
gnsys(sname, dir, pre)
char *sname, *dir, pre;
{
	register DIR *fp;	/* SPOOL directory file */
	register FILE *ls;	/* L.sys file */
	char *p1;
	char px[3];
	char sysname[NAMESIZE], filename[NAMESIZE];
	long	time();
	void free();

	px[0] = pre;		/* affects PRESIZE and TRUNCSIZE */
	px[1] = '.';
	px[2] = '\0';
	if (nitem == 0) {

		/*
		 * get list of systems with work
		 */
		fp = opendir(dir);
		ASSERT(fp != NULL, "BAD DIRECTORY", dir, 0);
		ls = fopen(SYSFILE, "r");
		ASSERT(ls != NULL, "BAD L.sys file", SYSFILE, 0);
		{
			register int i;

			for (i = 0; i < LSIZE; i++)
				list[i] = NULL;
		}
		while (gnamef(fp, filename) == TRUE) {
			DEBUG(4,"gns %s\n",filename);
			if (prefix(px, filename) == FALSE)
				continue;
			strncpy(sysname,filename+PRESIZE,TRUNCSIZE);
			DEBUG(4,"gns1:truncated sysname:%s\n",sysname);
			if (sysname[0] == '\0')
				continue;
				/* FAIL iff list table full */
			if (mapsys(ls,sysname) == FAIL)
				break;
		}

		/*
		 * Random selection of system to call
		 * When debugging you don't want to do this
		 */
#ifndef NOROT
		if(Debug == 0){
			if(nitem){
				rotindx = (time((long *)0))%(long)nitem;
			}else
				rotindx = 0;
		}else
#endif
			rotindx = 0;
		closedir(fp);	/* close SPOOL directory */
		fclose(ls);	/* close L.sys file */
	}

	if (nitem == 0)
		return(FALSE);
	while(nitem > n) {
		if(rotindx >= nitem)
			rotindx = 0;
		strcpy(sname, list[rotindx]);
		rotindx++;n++;
		if (callok(sname) == 0)
			return(TRUE);
	}

	{
		register int i;

		for (i = 0; i < nitem; i++)
			if (list[i] != NULL)
				free(list[i]);
	}
	nitem = n = 0;
	return(FALSE);
}

/*
 * do a linear search of list (list)
 * to find name (name).
 * If the name is not found, it is added to the
 * list.
 * The number of items in the list (n) is
 * returned (incremented if a name is added).
 *	name	-> system name
 *	list	-> array of item
 *	n	-> # of items in list
 * return:
 *	n	-> the number of items in the list
 */
srchst(name, list, n)
register char *name, **list;
int n;
{
	register int i;
	char *p;
	extern char *calloc();

	DEBUG(4,"srch %s\n",name);
	for (i = 0; i < n; i++){
		DEBUG(4, "srchst1 %s\n",list[i]);
		if (strcmp(name, list[i]) == SAME)
			break;
	}
	if (i >= n) {
		if ((p = calloc((unsigned) strlen(name) + 1, sizeof (char)))
		  == NULL)
			return(n);
		strcpy(p, name);
		list[n++] = p;
	}
	return(n);
}


/*
 * map system name
 *   when called:
 *	name	-> truncated system name (derived from file name)
 *   upon return:
 *	name	-> full system name from L.sys
 *	0	-> success
 *	FAIL	-> failure
 */
mapsys(ls,sysname)
FILE *ls;
char *sysname;
{
	register char *iptr;
	char line[1024];
	char *strchr(), *strpbrk();

	rewind(ls);
	while (fgets(line, sizeof(line), ls) != NULL) {
		if ((line[0] == '#') || (line[0] == ' ') || (line[0] == '\t')
		  || (line[0] == '\n'))
			continue;
		if ((iptr=strpbrk(line, " \t")) != NULL)
			*iptr = '\0';
		line[SYSNSIZE] = '\0';
		DEBUG(5,"gns2:comparing to %s in L.sys:%s\n",line);
		if (strncmp(sysname, line, (strlen(line) < 6)?strlen(line):6) == SAME) {
			DEBUG(4,"gns2:matched full system name:%s\n",line);
			if (mlock(line) != 0)
				continue;
			else
				delock(line);
			nitem = srchst(line, list, nitem);
			if (nitem >= LSIZE)
				return FAIL;
		} else
			DEBUG(5,"  gns2:match failed\n",0);
	}
	return 0;
}
