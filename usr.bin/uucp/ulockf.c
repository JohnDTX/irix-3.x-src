/* @(#)ulockf.c	1.3 */
/* $Source: /d2/3.7/src/usr.bin/uucp/RCS/ulockf.c,v $*/
/* $Revision: 1.1 $*/
/* $Date: 89/03/27 18:30:44 $*/

#include "uucp.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>


/*
 * create a lock file (file).
 * If one already exists, the create time is checked for
 * older than the age time (atime).
 * If it is older, an attempt will be made to unlink it
 * and create a new one.
 * return:
 *	0	-> success
 *	FAIL	-> failure
 */
ulockf(file, atime)
register char *file;
time_t atime;
{
	register int ret;
	struct stat stbuf;
	static int pid = -1;
	static char tempfile[NAMESIZE];
	time_t ptime, time();

	if (pid < 0) {
		pid = getpid();
		sprintf(tempfile, "LTMP.%d", pid);
	}
	if (onelock(pid, tempfile, file) == -1) {

		/*
		 * lock file exists
		 * get status to check age of the lock file
		 */
		ret = stat(file, &stbuf);
		if (ret != -1) {
			time(&ptime);
			if ((ptime - stbuf.st_ctime) < atime) {
				DEBUG(4,"File %s exists and too new to remove\n",
					file);

				/*
				 * file not old enough to delete
				 */
				return(FAIL);
			}
		}
		ret = unlink(file);
		ret = onelock(pid, tempfile, file);
		if (ret != 0) {
			DEBUG(4,"ulockf failed in unlink() or onelock()\n","");
			return(FAIL);
		}
	}
	stlock(file);
	return(0);
}


#define MAXLOCKS 10	/* maximum number of lock files */
char *Lockfile[MAXLOCKS];
int Nlocks = 0;

/*
 * put name in list of lock files
 * return:
 *	none
 */
stlock(name)
char *name;
{
	register int i;
	char *p, *strcpy(), *calloc();

	for (i = 0; i < Nlocks; i++) {
		if (Lockfile[i] == NULL)
			break;
	}
	ASSERT(i < MAXLOCKS, "TOO MANY LOCKS", "", i);
	if (i >= Nlocks)
		i = Nlocks++;
	p = calloc((unsigned) strlen(name) + 1, sizeof (char));
	ASSERT(p != NULL, "CAN NOT ALLOCATE FOR", name, 0);
	strcpy(p, name);
	Lockfile[i] = p;
	return;
}


/*
 * remove all lock files in list
 * return:
 *	none
 */
rmlock(name)
register char *name;
{
	register int i;
	void free();

	for (i = 0; i < Nlocks; i++) {
		if (Lockfile[i] == NULL)
			continue;
		if (name == NULL
		|| strcmp(name, Lockfile[i]) == SAME) {
			unlink(Lockfile[i]);
			free(Lockfile[i]);
			Lockfile[i] = NULL;
		}
	}
	return;
}

/*
 * return:
 *	0	-> if the name is a lock
 */
isalock(name) 
char *name;
{
	struct stat xstat;

	if(stat(name,&xstat)<0) return(0);
	if(xstat.st_size!=sizeof(int)) return(0);
	return(1);
}

/*
 * makes lock a name on behalf of pid.
 * Tempfile must be in the same file system as name.
 */
onelock(pid,tempfile,name) 
char *tempfile, *name;
{	
	register int fd;
	char	cb[100];
	extern errno;

	fd=creat(tempfile,0444);
	if(fd < 0){
		sprintf(cb, "%s %s %d",tempfile,name,errno);
		logent("ULOCKC", cb);
		if((errno == EMFILE) || (errno == ENFILE))
			unlink(tempfile);
		return(-1);
	}
	write(fd,(char *) &pid,sizeof(int));
	close(fd);
	if(link(tempfile,name)<0){
		if(unlink(tempfile)< 0){
			sprintf(cb, "ULK err %s %d", tempfile,  errno);
			logent("ULOCKLNK", cb);
		}
		DEBUG(4,"link failed in ulockf: %d\n",errno);
		return(-1);
	}
	if(unlink(tempfile)<0){
		sprintf(cb, "%s %d",tempfile,errno);
		logent("ULOCKF", cb);
	}
	return(0);
}

#define LOCKPRE "LCK."

/*
 * remove a lock file
 * return:
 *	0	-> success
 *	FAIL	-> failure
 */
delock(s)
char *s;
{
	char ln[30];

	sprintf(ln, "%s.%s", LOCKPRE, s);
	rmlock(ln);
}


/*
 * create system lock
 * return:
 *	0	-> success
 *	FAIL	-> failure
 */
mlock(sys)
char *sys;
{
	char lname[30];

	sprintf(lname, "%s.%s", LOCKPRE, sys);
	return(ulockf(lname, (time_t) SLCKTIME ) < 0 ? FAIL : 0);
}


/*
 * update access and modify times for lock files
 * return:
 *	none
 */
ultouch()
{
	register int i;
	time_t time();

	struct ut {
		time_t actime;
		time_t modtime;
	} ut;

	ut.actime = time(&ut.modtime);
	for (i = 0; i < Nlocks; i++) {
		if (Lockfile[i] == NULL)
			continue;
		utime(Lockfile[i], &ut);
	}
	return;
}
