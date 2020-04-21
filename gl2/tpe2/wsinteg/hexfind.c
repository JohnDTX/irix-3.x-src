/**************************************************************************
 *									  *
 * 		 Copyright (C) 1985, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/


#include <sys/types.h>
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#define BUFSIZE	4096

/*
**	Externals
*/
extern int	strcmp();
 

struct stat	Fstat;
int		lfd, m, o;
u_char		pattern[4] = 0;

main(argc, argv)
int argc;
char *argv[];
{
	u_char buf[BUFSIZE];
	register col, i, n;
	register u_char c;
	register u_char *arg1;

	if (argc == 3) {
		arg1 = (u_char *)&argv[1][0];
		if (argv[1][0] == 'x') 
			arg1 = (u_char *)&argv[1][1];
		else if (argv[1][0] == '0' && argv[1][1] == 'x')
			arg1 = (u_char *)&argv[1][2];
		c = o = 0;
		while (*arg1)
			buf[o++] = *arg1++;
		n = (o + 1) >> 1;
		sscanf(buf,"%lx",&m);
		while (--n >= 0) {
			pattern[c++] = (u_char)(m >> (8 * n));
		}
		o = c;
/*		printf("Searching for %02x %02x %02x %02x\n",
		pattern[0],pattern[1],pattern[2],pattern[3]);*/
	} else
		usage();

	if ((lfd = (stat(argv[2], &Fstat))) < 0) {
		printf("path bad status %d errno %d\n",lfd,errno);
		exit(0);
	}
	if ((lfd = (open(argv[2], O_RDONLY,"r"))) <= 0) {
		printf("EXIT from od3279 with bad open \n");
		exit(0);
	}
	m = 0;
	n = Fstat.st_size;
/*	printf("\n%s 0x%x\n",argv[2],n);*/
	col = 0;
	while (n > 0) {
		n = read(lfd,buf,BUFSIZE);
		arg1 = buf;
		for (i=1; i <= n; i++) {
			c = *arg1++;
			if (c == pattern[col]) {
				col++;
				if (col == o) {
					printf("%x ",m + i - o);
					col = 0;
				}
			} else
				col = 0;
		}
		m += n;
	}
}

/*
**	Display usage message and exit program
*/
usage()
{
	(void)printf("\007\nUsage: bfind hex_pattern filename\n");
	exit(1);
}
