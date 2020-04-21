/* @(#)uucpdefs.c	1.6 */
#include "uucp.h"

int Ifn, Ofn;
int Debug = 0;
int Pkdrvon = 0;
char Progname[NAMESIZE];
char	Pchar;
char Pprot[NAMESIZE];
char Rmtname[NAMESIZE];
char Fwdname[NAMESIZE];
char User[MAXFULLNAME];
char	Uucp[NAMESIZE];
char Loginuser[NAMESIZE];
char Myname[8];
char Wrkdir[WKDSIZE];

char *Spool = SPOOL;
long Retrytime;
short Usrf = 0;			/* Uustat global flag */
struct nstat nstat;
char	dn[15];			/* auto dialer name			*/
char	dc[15];			/* line name				*/
int	seqn;			/* sequence #				*/
long	tconv;			/* conversation time			*/
int	jobid;
int	Role;
char	subjob[2]   =	{"@"};
