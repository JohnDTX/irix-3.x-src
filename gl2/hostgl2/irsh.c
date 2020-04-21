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

/*
**			IRIS Remote Shell
**
**	Allows a shell command to be executed on a remote IRIS running the
**	terminal emulator program, wsiris.  The remote shell's stdin,
**	stdout, and stderr are redirected to /dev/null.  
**	
*/

#include <errno.h>
#include <stdio.h>

extern int errno;

void endsend();
void endrecv();

main(argc, argv)
int argc;
char *argv[];
{
    register int i;
    register char *cmd;
    register int size;

    if (argc < 2) {
	fprintf(stderr,"Usage: irsh command [arg ...]\n");
	exit(1);
    }
    
    size = 1;				/* room for NUL */
    for (i = 1; i < argc; i++)
	size += strlen(argv[i]) + 1;	/* +1 is for space */
    if ((cmd = (char *)malloc(size)) == NULL) {
	fprintf(stderr,"irsh: can't malloc\n");
	exit(1);
    }
    strcpy(cmd,argv[1]);
    for (i = 2; i < argc; i++) {
	strcat(cmd," ");
	strcat(cmd,argv[i]);
    }

    netinit();
    iftpsetup(cmd,"s");
    echoff();
    errno = recerrno();
    echoon();
    endprim();

    if (errno == EPERM)
	fprintf(stderr,"irsh: remote command failed\n");
    else if (errno)
	perror("irsh: remote command failed");
    ttyrestore();
    exit(errno);
}

int
recerrno()
{
    int rv;

    endsend();
    rv = (int)recL();
    endrecv();
    return rv;
}

void
endsend()
{
    flushg();
}

void
endrecv()
{
    reccr();
}

