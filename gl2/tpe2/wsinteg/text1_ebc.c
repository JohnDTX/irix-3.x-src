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
#include <sys/termio.h>
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <gl.h>
#include "rpc.h"
#include "term.h"
#include "hostio.h"
#include "pxw.h"
#define BUFSIZE	2400


FILE		*lod = NULL;
u_char		buf[BUFSIZE+4], pbuf[256];	

main(argc, argv)
int argc;
char *argv[];
{
	register int col, i, n;

	if ((lod = (fopen("CUT","w"))) <= 0) {
		printf("EXIT from od3279 with bad open \n");
		exit(0);
	}
	buf[0] = 0xf0;
	buf[1] = 0xf0;
	buf[2] = 0x40;
	buf[3] = 0xf0;
	buf[4] = 0x40;
	buf[5] = 0;
	fprintf(lod,buf);
	for (col = 1; col < 257; col++) {
		i = col / 16;
		n = col % 16;
		if (i > 9)
			buf[0] = (u_char)(i + 0x77);
		else
			buf[0] = (u_char)(i + 0xf0);
		if (n > 9)
			buf[1] = (u_char)(n + 0x77);
		else
			buf[1] = (u_char)(n + 0xf0);
		buf[2] = 0x40;
		buf[3] = (u_char)col;
		if (i == 2 && n == 5)
			buf[3] = (u_char)0xd8;
		buf[4] = 0x40;
		buf[5] = 0;
		fprintf(lod,buf);
	}
	fflush(lod);
	fclose(lod);
}

/*
**	Display usage message and exit program
*/
usage()
{
	(void)printf("\007\nUsage: cutr -[cmrw] infilename [+offset]\n");
	exit(1);
}
