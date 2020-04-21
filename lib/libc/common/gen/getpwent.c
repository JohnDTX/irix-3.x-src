/*	@(#)getpwent.c	1.3	*/
/*LINTLIBRARY*/
#include <sys/param.h>
#include <stdio.h>
#include <pwd.h>

extern void rewind();
extern long atol();
extern FILE *fopen();
extern int fclose();
extern char *fgets();

static char PASSWD[] = "/etc/passwd";
static char EMPTY[] = "";
static FILE *pwf = NULL;
static char line[BUFSIZ+1];
static struct passwd passwd;

void
setpwent()
{
	if(pwf == NULL)
		pwf = fopen(PASSWD, "r");
	else
		rewind(pwf);
}

void
endpwent()
{
	if(pwf != NULL) {
		(void) fclose(pwf);
		pwf = NULL;
	}
}

static char *
pwskip(p)
register char *p;
{
	while(*p && *p != ':' && *p != '\n')
		++p;
	if(*p == '\n')
		*p = '\0';
	else if(*p)
		*p++ = '\0';
	return(p);
}

struct passwd *
getpwent()
{
	register char *p;
	long	x;

	if(pwf == NULL) {
		if((pwf = fopen(PASSWD, "r")) == NULL)
			return(0);
	}
	p = fgets(line, BUFSIZ, pwf);
	if(p == NULL)
		return(0);
	passwd.pw_name = p;
	p = pwskip(p);
	passwd.pw_passwd = p;
	p = pwskip(p);
	x = atol(p);	
	passwd.pw_uid = (x < 0 || x > MAXUID)? (MAXUID+1): x;
	p = pwskip(p);
	x = atol(p);
	passwd.pw_gid = (x < 0 || x > MAXUID)? (MAXUID+1): x;
	passwd.pw_comment = EMPTY;
	p = pwskip(p);
	passwd.pw_gecos = p;
	p = pwskip(p);
	passwd.pw_dir = p;
	p = pwskip(p);
	passwd.pw_shell = p;
	(void) pwskip(p);

	p = passwd.pw_passwd;
	while(*p && *p != ',')
		p++;
	if(*p)
		*p++ = '\0';
	passwd.pw_age = p;
	return(&passwd);
}
