char _Origin_[] = "System V";
static	char	*Sccsid = "@(#)$Header: /d2/3.7/src/bin/RCS/su.c,v 1.1 89/03/27 14:51:12 root Exp $";
/*
 * $Log:	su.c,v $
 * Revision 1.1  89/03/27  14:51:12  root
 * Initial check-in for 3.7
 * 
 * Revision 1.6  87/07/22  19:29:03  vjs
 * pass TERM to child
 * 
 * Revision 1.5  87/01/28  14:34:36  paulm
 * Add /usr/etc to superuser's path
 * 
 * Revision 1.4  85/04/10  18:34:57  bob
 * Fixed to set up environment variables USER, TZ, and SHELL, for single-user.
 * 
 * Revision 1.3  85/03/11  23:18:40  bob
 * Fixed to not include /usr/ucb in path since we don't have it.
 * 
 */

/*	@(#)su.c	1.1	*/
/*
 *	su [-] [name [arg ...]] change userid, `-' changes environment
 *	if SULOG is defined, all attemts to su to uid 0 are logged there.
 *	if CONSOLE is defined, all successful attempts are also logged there.
 */
#include <stdio.h>
#include <pwd.h>
#include <time.h>
#include <signal.h>
#include <strings.h>
#define SULOG	"/usr/adm/sulog"
#define PATH	"PATH=:/usr/local/bin:/usr/bin:/bin"
#define SUPATH	"PATH=/usr/local/bin:/usr/bin:/bin:/etc:/usr/etc"
#define ELIM 128
#define	TZFILE		"/etc/TZ"
long time();
void pause();
struct	passwd *pwd, *getpwnam();
struct	tm *localtime();
char	*malloc(), *strcpy();
char	*getpass(), *ttyname(), *strrchr();
char	*shell = "/bin/sh";
#define ENV_SIZE 64
char	su[16] = "su";
char	homedir[ENV_SIZE] = "HOME=";
char	logname[9+ENV_SIZE] = "LOGNAME=";
char	user[6+ENV_SIZE] = "USER=";
char	Shell[7+ENV_SIZE] = "SHELL=";
char	term[6+ENV_SIZE] = "TERM=";
char	tz[4+ENV_SIZE] = "TZ=";
int	tzfd;		/* file descriptor for TZFILE */
char	*path = PATH;
char	*supath = SUPATH;
char	*envinit[ELIM];
char	*tp;
extern	char **environ;
extern	char *getenv();
char	*ttyn;

main(argc, argv)
char	**argv;
{
	char *nptr, *password;
	char	*pshell = shell;
	int badsw = 0;
	int eflag = 0;
	int uid, gid;
	char *dir, *shprog, *name;

	if (argc > 1 && *argv[1] == '-') {
		eflag++;
		argv++;
		argc--;
	}
	nptr = (argc > 1)? argv[1]: "root";
	if((pwd = getpwnam(nptr)) == NULL) {
		fprintf(stderr,"Unknown id: %s\n",nptr);
		exit(1);
	}
	uid = pwd->pw_uid;
	gid = pwd->pw_gid;
	dir = strcpy(malloc(strlen(pwd->pw_dir)+1),pwd->pw_dir);
	shprog = strcpy(malloc(strlen(pwd->pw_shell)+1),pwd->pw_shell);
	name = strcpy(malloc(strlen(pwd->pw_name)+1),pwd->pw_name);
	if((ttyn=ttyname(0))==NULL)
		if((ttyn=ttyname(1))==NULL)
			if((ttyn=ttyname(2))==NULL)
				ttyn="/dev/tty??";
	if(pwd->pw_passwd[0] == '\0' || getuid() == 0 )
		goto ok;
	password = getpass("Password:");
	if(badsw || (strcmp(pwd->pw_passwd, crypt(password, pwd->pw_passwd)) != 0)) {
#ifdef SULOG
		log(SULOG, nptr, 0);
#endif
		fprintf(stderr,"Sorry\n");
		exit(2);
	}
ok:
	endpwent();
#ifdef SULOG
		log(SULOG, nptr, 1);
#endif
	if((setgid(gid) != 0) || (setuid(uid) != 0)) {
		printf("Invalid ID\n");
		exit(2);
	}
	if (eflag) {
		strcat(homedir, dir);
		strcat(logname, name);
		strcat(user, name);
		strcat(Shell, pwd->pw_shell[0] ? &pwd->pw_shell[0] : "/bin/sh");
					/* add in time-zone - SGI */
		tp = getenv("TZ");
		if (tp > 0)
			strncpy(tz+3,tp,ENV_SIZE);
		else if ((tzfd=open(TZFILE,0)) >= 0) {
			read(tzfd,tz+3,9);
			for (tp=tz; tp<tz + sizeof tz; tp++)
				if (*tp == '\n')
					*tp = '\0';
			tz[sizeof tz - 1] = '\0';
			close(tzfd);
		} else {
			strcpy(tz+3,"PST8PDT");
		}

		tp = getenv("TERM");
		environ = envinit;
		*environ++ = homedir;
		*environ++ = uid ? path : supath;
		*environ++ = logname;
		*environ++ = user;	/* 4.2bsd compatibility */
		*environ++ = Shell;	/* 4.2bsd compatibility */
		*environ++ = tz;	/* allow for TZ */
		if (0 != tp)
			*environ++ = strncat(term,tp,ENV_SIZE);
		*environ++ = NULL;
		environ = envinit;
		chdir(dir);
		strcpy(su, "-su");
	}

	if (uid == 0)
	{
#ifdef CONSOLE
		if(strcmp(ttyn, CONSOLE) != 0) {
			signal(SIGALRM, to);
			alarm(30);
			log(CONSOLE, nptr, 1);
			alarm(0);
		}
#endif
		if (!eflag) envalt();
	}
	if (argc > 2) {
		if(shprog[0] != '\0' && !strcmp(shprog, "/bin/sh") &&
		   !strcmp(shprog, "/bin/csh")) {
			fprintf(stderr,"Non-standard shell - denied\n");
			exit(2);
		}
		argv[1] = su;
		execv(shell, &argv[1]);
		fprintf(stderr, "Failed to exec %s\n",shell);
		exit(3);
	} else {
		if(shprog[0] != '\0') {
			pshell = shprog;
			strcpy(su, eflag ? "-" : "");
			strcat(su, strrchr(pshell,'/') + 1);
		}
		execl(pshell, su, 0);
		fprintf(stderr, "Failed to exec %s\n",pshell);
		exit(3);
	}
	fprintf(stderr,"No shell\n");
	exit(3);
}

envalt()
{
int i, pset=0, j=0;
char **eptr=environ;

	for(i=0;i<130;i++)
	{
		if(*eptr == (char *)0) break;
		if(strncmp(*eptr,"PATH=",5)==0)
		{
			envinit[i-j]=supath;
			pset++;
		}
		else if(strncmp(*eptr,"PS1=",4)==0)
			j++;
		else
  			envinit[i-j] = *eptr;

		eptr++;
	}

	if(!pset)
	{
		i = (i < ELIM-2) ? i : ELIM-2;
		envinit[i++]=supath;
	}
	envinit[((i < ELIM-1) ? i : ELIM-1)]=(char *)0;

	environ = envinit;
}
log(where, towho, how)
char *where, *towho;
int how;
{
	FILE *logf;
	long now;
	struct tm *tmp;

	now = time(0);
	tmp = localtime(&now);
	if((logf=fopen(where,"a")) == NULL) return;
	fprintf(logf,"SU %.2d/%.2d %.2d:%.2d %c %s %s-%s\n",
		tmp->tm_mon+1,tmp->tm_mday,tmp->tm_hour,tmp->tm_min,
		how?'+':'-',(strrchr(ttyn,'/')+1),cuserid((char *)0),towho);
	fclose(logf);
}
void to(){}
