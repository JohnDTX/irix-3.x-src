/* sendmail(user, msg) -- send msg to user's mailbox */

#include	"lp.h"

SCCSID("@(#)sendmail.c	3.1")

sendmail(user, msg)
char *user;
char *msg;
{
	FILE *pfile, *popen();
	char mailcmd[LOGMAX + 6];

	if(isnumber(user))
		return;

	sprintf(mailcmd, "mail %s", user);
	if((pfile = popen(mailcmd, "w")) != NULL) {
		fprintf(pfile, "%s\n", msg);
		pclose(pfile);
	}
}

isnumber(s)
char *s;
{
	register c;

	while((c = *(s++)) != '\0')
		if(c < '0' || c > '9')
			return(FALSE);
	return(TRUE);
}
