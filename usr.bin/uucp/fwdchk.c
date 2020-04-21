/* @(#)fwdchk.c	1.3 */
#include "uucp.h"

/* 
 * check forwarding file for permission to forward data to
 * a specified machine.
 *	machine		-> destination machine
 *	user		-> user who started the transfer
 * returns:
 *	0		-> ok to forward
 *	-1		-> forwarding disallowed for all users
 *	-2		-> forwarding denied to specified user only
 *	-4		-> bad entry in FWDFILE
 */


fwdchk(machine, user, file)
char	*machine, *user;
char	*file;
{
	FILE *fwdfile;
	char buf[BUFSIZ+1];
	register char *s, *i;
	char *strchr();
	int *strlen();
	char *strpbrk();

	if ((fwdfile = fopen(file, "r")) == NULL)
		return(0);
	while (fgets(buf, sizeof(buf)-1, fwdfile) != NULL) {
	/*
	 * Any users specified
	 */
	if ((s = strchr(buf, '!')) != NULL) {
		/*
	 	 * check for user permission and machine permission to forward
	 	 */
		*s++ = '\0';
		if (strncmp(buf, machine, SYSNSIZE) == SAME) {
			while (*s != '\n') {
				if ((i = strpbrk(s,"!\n\0")) != NULL){
					if (*i == '!')
						*i++ = '\0';
					else
					if (*i == '\n') {
						*i++ = '\0';
						*i = '\n';
					}else 
						return(-4);
				}
				DEBUG(5, "user: %s\n",s);
				if (strcmp(user, s) == SAME) {
					fclose(fwdfile);
					return(0);
				}
				s = i;
			}
			fclose(fwdfile);
			return(-2);
		}else 
			continue;
	}else
	/*
	 * user permission not required - check machine permission to forward
	 */
	if ((i = strpbrk(buf,"\n\0")) != NULL)
		*i='\0';
	else 
		return(-4);
	if (strncmp(buf, machine, SYSNSIZE)==SAME) {
		fclose(fwdfile);
		return(0);
	}else 
		continue;
 	}
	fclose(fwdfile);
	return(-1);
}


