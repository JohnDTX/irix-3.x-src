/* @(#)mailst.c	1.4 */
#include "uucp.h"
#include <stdio.h>



/*
 * fork and execute a mail command sending 
 * string (str) to user (user).
 * If file is non-null, the file is also sent.
 * (this is used for mail returned to sender.)
 *	user	 -> user to send mail to
 *	str	 -> string mailed to user
 *	file 	 -> optional file mailed to user
 */
mailst(user, str, file)
char *user, *str, *file;
{
	register FILE *fp, *fi;
	register int nc;
	extern FILE *popen();
	char cmd[100], buf[BUFSIZ];

	sprintf(cmd, "mail %s", user);
	if ((fp = popen(cmd, "w")) == NULL)
		return;
	fprintf(fp, "%s", str);

	if (*file != '\0' && (fi = fopen(file, "r")) != NULL) {

		/*
		 * read and write should be checked
 		 */
		while ((nc = fread(buf, sizeof (char), BUFSIZ, fi)) > 0)
			fwrite(buf, sizeof (char), nc, fp);
		fclose(fi);
	}

	pclose(fp);
	return;
}

char un[2*NAMESIZE];
setuucp(p)
char	*p;
{
	char **envp;

	envp = Env;
	for(;*envp;envp++) {
				/*
				 * SGI: the CONEHEADS left out the curly
				 * braces for the IF statement as well as not
				 * allowing for the '=' delimiting LOGNAME.
				 * I also cleaned up the code as to using
				 * *envp instead of envp[0] or the very
				 * stupid envp[SAME] (#define SAME 0) as
				 * envp is used as a pointer, not an array!
				 */
		if (strncmp(*envp, "LOGNAME=", 8) == 0) {
			sprintf(un, "LOGNAME=%s", p);
			*envp = un;
		}
	}
}
