char _Origin_[] = "System V";

/*	@(#)wall.c	1.6	*/
/* $Source: /d2/3.7/src/etc/RCS/wall.c,v $ */
static	char	*Sccsid = "@(#)$Revision: 1.1 $";
/* $Date: 89/03/27 15:38:54 $ */

#define	FAILURE	(-1)
#define	TRUE	1
#define	FALSE	0

#include	<signal.h>

char	mesg[1024*4];

#include <sys/types.h>
#include <utmp.h>
#include <grp.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <pwd.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>

unsigned int	usize;
int	entries;
struct utmp *utmp;
char	*line = "???";
char	who[9]	= "???";
char	*infile;
int	group;
struct	group *pgrp;
extern struct group *getgrnam();
extern char *ttyname();
char *grpname;
char	*timstr;
long	tloc, time();
extern int errno;

#define equal(a,b)		(!strcmp( (a), (b) ))

main(argc, argv)
int argc;
char *argv[];
{
	int i, fd;
	register struct utmp *p,*pmax;
	FILE *f;
	struct utmp *getutent();
	struct stat statbuf;
	struct direct direntry;
	int devfd = FAILURE;
	register char *ptr;
	struct passwd *pwd;
	extern struct passwd *getpwuid();
	char *malloc();

	if(stat(UTMP_FILE, &statbuf) < 0) {
		fprintf(stderr, "stat failed on %s\n", UTMP_FILE);
		exit(1);
	}
/* get usize (an unsigned int) for malloc call
 * and check that there is no truncation (for those 16 bit CPUs)
 */
	usize = statbuf.st_size;
	if(usize != statbuf.st_size) {
		fprintf(stderr, "'%s' too big.\n", UTMP_FILE);
		exit(1);
	}
	entries = usize / sizeof(struct utmp);
	if((utmp=(struct utmp *)malloc(usize)) == NULL) {
		fprintf(stderr, "cannot allocate memory for '%s'.\n", UTMP_FILE);
		exit(1);
	}

	if((fd=open(UTMP_FILE, O_RDONLY)) < 0) {
		fprintf(stderr,"cannot open '%s'\n", UTMP_FILE);
		exit(1);
	}
	if(read(fd, utmp, usize) != usize) {
		fprintf(stderr, "cannot read '%s'\n", UTMP_FILE);
		exit(1);
	}
	close(fd);
	readargs(argc, argv);

/*	Get the name of the terminal wall is running from.		*/

	if (isatty(fileno(stderr)) && fstat(fileno(stderr),&statbuf) !=
		FAILURE && (devfd = open("/dev",0)) != FAILURE) {

		while (who[0] == '?') {

/*	Read in directory entries for /dev and look for ones with	*/
/*	an inode number equal to the inode number of our standard	*/
/*	error output.							*/

			for (direntry.d_ino=0; direntry.d_ino !=
				statbuf.st_ino;) {

				direntry.d_ino = 0;
				if (read(devfd,&direntry,sizeof(direntry))
					!= sizeof(direntry)) {
					direntry.d_ino = 0;
					break;
				}
			}

/*	If we've reached the end of the dev directory and can't find	*/
/*	another name for this terminal, finally give up.		*/

			if (direntry.d_ino != statbuf.st_ino) break;
			else line = &direntry.d_name[0];

/*	Search for self so that we can identify ourselves when we send.	*/

			for(i=0;i<entries;i++) {
				p = &utmp[i];
				if(p->ut_type != USER_PROCESS) continue;
				if (strncmp(line,&p->ut_line[0],
					sizeof(p->ut_line)) == 0) {
					strncpy(&who[0],&p->ut_user[0],
					    sizeof(p->ut_user));
					who[sizeof(p->ut_user)] = '\0';
					break;
				}
			}

/*	If we didn't find the utmp entry, reset line to "???" and	*/
/*	continue looking.						*/

			if (p == pmax) {
				strcpy(&who[0],"???");
				line = "???";
			}
		}
	} else {
		if (pwd = getpwuid(getuid()))
			strncpy(&who[0],pwd->pw_name,sizeof(who));
	}
	if (devfd != FAILURE)
		close(devfd);

	f = stdin;
	if(infile) {
		f = fopen(infile, "r");
		if(f == NULL) {
			fprintf(stderr,"%s??\n", infile);
			exit(1);
		}
	}
/*
 *	for(ptr= &mesg[0]; fgets(ptr,&mesg[sizeof(mesg)]-ptr, f) != NULL;
 *	  ptr += strlen(ptr))
 *		;
 */
	ptr = mesg;
	while (ptr < mesg+sizeof mesg-3 && (i=getc(f)) != EOF) {
		if (i == '\n')
			*ptr++ = '\r';
		*ptr++ = i;
	}
	fclose(f);
	time(&tloc);
	timstr = ctime(&tloc);
	for(i=0;i<entries;i++) {
		if((p=(&utmp[i]))->ut_type != USER_PROCESS) continue;
		sendmes(p);
	}
	alarm(60);
	do {
		i = wait((int *)0);
	} while(i != -1 || errno != ECHILD);
	exit(0);
}

sendmes(p)
struct utmp *p;
{
	register i;
	register char *s;
	static char device[] = "/dev/123456789012";
	FILE *f;

	if(group)
		if(!chkgrp(p->ut_user))
			return;
	while((i=fork()) == -1) {
		alarm(60);
		wait((int *)0);
		alarm(0);
	}

	if(i)
		return;

	signal(SIGHUP, SIG_IGN);
	alarm(60);
	s = &device[0];
	sprintf(s,"/dev/%s",&p->ut_line[0]);
#ifdef DEBUG
	f = fopen("wall.debug", "a");
#else
	f = fopen(s, "w");
#endif
	if(f == NULL) {
		printf("Cannot send to %-.8s\n", &p->ut_user[0]);
		perror("open");
		exit(1);
	}
	fprintf(f, "\r\07\07\07Broadcast Message from %s (%s) %19.19s",
		 who, line, timstr);
	if(group)
		fprintf(f, " to group %s", grpname);
	fprintf(f, "...\r\n");
#ifdef DEBUG
	fprintf(f,"DEBUG: To %.8s on %s\n", p->ut_user, s);
#endif
	fprintf(f, "%s\r\n", mesg);
	fclose(f);
	exit(0);
}

readargs(ac, av)
int ac;
char **av;
{
	register int i, j;

	for(i = 1; i < ac; i++) {
		if(equal(av[i], "-g")) {
			if(group) {
				fprintf(stderr, "Only one group allowed\n");
				exit(1);
			}
			i++;
			if((pgrp=getgrnam(grpname= av[i])) == NULL) {
				fprintf(stderr, "Unknown group %s\n", grpname);
				exit(1);
			}
			endgrent();
			group++;
		}
		else
			infile = av[i];
	}
}
#define BLANK		' '
chkgrp(name)
register char *name;
{
	register int i;
	register char *p;

	for(i = 0; pgrp->gr_mem[i] && pgrp->gr_mem[i][0]; i++) {
		for(p=name; *p && *p != BLANK; p++);
		*p = 0;
		if(equal(name, pgrp->gr_mem[i]))
			return(1);
	}

	return(0);
}
