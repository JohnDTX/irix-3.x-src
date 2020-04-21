/* @(#)cntrl.c	1.7 */
#include "uucp.h"
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "uust.h"

struct Proto {
	char P_id;
	int (*P_turnon)();
	int (*P_rdmsg)();
	int (*P_wrmsg)();
	int (*P_rddata)();
	int (*P_wrdata)();
	int (*P_turnoff)();
};

extern int gturnon(), gturnoff();
extern int errno;
extern int grdmsg(), grddata();
extern int gwrmsg(), gwrdata();
extern int xturnon(), xturnoff();
extern int xrdmsg(), xrddata();
extern int xwrmsg(), xwrdata();
extern int imsg();
extern int omsg();
char *strrchr(), *lxp, *strcat(), *strcpy(), *strchr();
int strcmp();

struct Proto Ptbl[]={
	'g', gturnon, grdmsg, gwrmsg, grddata, gwrdata, gturnoff,
	'x', xturnon, xrdmsg, xwrmsg, xrddata, xwrdata, xturnoff,
	'\0'
};

int (*Rdmsg)()=imsg, (*Rddata)();
int (*Wrmsg)()=omsg, (*Wrdata)();
int (*Turnon)(), (*Turnoff)();


#define YES "Y"
#define NO "N"

/*
 * failure messages
 */
#define EM_MAX		7
#define EM_LOCACC	"N1"	/* local access to file denied */
#define EM_RMTACC	"N2"	/* remote access to file/path denied */
#define EM_BADUUCP	"N3"	/* a bad uucp command was generated */
#define EM_NOTMP	"N4"	/* remote error - can't create temp */
#define EM_RMTCP	"N5"	/* can't copy to remote directory - file in public */
#define EM_LOCCP	"N6"	/* can't copy on local system */
#define EM_FOWARD	"N7"	/* a bad remote system name */

char *Em_msg[] = {
	"COPY FAILED (reason not given by remote)",
	"local access to file denied",
	"remote access to path/file denied",
	"system error - bad uucp command generated",
	"remote system can't create temp file",
	"can't copy to file/directory - file left in PUBDIR/user/file",
	"can't copy to file/directory - file left in PUBDIR/user/file",
	"forwarding error"
};


#define XUUCP 'X'	/* execute uucp (string) */
#define SLTPTCL 'P'	/* select protocol  (string)  */
#define USEPTCL 'U'	/* use protocol (character) */
#define RCVFILE 'R'	/* receive file (string) */
#define SNDFILE 'S'	/* send file (string) */
#define RQSTCMPT 'C'	/* request complete (string - yes | no) */
#define HUP     'H'	/* ready to hangup (string - yes | no) */
#define RESET	'X'	/* reset line modes */


#define W_TYPE		wrkvec[0]
#define W_FILE1		wrkvec[1]
#define W_FILE2		wrkvec[2]
#define W_USER		wrkvec[3]
#define W_OPTNS		wrkvec[4]
#define W_DFILE		wrkvec[5]
#define W_MODE		wrkvec[6]
#define W_NUSER		wrkvec[7]
#define W_SFILE		wrkvec[8]
#define W_RFILE		wrkvec[5]
#define W_XFILE		wrkvec[5]
char	*mf;

#define RMESG(m, s) if (rmesg(m, s) != 0) {(*Turnoff)(); return(FAIL);}
#define RAMESG(s) if (rmesg('\0', s) != 0) {(*Turnoff)(); return(FAIL);}
#define WMESG(m, s) if(wmesg(m, s) != 0) {(*Turnoff)(); return(FAIL);}

char Wfile[MAXFULLNAME] = {'\0'};
char Dfile[MAXFULLNAME];

/*
 * execute the conversation between the two machines 
 * after both programs are running.
 * returns:
 *	SUCCESS 	-> ok
 *	FAIL 		-> failed
 */
char	*wrkvec[20];
int	statfopt;
cntrl(role, wkpre)
register int role;
char *wkpre;
{
	FILE *fp;
	struct stat stbuf;
	extern (*Rdmsg)(), (*Wrmsg)();
	int filemode;
	int status = 1;
	int i, narg;
	int mailopt, ntfyopt;
	int ret;
	char msg[BUFSIZ], rqstr[BUFSIZ];
	char filename[MAXFULLNAME], wrktype;
	static int pnum, tmpnum = 0;
	char	*cp;
#ifdef FOWARD
		char file1[MAXFULLNAME], file2[MAXFULLNAME];
		char system[MAXFULLNAME], user[MAXFULLNAME];
		char origsys[MAXFULLNAME];
		char so1[NAMESIZE], so2[NAMESIZE];
		char *s;
#endif

	pnum = getpid();
top:
	strcpy(User, Uucp);
	jobid = 0;
	statfopt = 0;
	DEBUG(4, "*** TOP ***  -  role=%d, ", role);
	setline(RESET);
	if (role == MASTER) {

		/*
		 * get work
		 */
		if ((narg = gtwvec(Wfile, Spool, wkpre, wrkvec)) == 0) {
			WMESG(HUP, "");
			RMESG(HUP, msg);
			goto process;
		}
		wrktype = W_TYPE[0];
		mailopt = strchr(W_OPTNS, 'm') != NULL;
		statfopt = strchr(W_OPTNS, 'o') != NULL;
		ntfyopt = strchr(W_OPTNS, 'n') != NULL;

DEBUG(4,"cntrl %d\n",narg);
DEBUG(4,"cntrl%s\n",W_SFILE?W_SFILE:"<Null W_SFILE>");
DEBUG(4,"cntrl%s\n",W_USER?W_USER:"<Null W_USER>");
		msg[0] = '\0';
		for (i = 1; i < narg; i++) {
			strcat(msg, " ");
			strcat(msg, wrkvec[i]);
		}

		if (wrktype == XUUCP) {
			mf = W_XFILE;
			sprintf(rqstr, "X %s", msg);
			logent(rqstr, "REQUEST");
			goto sendmsg;
		}

		ASSERT(narg > 4, "ARG COUNT<5", "", i);
		sprintf(User, "%s", W_USER);
		sprintf(rqstr, "%s %s %s %s", W_TYPE, W_FILE1,
		  W_FILE2, W_USER);
		logent(rqstr, "REQUEST");
		if(Debug)
		sprintf(rqstr, "%s %s %s %s %s %s %s",
		  W_TYPE,
		  W_FILE1,
		  W_FILE2,
		  W_USER,
		  W_OPTNS ? W_OPTNS : "W_OPTNS null",
		  W_DFILE ? W_DFILE : "W_DFILE null",
		  W_MODE  ? W_MODE  : "W_MODE  null",
		  W_NUSER ? W_NUSER : "W_NUSER null");
		dlogent(rqstr, "REQUEST");
		if (wrktype == SNDFILE ) {
			mf = W_SFILE;
			strcpy(filename, W_FILE1);
#ifdef FOWARD
			if (strcmp(W_DFILE,"D.0") == SAME){
#endif
			expfile(filename);
			if (chkpth(User, "", filename) || someread(filename)) {

				/*
				 * access denied
				 */
				logent("DENIED", "ACCESS");
				USRF(USR_LOCACC);
				unlinkdf(W_DFILE);
				lnotify(User, filename, "access denied");
				goto top;
			}
#ifdef FOWARD
			}
#endif

			strcpy(Dfile, W_DFILE);
			fp = NULL;
			if (strchr(W_OPTNS, 'c') == NULL)
				fp = fopen(Dfile, "r");
#ifdef FOWARD
			if ((strcmp(Dfile,"D.0") != SAME) &&
				(fp == NULL) &&
				((fp = fopen(Dfile,"r")) == NULL)){
				sprintf(rqstr, "CAN'T READ %s %d",Dfile, errno);
				logent(rqstr, "FAILED");
				USRF(USR_LOCACC);
				unlinkdf(Dfile);
				strcpy(msg,"N2");
				notify(mailopt, W_USER, Dfile, Myname, msg);
				goto top;
			}else 
			if (strcmp(Dfile,"D.0")==SAME)
#endif
			if (fp == NULL &&
			   (fp = fopen(filename, "r")) == NULL) {

				/*
				 * can not read data file
				 */
				sprintf(rqstr, "CAN'T RREAD %s %d",filename, errno);
				logent(rqstr, "FAILED");
				USRF(USR_LOCACC);
				unlinkdf(Dfile);
				lnotify(User, filename, "can't access");
				goto top;
			}
			seqn++;
			setline(SNDFILE);
		}

		if (wrktype == RCVFILE) {
			mf = W_RFILE;
			strcpy(filename, W_FILE2);
			expfile(filename);
			if (chkpth(User, "", filename)
			 || chkperm(filename, strchr(W_OPTNS, 'd'))) {

				/*
				 * access denied
				 */
				logent("DENIED", "ACCESS");
				USRF(USR_LOCACC);
				lnotify(User, filename, "access denied");
				goto top;
			}
			sprintf(Dfile, "%s/TM.%.5d.%.3d", Spool, pnum, tmpnum++);
			if ((fp = fopen(Dfile, "w")) == NULL) {

				/*
				 * can not create temp
				 */
				logent("CAN'T CREATE TM", "FAILED");
				USRF(USR_LNOTMP);
				unlinkdf(Dfile);
				goto top;
			}
			seqn++;
			setline(RCVFILE);
		}
sendmsg:
		DEBUG(4, "wrktype - %c\n ", wrktype);
		WMESG(wrktype, msg);
		RMESG(wrktype, msg);
		goto process;
	}

	/*
	 * role is slave
	 */
	RAMESG(msg);
	goto process;

process:

	/*
	 * touch all lock files
	 */
	ultouch();	
	DEBUG(4, " PROCESS: msg - %s\n", msg);
	switch (msg[0]) {

	case RQSTCMPT:
		DEBUG(4, "%s\n", "RQSTCMPT:");
		if (msg[1] == 'N') {
			i = atoi(&msg[2]);
			if (i<0 || i>EM_MAX) i=0;
			USRF( 1 << i );
		}
		if (msg[1] = 'Y')
			USRF(USR_COK);
		logent(msg, "REQUESTED");
		if (role == MASTER) {
#ifdef FOWARD
			/*
			 * notify only if dest. was last site in forward chain
			 * or file successfuly left requesters site
			 */
			if ((strchr(W_FILE2,'!') == NULL) || (strchr(W_USER,
								'!') == NULL))
#endif
			notify(mailopt, W_USER, W_FILE1, Rmtname, &msg[1]);
		}
		goto top;

	case HUP:
		DEBUG(4, "%s\n", "HUP:");
		if (msg[1] == 'Y') {
			WMESG(HUP, YES);
			(*Turnoff)();
			Rdmsg = imsg;
			Wrmsg = omsg;
			return(0);
		}

		if (msg[1] == 'N') {
			ASSERT(role == MASTER,
			  "WRONG ROLE", "", role);
			role = SLAVE;
			goto top;
		}

		/*
		 * get work
		 */
		if (!iswrk(Wfile, "chk", Spool, wkpre)) {
			WMESG(HUP, YES);
			RMESG(HUP, msg);
			goto process;
		}

		WMESG(HUP, NO);
		role = MASTER;
		goto top;

	case XUUCP:
	/*
	 *	notify user that the command left the origin site
	 */
		if (role == MASTER) {
			if ((msg[1] == 'Y') && (strchr(W_USER,'!')==NULL))
				notify(mailopt,W_USER,W_FILE1,Rmtname,&msg[1]);
			goto top;
		}

		/*
		 * slave part
		 */
		i = getargs(msg, wrkvec);
		strcpy(filename, W_FILE1);
		DEBUG(4,"XUUCP-slave: filename is %s\n",filename);
		mf = W_XFILE;
#ifdef FOWARD
		if (filename[0] != '!'){
#endif
		if (strchr(filename, ';') != NULL
		  || strchr(W_FILE2, ';') != NULL
		  || i < 3) {
			WMESG(XUUCP, NO);
			goto top;
		}
		mailopt = strchr(W_OPTNS,'m') != NULL;
		expfile(filename);
		if (chkpth("", Rmtname, filename)) {
			WMESG(XUUCP, NO);
			logent("XUUCP DENIED", filename);
			USRF(USR_XUUCP);
#ifdef FOWARD
			strcpy(msg,"N2");
			notify(mailopt,W_USER,filename,Rmtname,msg);
#endif
			goto top;
		}
		sprintf(rqstr, "%s %s", filename, W_FILE2);
		seqn++;
#ifdef FOWARD
		if (stat(filename,&stbuf) != 0) {	
			strcpy(msg,"N2");
			notify(mailopt,W_USER,filename,Rmtname, msg);
			WMESG(XUUCP, NO);
			logent("XUUCP DENIED - NO STATUS",filename);
			goto top;
		}
		/*
		 * for remote receive execute uucp with -n option if
		 * requesting user desires notification
		 */

		/*
		 *  the following code does not work correctly
		 *  when receiving a remote file and sending to
		 *  another remote site (it will notify the receiver)
		 
		if (mailopt) {
			strcpy(User,(lxp=strrchr(W_USER,'!'))?lxp+1:W_USER);
			sprintf(rqstr,"-n%s %s %s",User,filename,W_FILE2);
		}

		 *
		 */
#endif
		logent(rqstr,"xuucp string");
		xuucp(rqstr);
		WMESG(XUUCP, YES);
#ifdef FOWARD
		} else {
		/*
		 * Routing Modification
		 */		
		if ((cp = strchr(&filename[1], '!')) != NULL) {
		char	string[300];
		char *fil2, *i;
		register char *sys;
		FILE	*cfp;
		extern char Cfile[];
			/*
			 * Destination after bang
			 */
			strcpy(file1, cp+1);
			/*
			 *  Copy destination including bang if more forwarding
			 */
			if (strchr(file1, '!') != NULL)
				strcpy(file1, cp);
			strcpy(user, Rmtname);
			strcat(user, "!");
			strcat(user, W_USER);
			sys = W_FILE1;
			s = strrchr(W_FILE1,'!');
			s++;
	/*
	 *	Ensure forwarding to/from public spool directory
	 */
			if (!prefix("~/",s) && !prefix(PUBDIR,s)) {
				logent(W_FILE1, "NO FWDING TO NONPUB DIR");
				WMESG(XUUCP, EM_FOWARD);
				goto top;
			}
	/*
	 *	Detect illegal attempts to forward though (~/../) public dir.
	 */
			for (lxp = ++s; *lxp != '\0'; lxp++)
				if (*lxp == '/' && prefix("../",(++lxp)))  { 
				logent(W_FILE1, "ILLEGAL FWD ATTEMPT");
				WMESG(XUUCP, EM_FOWARD);
				goto top;
			}
				
			/*
			 * Strip off bangs for beginning
			 */
			while(*sys == '!')
				sys++;
			strcpy(system, sys);
			i = strchr(system,'!');
			*i = '\0';
			if (versys(system) != 0){
				logent(system, "INVALID FWDR SYSTEM");
				goto ickr;
			}
			/*
			 *  find origin site of user
			 */
			strcpy(origsys,user);
			*(sys = strrchr(origsys,'!')) = '\0';
			if ((sys = strrchr(origsys,'!')) == NULL)
				sys = origsys;	
			else sys++;
			/*
			 *  find user name
			 */

			if((cp = strrchr(user, '!')) == NULL)
				cp = user;
			else
				cp = cp + 1;
			/*
			 *  Willing to allow original user to foward
			 *  through this site?
			 */

			if(fwdchk(sys, cp, ORIGFILE) != 0){
				logent(sys, "NO FWDING FROM");
				goto ickr;
			}
			/*
			 * Willing to foward to next system
			 */
			if(fwdchk(system, cp, FWDFILE) != 0){
				logent(system, "NO FWDING XUU");
ickr:
				WMESG(XUUCP, EM_FOWARD);
				goto top;
			}
			WMESG(XUUCP,YES);
			if (strncmp(system, Rmtname, SYSNSIZE) != SAME)
				strncpy(Fwdname, system, NAMESIZE);
			else 
				*Fwdname = '\0';
	/*
	 *  prevent -o option while forwarding
	 */
			if ((lxp=strchr(W_OPTNS,'o'))!=NULL) {
				*lxp++ = '\0';
				strcat(W_OPTNS,lxp);
				sscanf(W_OPTNS,"%[^m]m%s",so1,so2);
				strcat(so1,so2);
			}else
				strcpy(so1, W_OPTNS);
			sprintf(string, "testX  %s %s %s %s %s %s %s %s %s\n",
				file1, W_FILE2, user, so1, "D.0",
				W_MODE, W_NUSER, mf, system);
			dlogent(string, "DEBUGX-");
			if((cfp = (FILE *)gtcfile(system,'n')) == NULL){
				logent(system, "BAD XFILE");
				WMESG(XUUCP, EM_FOWARD);
				goto top;
			}
			DEBUG(4,"XUUCP-SLAVE file1: %s\n",file1);
			DEBUG(4,"XUUCP-SLAVE W_FILE2: %s\n",W_FILE2);
			fprintf(cfp, "X  %s %s %s %s %s\n",
				file1, W_FILE2, user, so1, mf);
			DEBUG(4,"XUUCP: system.0 - %s\n",system);
			if(strncmp(system, Rmtname, SYSNSIZE) == SAME){
				entflst(Cfile);
				DEBUG(4,"cntrl.c: system - %s\n",system);
			}
			clscfile(); /* close Cfile; make entry in R_stat */
		}
		}
#endif
		goto top;

	case SNDFILE:

		/*
		 * MASTER section of SNDFILE
		 */
		DEBUG(4, "%s\n", "SNDFILE:");
		if (msg[1] == 'N') {
			i = atoi(&msg[2]);
			if (i < 0 || i > EM_MAX)
				i = 0;
			logent(Em_msg[i], "REQUEST");
			USRF( 1 << i );
			notify(mailopt, W_USER, W_FILE1, Rmtname, &msg[1]);
			ASSERT(role == MASTER,
			  "WRONG ROLE", "", role);
			fclose(fp);
			if (msg[1] != '4')
				unlinkdf(W_DFILE);
			goto top;
		}

		if (msg[1] == 'Y') {

			/*
			 * send file
			 */
			ASSERT(role == MASTER,
			  "WRONG ROLE", "", role);
			ret = (*Wrdata)(fp, Ofn);
			fclose(fp);
			if (ret != 0) {
				(*Turnoff)();
				USRF(USR_CFAIL);
				return(FAIL);
			}
			dlogent(W_DFILE, "file removed in cntrl.c");
			unlinkdf(W_DFILE);
			RMESG(RQSTCMPT, msg);
			goto process;
		}

		/* 
		 * SLAVE section of SNDFILE
		 */
		ASSERT(role == SLAVE,
			  "WRONG ROLE", "", role);

		/*
		 * request to receive file
		 * check permissions
		 */
		i = getargs(msg, wrkvec);
		ASSERT(i > 4, "ARG COUNT<5", "", i);
		mf = W_SFILE;
		sprintf(rqstr, "%s %s %s %s", W_TYPE, W_FILE1,
		  W_FILE2, W_USER);
		logent(rqstr, "REQUESTED");
		DEBUG(4, "msg - %s\n", msg);
		strcpy(filename, W_FILE2);
		expfile(filename);
                DEBUG(4,"SLAVE - filename: %s\n",filename);
#ifdef FOWARD
		if(filename[0] != '!'){
#endif
			Fwdname[0] = '\0';
			if (chkpth("", Rmtname, filename)
			 || chkperm(filename, strchr(W_OPTNS, 'd'))) {
				WMESG(SNDFILE, EM_RMTACC);
				logent("DENIED", "PERMISSION");
				goto top;
			}
			if (isdir(filename)) {
				strcat(filename, "/");
				strcat(filename,
				(lxp = strrchr(W_FILE1, '/'))?lxp+1:W_FILE1);
			}
			sprintf(User, "%s", W_USER);
#ifdef FOWARD
		}else{
			char	*i;
			register char *sys;

			sprintf(User, "%s", W_USER);
			if((cp = strchr(&filename[1], '!')) != NULL){
			/*
			 * Destination after bang
			 */
			strcpy(file2, cp+1);
			/*
			 * Destination including bang
			 */
			if (strchr(file2, '!') != NULL)
				strcpy(file2, cp);
			strcpy(user, Rmtname);
			strcat(user, "!");
			strcat(user, W_USER);
			sys = W_FILE2;
			s = strrchr(W_FILE2,'!');
			s++;
	/*
	 *	Ensure forwarding to/from public directory.
	 *	Allow sending to spool directory so uux will work.
	 */
			if (*s != '~') {
				if (!prefix(PUBDIR,s) && !prefix(SPOOL,s))
				if (strchr(s,'/') != NULL) {
				/* not spool directory */
				logent(W_FILE2, "NOT SPOOL DIR");
				WMESG(SNDFILE, EM_FOWARD);
				goto top;
				} 
			}
			else
				if (*(++s) != '/')  {
				/* not public directory */
				logent(W_FILE2, "NO FWDING TO NONPUB DIR");
				WMESG(SNDFILE, EM_FOWARD);
				goto top;
				}
				
	/*
	 *	Detect illegal attempts to forward though (~/../) public dir.
	 */
			for (lxp = s; *lxp != '\0'; lxp++)
				if (*lxp == '/' && prefix("../",(++lxp))) {  
				logent(W_FILE2, "ILLEGAL FWD ATTEMPT");
				WMESG(SNDFILE, EM_FOWARD);
				goto top;
				}
			/*
			 * Strip off bangs from beginning
			 */
			while(*sys == '!')
				sys++;
			strcpy(system, sys);
			i = strchr(system,'!');
			*i = '\0';
			if (versys(system) != 0){
				logent(system, "INVALID FWDS SYSTEM");
				goto icks;
			}
			/*
			 *  find origin site of user
			 */
			strcpy(origsys,user);
			*(sys = strrchr(origsys,'!')) = '\0';
			if ((sys = strrchr(origsys,'!')) == NULL)
				sys = origsys;	
			else sys++;
			/*
			 *  find user name
			 */

			if((cp = strrchr(user, '!')) == NULL)
				cp = user;
			else
				cp = cp + 1;
			/*
			 * Willing to allow user on origin system
			 * to foward through this site?
			 */

			if(fwdchk(sys, cp, ORIGFILE) != 0){
				logent(sys, "NO FWDING FROM SND");
				goto icks;
			}
			/*
			 * Willing to foward to next system
			 */
			if(fwdchk(system, cp, FWDFILE) != 0){
				logent(system, "NO FWDING SND");
icks:
				WMESG(SNDFILE, EM_FOWARD);
				goto top;
			}
		}
		}
#endif

		DEBUG(4, "chkpth ok Rmtname - %s\n", Rmtname);
		sprintf(Dfile, "%s/TM.%.5d.%.3d", Spool, pnum, tmpnum++);
		dlogent(Dfile,"temp file created");
		if((fp = fopen(Dfile, "w")) == NULL) {
			WMESG(SNDFILE, EM_NOTMP);
			logent("CAN'T OPEN", "DENIED");
			unlinkdf(Dfile);
			goto top;
		}

		seqn++;
		WMESG(SNDFILE, YES);
		ret = (*Rddata)(Ifn, fp);
		fclose(fp);
		if (ret != 0) {
			(*Turnoff)();
			return(FAIL);
		}
		/* copy to user directory */
#ifdef FOWARD
		if(filename[0] != '!'){
#endif
			ntfyopt = strchr(W_OPTNS, 'n') != NULL;
			status = xmv(Dfile, filename);
			WMESG(RQSTCMPT, status ? EM_RMTCP : YES);
			logent(status ? "FAILED" : "SUCCEEDED", "COPY");
			if (status == 0) {
				sscanf(W_MODE, "%o", &filemode);
				if (filemode <= 0)
					filemode = 0666;
				chmod(filename, filemode | 0666);
				arrived(ntfyopt, filename, W_NUSER, Rmtname, User);
			}else {
				status = putinpub(filename, Dfile,
				  (lxp = strrchr(W_USER,'!'))?lxp+1:W_USER);
				DEBUG(4, "->PUBDIR %d\n", status);
				if (status == 0)
					arrived(ntfyopt, filename, W_NUSER,
					  Rmtname, User);
			}
#ifdef FOWARD
		}else{
		dlogent("DEB1", W_FILE2);
		/*
		 * Routing modification
		 */
		if ((cp = strchr(&filename[1], '!')) != NULL) {
			char string[300];
			char *fil2;
			FILE *cfp;
			extern char Cfile[];

			if (strncmp(system,Rmtname, SYSNSIZE) != SAME)
				strncpy(Fwdname, system, NAMESIZE);
			else
				*Fwdname = '\0';
	/*
	 *  prevent -o option while forwarding
	 */
			if ((lxp=strchr(W_OPTNS,'o'))!=NULL) {
				*lxp++ = '\0';
				strcat(W_OPTNS,lxp);
				sscanf(W_OPTNS,"%[^m]m%s",so1,so2);
				strcat(so1,so2);
			}else
				strcpy(so1, W_OPTNS);
			sprintf(string, "test1  %s %s %s %s %s %s %s %s %s\n",
				W_FILE1, file2, user, so1, Dfile,
				W_MODE, W_NUSER, mf, system);
			dlogent(string, "DEBUG-");
			if((cfp = (FILE *)gtcfile(system,'n')) == NULL){
				logent(system, "BAD SFILE");
				WMESG(RQSTCMPT, EM_FOWARD);
				unlink(Dfile);
				goto top;
			}
			WMESG(RQSTCMPT, YES);
			DEBUG(4,"SLAVE Dfile: %s\n",Dfile);
			DEBUG(4,"SLAVE file2: %s\n",file2);
			fprintf(cfp, "S  %s %s %s %s %s %s %s %s\n",
				W_FILE1, file2, user, so1, Dfile,
				W_MODE, W_NUSER, mf);
			DEBUG(4,"cntrl.c: system.0 - %s\n",system);
			/*
			 * Arrange for system to continue if 
			 * sending back
			 */
			if(strncmp(system, Rmtname, SYSNSIZE) == SAME){
				entflst(Cfile);
				DEBUG(4,"cntrl.c: system - %s\n",system);
			}
			clscfile(); /* close Cfile; make entry in R_stat */
		}

		}
#endif
		goto top;

	case RCVFILE:

		/*
		 * MASTER section of RCVFULE 
		 */
		DEBUG(4, "%s\n", "RCVFILE:");
		if (msg[1] == 'N') {
			i = atoi(&msg[2]);
			if (i < 0 || i > EM_MAX)
				i = 0;
			logent(Em_msg[i], "REQUEST");
			USRF( 1 << i );
			notify(mailopt, W_USER, W_FILE1, Rmtname, &msg[1]);
			ASSERT(role == MASTER,
			  "WRONG ROLE", "", role);
			fclose(fp);
			unlinkdf(Dfile);
			goto top;
		}

		if (msg[1] == 'Y') {

			/*
			 * receive file
			 */
			ASSERT(role == MASTER,
			  "WRONG ROLE", "", role);
			ret = (*Rddata)(Ifn, fp);
			fclose(fp);
			if (ret != 0) {
				(*Turnoff)();
				USRF(USR_CFAIL);
				return(FAIL);
			}

			/*
			 * copy to user directory
			 */
			if (isdir(filename)) {
				strcat(filename, "/");
				strcat(filename, (lxp=strrchr(W_FILE1, '/'))?lxp+1:W_FILE1);
			}
			status = xmv(Dfile, filename);
			WMESG(RQSTCMPT, status ? EM_RMTCP : YES);
			logent(status ? "FAILED" : "SUCCEEDED", "COPY");
			notify(mailopt, W_USER, filename, Rmtname,
			  status ? EM_LOCCP : YES);
			if (status == 0) {
				sscanf(&msg[2], "%o", &filemode);
				if (filemode <= 0)
					filemode = 0666;
				chmod(filename, filemode | 0666);
				USRF(USR_COK);
			} else {
				putinpub(filename, Dfile, W_USER);
				USRF(USR_LOCCP);
			}
			goto top;
		}

		/*
		 * SLAVE section of RCVFILE
		 */
		ASSERT(role == SLAVE,
			  "WRONG ROLE", "", role);


		/*
		 * request to send file
		 */
		strcpy(rqstr, msg);
		logent(rqstr, "REQUESTED");


		/*
		 * check permissions
		 */
		i = getargs(msg, wrkvec);
		ASSERT(i > 3, "ARG COUNT<4", "", i);
		mf = W_RFILE;
		DEBUG(4, "msg - %s\n", msg);
		DEBUG(4, "W_FILE1 - %s\n", W_FILE1);
		strcpy(filename, W_FILE1);
		expfile(filename);
		if (isdir(filename)) {
			strcat(filename, "/");
			strcat(filename, (lxp=strrchr(W_FILE2, '/'))?lxp+1:W_FILE2);
		}
		sprintf(User, "%s", W_USER);
		if (chkpth("", Rmtname, filename) || anyread(filename)) {
			WMESG(RCVFILE, EM_RMTACC);
			logent("DENIED", "PERMISSION");
			goto top;
		}
		DEBUG(4, "chkpth ok Rmtname - %s\n", Rmtname);

		if ((fp = fopen(filename, "r")) == NULL) {
			WMESG(RCVFILE, EM_RMTACC);
			logent("CAN'T OPEN", "DENIED");
			goto top;
		}

		/*
		 * ok to send file
		 */
		ret = stat(filename, &stbuf);
		ASSERT(ret != -1, "STAT FAILED", filename, 0);
		sprintf(msg, "%s %o", YES, stbuf.st_mode & 0777);
		WMESG(RCVFILE, msg);
		seqn++;
		ret = (*Wrdata)(fp, Ofn);
		fclose(fp);
		if (ret != 0) {
			(*Turnoff)();
			return(FAIL);
		}
		RMESG(RQSTCMPT, msg);
		goto process;
	}
	(*Turnoff)();
	return(FAIL);
}


/*
 * read message
 * returns:
 *	0	-> success
 *	FAIL	-> failure
 */
rmesg(c, msg)
char *msg, c;
{
	char str[50];

	DEBUG(4, "rmesg - '%c' ", c);
	if ((*Rdmsg)(msg, Ifn) != 0) {
		DEBUG(4, "got %s\n", "FAIL");
		sprintf(str, "expected '%c' got FAIL", c);
		logent(str, "BAD READ");
		return(FAIL);
	}
	if (c != '\0' && msg[0] != c) {
		DEBUG(4, "got %s\n", msg);
		sprintf(str, "expected '%c' got %.25s", c, msg);
		logent(str, "BAD READ");
		return(FAIL);
	}
	DEBUG(4, "got %.25s\n", msg);
	return(0);
}


/*
 * write a message
 * returns:
 *	0	-> ok
 *	FAIL	-> ng
 */
wmesg(m, s)
char *s, m;
{
	DEBUG(4, "wmesg '%c'", m);
	DEBUG(4, "%.25s\n", s);
	return((*Wrmsg)(m, s, Ofn));
}


/*
 * mail results of command
 * return: 
 *	none
 */
notify(mailopt, user, file, sys, msgcode)
char *user, *file, *sys;
register char *msgcode;
{
	register int i;
	char str[200];
	register char *msg;

DEBUG(4,"notif %d\n", mailopt);
DEBUG(4,"notif %d\n", statfopt);
	if (statfopt == 0 && mailopt == 0 && *msgcode == 'Y')
		return;
	if (*msgcode == 'Y')
		msg = "copy succeeded";
	else {
		i = atoi(msgcode + 1);
		if (i < 1 || i > EM_MAX)
			i = 0;
		msg = Em_msg[i];
	}
	if(statfopt){
		stmesg(user, file, msg);
		return;
	}
	sprintf(str, "file %s, system %s\n%s\n",
		file, sys, msg);
	mailst(user, str, "");
	return;
}

/*
 * local notify
 * return:
 *	none
 */
lnotify(user, file, mesg)
char *user, *file, *mesg;
{
	char mbuf[200];

	if(statfopt){
		stmesg(user, file, mesg);
		return;
	}
	sprintf(mbuf, "file %s on %s\n%s\n", file, Myname, mesg);
	mailst(user, mbuf, "");
	return;
}
stmesg(u, f, m)
char	*u, *f, *m;
{
	struct tm *tp;
	FILE	*Cf;
	extern struct tm *localtime();
	time_t	time();
	time_t	clock;
	long	td, th, tm, ts;
	int	mask, n;
	char	*s;

DEBUG(4,"STMES %s\n",mf);
	mask = umask(0);
	if((Cf = fopen(mf, "a+")) == NULL){
		umask(mask);
		return;
	}
	umask(mask);
DEBUG(4,"STM %d\n",Cf);
	if((n = strlen(Wfile)) < 4)
		s = &Wfile[0];
	else
		s = &Wfile[n-4];
	fprintf(Cf, "uucp job %.4s ", s);
	time(&clock);
	tp = localtime(&clock);
	fprintf(Cf, "(%d/%d-%d:%2.2d:%2.2d) ", tp->tm_mon + 1,
		tp->tm_mday, tp->tm_hour, tp->tm_min, tp->tm_sec);
	td = clock - nstat.t_qtime;
	ts = td%60;
	td /= 60;
	tm = td%60;
	td /= 60;
	th = td;
	fprintf(Cf, "(%ld:%ld:%ld) ", th, tm, ts);
	fprintf(Cf, "%s %s\n", f, m);
	fclose(Cf);

}


/*
 * converse with the remote machine, agree upon a 
 * protocol (if possible) and start the protocol.
 * return:
 *	SUCCESS	-> successful protocol selection
 *	FAIL	-> can't find common or open failed
 */
startup(role)
register int role;
{
	extern (*Rdmsg)(), (*Wrmsg)();
	extern imsg(), omsg();
	extern char *blptcl(), fptcl();
	char msg[BUFSIZ], str[BUFSIZ];

	Rdmsg = imsg;
	Wrmsg = omsg;
	if (role == MASTER) {
		RMESG(SLTPTCL, msg);
		if ((str[0] = fptcl(&msg[1])) == NULL) {

			/*
			 * no protocol match
			 */
			WMESG(USEPTCL, NO);
			return(FAIL);
		}
		str[1] = '\0';
		WMESG(USEPTCL, str);
		if (stptcl(str) != 0)
			return(FAIL);
		DEBUG(4, "protocol %s\n", str);
		return(SUCCESS);
	} else {
		WMESG(SLTPTCL, blptcl(str));
		RMESG(USEPTCL, msg);
		if (msg[1] == 'N') {
			return(FAIL);
		}

		if (stptcl(&msg[1]) != 0)
			return(FAIL);
		DEBUG(4, "Protocol %s\n", msg);
		return(SUCCESS);
	}
}


/*
 * choose a protocol from the input string (str)
 * and return the found letter.
 * return:
 *	'\0'		-> no acceptable protocol
 *	any character	-> the chosen protocol
 */
char
fptcl(str)
register char *str;
{
	register struct Proto *p;

	if(Pprot[0] && strchr(str, Pprot[0]) != NULL)
		return(Pprot[0]);
	for (p = Ptbl; p->P_id != '\0'; p++) {
		if (strchr(str, p->P_id) != NULL) {
			return(p->P_id);
		}
	}

	return('\0');
}

/*
 * build a string of the letters of the available
 * protocols and return the string (str).
 * return:
 *	a pointer to string (str)
 */
char *
blptcl(str)
register char *str;
{
	register struct Proto *p;
	char *s;

	for (p = Ptbl, s = str; (*s++ = p->P_id) != '\0'; p++);
	return(str);
}

/*
 * set up the six routines (Rdmg. Wrmsg, Rddata
 * Wrdata, Turnon, Turnoff) for the desired protocol.
 * returns:
 *	SUCCESS 	-> ok
 *	FAIL		-> no find or failed to open
 */
stptcl(c)
register char *c;
{
	register struct Proto *p;

	for (p = Ptbl; p->P_id != '\0'; p++) {
		if (*c == p->P_id) {

			/*
			 * found protocol 
			 * set routine
			 */
			Rdmsg = p->P_rdmsg;
			Wrmsg = p->P_wrmsg;
			Rddata = p->P_rddata;
			Wrdata = p->P_wrdata;
			Turnon = p->P_turnon;
			Turnoff = p->P_turnoff;
			if ((*Turnon)() != 0)
				return(FAIL);
			DEBUG(4, "Proto started %c\n", *c);
			return(SUCCESS);
		}
	}
	DEBUG(4, "Proto start-fail %c\n", *c);
	return(FAIL);
}

/*
 * put file in public place
 * if successful, filename is modified
 * returns:
 *	0	-> success
 *	FAIL	-> failure
 */
putinpub(file, tmp, user)
char *file, *user, *tmp;
{
	int status;
	char fullname[MAXFULLNAME];

	sprintf(fullname, "%s/%s/", PUBDIR, user);
	if (mkdirs(fullname) != 0) {

		/*
		 * can not make directories
		 */
		return(FAIL);
	}
	strcat(fullname, (lxp=strrchr(file, '/'))?lxp+1:file);
	status = xmv(tmp, fullname);
	if (status == 0)
		strcpy(file, fullname);
	return(status);
}

/*
 * unlink D. file
 * returns:
 *	none
 */
unlinkdf(file)
register char *file;
{
	if (strlen(file) > 6)
		unlink(file);
	return;
}

/*
 * notify receiver of arrived file
 * returns:
 *	none
 */
arrived(opt, file, nuser, rmtsys, rmtuser)
char *file, *nuser, *rmtsys, *rmtuser;
{
	char mbuf[200];

	if (!opt)
		return;
	sprintf(mbuf, "%s from %s!%s arrived\n", file, rmtsys, rmtuser);
	mailst(nuser, mbuf, "");
	return;
}
