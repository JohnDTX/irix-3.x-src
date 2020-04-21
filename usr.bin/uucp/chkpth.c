/* @(#)chkpth.c	1.6 */
#include "uucp.h"
#include <sys/types.h>
#include <sys/stat.h>



#define DFLTNAME "default"
#define MAXUSERS 20
struct userpath {
	char *us_lname;		/* login name	*/
	char *us_mname;		/* machine name */
	char us_callback;	/* callback option */
	short  *us_not;		/* lang present       */
	char **us_path;		/* pathname */
};
struct userpath Upt[MAXUSERS];
struct userpath *Mchdef = NULL, *Logdef = NULL;
int Nbrusers = 0;
int Uptfirst = 1;
char *strrchr(), *strcpy(), *calloc(), *strchr();

/*
 * check the path table for the machine or log name
 * (non-null parameter) to see if the input path (path)
 * starts with an acceptable prefix.
 *	logname		-> login name
 *	machine		-> machine name
 *	path		-> pathname
 * returns:
 *	0		-> success
 *	FAIL		-> failure
 */
chkpth(logname, mchname, path)
char *path, *logname, *mchname;
{
	register struct userpath *u;
	register char c;
	register int i;
	int ret;
	char *lxp;

	/*
	 * build path table on first entry
	 */
	if (Uptfirst) {
		ret = rdpth(Upt);
		ASSERT(ret == 0, "BAD USERFILE", "", Nbrusers);
		Uptfirst = 0;

	}

	/*
	 * find entry corresponding to
	 * login name and machine name
	 */
	for (u = Upt, i = 0; i < Nbrusers; i++, u++) {
		if (*logname != '\0' && strcmp(logname, u->us_lname) == SAME)
			break;
		if (*mchname != '\0' && strncmp(mchname, u->us_mname, SYSNSIZE) == SAME)
			break;

	}
	if (i >= Nbrusers) {
		if (*logname == '\0')
			u = Mchdef;
		else
			u = Logdef;
		if (u == NULL)
			return(FAIL);
	}

	/*
	 * check for /../ in path name
	 */
	{
		register char *s;
		for (s = path; *s != '\0'; s++) {
			if (*s == '/' && prefix("../", (++s)))
				return(FAIL);
		  }
	}

	{ 
		register short *notptr;
		register int ok;
		register char **p;

		ok = 0;
		notptr = u->us_not;
		for (p = u->us_path; *p != NULL; p++, notptr++)
			if (prefix(*p, path))
				if (*notptr) 
				return(FAIL);
				else ok++;
		if (ok) 
		return(0);
	}

	if (prefix(Spool, path)) {
		if ((c = *((lxp=strrchr(path, '/'))?lxp+1:path)) == DATAPRE
		  || c == XQTPRE)
			return(0);
	}

	/*  
	 * path name not valid
	 */
	return(FAIL);
}


/*
 * read the USERFILE and
 * construct the userpath structure pointed to by (u);
 *	u	-> user path structure
 * returns:
 *	0	-> success
 *	FAIL	-> failure
 */
rdpth(u)
register struct userpath *u;
{
	register char **cp;
	register short *np;
	FILE *uf;
	char buf[BUFSIZ + 1], *pbuf[BUFSIZ + 1], *pc;

	if ((uf = fopen(USERFILE, "r")) == NULL) {
		return(FAIL);
	}

	while (fgets(buf, BUFSIZ, uf) != NULL) {
		register int nargs, i;

		if((buf[0] == '#') || (buf[0] == ' ') || (buf[0] == '\t') || 
			(buf[0] == '\n'))
			continue;
		if (++Nbrusers > MAXUSERS) {
			fclose(uf);
			return(FAIL);
		}
		if ((pc = calloc((unsigned) strlen(buf) + 1, sizeof (char)))
			== NULL) {

			/*
			 * can not allocate space
			 */
			fclose(uf);
			return(FAIL);
		}

		strcpy(pc, buf);
		nargs = getargs(pc, pbuf);
		u->us_lname = pbuf[0];

		/*
		 * set login name and machine name
		 */
		pc = strchr(u->us_lname, ',');
		if (pc != NULL)
			*pc++ = '\0';
		else
			pc = u->us_lname + strlen(u->us_lname);
		u->us_mname = pc;
		if (*u->us_lname == '\0' && Logdef == NULL)
			Logdef = u;
		else if (*u->us_mname == '\0' && Mchdef == NULL)
			Mchdef = u;

		/*
		 * callback option?
		 */
		i = 1;
		if (strcmp(pbuf[1], "c") == SAME) {
			u->us_callback = 1;
			i++;
		} else
			u->us_callback = 0;
		if ((cp = u->us_path = (char **) calloc((unsigned) nargs - i+1,
		   sizeof (char *))) == NULL) {

			/*
			 * can not allocate space
			 */
			fclose(uf);
			return(FAIL);
		}

		if ((np = u->us_not = (short *) calloc((unsigned) nargs - i + 1,
		   sizeof (short))) == NULL) {

			/*
			 * can not allocate space
			 */
			fclose(uf);
			return(FAIL);
		}

		/*
		 * split out pathname
		 */
		while (i < nargs) {
			*cp = pbuf[i];
			if (**cp != '!') {
				cp++;
				i++;
				*np++ = 0;
				continue;
			}
			*np++ = 1;
			if ( *++(*cp) == '\0')
				*cp = pbuf[++i];
			cp++;
			i++;
		}
		*cp = NULL;
		u++;
	}

	fclose(uf);
	return(0);
}


/*
 * check for callback
 *	name	-> login name
 * returns:
 *	0	-> no call back
 *	1	-> call back
 */
callback(name)
register char *name;
{
	register struct userpath *u;
	register int i;
	int ret;

	if (Uptfirst) {
		ret = rdpth(Upt);
		ASSERT(ret == 0, "BAD USERFILE", "", Nbrusers);
		Uptfirst = 0;
	}

	for (u = Upt, i = 0; i < Nbrusers; u++, i++) {
		if (strcmp(u->us_lname, name) != SAME)
			continue;

		/*
		 * found user name
		 */
		return(u->us_callback);
	}

	/*
	 * userid not found
	 */
	return(0);
}


/*
 * check write permission of file
 * none NULL - create directories
 * if mopt != NULL and permissions are ok,
 * a side effect of this routine is to make
 * directories up to the last part of the
 * filename ( if they do not exit).
 * returns:
 *	0	->success
 *	FAIL	-> failure
 */
chkperm(file, mopt)
char *file, *mopt;
{
	register int ret;
	register char *lxp;
	struct stat s;
	char dir[MAXFULLNAME];

	if (access(file, 0) != -1)
		return(0);

	strcpy(dir, file);
	if (lxp=strrchr(dir, '/'))
		*lxp = '\0';
	if ((ret = stat(dir, &s)) == -1
	  && mopt == NULL)
		return(FAIL);

	/*
	 * if directory exists check
	 * for write permission
	 */
	if (ret != -1) {
		if (prefix(SPOOL, dir))
			return(0);
		if ((s.st_mode & ANYWRITE) == 0)
			return(FAIL);
		else
			return(0);
	}

	/*
	 * make directories
	 */
	return(mkdirs(file));
}
