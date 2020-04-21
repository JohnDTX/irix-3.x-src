#ifndef lint
static	char sccsid[] = "@(#)ruserpass.c 1.1 85/05/30 SMI"; /* from UCB 4.2 82/10/10 */
#endif

#include <stdio.h>
#include <sys/types.h>
#include <utmp.h>
#include <ctype.h>
#include <sys/stat.h>
#include <errno.h>

char	*malloc(), *index(), *getenv(), *getpass(), *getlogin();
static	FILE *cfile;

_ruserpass(host, aname, apass)
	char *host, **aname, **apass;
{

	if (*aname == 0 || *apass == 0)
		rnetrc(host, aname, apass);
	if (*aname == 0) {
		char *myname = getlogin();
		*aname = malloc(16);
		printf("Name (%s:%s): ", host, myname);
		fflush(stdout);
		if (read(2, *aname, 16) <= 0)
			exit(1);
		if ((*aname)[0] == '\n')
			*aname = myname;
		else
			if (index(*aname, '\n'))
				*index(*aname, '\n') = 0;
	}
	if (*aname && *apass == 0) {
		printf("Password (%s:%s): ", host, *aname);
		fflush(stdout);
		*apass = getpass("");
	}
}

#define	DEFAULT	1
#define	LOGIN	2
#define	PASSWD	3
#define	NOTIFY	4
#define	WRITE	5
#define	YES	6
#define	NO	7
#define	COMMAND	8
#define	FORCE	9
#define	ID	10
#define	MACHINE	11

static char tokval[100];

static struct toktab {
	char *tokstr;
	int tval;
} toktab[]= {
	"default",	DEFAULT,
	"login",	LOGIN,
	"password",	PASSWD,
	"notify",	NOTIFY,
	"write",	WRITE,
	"yes",		YES,
	"y",		YES,
	"no",		NO,
	"n",		NO,
	"command",	COMMAND,
	"force",	FORCE,
	"machine",	MACHINE,
	0,		0
};

static
rnetrc(host, aname, apass)
	char *host, **aname, **apass;
{
	char *hdir, buf[BUFSIZ];
	int t;
	struct stat stb;
	extern int errno;

	hdir = getenv("HOME");
	if (hdir == NULL)
		hdir = ".";
	sprintf(buf, "%s/.netrc", hdir);
	cfile = fopen(buf, "r");
	if (cfile == NULL) {
		if (errno != ENOENT)
			perror(buf);
		return;
	}
next:
	while ((t = token())) switch(t) {

	case DEFAULT:
		(void) token();
		continue;

	case MACHINE:
		if (token() != ID || strcmp(host, tokval))
			continue;
		while ((t = token()) && t != MACHINE) switch(t) {

		case LOGIN:
			if (token())
				if (*aname == 0) { 
					*aname = malloc(strlen(tokval) + 1);
					strcpy(*aname, tokval);
				} else {
					if (strcmp(*aname, tokval))
						goto next;
				}
			break;
		case PASSWD:
			if (fstat(fileno(cfile), &stb) >= 0
			    && (stb.st_mode & 077) != 0) {
	fprintf(stderr, "Error - .netrc file not correct mode.\n");
	fprintf(stderr, "Remove password or correct mode.\n");
				exit(1);
			}
			if (token() && *apass == 0) {
				*apass = malloc(strlen(tokval) + 1);
				strcpy(*apass, tokval);
			}
			break;
		case COMMAND:
		case NOTIFY:
		case WRITE:
		case FORCE:
			(void) token();
			break;
		default:
	fprintf(stderr, "Unknown .netrc option %s\n", tokval);
			break;
		}
		goto done;
	}
done:
	fclose(cfile);
}

static
token()
{
	char *cp;
	int c;
	struct toktab *t;

	if (feof(cfile))
		return (0);
	while ((c = getc(cfile)) != EOF &&
	    (c == '\n' || c == '\t' || c == ' ' || c == ','))
		continue;
	if (c == EOF)
		return (0);
	cp = tokval;
	if (c == '"') {
		while ((c = getc(cfile)) != EOF && c != '"') {
			if (c == '\\')
				c = getc(cfile);
			*cp++ = c;
		}
	} else {
		*cp++ = c;
		while ((c = getc(cfile)) != EOF
		    && c != '\n' && c != '\t' && c != ' ' && c != ',') {
			if (c == '\\')
				c = getc(cfile);
			*cp++ = c;
		}
	}
	*cp = 0;
	if (tokval[0] == 0)
		return (0);
	for (t = toktab; t->tokstr; t++)
		if (!strcmp(t->tokstr, tokval))
			return (t->tval);
	return (ID);
}
