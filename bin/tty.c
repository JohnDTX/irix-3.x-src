char _Origin_[] = "System V";
/*	@(#)tty.c	1.2	*/
/* $Source: /d2/3.7/src/bin/RCS/tty.c,v $ */
static	char	*Sccsid = "@(#)$Revision: 1.1 $";
/* $Date: 89/03/27 14:51:26 $ */

/*
** Type tty name
*/

#include	"stdio.h"
/* #include	"sys/stermio.h" */

char	*ttyname();

extern int	optind;
#ifdef	STWLINE
int		lflg;
#endif
int		sflg;

main(argc, argv)
char **argv;
{
	register char *p;
	register int	i;

#ifdef	STWLINE
	while((i = getopt(argc, argv, "ls")) != EOF)
#else
	while((i = getopt(argc, argv, "s")) != EOF)
#endif
		switch(i) {
#ifdef	STWLINE
		case 'l':
			lflg = 1;
			break;
#endif
		case 's':
			sflg = 1;
			break;
		case '?':
#ifdef	STWLINE
			printf("Usage: tty [-l] [-s]\n");
#else
			printf("Usage: tty [-s]\n");
#endif
			exit(2);
		}
	p = ttyname(0);
	if(!sflg)
		printf("%s\n", (p? p: "not a tty"));
#ifdef	STWLINE
	if(lflg) {
		if((i = ioctl(0, STWLINE, 0)) == -1)
			printf("not on an active synchronous line\n");
		else
			printf("synchronous line %d\n", i);
	}
#endif
	exit(p? 0: 1);
}
