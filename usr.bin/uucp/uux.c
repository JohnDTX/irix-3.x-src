char _Origin_[] = "System V";
/* @(#)uux.c	1.6 */
/* $Source: /d2/3.7/src/usr.bin/uucp/RCS/uux.c,v $*/
/* $Revision: 1.1 $*/
/* $Date: 89/03/27 18:31:06 $*/

#include "uucp.h"

#define NOSYSPART 0
#define HASSYSPART 1

#define APPCMD(d) {\
char *p;\
for (p = d; *p != '\0';) *cmdp++ = *p++;\
*cmdp++ = ' ';\
*cmdp = '\0';}

#define GENSEND(f, a, b, c, d, e) {\
fprintf(f, "S %s %s %s -%s %s %s 0666 %s %s\n", a, b, c, statop?"o":"", d, e, Nuser, Sfile);\
}
#define GENRCV(f, a, b, c) {\
fprintf(f, "R %s %s %s -%s %s\n", a, b, c, 0?"o":"", Sfile);\
}

#ifdef FOWARD
#define GENXUUCP(f, a, b, c) {\
fprintf(f, "X %s %s %s - \n", a, b, c);\
}

#define FORWARDING(a) (a[0] != '\0')
#endif

char *strcpy();
#define JOBON	1
#define JOBOFF	0
#ifdef ONJOB
int	prjob = JOBON;
#else
int	prjob	= JOBOFF;
#endif

char	Sfile[MAXFULLNAME], Optns[10];
int	statop;
/*
 *	uux
 */
char	**Env;
char	Nuser[32];
main(argc, argv, envp)
char *argv[];
char	**envp;
{
	FILE *fprx, *fpc, *fpd, *fp;
	extern FILE *ufopen();
	int cflag = 0;		/*  commands in C. file flag  */
	int rflag = 0;		/*  C. files for receiving flag  */
	int pipein = 0;
	int startjob = 1;
	int Copy = 1;
	int nonoti = 0;
#ifdef X_NONZERO
	int nonzero = 0;
#endif
	int	uid, ret;
	char *getprm(), *strchr(), *strrchr(), *strcat(), *lxp;
	char redir = '\0';
	char cfile[NAMESIZE];	/* send commands for files from here */
	char dfile[NAMESIZE];	/* used for all data files from here */
	char rxfile[NAMESIZE];	/* to be sent to xqt file (X. ...) */
	char tfile[NAMESIZE];	/* temporary file name */
	char tcfile[NAMESIZE];	/* temporary file name */
	char t2file[NAMESIZE];	/* temporary file name */
	char buf[BUFSIZ];
	char inargs[BUFSIZ];
	char path[MAXFULLNAME];
	char cmd[BUFSIZ];
	char *ap, *cmdp;
	char prm[BUFSIZ];
	char syspart[NAMEBUF], rest[MAXFULLNAME];
	char xsys[NAMEBUF], local[NAMEBUF];
	char	*ep;
	char	*fopt = NULL;

#ifdef FOWARD
	char fsys[NAMEBUF];  	      /* temp var to hold fowarding site */
	char xfwdroute[MAXFULLNAME];  /* route from local sys to xeq system */
	char revroute[MAXFULLNAME];   /* route from xeq sys to local system */
	char fwdroute[MAXFULLNAME];   /* route to file from xeq system */
	char rfwdroute[MAXFULLNAME];  /* route from file to xeq system */
	char tfile1[MAXFULLNAME];     /* temp var to hold route */
	char droute[MAXFULLNAME];     /* dest route including file name */
	char *i;
	char *strrchar();
	char *strcat();
#endif

	(void) tzset();		/* SGI: (^*~%!$@%^&. */
	Env = envp;
	if((ep = (char *)getenv("JOBNO="))){
		if(strcmp("ON", ep) == SAME)
			prjob = JOBON;
		if(strcmp("OFF", ep) == SAME)
			prjob = JOBOFF;
	}
	/*
	 * determine local system name
	 */
	strcpy(Progname, "uux");
	Pchar = 'X';
	uucpname(Myname);
	umask(WFMASK);
	Ofn = 1;
	Ifn = 0;
	Nuser[0] = Sfile[0] = '\0';
	while (argc>1 && argv[1][0] == '-') {
		switch(argv[1][1]){

		/*
		 * -p or - option specifies input from pipe
		 */
		case 'p':
		case '\0':
			pipein = 1;
			break;
		case 'm':
			strcat(Optns, "m");
			if(argv[1][2] != '\0'){
				fopt = &argv[1][2];
				strcat(Optns, "o");
				statop++;
			}
			break;
		case 'j':
			prjob = (prjob == JOBON)?JOBOFF:JOBON;
			break;

		/*
		 * do not start transfer
		 */
		case 'r':
			startjob = 0;
			break;

		case 'c':
			Copy = 0;
			break;

		/*
		 * debugging level
		 */
		case 'x':
			Debug = atoi(&argv[1][2]);
			if (Debug <= 0)
				Debug = 1;
			break;

		/*
		 * do not send notification to user
		 */
		case 'n':
			nonoti = 1;
			break;
#ifdef X_NONZERO
		case 'z':
			nonzero = 1;
			break;
#endif
		default:
			fprintf(stderr, "unknown flag %s\n", argv[1]);
				break;
		}
		--argc;  argv++;
	}

	DEBUG(4, "\n\n** %s **\n", "START");

	/*
	 * copy arguments into a buffer for later
	 * processing
	 */
	inargs[0] = '\0';
	for (argv++; argc > 1; argc--) {
		DEBUG(4, "arg - %s:", *argv);
		strcat(inargs, " ");
		strcat(inargs, *argv++);
	}

	/*
	 * get working directory and change
	 * to spool directory
	 */
	DEBUG(4, "arg - %s\n", inargs);
	gwd(Wrkdir);
	if(fopt){
		if(*fopt != '/')
			sprintf(Sfile, "%s/%s", Wrkdir, fopt);
		else
			sprintf(Sfile, "%s", fopt);

	}
	ret = chdir(Spool);
	ASSERT(ret == 0, "CANNOT CHDIR TO SPOOL - ", Spool, ret);
	if(ret != 0) {
		fprintf(stderr, "No spool directory - %s - get help\n", Spool);
		cleanup(0);
	}

	/*
	 * determine id of user starting remote 
	 * execution
	 */
	uid = getuid();
	guinfo(uid, User, path);
	if(Nuser[0] == '\0')
		strcpy(Nuser, User);

	sprintf(local, "%.*s", SYSNSIZE, Myname);

	/*
	 * initialize command buffer
	 */
	cmdp = cmd;
	*cmdp = '\0';

	/*
	 * generate JCL files to work from
	 */
	gename(DATAPRE, local, 'X', rxfile);
	fprx = ufopen(rxfile, "w");
	ASSERT(fprx != NULL, "CAN'T OPEN", rxfile, 0);
	updjb(); sprintf(tcfile, "%c.%.6s%c%.1s%.4d", DATAPRE, local, 'T', subjob, jobid);
	fpc = ufopen(tcfile, "w");
	ASSERT(fpc != NULL, "CAN'T OPEN", tcfile, 0);
#ifndef FOWARD
	fprintf(fprx, "%c %s %s\n", X_USER, User, local);
#endif
	if (nonoti)
		fprintf(fprx,"%c\n", X_NONOTI);
#ifdef X_NONZERO
	if (nonzero)
		fprintf(fprx,"%c\n", X_NONZERO);
#endif
	if (statop)
		fprintf(fprx,"%c %s\n", X_MAILF, Sfile);

	/*
	 * find remote system name
	 * remote name is first to know that 
	 * is not > or <
	 */
	ap = inargs;
	xsys[0] = '\0';
	while ((ap = getprm(ap, prm)) != NULL) {
		if (prm[0] == '>' || prm[0] == '<') {
			ap = getprm(ap, prm);
			continue;
		}

		/*
		 * split name into system name
		 * and command name
		 */
		split(prm, xsys, rest);
#ifdef FOWARD
		xfwdroute[0] = '\0';
		while (split(rest, fsys, rest) != NOSYSPART)  {
			strncat(xfwdroute, xsys, SYSNSIZE);
			strcat(xfwdroute, "!");
			strcpy(xsys, fsys);
			DEBUG(4, "fsys - %s\n", fsys);
		}	
		if (xfwdroute[0] != '\0')  
			strncat(xfwdroute, xsys, SYSNSIZE);
		DEBUG(4, "xfwdroute - %s\n", xfwdroute);
		/*
		 *  create return route to user
		 */
		revroute[0] = '\0';
		strcpy(tfile1, xfwdroute);
		if ((i=strrchr(tfile1, '!')) != NULL)   {
			*i = '\0';  /* strip off destination system */
			while ((i=strrchr(tfile1, '!')) != NULL)
				{
				strncat(revroute, i+1, SYSNSIZE);
				strcat(revroute, "!");
				*i = '\0';
				}
			strncat(revroute, tfile1, SYSNSIZE);
			strcat(revroute, "!");
			strncat(revroute, local, SYSNSIZE);
		}
		DEBUG(4,"revroute - %s\n",revroute);
		fprintf(fprx, "%c %s %s\n", X_USER, User, (revroute[0]=='\0')?local:revroute);
#endif
		break;
	}
	if (xsys[0] == '\0')
		strncpy(xsys, local, SYSNSIZE);
	sprintf(Rmtname, "%.*s", SYSNSIZE, xsys);
	DEBUG(4, "xsys %s\n", xsys);

	/*
	 * check to see if system name is valid
	 */
	if (versys(xsys) != 0) {

		/*
		 * bad system name
		 */
		fprintf(stderr, "bad system name: %s\n", xsys);
		fclose(fprx);
		fclose(fpc);
		cleanup(101);
	}

	/*
	 * create a JCL file to spool pipe input into
	 */
	if (pipein) {
		updjb(); sprintf(dfile, "%c.%.6s%c%.1s%.4d", DATAPRE, xsys, 'B', subjob, jobid);
		fpd = ufopen(dfile, "w");
		ASSERT(fpd != NULL, "CAN'T OPEN", dfile, 0);

		/*
		 * read pipe to EOF
		 */
		while (!feof(stdin)) {
			ret = fread(buf, 1, BUFSIZ, stdin);
			fwrite(buf, 1, ret, fpd);
		}

		/*
		 * if command is to be executed on remote
		 * create extra JCL
		 */
		fclose(fpd);
#ifdef FOWARD
		if (FORWARDING(xfwdroute)) {
			sprintf(droute, "%s!%s", strchr(xfwdroute, '!'), dfile);
			GENSEND(fpc, dfile, droute, User, "", dfile);
			cflag++;
		}
		else    
			if (strncmp(local, xsys, SYSNSIZE) != SAME) {
				GENSEND(fpc, dfile, dfile, User, "", dfile);
				cflag++;
			}
#else
		if (strncmp(local, xsys, SYSNSIZE) != SAME) {
			GENSEND(fpc, dfile, dfile, User, "", dfile);
			cflag++;
		}
#endif

		/*
		 * create JCL specifying pipe input
		 */
		fprintf(fprx, "%c %s\n", X_RQDFILE, dfile);
		fprintf(fprx, "%c %s\n", X_STDIN, dfile);
	}
	if(prjob == JOBON)
		fprintf(stdout,"uucp job %d\n", jobid);

	/*
	 * parse command
	 */
	ap = inargs;
	while ((ap = getprm(ap, prm)) != NULL) {
		DEBUG(4, "prm - %s\n", prm);

		/*
		 * redirection of I/O
		 */
		if (prm[0] == '>' || prm[0] == '<') {
			redir = prm[0];
			continue;
		}

		/*
	 	 * sequential execution
	 	 */
		if (prm[0] == ';') {
			APPCMD(prm);
			continue;
		}

		/*
		 * pipe in command line
		 */
		if (prm[0] == '|' || prm[0] == '^') {
			if (cmdp != cmd)
				APPCMD(prm);
			continue;
		}

		/*
		 * process command or file or option
		 * break out system and file name and
		 * use default if necessary
		 */
		ret = split(prm, syspart, rest);
#ifdef FOWARD
		fwdroute[0] = '\0';
		if (ret==HASSYSPART) 
			while (split(rest,fsys,rest) != NOSYSPART)  {
				strncat(fwdroute, syspart, SYSNSIZE);
				strcat(fwdroute, "!");
				strcpy(syspart, fsys);
				DEBUG(4,"fsys - %s\n",fsys);
				}	
		if (fwdroute[0] != '\0')  
			strncat(fwdroute, syspart, SYSNSIZE);
		DEBUG(4,"fwdroute - %s\n",fwdroute);
		/*
		 *  create reverse of the forwarding route
		 */
		rfwdroute[0] = '\0';
		strcpy(tfile1, fwdroute);
		if ((i=strrchr(tfile1, '!')) != NULL)   {
			*i = '\0';  /* stip off destination system */
			while ((i=strrchr(tfile1, '!')) != NULL)
				{
				strncat(rfwdroute, i+1, SYSNSIZE);
				strcat(rfwdroute, "!");
				*i = '\0';
				}
			strncat(rfwdroute, tfile1, SYSNSIZE);
			strcat(rfwdroute, "!");
			strncat(rfwdroute, xsys, SYSNSIZE);
		}
		DEBUG(4,"rfwdroute - %s\n",rfwdroute);
#endif
		DEBUG(4, "s - %s, ", syspart);
		DEBUG(4, "r - %s, ", rest);
		DEBUG(4, "ret - %d\n", ret);
		if (syspart[0] == '\0')
			strncpy(syspart, local, SYSNSIZE);

		if (cmdp == cmd && redir == '\0') {

			/*
			 * command
			 */
			APPCMD(rest);
			continue;
		}

		/*
		 * process file or option
		 */
		DEBUG(4, "file s- %s, ", syspart);
		DEBUG(4, "local - %s\n", local);

		/*
		 * proess file argument
		 * expand filename and create JCL
		 * redirected output
		 */
		if (redir == '>') {
			if (rest[0] != '~')
				if (ckexpf(rest))
					cleanup(2);
#ifdef FOWARD
			if (FORWARDING(fwdroute)) {
				if (chkpub(rest) == FAIL) {
				fprintf(stderr,"\nredirected output to nonpublic directory\n");
				cleanup(10);
				}
				
				sprintf(droute, "%s!%s", (i=strchr(fwdroute, '!')), rest);
				strcpy(rest, droute);
				*i = '\0';
				strcpy(syspart, fwdroute);
				}
#endif
			fprintf(fprx, "%c %s %s\n", X_STDOUT, rest,
			 syspart);
			redir = '\0';
			continue;
		}

		/*
		 * if no system specified, then being
		 * processed locally
		 */
		if (ret == NOSYSPART && redir == '\0') {

			/*
			 * option
			 */
			APPCMD(rest);
			continue;
		}


		/*
		 * if file specified is on local system and
		 * command being executed locally create JCL
		 */
		if (strncmp(xsys, local, SYSNSIZE) == SAME
		 && strcmp(xsys, syspart) == SAME) {
			if (ckexpf(rest))
				cleanup(2);
			if (redir == '<')
				fprintf(fprx, "%c %s\n", X_STDIN, rest);
			else
				APPCMD(rest);
			redir = '\0';
			continue;
		}

		/*
		 * command executed remotely but file local
		 */
		if (strncmp(syspart, local, SYSNSIZE) == SAME) {

			/*
			 * generate send file
			 */
			if (ckexpf(rest))
				cleanup(2);
			updjb(); sprintf(dfile, "%c.%.6s%c%.1s%.4d", DATAPRE, xsys, 'A', subjob, jobid);
			DEBUG(4, "rest %s\n", rest);

			/*
			 * check access to local file and copy it
			 * to spool directory
			 */
			if ((chkpth(User, "", rest) || anyread(rest)) != 0) {
				fprintf(stderr, "permission denied %s\n", rest);
				cleanup(1);
			}
			if (Copy && xcp(rest, dfile) != 0) {
				fprintf(stderr, "can't copy %s to %s\n", rest, dfile);
				cleanup(1);
			}

			/*
			 * generate entry in command file to
			 * have file send
			 */
#ifdef FOWARD
			if (FORWARDING(xfwdroute))
				sprintf(droute, "%s!%s", strchr(xfwdroute, '!'),
				  dfile);
			GENSEND(fpc, rest,
			  FORWARDING(xfwdroute) ? droute : dfile,
			  User, Copy ? "" : "c", dfile);
#else
			GENSEND(fpc, rest, dfile, User, Copy ? "" : "c", dfile);
#endif
			cflag++;

			/*
			 * generate entry for redirected input
			 */
			if (redir == '<') {
				fprintf(fprx, "%c %s\n", X_STDIN, dfile);
				fprintf(fprx, "%c %s\n", X_RQDFILE, dfile);
			} else {
				APPCMD((lxp=strrchr(rest, '/'))?lxp+1:rest);
				fprintf(fprx, "%c %s %s\n", X_RQDFILE,
				 dfile, (lxp=strrchr(rest, '/'))?lxp+1:rest);
			}
			redir = '\0';
			continue;
		}

		/*
		 * if command executed locally prepare
		 * JCL to receive file
		 */
		if (strncmp(local, xsys, SYSNSIZE) == SAME) {

			/*
			 * generate local receive command file
			 */
#ifdef FOWARD
			if (FORWARDING(fwdroute))  {
			/*
			 * find next site in route
			 */
				*(i= strchr(fwdroute, '!')) = '\0';
				strcpy(syspart, fwdroute);
				*i = '!';
				}
#endif
			updjb(); sprintf(tfile, "%c.%.6s%c%.1s%.4d", CMDPRE, syspart, 'R', subjob, jobid);
			strcpy(dfile, tfile);
			dfile[0] = DATAPRE;
			fp = ufopen(tfile, "w");
			ASSERT(fp != NULL, "CAN'T OPEN", tfile, 0);

			/*
			 * expand receive file name
			 */
			if (ckexpf(rest))
				cleanup(2);
#ifdef FOWARD
			if (FORWARDING(fwdroute))  {
			/*
			 * format a message to receive data via forwarding
			 * - method: execute uucp on system that has 
			 *           the needed file
			 */
				char to[MAXFULLNAME];
				char from[MAXFULLNAME];
				if (chkpub(rest) == FAIL) {
				fprintf(stderr,"\ncannot receive file via forwarding from non public directory\n");
				cleanup(11);
				}
				sprintf(from, "%s!%s", strchr(fwdroute, '!'), rest);
				sprintf(to,"%s!%s",rfwdroute,dfile);
				GENXUUCP(fp,from,to,User);
				}
			else
				GENRCV(fp, rest, dfile, User);
#else
			GENRCV(fp, rest, dfile, User);
#endif
			fclose(fp);
			rflag++;
			if (rest[0] != '~')
				if (ckexpf(rest))
					cleanup(2);

			/*
			 * generate receive entries
			 */
			if (redir == '<') {
				fprintf(fprx, "%c %s\n", X_RQDFILE, dfile);
				fprintf(fprx, "%c %s\n", X_STDIN, dfile);
			} else {
				fprintf(fprx, "%c %s %s\n", X_RQDFILE, dfile,
				  (lxp=strrchr(rest, '/'))?lxp+1:rest);
				APPCMD((lxp=strrchr(rest, '/'))?lxp+1:rest);
			}

			redir = '\0';
			continue;
		}

		/*
		 * file not on same system as execution
		 * create JCL for remote receives
		 */
		if (strcmp(syspart, xsys) != SAME) {

			/*
			 * generate remote receives for remote execution
			 */
#ifdef FOWARD
			if (FORWARDING(fwdroute))  {
			/*
			 * find next site in route
			 */
				*(i= strchr(fwdroute, '!')) = '\0';
				strcpy(syspart, fwdroute);
				*i = '!';
				}
#endif
			updjb(); sprintf(dfile, "%c.%.6s%c%.1s%.4d", DATAPRE, syspart, 'R', subjob, jobid);
			strcpy(tfile, dfile);
			tfile[0] = CMDPRE;

			/*
			 * make a data file which contains what
			 * a command file would contain on the
			 * remote to receive a file
			 */
			fpd = ufopen(dfile, "w");
			ASSERT(fpd != NULL, "CAN'T OPEN", dfile, 0);
			updjb(); sprintf(t2file, "%c.%.6s%c%.1s%.4d", DATAPRE, xsys, 'T', subjob, jobid);
#ifdef FOWARD
			if (FORWARDING(fwdroute))  {
				char from[MAXFULLNAME];
				char to[MAXFULLNAME];
				if (chkpub(rest) == FAIL) {
				fprintf(stderr,"\ncannot receive file via forwarding from non public directory\n");
				cleanup(12);
				}
				sprintf(from, "%s!%s", strchr(fwdroute, '!'), rest);
				sprintf(to,"%s!%s",rfwdroute,t2file);
				GENXUUCP(fpd, from, to, User);
				}
			else    
				GENRCV(fpd, rest, t2file, User);
#else
			GENRCV(fpd, rest, t2file, User);
#endif
			fclose(fpd);
#ifdef FOWARD
			if (FORWARDING(xfwdroute))  {
				char to[MAXFULLNAME];
				sprintf(to, "%s!%s", strchr(xfwdroute, '!'), tfile);
				GENSEND(fpc, dfile, to, User, "", dfile);
				}
			else
				GENSEND(fpc, dfile, tfile, User, "", dfile);
#else
			GENSEND(fpc, dfile, tfile, User, "", dfile);
#endif
			cflag++;
			if (redir == '<') {
				fprintf(fprx, "%c %s\n", X_RQDFILE, t2file);
				fprintf(fprx, "%c %s\n", X_STDIN, t2file);
			} else {
				fprintf(fprx, "%c %s %s\n", X_RQDFILE, t2file,
				  (lxp=strrchr(rest, '/'))?lxp+1:rest);
				APPCMD((lxp=strrchr(rest, '/'))?lxp+1:rest);
			}
			redir = '\0';
			continue;
		}

		/*
		 * file on remote system
		 */
		if (rest[0] != '~')
			if (ckexpf(rest))
				cleanup(2);
		if (redir == '<')
			fprintf(fprx, "%c %s\n", X_STDIN, rest);
		else
			APPCMD(rest);
		redir = '\0';
		continue;

	}

	/*
	 * place command to be executed in JCL file
	 */
	fprintf(fprx, "%c %s\n", X_CMD, cmd);
	logent(cmd, "XQT QUE'D");
	fclose(fprx);

	strcpy(tfile, rxfile);
	tfile[0] = XQTPRE;

	/*
	 * see if execution is to take place locally
	 */
	if (strncmp(xsys, local, SYSNSIZE) == SAME) {
		dlogent(rxfile, "rxfile1");
		link(rxfile, tfile);
		unlink(rxfile);

		/*
		 * see if -r option requested JCL to be queued only
		 */
		if (startjob)
			if (rflag)
				xuucico("");
			else
				xuuxqt();
	} else {
#ifdef FOWARD
		if (FORWARDING(xfwdroute)) {
			sprintf(droute, "%s!%s", strchr(xfwdroute, '!'), tfile);
			GENSEND(fpc, rxfile, droute, User, "", rxfile);
		}else    {
			dlogent(rxfile, "rxfile3");
			GENSEND(fpc, rxfile, tfile, User, "", rxfile);
		}
#else
		dlogent(rxfile, "rxfile4");
		GENSEND(fpc, rxfile, tfile, User, "", rxfile);
#endif
		cflag++;
	}

	fclose(fpc);

	/*
	 * has any command been placed in command JCL file
	 */
	if (cflag) {
#ifdef FOWARD
		if (FORWARDING(xfwdroute)) {
			*(strchr(xfwdroute, '!')) = '\0';
			strcpy(xsys, xfwdroute);
		}
#endif
		updjb(); sprintf(cfile, "%c.%.6s%c%.1s%.4d", CMDPRE, xsys, 'A', subjob, jobid);
		link(tcfile, cfile);
		unlink(tcfile);

		/*
		 * see if -r option requested JCL to be queued only
		 */
		if (startjob)
			xuucico(xsys);
		cleanup(0);
	}
	else
		unlink(tcfile);
}

#define FTABSIZE 30
char Fname[FTABSIZE][NAMESIZE];
int Fnamect = 0;

/*
 * cleanup and unlink if error
 *	code	-> exit code
 * return:
 *	none
 */
cleanup(code)
register int code;
{
	register int i;
	void exit();

	rmlock(CNULL);
	if (code) {
		for (i = 0; i < Fnamect; i++)
			unlink(Fname[i]);
		fprintf(stderr, "uux failed. code %d\n", code);
	}
	DEBUG(1, "exit code %d\n", code);
	exit(code);
}

/*
 * open file and record name
 *	file	-> file to create
 *	mode	-> open permissions (r, w, rx, etc)
 * return:
 *	*file	-> file pointer
 */
FILE *ufopen(file, mode)
char *file, *mode;
{
	if (Fnamect < FTABSIZE)
		strcpy(Fname[Fnamect++], file);
	return(fopen(file, mode));
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

	for (i = s; *i != '\0'; i++)
		if (*i == '/' && prefix("../",(++i)))  
			return(FAIL);
	return(0);
}
				
