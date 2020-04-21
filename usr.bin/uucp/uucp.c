char _Origin_[] = "System V";
/* @(#)uucp.c	1.6 */
static	char	*Sccsid = "@(#)$Header: /d2/3.7/src/usr.bin/uucp/RCS/uucp.c,v 1.1 89/03/27 18:30:49 root Exp $";
/*
 * $Log:	uucp.c,v $
 * Revision 1.1  89/03/27  18:30:49  root
 * Initial check-in for 3.7
 * 
 * Revision 1.1  87/09/23  17:05:19  vjs
 * Initial revision
 * 
 * Revision 1.5  85/02/07  21:35:15  bob
 * Fixed 8 char sys name bugs
 * 
 */

#include "uucp.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include "uust.h"

/*
 * uucp
 * user id 
 * make a copy in spool directory
 */
int Uid;
int Copy = 0;
char Nuser[32];
char *Ropt = " ";
char Path[MAXFULLNAME], Optns[10], Ename[NAMEBUF];
char Grade = 'n';

#define MAXCOUNT 20	/* maximun number of commands per C. file */

char	Sfile[MAXFULLNAME];
#define JOBON	1
#define JOBOFF	0
#ifdef ONJOB
int	prjob = JOBON;
#else
int	prjob	= JOBOFF;
#endif
char	**Env;
int	notifopt;
char	*getenv();

main(argc, argv, envp)
char *argv[];
char	**envp;
{
	int ret;
	char *sysfile1, *sysfile2, *cp;
	char file1[MAXFULLNAME], file2[MAXFULLNAME];
#ifdef FOWARD
	char tsys2[MAXFULLNAME];
	char tfile1[MAXFULLNAME];
	char sys2save[MAXFULLNAME];
#endif
	char	sqn[5];
	char *strchr(), *strcpy(), *strcat();
	char *strrchr();
	char	*ep;
	char	*fopt;

	(void) tzset();		/* SGI: (^*~%!$@%^&. */
	fopt = NULL;
	Env = envp;
	if((ep = (char *)getenv("JOBNO="))){
		if(strcmp("ON", ep) == SAME)
			prjob = JOBON;
		if(strcmp("OFF", ep) == SAME)
			prjob = JOBOFF;
	}
	strcpy(Progname, "uucp");
	Pchar = 'U';

	/*
	 * find name of local system
	 */
	uucpname(Myname);
	umask(WFMASK);
	Optns[0] = '-';
	Optns[1] = 'd';
	Optns[2] = 'c';
	Ename[0] = Nuser[0] = Optns[3] = '\0';
	Sfile[0] = '\0';
	while(argc>1 && argv[1][0] == '-'){
		switch(argv[1][1]){

		/*
		 * make a copy of the file in the spool
		 * directory.
		 */
		case 'C':
			Copy = 1;
			Optns[2] = 'C';
			break;

		/*
		 * not used
		 */
		case 'c':
			break;

		/*
		 * not used
		 */
		case 'd':
			break;
		case 'f':
			Optns[1] = 'f';
			break;

		/*
		 * invoke uux to execute command
		 */
		case 'e':
#ifdef FOWARD
			sprintf(Ename, "%s", &argv[1][2]);
#else
			sprintf(Ename, "%.8s", &argv[1][2]);
#endif
			break;

		/*
		 * set service grade (not used)
		 */
		case 'g':
			Grade = argv[1][2]; 
			break;
		case 'j':
			prjob = (prjob == JOBON)?JOBOFF:JOBON;
			break;

		/*
		 * send notification to local user
		 */
		case 'm':
			strcat(Optns, "m");
			if(argv[1][2] != '\0'){
				fopt = &argv[1][2];
				strcat(Optns, "o");
			}
			break;

		/*
		 * send notification to user on remote
		 * if no user specified do not send notification
		 */
		case 'n':
			notifopt++;
			sprintf(Nuser, "%.8s", &argv[1][2]);
			break;

		/*
		 * create JCL files but not start uucico
		 */
		case 'r':
			Ropt = argv[1];
			break;
		case 's':
			Spool = &argv[1][2]; 
			break;

		/*
		 * turn on debugging
		 */
		case 'x':
			Debug = atoi(&argv[1][2]);
			if (Debug <= 0)
				Debug = 1;
			break;
		default:
			printf("unknown flag %s\n", argv[1]); 
			break;
		}
		--argc;  argv++;
	}
	DEBUG(4, "\n\n** %s **\n", "START");
	gwd(Wrkdir);
	if(fopt){
		if(*fopt != '/')
			sprintf(Sfile, "%s/%s", Wrkdir, fopt);
		else
			sprintf(Sfile, "%s", fopt);

	}
	/*
	 * work in spool directory
	 */
	ret = chdir(Spool);
	ASSERT(ret == 0, "CANNOT CHDIR TO SPOOL - ", Spool, ret);
	if(ret != 0) {
		fprintf(stderr, "No spool directory - %s - get help\n", Spool);
		cleanup(0);
	}

	Uid = getuid();

	/*
	 * find id of user who spawned command to 
	 * determine
	 */
	ret = guinfo(Uid, User, Path);
	if(Nuser[0] == '\0')
		strcpy(Nuser, User);
	ASSERT(ret == 0, "CAN NOT FIND UID", "", Uid);
	DEBUG(4, "UID %d, ", Uid);
	DEBUG(4, "User %s,", User);
	DEBUG(4, "Ename (%s) ", Ename);
	DEBUG(4, "PATH %s\n", Path);
	if (argc < 3) {
		fprintf(stderr, "usage: uucp from ... to\n");
		cleanup(0);
	}


	/*
	 * set up "to" system and file names
	 */
	if ((cp = strchr(argv[argc - 1], '!')) != NULL) {
		sysfile2 = argv[argc - 1];
		*cp = '\0';
		if (*sysfile2 == '\0')
			sysfile2 = Myname;
		else
			sprintf(Rmtname, "%.*s", SYSNSIZE, sysfile2);
		if (versys(sysfile2) != 0) {
			fprintf(stderr, "bad system name: %s\n", sysfile2);
			cleanup(0);
		}
		strcpy(file2, cp + 1);
#ifdef FOWARD

		/*
		 * Fowarding
		 */
		if ( strchr(file2, '!') != NULL && file2[0] != '~' ) {
			if (chkpub(file2)==FAIL) {
				fprintf(stderr,"\n Forwarding to nonpublic directories denied - uucp failed\n");
				cleanup(0);
			}
			*cp = '!';
			strcpy(file2, cp);
			*cp = '\0';
		}
#endif
	} else {
		sysfile2 = Myname;
		strcpy(file2, argv[argc - 1]);
	}

	/*
	 * if there are more than 2 argc, file2 is a directory
	 */
	if (argc > 3)
		strcat(file2, "/");

	/*
	 * system names limited to SYSNSIZE chars
	 */
	if (strlen(sysfile2) > SYSNSIZE)
		*(sysfile2 + SYSNSIZE) = '\0';


#ifdef FOWARD
	strncpy(sys2save, sysfile2, SYSNSIZE);
#endif
	/*
	 * do each from argument
	 */
	getseq(sqn);
	while (argc > 2) {
		if ((cp = strchr(argv[1], '!')) != NULL) {
			sysfile1 = argv[1];
			*cp = '\0';
			if (strlen(sysfile1) > SYSNSIZE)
				*(sysfile1 + SYSNSIZE) = '\0';
			if (*sysfile1 == '\0')
				sysfile1 = Myname;
			else
				sprintf(Rmtname, "%.*s", SYSNSIZE, sysfile1);
			if (versys(sysfile1) != 0) {
				fprintf(stderr, "bad system name: %s\n", sysfile1);
				cleanup(0);
			}
			strcpy(file1, cp + 1);
#ifdef FOWARD
			/*
			 * if forwarding for a file to receive 
			 * then simulate a remote
			 * xuucp by prepending reverse order of
			 * of forwarding route to file and xeq uucp on
			 * remote site 
			 */
			if (strchr(file1,'!') != NULL)	
				{
				register char *i;
				if (chkpub(file1)==FAIL) {
					fprintf(stderr,"\n Forwarding from nonpublic directories denied - uucp failed\n");
					cleanup(0);
				}
				dlogent("sysfile2", sysfile2);
				if (strncmp(sysfile2, Myname, SYSNSIZE) == SAME) {

			/*
			 * due to remote fwding mechanism file must
			 * return to public directory
			 */
					if (chkpub(file2)==FAIL) {
					fprintf(stderr,"file must be returned to a public spool directory - uucp failed\n");
					cleanup(0);
					}
				}
				else 
				if (chkpub(file2) == FAIL) {
					fprintf(stderr,"\nforwarding to nonpublic directory not permitted - uucp failed\n");
					cleanup(0);
				}

				/*
			 	* execute uux
			 	* remote uucp
			 	*/
				if (Ename[0] != '\0') {
					xuux(Ename, sysfile1, file1, sysfile2, file2, "");
					--argc;
					argv++;
					continue;
				}
				/*
				 * Form reverse order of fowarding sites
				 */
				*tsys2 = '\0';
				strcpy(tfile1, file1);
				/*
				 * Strip off destination file
				 */
				i = strrchr(tfile1, '!');
				*i = '\0';
				/*
				 * Strip off last site
				 */
				if ((i=strrchr(tfile1, '!')) != NULL){
					*i = '\0';
					/*
					 * Get next site
					 */
					while ((i=strrchr(tfile1,'!')) != NULL){
						strncat(tsys2, i+1, SYSNSIZE);
						strcat(tsys2, "!");
						*i = '\0';
					}
					strcat(tsys2, tfile1);
					strcat(tsys2, "!");
				}
				strncat(tsys2, sysfile1, SYSNSIZE);
				strcat(tsys2, "!");
				strcat(tsys2, Myname);
				if (strncmp(sys2save, Myname, SYSNSIZE) != SAME) {
					strcat(tsys2, "!");
					dlogent(sys2save, "sys2save");
					strcat(tsys2, sys2save);
				}
				sysfile2 = tsys2;
				dlogent(sysfile2,"reverse sysfile2");
			}else 
				if ((strncmp(sysfile1, Myname, SYSNSIZE) != SAME) &&
					(strncmp(sysfile2, Myname, SYSNSIZE) != SAME))
				/*
				 * both sites are remote - no forwarding
				 * to source file
				 */

				{
				if (Ename[0] != '\0')  {
					xuux(Ename, sysfile1, file1, sysfile2,
						file2, "");
					--argc;
					argv++;
					continue;
				}
				if (chkpub(file2) == FAIL) {
					fprintf(stderr,"\nuse -e option for remote transfer to nonpublic directories\n");
					cleanup(0);
				}
				sprintf(tsys2, "%s!%s", Myname, sys2save);
				strcpy(sysfile2, tsys2);
				dlogent(sysfile2, "reverse sys2 -no fowarding");
				}
			/*
			 *  If fowarding then prepend ! to fowarding string
			 */
			if (strchr(file1, '!') != NULL)	{
				*cp = '!';
				strcpy(file1, cp);
				*cp = '\0';
			}
			if ((strncmp(sysfile1, Myname, SYSNSIZE) != SAME) && (file2[0] == '!')){
				/*
				 *    remove leading ! from file2 so that
				 *    copy case 3 will not produce !!
				 */
				strcpy(tfile1, &file2[1]);
				strcpy(file2, tfile1);
			}
#endif
		} else {
			sysfile1 = Myname;
			strcpy(file1, argv[1]);
		}
		DEBUG(4, "file1 - %s\n", file1);
		copy(sysfile1, file1, sysfile2, file2);
#ifdef FOWARD
		strcpy(sysfile2, sys2save);
#endif
		--argc;
		argv++;
	}

	clscfile();

	/*
	 * do not spawn daemon if -r option specified
	 */
	if (*Ropt != '-')
		xuucico("");
	cleanup(0);
}

/*
 * cleanup lock files before exiting
 */
cleanup(code)
register int code;
{
	void exit();

	logcls();
	rmlock(CNULL);
	if (code)
		fprintf(stderr, "uucp failed. code %d\n", code);
	exit(code);
}


/*
 * generate copy files
 * return:
 *	0	-> success
 *	FAIL	-> failure
 */
copy(s1, f1, s2, f2)
char *s1, *f1, *s2, *f2;
{
	FILE *cfp, *gtcfile();
	struct stat stbuf, stbuf1;
	int type, statret;
	char dfile[NAMESIZE];
	char file1[MAXFULLNAME], file2[MAXFULLNAME];
	char *strchr(), *strcat(), *strcpy();
	char *strrcpy();
	char opts[100];

	type = 0;
	opts[0] = '\0';
	strcpy(file1, f1);
	strcpy(file2, f2);
	if (strncmp(s1, Myname, SYSNSIZE) != SAME)
		type = 1;
	if (strncmp(s2, Myname, SYSNSIZE) != SAME)
		type += 2;
	if (type & 01)
		if ((strchr(file1, '*') != NULL
		  || strchr(file1, '?') != NULL
		  || strchr(file1, '[') != NULL))
			type = 4;

	switch (type) {
	case 0:

		/*
		 * all work here
		 */
		DEBUG(4, "all work here %d\n", type);

		/*
		 * check access control permissions
		 */
		if (ckexpf(file1))
			 return(FAIL);
		if (ckexpf(file2))
			 return(FAIL);
		if (stat(file1, &stbuf) != 0) {
			fprintf(stderr, "can't get file status %s\ncopy failed\n",
			  file1);
			return(0);
		}
		statret = stat(file2, &stbuf1);
		if (statret == 0
		  && stbuf.st_ino == stbuf1.st_ino
		  && stbuf.st_dev == stbuf1.st_dev) {
			fprintf(stderr, "%s %s - same file; can't copy\n", file1, file2);
			return(0);
		}
		if (chkpth(User, "", file1) != 0
		  || chkperm(file2, strchr(Optns, 'd'))
		  || chkpth(User, "", file2) != 0) {
			fprintf(stderr, "permission denied\n");
			cleanup(1);
		}
		if ((stbuf.st_mode & ANYREAD) == 0) {
			fprintf(stderr, "can't read file (%s) mode (%o)\n",
			  file1, stbuf.st_mode);
			return(FAIL);
		}
		if (statret == 0 && (stbuf1.st_mode & ANYWRITE) == 0) {
			fprintf(stderr, "can't write file (%s) mode (%o)\n",
			  file2, stbuf.st_mode);
			return(FAIL);
		}

		/*
		 * copy file locally
		 */
		xxcp(file1, file2, stbuf.st_mode | 0666);
		logent("WORK HERE", "DONE");
		return(0);
	case 1:

		/*
		 * receive file
		 */
		DEBUG(4, "receive file - %d\n", type);

		/*
		 * expand source and destination file names
		 * and check access permissions
		 */
		if (file1[0] != '~')
			if (ckexpf(file1))
				 return(FAIL);
		if (ckexpf(file2))
			 return(FAIL);
		if (chkpth(User, "", file2) != 0) {
			fprintf(stderr, "permission denied\n");
			return(FAIL);
		}

			/*
			 * execute uux
			 * remote uucp
			 */
		if (Ename[0] != '\0') {
			xuux(Ename, s1, file1, s2, file2, opts);
			return(0);
		}

		/*
		 * insert JCL in file
		 */
		cfp = gtcfile(s1);
		fprintf(cfp, "R %s %s %s %s %s\n", file1, file2, User, Optns, Sfile);
		break;
	case 2:

		/*
		 * send file
		 */
		if (ckexpf(file1))
			 return(FAIL);
		if (file2[0] != '~')
			if (ckexpf(file2))
				 return(FAIL);
		DEBUG(4, "send file - %d\n", type);

		if (chkpth(User, "", file1) != 0) {
			fprintf(stderr, "permission denied %s\n", file1);
			return(FAIL);
		}
		if (stat(file1, &stbuf) != 0) {
			fprintf(stderr, "can't get status for file %s\n", file1);
			return(FAIL);
		}
		if ((stbuf.st_mode & S_IFMT) == S_IFDIR) {
			fprintf(stderr, "directory name illegal - %s\n",
			  file1);
			return(FAIL);
		}
		if ((stbuf.st_mode & ANYREAD) == 0) {
			fprintf(stderr, "can't read file (%s) mode (%o)\n",
			  file1, stbuf.st_mode);
			return(FAIL);
		}
		if (notifopt && (strchr(Optns, 'n') == NULL))
			strcat(Optns, "n");
		/*
		 * execute uux
		 * remote uucp
		 */
		if (Ename[0] != '\0') {
			if(notifopt)
				sprintf(opts, "-n%s", Nuser);
			xuux(Ename, s1, file1, s2, file2, opts);
			return(0);
		}

		/*
		 * make a copy of file in spool directory
		 */
		if (Copy) {
			updjb();
			sprintf(dfile, "%c.%.6s%c%.1s%.4d", DATAPRE, s2, Grade,
			  subjob, jobid); 
			if (xcp(file1, dfile) != 0) {
				fprintf(stderr, "can't copy %s\n", file1);
				return(FAIL);
			}
			chmod(dfile, DFILMODE);
		} else {

			/*
			 * make a dummy D. name
			 * cntrl.c knows names < 6 chars are dummy D. files
			 */
			strcpy(dfile, "D.0");
		}

		/*
		 * insert JCL in file
		 */
		cfp = gtcfile(s2);
		fprintf(cfp, "S  %s %s %s %s %s %o %s %s\n", file1, file2,
			User, Optns, dfile, stbuf.st_mode & 0777, Nuser, Sfile);
		break;
	case 3:
	case 4:

		/*
		 * send uucp command for execution on s1
		 */
		DEBUG(4, "send uucp command - %d\n", type);
		if (strncmp(s2,  Myname, SYSNSIZE) == SAME) {
			if (ckexpf(file2))
				 return(FAIL);
			if (chkpth(User, "", file2) != 0) {
				fprintf(stderr, "permission denied\n");
				return(FAIL);
			}
		}

			/*
			 * execute uux
			 * remote uucp
			 */
		if (Ename[0] != '\0') {
			xuux(Ename, s1, file1, s2, file2, opts);
			return(0);
		}

		/*
		 * insert JCL in file
		 */
		cfp = gtcfile(s1);
		fprintf(cfp, "X %s %s!%s %s %s %s\n", file1, s2, file2, User, Optns, Sfile);
		break;
	}
	return(0);
}

/*
 * check security violations - forwarding to/from nonpublic directories
 *	ckfile	-> src/dst path
 *	return:	0 -> ok
 *		FAIL -> not ok
 */
chkpub(ckfile)
char *ckfile;
{
	char *s;
	register char *i;
	s = (i=strrchr(ckfile,'!'))?i+1:ckfile;

	/*
	 *	Ensure forwarding to/from public spool directory
	 */

	if (!prefix("~/",s) && !prefix(PUBDIR,s))
		return(FAIL);
	
	/*
	 *	Detect illegal attempts to forward though (~/../) public dir.
	 */

	for (i = ++s; *i != '\0'; i++)
		if (*i == '/' && prefix("../",(++i)))  
			return(FAIL);
	return(0);
}
				
/*
 * execute uux for remote uucp
 *	ename	-> remote system name to execute uucp on
 *	s1	-> remote system name
 *	f1	-> remote file name
 *	s2	-> remote system name
 *	f2	-> remote file name
 *	opts	-> uucp options
 * return:
 *	none
 */
xuux(ename, s1, f1, s2, f2, opts)
char *ename, *s1, *s2, *f1, *f2, *opts;
{
	char cmd[200];

	DEBUG(4, "Ropt(%s) ", Ropt);
	DEBUG(4, "ename(%s) ", ename);
	DEBUG(4, "s1(%s) ", s1);
	DEBUG(4, "f1(%s) ", f1);
	DEBUG(4, "s2(%s) ", s2);
	DEBUG(4, "f2(%s)\n", f2);
#ifdef FOWARD
	if (*f2=='!') f2++;
#endif
	sprintf(cmd, "uux %s %s!uucp -C %s %s!%s \\(%s!%s\\)",
	 Ropt, ename, opts,  s1, f1, s2, f2);
	DEBUG(4, "cmd (%s)\n", cmd);
	system(cmd);
	return;
}

FILE *Cfp = NULL;
char Cfile[NAMESIZE];
#define NSYS	20
struct presys{
	char	pre_name[NAMESIZE];
	char	pre_id[2];
	char	pre_grade;
}presys[NSYS];
int	nsys	= 0;
struct presys *csys();

/*
 * get a Cfile descriptor
 *	sys	-> system name
 * return:
 *	an open file descriptor
 */
FILE *
gtcfile(sys)
register char *sys;
{
	static int cmdcount = 0;
	register int i;
	struct presys *p;
	extern int errno;
	char *strcpy();

	/*
	 * a new c.file is generated each time a different 
	 * system is referenced
	 */
	if((p = csys(sys, Grade)) == NULL){

		if(nsys != 0){
			clscfile();
		}
		if(cmdcount == 0){
			if(prjob == JOBON)
				fprintf(stdout,"uucp job %d\n", jobid);
		}
		updjb();
		sprintf(Cfile, "%c.%.6s%c%.1s%.4d", CMDPRE, sys, Grade, subjob,
		  jobid); 
		cmdcount = 1;
		Cfp = fopen(Cfile, "a+");
		ASSERT(Cfp != NULL, "CAN'T OPEN", Cfile, 0);
		insys(sys, subjob[0], Grade);
	}else
	if(strcmp(presys[nsys-1].pre_name, sys) != SAME){
		clscfile();
		sprintf(Cfile, "%c.%.6s%c%.1s%.4d", CMDPRE, sys, Grade,
		  p->pre_id, jobid); 
		Cfp = fopen(Cfile, "a+");
		ASSERT(Cfp != NULL, "CAN'T OPEN", Cfile, 0);
	}
		return(Cfp);
}
struct presys *
csys(p, g)
register char *p, g;
{
	register int i;
	static int jid = 0;

	if(jobid != jid){
		jid = jobid;
		nsys = 0;
	}
	for(i=0;i<nsys;i++){
		if(strcmp(p,&presys[i].pre_name[0]) == SAME)
			if(g == presys[i].pre_grade)
				return(&presys[i]);
	}
	return(NULL);
}

insys(p, c, g)
register char *p;
char	c;
{

	strcpy(presys[nsys].pre_name, p);
	presys[nsys].pre_grade = g;
	presys[nsys++].pre_id[0] = c;
}

/*
 * close cfile
 * return:
 *	none
 */
clscfile()
{
	if (Cfp == NULL)
		return;
	fclose(Cfp);
/*
	chmod(Cfile, ~WFMASK & 0777);
*/
	logent(Cfile, "QUE'D");
	US_CRS(Cfile);
	Cfp = NULL;
	return;
}
