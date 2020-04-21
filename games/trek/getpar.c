static char ID[] = "@(#)getpar.c	1.1";

# include	"trek.h"
#include <sys/termio.h>

extern	char	eof;
struct termio otermio, ntermio;

/**
 **	get integer parameter
 **/


getintpar(s, n)
char	*s;
int	*n;
{
	register int		i;

	for ever {
		while ((eof|mkfault)==0 && lineended() && s) {
			printf("%s ", s);
			newline();
		}
		if((i=readint(n)) < 0)
			return(0);
		if(i>0)
			return(1);
		printf("? ");
		flushin();
	}
}

/**
 **	get floating parameter
 **/

getfltpar(s, f)
char	*s;
float	*f;
{
	register int		i;

	for ever {
		while ((eof|mkfault)==0 && lineended() && s) {
			printf("%s: ", s);
			newline();
		}
		if((i=readreal(f)) < 0)
			return(0);
		if(i>0)
			return(1);
		printf("? ");
		flushin();
	}
}

/**
 **	get yes/no parameter
 **/

CVNTAB	Yntab[] =
{
	"n",	"o",
	"y",	"es",
	0
};

getynpar(s)
char	*s;
{
	return(getcodpar(s, Yntab));
}


/**
 **	get coded parameter
 **/

getcodpar(s, tab)
char	*s;
CVNTAB	tab[];
{
	char	input[100];
	register CVNTAB		*r;
	register char		*p, *q;
	int			c;

	for ever {
		while ((eof|mkfault)==0 && lineended() && s) {
			printf("%s ", s);
			newline();
		}
		if((c=reads(" \t\n0123456789-./;", input)) < 0)
			return(-1);
		if (c) {
			if (*input == '?') {
				for(r=tab; r->abrev; r++)
					printf("\t%s-%s\n", r->abrev, r->full);
				continue;
			}
			for (r = tab; r->abrev; r++)
			{
				p = input;
				for (q = r->abrev; *q; q++)
					if (*p++ != *q)
						break;
				if (!*q)
				{
					for (q = r->full; *p && *q; q++, p++)
						if (*p != *q)
							break;
					if (!*p || !*q)
						break;
				}
			}
		}
		if (c==0 || r->abrev==0)
		{
			printf("? ");
			flushin();
			continue;
		}
		return(r-tab);
	}
}


/**
 **	get password
 **/

getpasswd(buf)
char buf[];
{
	int s;
	register char c; register int ptr;

	s=signal(SIGINT,1);
	ioctl (0, TCGETA, &otermio);
	ntermio = otermio;
	ntermio.c_lflag &= ~ECHO;
	ioctl (0, TCSETA, &ntermio);
	flushin();
	printf("Enter password: ");
	ptr=0;
	while((c=readchar())!='\n') {
		if(ptr<PWDLEN) buf[ptr++]=c;
	}
	while(ptr<PWDLEN) buf[ptr++]=0;
	ioctl (0, TCSETA, &otermio);
	printf("\n");
	signal(SIGINT,s);
	flushin();
}

/*
**	get device number from name
*/
getdev(s)
char *s;
{
	register i;
	register DEVICE	*c;

	i = 0;
	for(c = &Device[0];c < &Device[NDEV]; c++, i++)
		if(!strcmp(s, c->name))
			return(i);
}
readsep(s)
char *s;
{
	register char rc;

	if(!(rc=any(nextchar(),s)))
		backspace();
	return(mkfault?(-1):rc);
}
