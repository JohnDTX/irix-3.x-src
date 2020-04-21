/*	@(#)daemon.c	1.3	*/
/*	3.0 SID #	1.2	*/
/*
 *  daemon.c -- main control routines for spider and phone dpd daemons,
 *			and for line printer daemon.
 */

long	snsum = 0;		/*number of characters written*/

#include	"daemon0.c"

#include	<setjmp.h>
#include	<dirent.h>

struct	dirent	*entp;

#define	ODIRSIZ	14	/* DIRSIZ is no longer the same with file system
			||  independent directories, but the algorithms
			||  used by lpr guarantee that the name lengths
			||  won't exceed 14 chars, so rename DIRSIZ.
			*/

int	LDIRNAM	= 0;		/*length of directory name.*/
int	LCHAR	= 0;		/*length of file name thru 'df'.*/
int	LPID	= 0;		/*length of file name to process-id.*/
int	retcode	= 1;
int	nwait	= 0;
char	*fowner;
char	baddf[30];		/*name for bad df-files.*/
FILE	*popen();
int	waittm;

main(argc, argv)
{
	register char *p1, *p2;
	DIR *df;

	dem_setup(argc, argv);
	if(tlimit < WAITTM)
		WAITTM = tlimit;
	waittm = WAITTM;
	LDIRNAM = strlen(dfname);
	if(dfname[LDIRNAM-1] != '/')
		dfname[LDIRNAM++] = '/';
	if(LDIRNAM+ODIRSIZ+1 > sizeof(dfname)){
		logerr("Program bug: dfname overflow.");
		unlink(lock);
		exit(1);
	}
	LCHAR = LDIRNAM + 2;
	LPID = LDIRNAM + 3;

again:
	snsum = 0;
	if(access(lock, 0) < 0){
		logerr("Lock has disappeared.");
		dem_dis();
		exit(1);
	}
	df = opendir(dpd);
	if (df) {
		do {
			if((entp = readdir(df)) == NULL) {
				dem_dis();	/*disconnect phone line.*/
				unlink(lock);
				exit(0);
			}
		} while (entp->d_ino==0 || entp->d_name[0]!='d' || entp->d_name[1]!='f');
		closedir(df);
	}
	p1 = entp->d_name;
	p2 = &dfname[LDIRNAM];
	while (p1 < &entp->d_name[ODIRSIZ])
		*p2++ = *p1++;
	if (trysend() == 0) {
		waittm = WAITTM;
		nwait = 0;
		goto again;
	}
	if(tlimit == 0){
		logerr("Daemon dies on request");
		exit(2);
	}
#if PHONE
	if(nwait > 10){		/*after 3 hours try fresh daemon.*/
		unlink(lock);
		execl("/usr/lib/dpd", "dpd", argv1, (char *)0);
		execl("/etc/dpd", "dpd", argv1, (char *)0);
		logerr("Can't find dpd.");
		exit(1);
	}
#endif
	sleep(waittm);
	if (waittm < tlimit)
		waittm *= 2;
	else
		nwait++;
	goto again;
}

/*
 * The remaining part is the device interface, to spider, the dataphone,
 *    or the line printer.
 */

#define	MXLINE	128
#define	MXMESS	500

char	*snumb;
char	line[MXLINE];
char	message[MXMESS+1];
char	*mesp = message;
char	mailfname[64];
int	linel;
FILE	*dfb = NULL;
jmp_buf	env;
char	ff	= '\014';		/*formfeed*/
char	*copline();

trysend()
{
	if(retcode != 0)
		if(retcode = dem_con())		/*connect phone line.*/
			return(retcode);
	retcode = 1;
	if(setjmp(env))
		return(retcode);
	xtrysend();
	return(0);
}

xtrysend()
{
	dem_open(dfname);		/*open spider connection.*/
	if((dfb = fopen(dfname, "r")) == NULL){
		mvbaddf();
		trouble("Can't read %s.", dfname);
	}
	getowner(dfname);
	mesp = message;
	*mesp = 0;
	copline(dfname, 30, mailfname);
	while (getline()) switch (line[0]) {

	case 'S':
		get_snumb();		/*get snumb for GCOS.*/
		continue;

	case 'B':
	case 'F':
	case 'P':
	case 'T':
		if(sascii(line[0]))
			trouble("Can't send %s.", &line[1]);
		continue;

	case 'I':			/*mail back $IDENT card.*/
		mesp = copline(&line[1], linel-1, mesp);

	case 'L':
		lwrite();		/*write a literal line.*/
		continue;

	case 'M':
		continue;

	case 'N':			/*mail back file name.*/
		copline(&line[1], linel-1, mailfname);
		continue;

	case 'Q':			/*additional text to mail back*/
		if(mesp+linel <= message+MXMESS)
			mesp = copline(&line[1], linel-1, mesp);
			continue;

	case 'U':
		continue;

	default:
		etcp1(line[0]);
		continue;

	}
/*
 * Second pass.
 * Unlink files and send mail.
 */
	alarm(0);
	fseek(dfb, (long)0, 0);
	while (getline()) switch (line[0]) {

	default:
		continue;

	case 'U':
		unlink(&line[1]);
		continue;

	case 'M':
		sendmail();
		continue;
	}
	FCLOSE(dfb);
	dem_close();		/*close connection to spider.*/
	unlink(dfname);
	retcode = 0;
	trouble("OK: %-5s %-7s %-9s", snumb, fowner, dfname+LDIRNAM);
}

char	mail[66]	= "mail ";
FILE	*pmail;

sendmail()
{
	register i;

	alarm(0);
	sprintf(mail, "mail %.60s\n", &line[1]);
	if((pmail = popen(mail, "w")) != NULL){
		maildname();
		if(pclose(pmail) == 0)
			return;
	}
	logerr("Can't mail: %-5s %-8s",snumb,dfname+LDIRNAM);
	return;
}

#if LPD == 0

maildname()		/*break up dfname into command name, and process-id
			    to send back in the mail.*/
{
	char c;
	char *command;

	c = dfname[LCHAR];
	switch (c){
	case 'A':
	case 'a':
		command = "dpr";
		break;

	case 'G':
	case 'g':
		command = "gcat";
		break;

	case 'j':
		command = "ibm";
		break;

	case 'k':
		command = "tekstare";
		break;

	case 'l':
		command = "labmake";
		break;

	case 'm':
		command = "mail";
		break;

	case 'N':
	case 'n':
		command = "fsend";
		break;

	case 't':
		command = "fget";
		break;

	default:
		command = &dfname[LDIRNAM];
		break;
	}
	fprintf(pmail,"Sent %s job %-5s:  file  %s%s",
		command, snumb, mailfname, message);
}

#endif


getline()
{
	register char *lp;
	register c;

	lp = line;
	linel = 0;
	while ((c = getc(dfb)) != '\n' && linel < MXLINE-2) {
		if (c == EOF)
			return(0);
		if (c=='\t') {
			do {
				*lp++ = ' ';
				linel++;
			} while ((linel & 07) != 0);
			continue;
		}
		*lp++ = c;
		linel++;
	}
	*lp++ = 0;
	return(1);
}

char *
copline(ab, n, ml)
char	*ab;
int	n;
char	ml[64];
{
	register char *b, *p, *eb;

	b = ab;
	eb = b+n;
	p = ml;
	while (*b && b<eb && p < &ml[62])
		*p++ = *b++;
	*p++ = '\n';
	*p = 0;	
	return(p);
}


mvbaddf()
{
	int i;

	baddf[0] = 0;
	if(LDIRNAM < (i = sizeof(baddf)-1)){
		strncpy(baddf, dfname, i);
		baddf[i] = '\0';
		baddf[LDIRNAM] = 'b';
		link(dfname, baddf);
	}
	unlink(dfname);
	retcode = 0;
	logerr("Bad job %s moved to %s.", dfname+LDIRNAM, baddf+LDIRNAM);
}


#if LPD == 0

/*
 * the following code determines who the file's owner is.
 */

struct passwd *getpwuid();

getowner(file)
char	*file;
{
	struct passwd *f;

	if(stat(file, &statbuf) < 0)
		return;
	if((f = getpwuid(statbuf.st_uid)) == NULL)
		fowner = "";
	else
		fowner = f->pw_name;
}

#endif
