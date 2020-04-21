char _Origin_[] = "System V";
/* @(#)cico.c	1.6 */
static	char	*Sccsid = "@(#)$Header: /d2/3.7/src/usr.bin/uucp/RCS/cico.c,v 1.1 89/03/27 18:30:14 root Exp $";
/*
 * $Log:	cico.c,v $
 * Revision 1.1  89/03/27  18:30:14  root
 * Initial check-in for 3.7
 * 
 * Revision 1.1  87/09/23  17:04:23  vjs
 * Initial revision
 * 
 * Revision 1.6  85/02/08  12:13:14  bob
 * Fixed bug whereby anyone can enable debugging and find out the phone
 * numbers and passwords for remote system by only allowing debugging if
 * real UID is root.
 * 
 */

#include "uucp.h"
#include <signal.h>
#include <setjmp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include "uust.h"
#include "uusub.h"

#define	read	Read
#define	write	Write

jmp_buf Sjbuf;

	/*
	 * call fail text
	 */
char *Stattext[] = {
	"",
	"BAD SYSTEM",
	"WRONG TIME",
	"SYSTEM LOCKED",
	"NO DEVICE",
	"DIAL FAILED",
	"LOGIN FAILED",
	"BAD SEQUENCE"
	};

char *getlogin();
char *ttyname();
void exit();

	/*
	 * call fail codes
	 */
int Stattype[] = {0, 0, 0, 0,
	SS_NODEVICE, SS_DFAIL, SS_FAIL, SS_BADSEQ
	};


extern int Errorrate;

/*
 * to place a call to a remote machine, login, and
 * copy files between the two machines.
 */
#define ALLSYS		0
#define ONESYS		1
char	**Env;
main(argc, argv, envp)
char *argv[];
char **envp;
{
	extern onintr(), timeout();
	extern intrEXIT();
	extern char *pskip();
	int ret, seq;
	struct stat stbuf;
	int onesys = ALLSYS;
	char wkpre[NAMESIZE], file[NAMESIZE];
	char msg[BUFSIZ], *p, *q, *s;
	char rflags[30];
	char *ttyn, cmd[40], *alreas;
	char *strcat(), *strcpy();
	char	cb[128];
	unsigned alarm(), sleep();
	long	ts;

	(void) tzset();		/* SGI: (^*~%!$@%^&. */
	Env = envp;
#ifdef FOWARD
	Fwdname[0] = '\0';
#endif
	Role = SLAVE;

	closedem();
	nstat.t_beg = times(&nstat.t_tbb);
	nstat.t_scall = nstat.t_ecall = nstat.t_beg;
	time(&nstat.t_qtime);
	tconv = nstat.t_start = nstat.t_qtime;
	strcpy(Progname, "uucico");
	Pchar = 'C';
	signal(SIGILL, intrEXIT);
	signal(SIGTRAP, intrEXIT);
	signal(SIGIOT, intrEXIT);
	signal(SIGEMT, intrEXIT);
	signal(SIGFPE, intrEXIT);
	signal(SIGBUS, intrEXIT);
	signal(SIGSEGV, intrEXIT);
	signal(SIGSYS, intrEXIT);
	signal(SIGINT, onintr);
	signal(SIGHUP, onintr);
	signal(SIGQUIT, onintr);
	signal(SIGTERM, onintr);
	ret = guinfo(geteuid(), User, msg);
	strncpy(Uucp, User, NAMESIZE);
	dlogent("cico.c - euid",User);
	setuucp(User);
	ASSERT(ret == 0, "BAD UID ", "", ret);
	ret = guinfo(getuid(), Loginuser, msg);
	dlogent("cico.c - uid",Loginuser);
	ASSERT(ret == 0, "BAD UID ", "", ret);
	rflags[0] = '\0';
	uucpname(Myname);
	dlogent("cico-Myname",Myname);
	umask(WFMASK);
	strcpy(Rmtname, Myname);
	Ifn = Ofn = -1;
	while (argc>1 && argv[1][0] == '-') {
		switch (argv[1][1]) {
		case 'd':
			Spool = &argv[1][2];
			break;
#ifdef PROTODEBUG
		case 'E':
			Errorrate = atoi(&argv[1][2]);
			if (Errorrate <= 0)
				Errorrate = 100;
			break;
		case 'g':
			Pkdrvon = 1;
			break;
		case 'G':
			Pkdrvon = 1;
			strcat(rflags, " -g ");
			break;
#endif
		case 'r':
			Role = atoi(&argv[1][2]);
			break;
		case 's':
			sprintf(Rmtname, "%.*s", SYSNSIZE, &argv[1][2]);
			if (Rmtname[0] != '\0')
				onesys = ONESYS;
			break;
		case 'x':
			if (!getuid()) {
				Debug = atoi(&argv[1][2]);
				if (Debug <= 0)
					Debug = 1;
				strcat(rflags, argv[1]);
			} else
				fprintf(stderr,"Only root can enable debugging\n");
			break;
		default:
			printf("unknown flag %s\n", argv[1]);
			break;
		}
		--argc;  argv++;
	}

	ret = chdir(Spool);
	ASSERT(ret == 0, "CANNOT CHDIR TO SPOOL - ", Spool, ret);
	if(ret != 0) {
		DEBUG(1, "No spool dirctory - %s\n", Spool);
		rmlock(CNULL);
		exit(0);
	}
	strcpy(Wrkdir, Spool);

	if (Role == SLAVE) {

		/*
		 * initial handshake
		 */
		onesys = ONESYS;
		ret = savline();
		Ifn = 0;
		Ofn = 1;
		fixline(Ifn, 0);
		freopen(RMTDEBUG, "w", stderr);
		DEBUG(4,"cico.c: Myname - %s\n",Myname);
		DEBUG(4,"cico.c: Login - %s\n",getlogin());
		nstat.t_scall = times(&nstat.t_tga);
		sprintf(msg, "here=%s", Myname);
		omsg('S', msg, Ofn);
		signal(SIGALRM, timeout);
		alarm(2 * MAXMSGTIME);	/* SGI: added 2 * */
		if (setjmp(Sjbuf)) {

			/*
			 * timed out
			 */
			ret = restline();
			rmlock(CNULL);
			exit(0);
		}
		for (;;) {
			ret = imsg(msg, Ifn);
			if (ret != 0) {
				alarm(0);
				ret = restline();
				rmlock(CNULL);
				exit(0);
			}
			if (msg[0] == 'S')
				break;
		}
		nstat.t_ecall = times(&nstat.t_tga);
		alarm(0);
		q = &msg[1];
		p = pskip(q);
		sprintf(Rmtname, "%.*s", SYSNSIZE, q);
		dlogent("received Rmtname",Rmtname);
		DEBUG(4, "sys-%s\n", Rmtname);
		if (mlock(Rmtname)) {
			omsg('R', "LCK", Ofn);
			cleanup(0);
		}
		else if (callback(Loginuser)) {
			signal(SIGINT, SIG_IGN);
			signal(SIGHUP, SIG_IGN);
			omsg('R', "CB", Ofn);
			logent("CALLBACK", "REQUIRED");

			/*
			 * set up for call back
			 */
			systat(Rmtname, SS_CALLBACK, "CALL BACK");
			gename(CMDPRE, Rmtname, 'C', file);
			close(creat(file, 0666));
			xuucico(Rmtname);
			cleanup(0);
		}
		seq = 0;
		while (*p == '-') {
			q = pskip(p);
			switch(*(++p)) {
			case 'g':
				Pkdrvon = 1;
				break;
			case 'x':
				p++;
				if (!getuid()) {
					Debug = atoi(p);
					if (Debug <= 0)
						Debug = 1;
				} else
					fprintf(stderr,
					  "Only root can enable debugging\n");
				break;
			case 'Q':
				seq = atoi(++p);
				break;
			default:
				break;
			}
			p = q;
		}
		if (callok(Rmtname) == SS_BADSEQ) {
			logent("BADSEQ", "PREVIOUS");
			omsg('R', "BADSEQ", Ofn);
			cleanup(0);
		}
		if ((ret = gnxseq(Rmtname)) == seq) {
			omsg('R', "OK", Ofn);
			cmtseq();
		} else {
			systat(Rmtname, Stattype[7], Stattext[7]);
			logent("BAD SEQ", "HANDSHAKE FAILED");
			ulkseq();
			omsg('R', "BADSEQ", Ofn);
			cleanup(0);
		}
		ttyn = ttyname(Ifn);
		if (ttyn != NULL) {
			s = (char *)strrchr(ttyn, '/');
			if(*s == '/')
				s++;
			strcpy(&dc[0], s);
			strcpy(cmd, "chmod 600 ");
			strcat(cmd, ttyn);
			shio(cmd, CNULL, CNULL, CNULL);
		}
	}
loop:
	strcpy(User, Uucp);
	if (onesys == ALLSYS) {
		ret = gnsys(Rmtname, Spool, CMDPRE);
		if (ret == FAIL)
			cleanup(100);
		if (ret == 0)
			cleanup(0);
	} else
		if (Role == MASTER && callok(Rmtname) != 0) {
			logent("SYSTEM STATUS", "CAN NOT CALL");
			cleanup(0);
		}

	/*
	 * Make a prefix up consisting of Command prefix
	 * and system name
	 */
	sprintf(wkpre, "%c.%.6s", CMDPRE, Rmtname);

	if (Role == MASTER) {

		/*
		 * master part
		 */
		signal(SIGINT, SIG_IGN);
		signal(SIGHUP, SIG_IGN);
		signal(SIGQUIT, SIG_IGN);
		if (!iswrk(file, "chk", Spool, wkpre) && (onesys == ALLSYS)) {
			logent(Rmtname, "NO WORK");
			goto next;
		}
		if (Ifn != -1 && Role == MASTER) {
			write(Ofn, EOTMSG, (unsigned) strlen(EOTMSG));
			close(Ofn);
			close(Ifn);
			Ifn = Ofn = -1;
			clsacu();
			rmlock(CNULL);
			sleep(3);
		}
		sprintf(msg, "call to %s ", Rmtname);
		if (mlock(Rmtname) != 0) {
			logent(msg, "LOCKED");
			US_SST(US_S_LOCK);
 			goto next;
		}
		nstat.t_scall = times(&nstat.t_tga);
		Ofn = Ifn = conn(Rmtname);
		nstat.t_ecall = times(&nstat.t_tga);
		if (Ofn < 0) {
			delock(Rmtname);
			logent(msg, "FAILED");
			US_SST(-Ofn);
			UB_SST(-Ofn);
			systat(Rmtname, Stattype[-Ofn],
				Stattext[-Ofn]);
			goto next;
		} else {
			logent(msg, "SUCCEEDED");
			US_SST(US_S_COK);
			UB_SST(ub_ok);
		}
	
		if (setjmp(Sjbuf)) {
			delock(Rmtname);
			logent(msg, alreas);
			US_SST(US_S_LOGIN);
			UB_SST(US_S_LOGIN);
			DEBUG(1, "%s - wrong system\n", &msg[6]);
			goto next;
		}
		signal(SIGALRM, timeout);
		alarm(6 * MAXMSGTIME);	/* SGI: was 2 * */
		alreas = "BAD LOGIN/PASSWORD";
		for (;;) {
			ret = imsg(msg, Ifn);
			if (ret != 0) {
				alarm(0);
				msg[0] = 'c';
				delock(Rmtname);
				logent(msg, alreas);
				US_SST(US_S_LOGIN);
				UB_SST(US_S_LOGIN);
				DEBUG(1, "%s - failed\n", alreas);
				goto next;
			}
			if (msg[0] == 'S')
				break;
		}
		alarm(0);
		if(strncmp("here=", &msg[1], 5) == SAME){
			if(strncmp(&msg[6], Rmtname, strlen(Rmtname)) != SAME){
				delock(Rmtname);
				logent(&msg[6], "WRONG SYSTEM");
				US_SST(US_S_LOGIN);
				UB_SST(US_S_LOGIN);
				DEBUG(1, "%s - failed\n", alreas);
				goto next;
			}
		}
		DEBUG(1,"valid sys %s\n",&msg[0]);
		dlogent("HERE",msg);
		alreas = "TIMEOUT";
		seq = gnxseq(Rmtname);
		dlogent("send Myname", Myname);
		sprintf(msg, "%.*s -Q%d %s", SYSNSIZE, Myname, seq, rflags);
		omsg('S', msg, Ofn);
		alarm(2 * MAXMSGTIME);	/* SGI: added 2 * */
		for (;;) {
			ret = imsg(msg, Ifn);
			DEBUG(4, "msg-%s\n", msg);
			if (ret != 0) {
				alarm(0);
				delock(Rmtname);
				ulkseq();
				goto next;
			}
			if (msg[0] == 'R')
				break;
		}
		alarm(0);
		/*
		 * bad sequence
		 */
		if (msg[1] == 'B') {
			delock(Rmtname);
			logent("BAD SEQ", "HANDSHAKE FAILED");
			US_SST(US_S_HAND);
			systat(Rmtname, Stattype[7], Stattext[7]);
			ulkseq();
			goto next;
		}
		if (strcmp(&msg[1], "OK") != SAME)  {
			delock(Rmtname);
			logent(&msg[1], "HANDSHAKE FAILED");
			US_SST(US_S_HAND);
			ulkseq();
			goto next;
		}
		cmtseq();
	}
	DEBUG(1, " Rmtname %s, ", Rmtname);
	DEBUG(1, "Role %s,  ", Role ? "MASTER" : "SLAVE");
	DEBUG(1, "Ifn - %d, ", Ifn);
	DEBUG(1, "Loginuser - %s\n", Loginuser);

	ret = startup(Role);
	if (ret != SUCCESS) {
		delock(Rmtname);
		logent("startup", "FAILED");
		US_SST(US_S_START);
		systat(Rmtname, SS_FAIL, "STARTUP");
		goto next;
	} else {
		logent("startup", "OK");
		US_SST(US_S_GRESS);
		systat(Rmtname, SS_INPROGRESS, "TALKING");
		nstat.t_sftp = times(&nstat.t_tga);
		ret = cntrl(Role, wkpre);
		nstat.t_eftp = times(&nstat.t_tga);
		DEBUG(1, "cntrl - %d\n", ret);
		signal(SIGINT, SIG_IGN);
		signal(SIGHUP, SIG_IGN);
		signal(SIGALRM, timeout);
		if (ret == 0) {
			time(&ts);
			sprintf(cb, "%s %s %s %ld", "conversation complete", dn, dc, ts - tconv);
			logent(cb, "OK");
			US_SST(US_S_OK);
			rmstat(Rmtname);

		} else {
			logent("conversation complete", "FAILED");
			US_SST(US_S_CF);
			systat(Rmtname, SS_FAIL, "CONVERSATION");
		}
		alarm(4 * MAXMSGTIME);	/* SGI: added 4 * */
		omsg('O', "OOOOO", Ofn);
		DEBUG(4, "send OO %d,", ret);
		if (!setjmp(Sjbuf)) {
			for (;;) {
				ret = imsg(msg, Ifn);
				if (ret != 0)
					break;
				if (msg[0] == 'O')
					break;
			}
		}
		alarm(0);
	}
next:
	if (onesys == ALLSYS) {
		goto loop;
	}
	/*
	 * If fowarding restart as master; slave should be only
	 * one to do this
	 */
	if(Fwdname[0] != '\0'){
		dlogent( "CICO: Fwdsys - ",Fwdname);
/*
		euucico(Fwdname);
*/
		strcpy(Rmtname,Fwdname);
		close(Ofn);
		close(Ifn);
		Ifn = Ofn = -1;
		/*
		 * This should not hurt
		 */
		clsacu();
		rmlock(CNULL);
		Role = MASTER;
		*Fwdname = '\0';
		goto loop;
	}
	cleanup(0);
}

/*
 * clean and exit with "code" status
 */
cleanup(code)
register int code;
{
	int ret;
	char *ttyn, cmd[40];
	char *strcat(), *strcpy();

	signal(SIGINT, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
	clsacu();
	logcls();
	rmlock(CNULL);
	closedem();
	if (Role == SLAVE) {
		ret = restline();
		DEBUG(4, "ret restline - %d\n", ret);
		sethup(0);
		ttyn = ttyname(Ifn);
		if (ttyn != NULL) {
			strcpy(cmd, "chmod 666 ");
			strcat(cmd, ttyn);
			shio(cmd, CNULL, CNULL, CNULL);
		}
	}
	if (Ofn != -1) {
		if (Role == MASTER)
			write(Ofn, EOTMSG, (unsigned) strlen(EOTMSG));
		close(Ifn);
		close(Ofn);
	}
	DEBUG(1, "exit code %d\n", code);
	if (code == 0)
		xuuxqt();
/*
 * for debugging
	if (code != 333)
		exit(code);
*/
	exit(code);
}

/*
 * intrrupt - remove locks and exit
 */
onintr(inter)
register int inter;
{
	char str[30];
	signal(inter, SIG_IGN);
	sprintf(str, "SIGNAL %d", inter);
	logent(str, "CAUGHT");
	cleanup(inter);
}

intrEXIT(inter)
{
	char	cb[10];
	extern int errno;

	sprintf(cb, "%d", errno);
	logent("INTREXIT", cb);
	signal(SIGIOT, SIG_DFL);
	signal(SIGILL, SIG_DFL);
	setuid(getuid());
	abort();
}

/*
 * catch SIGALRM routine
 */
timeout()
{
	longjmp(Sjbuf, 1);
}

static char *
pskip(p)
register char *p;
{
	while( *p && *p != ' ' )
		++p;
	if( *p ) *p++ = 0;
	return(p);
}
closedem()
{
	register i;

	for(i=3;i<_NFILE;i++)
		close(i);
}
