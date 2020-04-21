/* @(#)uucpname.c	1.5 */
static	char	*Sccsid = "@(#)$Header: /d2/3.7/src/usr.bin/uucp/RCS/uucpname.c,v 1.1 89/03/27 18:30:54 root Exp $";
/*
 * $Log:	uucpname.c,v $
 * Revision 1.1  89/03/27  18:30:54  root
 * Initial check-in for 3.7
 * 
 * Revision 1.2  85/06/17  11:39:09  bob
 * Fixed to abort if node name is null since otherwise 4.2bsd uucp will
 * send every node's mail to the node!
 * 
 */
#include "uucp.h"

#ifdef UNAME
#include <sys/utsname.h>
#endif

/*
 * get the uucp name
 * return:
 *	none
 */
uucpname(name)
register char *name;
{
	register char *s, *d;
/*  the following 3 variables are for debugging */
	int ret;
	char msg[BUFSIZ];
	char loginusr[NAMESIZE];

#ifdef UNAME
	struct utsname utsn;

	uname(&utsn);
	s = utsn.nodename;
#endif

#ifndef UNAME
	s = MYNAME;
#endif

	d = name;
	while ((*d = *s++) && d < name + SYSNSIZE)
		d++;
	*(name + SYSNSIZE) = '\0';
	if (!*name) {
		fprintf(stderr, "Node name is empty\n");
		DEBUG(1, "Node name is empty\n", 0);
		logent("Node name is empty","uucico aborted");
		exit(3);
	}
/* the following statements are for debugging  */
#ifdef TESTB
	ret = guinfo(getuid(), loginusr, msg);
	switch (*loginusr){
		case 'd':
			name[4] = 'D';
			break;
		case 'e':
			name[4] = 'E';
			break;
		case 'f':
			name[4] = 'F';
			break;
		}
#endif
}
