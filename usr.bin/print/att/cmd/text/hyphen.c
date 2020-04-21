/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

char *xxxvers = "@(#)hyphen:hyphen.c	1.5";
#include <stdio.h>
char b[242];
char c[60];
int nread = 1;
char buf[BUFSIZ];

main(argc,argv)
int argc;
char **argv;
{
	int ifile,isw,k,i,j,r;
	extern int optind;
	int stdinflg = 0;

	while ((r = getopt(argc,argv,"")) != EOF) {
		switch (r) {
		default:
			fprintf(stderr,"Usage: hyphen files\n");
			exit(1);
		}
	}
	if (optind == argc) {
		argc++;
		stdinflg++;
	}
	for (argv = &argv[optind]; optind <argc; optind++, argv++){
		if ((*argv)[0]=='-' &&(*argv)[1]=='\0' || stdinflg)
			ifile = 0;
		else {
			printf("%s:\n \n",*argv);
			if ((ifile = open(*argv, 0)) < 0) {
				fprintf(stderr, "hyphen: cannot open %s\n", *argv);
				continue;
			}
		}
newl:	isw = j =  0;
	i = -1;

cont:	while((b[++i] = get(ifile)) != 0)
		{if((b[i] >= 'a' && b[i] <= 'z') ||
		(b[i] >= 'A' && b[i] <= 'Z'))
			{c[j++] = b[i];
			goto cont;
			}
		if(b[i] == '-')
		{c[j++] = b[i];
			if((b[++i] = get(ifile)) != '\n')
			{c[j++] = b[i];
			goto cont;
			}
			if(j == 1)goto newl;
			isw = 1;
			i = -1;
			while(((b[++i] = get(ifile)) == ' ')
			|| (b[i] == '\t') || (b[i] == '\n'));
			c[j++] = b[i];
			goto cont;
		}
		if(b[i] == '\n'){if(isw != 1)goto newl;
			i = -1; }
		if(isw == 1)
			{k = 0;
			c[j++] = '\n';
			while(k < j)putchar(c[k++]);
			}
		isw = j = 0;
		}
	}
}
/* ------------------------------------------------ */
get(ifile)
int ifile;
{
	static char *ibuf;

	if(--nread)return(*ibuf++);
	if(nread = read(ifile,buf,BUFSIZ))
		{if(nread < 0)goto err;
		ibuf = buf;
		return(*ibuf++);
		}

	nread = 1;
	return(0);

err:	nread = 1;
	fprintf(stderr,"read error\n");
	return(0);
}
