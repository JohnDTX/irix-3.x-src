char _Origin_[] = "Version 7";

static char *sccsid = "@(#)atrun.c	4.2 (Berkeley) 10/21/80";
/*
 * Run programs submitted by at.
 */
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include <sys/stat.h>

# define ATDIR "/usr/spool/at"
# define PDIR	"past"
# define LASTF "/usr/spool/at/lasttimedone"

int	nowtime;
int	nowdate;
int	nowyear;

main(argc, argv)
char **argv;
{
	int tt, day, year, uniq;
	struct dirent *dp;
	DIR *dirf;
	char file[MAXNAMLEN+1];

	chdir(ATDIR);
	makenowtime();
	if ((dirf = opendir(".")) == NULL) {
		fprintf(stderr, "Cannot read at directory\n");
		exit(1);
	}
	while ((dp = readdir(dirf)) != NULL) {
		if (dp->d_ino == 0)
			continue;
		strncpy(file, dp->d_name, MAXNAMLEN);
		file[MAXNAMLEN] = '\0';
		if (sscanf(file, "%2d.%3d.%4d.%2d", &year, &day, &tt, &uniq) != 4)
			continue;
		if (nowyear < year)
			continue;
		if (nowyear==year && nowdate < day)
			continue;
		if (nowyear==year && nowdate==day && nowtime < tt)
			continue;
		run(file);
	}
	closedir(dirf);
	updatetime(nowtime);
	exit(0);
}

makenowtime()
{
	long t;
	struct tm *localtime();
	register struct tm *tp;

	time(&t);
	tp = localtime(&t);
	nowtime = tp->tm_hour*100 + tp->tm_min;
	nowdate = tp->tm_yday;
	nowyear = tp->tm_year;
}

updatetime(t)
{
	FILE *tfile;

	tfile = fopen(LASTF, "w");
	if (tfile == NULL) {
		fprintf(stderr, "can't write lastfile\n");
		exit(1);
	}
#ifdef PWB
	fprintf(tfile, "%.4d\n", t);
#else
	fprintf(tfile, "%04d\n", t);
#endif
}

run(file)
char *file;
{
	struct stat stbuf;
	register pid, i;
	char sbuf[MAXNAMLEN];

	/* printf("running %s\n", file); */
	if (fork()!=0)
		return;
	for (i=0; i<15; i++)
		close(i);
	dup(dup(open("/dev/null", 0)));
	sprintf(sbuf, "%s/%s", PDIR, file);
	link(file, sbuf);
	unlink(file);
	chdir(PDIR);
	if (stat(file, &stbuf) == -1)
		exit(1);
	if (pid = fork()) {
		if (pid == -1)
			exit(1);
		wait((int *)0);
		unlink(file);
		exit(0);
	}
	setgid(stbuf.st_gid);
	setuid(stbuf.st_uid);
	execl("/bin/sh", "sh", file, 0);
	execl("/usr/bin/sh", "sh", file, 0);
	fprintf(stderr, "Can't execl shell\n");
	exit(1);
}
