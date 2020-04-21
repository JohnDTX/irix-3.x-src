/* @(#)systat.c	1.5 */
#include "uucp.h"
#include <sys/types.h>

#define STATNAME(f, n) sprintf(f, "%s/%s.%.7s", Spool, "STST", n)
#define S_SIZE 100

time_t time();

/*
 * make system status entry
 *	name -> system name
 *	text -> text string to read
 *	type -> ?
 * return:
 *	none
 */
systat(name, type, text)
register int type;
char *name, *text;
{
	register FILE *fp;
	int count;
	char filename[MAXFULLNAME], line[S_SIZE];
	time_t prestime;

	if (type == 0)
		return;
	line[0] = '\0';
	time(&prestime);
	count = 0;
	STATNAME(filename, name);

	fp = fopen(filename, "r");
	if (fp != NULL) {
		fgets(line, S_SIZE, fp);
		sscanf(&line[2], "%d", &count);
		if (count <= 0)
			count = 0;
		fclose(fp);
	}

	if (type == SS_FAIL)
		count++;

	fp = fopen(filename, "w");
	ASSERT(fp != NULL, "SYSTAT OPEN FAIL", "", 0);
	chmod(filename, 0666);
	fprintf(fp, "%d %d %ld %ld %s %s\n", type, count, prestime, Retrytime, text, name);
	fclose(fp);
	return;
}

/*
 * remove system status entry
 *	name -> system name;
 * return:
 *	none
 */
rmstat(name)
register char *name;
{
	char filename[MAXFULLNAME];

	STATNAME(filename, name);
	unlink(filename);
}

/*
 * check system status for call
 *	name -> system to check	
 * return:  
 *	0	-> ok
 *	>0	-> system status
 */
callok(name)
char *name;
{
	register FILE *fp;
	register int t;
	long retrytime;
	int count, type;
	char filename[MAXFULLNAME], line[S_SIZE];
	time_t lasttime, prestime;

	STATNAME(filename, name);
	fp = fopen(filename, "r");
	if (fp == NULL)
		return(SS_OK);

	if (fgets(line, S_SIZE, fp) == NULL) {

		/*
		 * no data
		 */
		fclose(fp);
		unlink(filename);
		return(SS_OK);
	}

	fclose(fp);
	time(&prestime);
	sscanf(line, "%d%d%ld%ld", &type, &count, &lasttime, &retrytime);
	t = type;

	switch(t) {
	case SS_BADSEQ:
	case SS_CALLBACK:
	case SS_NODEVICE:
	case SS_DFAIL:
	case SS_INPROGRESS:	/*let LCK take care of it */
		return(SS_OK);

	case SS_FAIL:
		if (count > MAXRECALLS) {
			logent("MAX RECALLS", "NO CALL");
			DEBUG(4, "MAX RECALL COUNT %d\n", count);
			return(type);
		}

		if (prestime - lasttime < retrytime) {
			logent("RETRY TIME NOT REACHED", "NO CALL");
			DEBUG(4, "RETRY TIME (%d) NOT REACHED\n", (long)  RETRYTIME);
			return(type);
		}

		return(SS_OK);
	default:
		return(SS_OK);
	}
}
