/* @(#)anlwrk.c	1.5 */
#include "uucp.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include "uust.h"



#define SUFSIZE	6
#define LLEN 10
#define MAXRQST 250
int Nfiles = 0;
char Filent[LLEN][NAMESIZE];
char *strcpy();


/*
 * create a vector of command arguments
 *	file	-> work file
 *	wvec	-> address of array to return arguments
 * returns:
 *	0	   ->  no more work in this file
 *	positive # -> number of arguments
 */
anlwrk(file, wvec)
char *file, **wvec;
{
	register FILE *afp;
	register short i;
	static FILE *fp = NULL;
	static char str[MAXRQST];
	static char afile[NAMESIZE];
	static short acount;
	struct stat stbuf;
	char *strrchr(), *lxp;

	if (fp == NULL) {
		if (file[0] == '\0')
			return(0);
		fp = fopen(file, "r");

		/*
	 	 * check for partial completion
	 	 */
		if (fp == NULL){
			logent("Can't open",file);
			return(0);
		}


		fstat(fp->_file, &stbuf);
		nstat.t_qtime = stbuf.st_mtime;
		strcpy(afile, (lxp=strrchr(file, '/'))?lxp+1:file);
		afile[0] = 'A';
		acount = 0;
		afp = fopen(afile, "r");
		if (afp != NULL) {


			/*
		 	 * get done count
		 	 */
			i = fscanf(afp, "%hd", &acount);
			fclose(afp);
			if (i == 0)
				acount = 0;
			for (i = 0; i < acount; i++) {
				if (fgets(str, MAXRQST, fp) == NULL)
					break;
			}
		}

		/*
		 * for UUSTAT stuff
		 */
		Usrf = 0;
	}

	if (fgets(str, MAXRQST, fp) == NULL) {
		fclose(fp);
		unlink(file);
		unlink(afile);
		USRF(USR_COMP);
		US_RRS(file, Usrf);
		Usrf = 0;
		file[0] = '\0';
		fp = NULL;
		return(0);
	}

	afp = fopen(afile, "w");
	if(afp == NULL){
		char	cb[20];
		extern int errno;

		sprintf(cb,"%d",errno);
		logent("FOPEN",cb);
		return(0);
	}
	fprintf(afp, "%d", acount);
	fclose(afp);
	acount += 1;
	return(getargs(str, wvec));
}


/*
 * check the work list (list).
 * If it is empty or the present work is exhausted, it
 * will call gtwrk to generate a new list.
 * The "reqst" field will be the string "chk" or "get" to
 * check for work, or get the next work file respectively.
 *
 *	file	-> address of array to return full pathname in
 *	reqst	-> type of request
 *	dir	-> directory files are in
 *	pre	-> file prefix to examine
 * returns:
 *	0	-> no more work (or some error)
 *	1	-> there is work
 */
iswrk(file, reqst, dir, pre)
char *file, *reqst, *dir, *pre;
{

	if (Nfiles == 0)
		bldflst(dir, pre);
	if (Nfiles == 0)
		return(0);

	if (*reqst == 'g')
		gtwrkf(dir, file);
	else
		Nfiles = 0;
	return(1);
}

/*
 * build list of work files for given system
 * Nfiles, Filent are global
 *	dir	-> spool directory
 *
 * return:
 *	none
 */
#define IREAD	0400
bldflst(dir, pre)
register char *dir, *pre;
{
	register DIR *pdir;
	struct stat s;
	char filename[NAMESIZE];
	extern errno;
	int	uid, gid, m;

	dlogent("bldfl",pre);
	Nfiles = 0;
	if ((pdir = opendir(dir)) == NULL)
		return;
	uid = geteuid();
	gid = getegid();
	while (gnamef(pdir, filename)) {
		if (!prefix(pre, filename) ||
		    !((strlen(filename)-strlen(pre)) == SUFSIZE))
			continue;
		/*
		 * The following simulates the access system call which
		 * is not implemented correctly on RT and possibly
		 * on older UNIX systems. 
		 */
		m = IREAD;
		if(stat(filename, &s) == -1)
			continue;
		if ((s.st_mode & 0444) == 0)
			continue;
		if(uid != 0){
			if(uid != s.st_uid){
				m >>= 3;
				if(gid != s.st_gid)
					m >>= 3;
			}
			if((s.st_mode&m) == 0){
				continue;
			}
		}
		entflst(filename);
	}
	closedir(pdir);
	return;
}

/*
 * put new name if list is not full
 * or new name is less than the MAX
 * now in the list.
 * Nfiles, Filent[] are modified.
 *	 file	-> file to put in list
 * return: 
 *	none
 */
entflst(file)
char *file;
{
	register char *p;
	register int i;

DEBUG(8,"entf %s\n",file);
	if (Nfiles < LLEN) {
		strncpy(Filent[Nfiles++], file, NAMESIZE);
		return;
	}

	/*
	 * find MAX in table
	 */
	p = Filent[0];
	for (i = 1; i < Nfiles; i++)
		if (strcmp(Filent[i], p) > 0)
			p = Filent[i];

	if (strcmp(p, file) > 0)
		strncpy(p, file, NAMESIZE);

	return;
}

/*
 * gtwrkf - get next work file
 * Nfiles, Filent[] are modified.
 *	 dir	-> spool directory
 *	 file	 -> array to return filename in
 * return: 
 *	none
 */
gtwrkf(dir, file)
char *file, *dir;
{
	register char *p;
	register int i;

DEBUG(8,"gtwrk\n",0);
	p = Filent[0];
	for (i = 1; i < Nfiles; i++) 
		if (strcmp(Filent[i], p) < 0)
			p = Filent[i];
	sprintf(file, "%s/%s", dir, p);
	strncpy(p, Filent[--Nfiles], NAMESIZE);
	return;
}

/*
 * get work return
 *	file	-> array to return file name
 *	dir	-> spool direcotry
 *	wkpre	-> prefix of files to examine
 *	wrkvec	-> array to return arguments
 * returns:
 *	nargs  	->  number of arguments
 *	0 	->  no arguments - fail
 */
gtwvec(file, dir, wkpre, wrkvec)
char *file, *dir, *wkpre, **wrkvec;
{
	register int nargs;

	while ((nargs = anlwrk(file, wrkvec)) == 0) {
		if (!iswrk(file, "get", dir, wkpre))
			return(0);
	}
	return(nargs);
}
