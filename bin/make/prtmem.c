/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef	sgi
#ident	"@(#)make:prtmem.c	1.2.1.1"
#endif
 
#ifdef GETU
#define udsize uu[0]
#define ussize uu[1]
#endif


prtmem()
{
#ifdef GETU
#include "stdio.h"
#include "sys/param.h"
#include "sys/dir.h"
#include "sys/user.h"
	unsigned uu[2];
	register int i;

	if(getu( &((struct user *)0)->u_dsize, &uu, sizeof uu) > 0)
	{
		udsize *= 64;
		ussize *= 64;
		printf("mem: data = %u(0%o) stack = %u(0%o)\n",
			udsize, udsize, ussize, ussize);
/*
 *	The following works only when `make' is compiled
 *	with I&D space separated (i.e. cc -i ...).
 *	(Notice the hard coded `65' below!)
 */
		udsize /= 1000;
		ussize /= 1000;
		printf("mem:");
		for(i=1; i<=udsize;i++)
		{
			if((i%10) == 0)
				printf("___");
			printf("d");
		}
		for(;i<=65-ussize;i++)
		{
			if((i%10) == 0)
				printf("___");
			printf(".");
		}
		for(;i<=65;i++)
		{
			if((i%10) == 0)
				printf("___");
			printf("s");
		}
		printf("\n");
		fflush(stdout);
	}
#endif
}
