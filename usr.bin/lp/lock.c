#include	"lp.h"

SCCSID("@(#)lock.c	3.1")

/* trylock(file, atime, tries, stime)

	tries to make file a lock up to "tries" times,
	sleeping "stime" seconds between tries.

	return codes: 0 - success
		      -1 - failure
 */

int
trylock(file, atime, tries, stime)
char *file;
time_t atime;
int tries;
unsigned stime;
{
	int i;
	unsigned sleep();

	for(i = 1; i <= tries; i++) {
		if(lock(file, atime) == 0)
			return(0);
		else
			sleep(stime);
	}
	return(-1);
}
/*	lock(file, atime)
 *	char *file;
 *	time_t atime;
 *
 *	lock  -  this routine will create a lock file (file).
 *	If one already exists and the create time is
 *	older than the age time (atime), then an attempt will be made
 *	to unlink it and create a new one.
 *
 *	return codes:  0  |  -1
 */

lock(file, atime)
char *file;
time_t atime;
{
	struct stat stbuf;
	time_t ptime, time();
	int pid;
	char tempfile[NAMEMAX];

	pid = getpid();
	sprintf(tempfile, "LTMP.%d", pid);
	if (onelock(pid, tempfile, file) == -1) {
		/* lock file exists */
		/* get status to check age of the lock file */
		if(stat(file, &stbuf) != -1) {
			time(&ptime);
			if ((ptime - stbuf.st_ctime) < atime) {
				/* file not old enough to delete */
				return(-1);
			}
		}
		unlink(file);
		if(onelock(pid, tempfile, file) != 0)
			return(-1);
	}
	return(0);
}

/* unlock(name) -- unlocks name */

unlock(name)
char *name;
{
	return(unlink(name));
}

/* tunlock() -- unlock temp lock if present */

tunlock()
{
	char templock[NAMEMAX];

	sprintf(templock, "LTMP.%d", getpid());
	unlink(templock);
}

/*	onelock(pid,tempfile,name) makes lock a name
	on behalf of pid.  Tempfile must be in the same
	file system as name. */

onelock(pid, tempfile, name)
int pid;
char *tempfile;
char *name;
{
	int fd;

	if((fd = creat(tempfile, 0444)) < 0)
		return(-1);
	write(fd, (char *) &pid, sizeof(int));
	close(fd);
	if(link(tempfile , name) < 0) {
		unlink(tempfile);
		return(-1);
	}
	unlink(tempfile);
	return(0);
}


/* ltouch -- update access and mod times of lock file */

ltouch(name)
char *name;
{
	struct utimbuf {
		long x, y;
	};

	utime(name, (struct utimbuf *) NULL);
}
