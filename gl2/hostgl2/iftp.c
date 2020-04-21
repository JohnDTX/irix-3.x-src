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
**			IRIS File Transfer Program
**
**	Allows the reading and writing of files on an IRIS running the
**	terminal emulator program, wsiris.
**
**	BUFFSIZE blocks are read or written over the communcations link
**	with the IRIS using the send and receive byte array routines
**	in the Remote Graphics Library.  The link is assumed to be
**	error-free.  However, checksums are computed on both ends and
**	compared as a precaution.
**
**	See banner of iftp() below for further information.
**
*/

#ifdef SYSTEM5
#include <fcntl.h>
#else UNIX4_2
#include <sys/file.h>
#endif
#include <stdio.h>

#define LOCREADERR	-1
#define LOCWRITEERR	-2
#define CHECKSUMERR	-3
#define BADMODE		-4
#define NOTABSPATH	-5

char *locerrmsg[] = {
    "local read error",
    "local write error",
    "checksum error - retry transfer",
    "bad mode argument to iftp()",
    "remote pathname not absolute"
};

extern int errno;

long sum();
void rec2L();
void endsend();
void endrecv();
void usage();

main(argc, argv)
int argc;
char *argv[];
{
    int fd;

    if (argc != 4)
	usage();
    switch (argv[1][0]) {
    case 'w':
    case 'o':
	if ((fd = open(argv[2], O_RDONLY)) >= 0) {
	    errno = iftp(fd, argv[3], argv[1]);
	}
	else {
	    perror("iftp: can't read input file");
	    exit (1);
	}
	break;
    case 'r':
	if ((fd = open(argv[2], O_WRONLY | O_CREAT, 0666)) >= 0) {
	    errno = iftp(fd, argv[3], argv[1]);
	}
	else {
	    perror("iftp: can't create output file");
	    exit (1);
	}
	break;
    default:
	usage();
	break;
    }
    ttyrestore();
    if (errno < 0)
	fprintf(stderr, "iftp: %s\n", locerrmsg[-errno-1]);
    else if (errno > 0)
	perror("iftp: IRIS error");
    exit(errno);
}

void
usage()
{
    fprintf(stderr,"Usage: iftp {rwo} locfile remfile\n");
    exit(1);
}


/*
** iftp - open a remote file and write or read data for it.  Mode can be:
**		r - Read the file from the IRIS writing to fd.
**		w - Write the file on the IRIS reading from fd.
**		    A temporary file is written to and then renamed.
**		o - Overwrite.  As 'w', but the temporary file is
**		    not made.
**	  Returns 0 if successful, errno from IRIS if it failed,
**	  or a negative number there was a local error.  The only absolute
**	  pathnames for the remote file are accepted.  If the rename of a 
**	  'w' mode command fails, EPERM (1) is always returned.
**
*/

#define BUFFSIZE	1024

static char buf[BUFFSIZE];

long
iftp(fd, remfile, mode)
int fd;
char *remfile;
char *
mode;
{
    register int err = 0;
    register int nbytes, last;
    register int i;
    char *cp;
    long errno, errno2, dummy;
    long xsum, ixsum;

    errno = 0;
    xsum = 0;
    
    if (remfile[0] != '/')
	return NOTABSPATH;

    netinit();
    iftpsetup(remfile, mode);
    echoff();
    errno = recerrno();
    if (errno)
	goto done;

    switch (mode[0]) {
    case 'w':
    case 'o':
	while ((nbytes = read(fd, buf, sizeof buf)) > 0) {
	    /*
	     * checksumming is always done on longs, so make sure
	     * ones past the end have a known value (zero)
	     */
	    if (i = (nbytes % 4)) {
		i = 4 - i;
		while (i-- > 0)
		    buf[nbytes + i] = 0;
	    }
	    xsum += sum(buf, nbytes);
	    i = iftpwrite(buf, nbytes);
	    if (err = (nbytes != i))
		break;
	}
	if (err) {
	    /* remote write error */
	    errno = -i;
	}
	else {
	    (void)iftpwrite(buf, nbytes);
	    if (nbytes >= 0) {
		rec2L(&errno, &ixsum);
		if (errno == 0 && ixsum != xsum)
		    errno = CHECKSUMERR;
	    }
	    else
		errno = LOCREADERR; 
	}
	break;

    case 'r':
	last = 0;
	while ((nbytes = iftpread(last,buf)) > 0) {
	    if ((last = write(fd, buf, nbytes)) != nbytes) {
		last = -1;
		err = 1;
	    }
	    else
		xsum += sum(buf, nbytes);
	}
	if (err)
	    /* local write error */
	    errno = LOCWRITEERR;
	else {
	    rec2L(&errno, &ixsum);
	    if (errno)
		; /* IRIS read error */
	    else if (ixsum != xsum)
		errno = CHECKSUMERR;
	}
	break;

    default:
	errno = BADMODE;
	break;
    }

done:
    echoon();
    endprim();
    return errno;
}

int
iftpread(last,buf)
int last;
char *buf;
{
    int rv;

    if (last) {
	sendl((long)last);
	endsend();
    }
    if (last >= 0) {
	recBs(buf);
	rv = (int)recL();
	endrecv();
    }
    else
	rv = -1;
    return rv;

}

int
iftpwrite(buf, nbytes)
char *buf;
int nbytes;
{
    int rv;

    sendbs(buf, (long)nbytes);
    sendl((long)nbytes);
    endsend();
    if (nbytes > 0) {
	rv = (int)recL();
	endrecv();
    }
    else
	rv = nbytes;
    return rv;
}

void
rec2L(l1, l2)
long *l1, *l2;
{
    *l1 = recL();
    *l2 = recL();
    endrecv();
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

/*
 * sum - sum the bytes in an array.  It would have been nice to sum
 *	 while sending or receiving the array, but sometimes the host
 *	 sends longs and other times chars.
 */

long
sum(buf, nbytes)
register char *buf;
register long nbytes;
{
    register long rv = 0;

    while (nbytes-- > 0)
	rv += *buf++;

    return rv;
}

