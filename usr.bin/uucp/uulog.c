char _Origin_[] = "System V";

/* @(#)uulog.c	1.4 */
#include "uucp.h"
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#define SUCCESS	0
#define FAIL	-1
int Stop = 0;
void exit();

/*
 * append all update files in directory (LOGDIR) to
 * the log file (logf) and remove the update files.
 * options:
 *	n	-> nominal time for delete of lock file
 *	s	-> system name for search
 *	u	-> user name for search
 *	x	-> turn on debug outputs
 * exit: 
 *	0	-> normal
 *	1	-> lock file problems
 */
#define NOMTIME 3600L


char	**Env;
main(argc, argv, envp)
char *argv[];
char	**envp;
{
	FILE *plogf;
	DIR *lsp;
	time_t nomtime;
	int ret;
	extern int onintr(), intr1();
	char *system, *user, *strcpy();
	char filename[NAMESIZE];
	char buf[BUFSIZ], u[32], s[32];
	register char *sp;
	char	*ssp;

	(void) tzset();		/* SGI: (^*~%!$@%^&. */
	Env = envp;
	strcpy(Progname, "uulog");
	Pchar = 'L';
	nomtime = NOMTIME;
	system = user = NULL;


	while (argc>1 && argv[1][0] == '-') {
		switch (argv[1][1]) {
		case 'd':
			printf("-d option removed\n");
			break;
		case 'n':
			nomtime = atoi(&argv[1][2]); break;
		case 's':
			system = &argv[1][2];
			break;
		case 'u':
			user = &argv[1][2];
			break;
		case 'x':
			Debug = atoi(&argv[1][2]);
			if (Debug <= 0)
				Debug = 1;
			break;
		default:
			printf("unknown flag %s\n", argv[1]); break;
		}
		--argc;  argv++;
	}

	DEBUG(4, "%s\n", "START");
	ret = chdir(LOGDIR);
	ASSERT(ret == 0, "CANNOT CHDIR TO - ", LOGDIR, ret);
	if(ret != 0) {
		DEBUG(1, "No LOGDIR - %s\n", LOGDIR);
		exit(0);
	}
	if (ulockf(LOGLOCK, nomtime) != 0)
		exit(0);
	signal(SIGHUP, intr1);
	signal(SIGINT,intr1);
	signal(SIGQUIT, intr1);

	/*
	 * No longer needed
	 * except when you change to this version of uucp
	 * so garbage LOG** files can be compacted for last time
	  */
	if ((plogf = fopen(LOGFILE, "a")) == NULL) {
		rmlock(LOGLOCK);
		printf("can't open %s\n", LOGFILE);
		exit(0);
	}
	lsp = opendir(LOGDIR);
	ASSERT(lsp != NULL, "CAN NOT OPEN", LOGDIR, 0);
	while ((gnamef(lsp, filename)) != 0) {
		DEBUG(4, "file-%s\n", filename);
		if (prefix(LOGPREFIX, filename)) {
			DEBUG(4, "copy file %s\n", filename);
			if (appendf(plogf, filename) == 0) {
				unlink(filename);
			}
		}
	}
	closedir(lsp);
	fclose(plogf);
	chmod(LOGFILE, 0666);
	rmlock(CNULL);
	if (user == NULL && system == NULL)
		exit(0);
	if (Stop)
		exit(0);
	signal(SIGHUP, onintr);
	signal(SIGINT, onintr);
	signal(SIGQUIT, onintr);

	plogf = fopen(LOGFILE, "r");
	ASSERT(plogf != NULL, "CAN NOT OPEN", LOGFILE, 0);
	while (fgets(buf, BUFSIZ, plogf) != NULL) {
		sp = buf;
		ssp = NULL;
		while(*sp){
			if(*sp == '!'){
				ssp = sp;
				*sp = ' ';
				break;
			}
			sp++;
		}
		sscanf(buf, "%s%s", s, u);
		DEBUG(4, "u s %s ", u);
		DEBUG(4, "%s  ", s);
		DEBUG(4, "%s", buf);
		if ((user != NULL) && (prefix(user, u) == FALSE))
			continue;
		if ((system != NULL) && (prefix(system, s) == FALSE))
			continue;
		if(ssp)
			*ssp = '!';
		fputs(buf, stdout);
	}
	exit(0);
}


/*
 * interrupt routine
 * remove lock file
 */
onintr()
{
	rmlock(CNULL);
	exit(0);
}


intr1()
{
	signal(SIGINT, intr1);
	signal(SIGHUP, intr1);
	signal(SIGQUIT, intr1);
	Stop = 1;
	return;
}

cleanup(code)
int code;
{
	exit(code);
}


/*
 * append file (entryf) to fp file
 * return:
 *	0	-> SUCCESS
 *	1	-> FAIL
 */

appendf(fp, entryf)
register FILE *fp;
char *entryf;
{
	FILE *pentryf;
	char ltext[513];

	if ((pentryf = fopen(entryf, "r")) == NULL) {

		/*
		 * file enteryf not readable
		 */
		DEBUG(3, "cannot open %s\n", entryf);
		return(1);
	}
	while (fgets(ltext, 512, pentryf)) fputs(ltext, fp);
	fclose(pentryf);
	return(0);
}
