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
**			File Transfer Protocol
**
**	Routines to do file transfer from within wsiris.  Main use is
**	updating 2300's and 3010's with only serial communications.
**
**			Charles Kuta - June 1985
*/

#include <sys/types.h>
/* #include <sys/dir.h> */
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include "term.h"
#include "hostio.h"

#define	MAXPATHNAMELEN	1024

#define BUFFSIZE	1024
#define basename(bn,file)   \
		*(bn) = (*(bn) = strrchr(file,'/')) ? *(bn)+1 : file

long sum();
void send2L();
void senderrno();
void endsend();
void endrecv();

/*
** 	iftpsetup - open a local file and write or read data for it.  
**		    iftpse is the FORTRAN version.  mode can be:
**		  	r - Read the file back to the remote machine.
**		  	w - Write the file with data from the remote machine.
**	      		    temporary file is written to and then renamed.
**			    If the rename fails, EPERM is returned to the
**			    remote machine.
**		  	o - Overwrite.  As 'w', but the temporary file is
**			    not made.
**		  	s - execute a Shell command.  Stdin, stdout, and
**			    stderr are redirected to /dev/null.  The current
**			    directory of the command is always `/'.  If the
**			    command does not exit with a zero status, EPERM
**		      	    is returned to the remote machine.
**
*/
void
iftpse(file, len, mode)
char *file;
long len;
char mode;
{
    register int err = 0;
    register int nbytes, last;
    register int i;
    char *buf;
    char tmpfile[MAXPATHNAMELEN + 5 + 1];
    int fd = -1;
    char *cp, *remfile;
    long xsum;
    int oldumask;
    
    file[len] = '\0';		/* it's guarenteed to be big enough */
    if ((remfile = (char *)malloc(len + 1)) == NULL) {
	errorm('W',"iftpsetup: can't allocate buffer");
	goto done;
    }
    strcpy(remfile,file);	/* file is free-ed by freearrays() */
    errno = 0;
    xsum = 0;
    switch (mode) {
    case 's':
	i = isystem(remfile);
	if (i && i != -1)
	    errno = EPERM;
	senderrno();
	break;

    case 'r':
	if ((fd = open(remfile, O_RDONLY)) >= 0) {
	    if ((buf = allocBa(BUFFSIZE)) == NULL)
		errno = ENOMEM;
	}
	senderrno();
	if (errno)
	    break;
	while ((nbytes = read(fd, buf, BUFFSIZE)) > 0) {
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
	    /* check for host write error */
	    if (err = (nbytes != iftpwrite(buf,nbytes)))
		break;
	}
	if (!err) {
	    (void)iftpwrite(buf,nbytes);
	    send2L((long)errno, xsum);
	}
	break;

    case 'w':
    case 'o':
	oldumask = umask(0);
	if (mode == 'w') {
	    basename(&cp,remfile);
	    cp[MAXPATHNAMELEN] = '\0';
	    strcpy(tmpfile,"/tmp/");
	    strcat(tmpfile,cp);
	    fd = open(tmpfile, O_WRONLY | O_CREAT, 0666);
	}
	else
	    fd = open(remfile, O_WRONLY | O_CREAT, 0666);
	senderrno();
	umask(oldumask);
	if (errno)
	    break;
	last = 0;
	while ((nbytes = iftpread(last,&buf)) > 0) {
	    if (buf == NULL) {
		last = -ENOMEM;
		err = 1;
	    }
	    else if ((last = write(fd, buf, nbytes)) != nbytes) {
		last = -errno;
		err = 1;
	    }
	    else
		xsum += sum(buf, nbytes);
	}
	/* nbytes < 0 is host read error */
	if (nbytes >= 0 && !err) {
	    if (mode == 'w' && strcmp(remfile, tmpfile)) {
		sprintf(buf,"mv %s %s",tmpfile,remfile);
		i = isystem(buf);
		if (i && i != -1)
		    errno = EPERM;
	    }
	    else
		errno = 0;
	    send2L((long)errno, xsum);
	}
	break;
    
    default:
	errorm('w',"invalid mode to iftpsetup: %c (0x%02x)",mode,mode);
	break;
    }

done:
    if (fd >= 0)
	close(fd);
    if (remfile)
	free(remfile);
}

void
iftpsetup(remfile, mode)
char *remfile, *mode;
{
    iftpse(remfile, strlen(remfile), mode[0]);
}

/*
** 	iftpread - read a buffer from the remote machine returning the number
**	      	   of bytes in the buffer.  <last> is sent to the remote
**		   machine first, however.
**
*/
static int
iftpread(last,buf)
int last;
char **buf;
{
    int rv;

    if (last) {
	sendL((long)last);
	endsend();
    }
    if (last >= 0) {
	freearrays();
	*buf = recbs();
	rv = (int)recl();
	endrecv();
    }
    else
	rv = -1;
    return rv;
}

/*
** 	iftpwrite - write a buffer of <nbytes> bytes to the remote machine.
**		    The number of bytes the remote machine actually wrote
**		    is returned.
**
*/
static int
iftpwrite(buf, nbytes)
char *buf;
int nbytes;
{
    int rv;

    sendBs(buf, (long)nbytes);
    sendL((long)nbytes);
    endsend();
    if (nbytes > 0) {
	rv = (int)recl();
	endrecv();
    }
    else
	rv = nbytes;
    return rv;
}

/*
**	send2L - send two longs to the remote machine
**
*/
static void
send2L(l1, l2)
long l1, l2;
{
    sendL(l1);
    sendL(l2);
    endsend();
}


/*
**	senderrno - send errno to the remote machine
**
*/
static void
senderrno()
{
    endrecv();
    sendL((long)errno);
    endsend();
}

/*
**	endsend - done with sending for a while
**
*/
static void
endsend()
{
    puthostchar('\r');
    flushhost();
}

/*
**	endrecv - done with receiving for a while
**
*/
static void
endrecv()
{
}


/*
** 	sum - sum the bytes in an array.  It would have been nice to sum
**	      while sending or receiving the array, but sometimes the host
**	      sends longs and other times chars.
*/
static long
sum(buf, nbytes)
register char *buf;
register nbytes;
{
    register long rv = 0;

    while (nbytes-- > 0)
	rv += *buf++;

    return rv;
}


/*
** isystem - just like libc system() except stdin, stdout, and stderr
**	     are redirected to /dev/null and a setuid(geteuid()) is
**           done (so that irsh commands are executed with real-uid of
**	     root
**
*/
static int
isystem(s)
char	*s;
{
    int	status, pid, w;

    savesig(SIGINT, SIG_IGN);
    savesig(SIGQUIT, SIG_IGN);

    if ((pid = fork()) == 0) {
	signal(SIGINT, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
	freopen("/dev/null","r",stdin);
	freopen("/dev/null","w",stdout);
	freopen("/dev/null","w",stderr);
	chdir("/");
	setuid(geteuid());
	(void) execl("/bin/sh", "sh", "-c", s, 0);
	_exit(127);
    }
    while ((w = wait(&status)) != pid && w != -1)
	continue;
    restoresig(SIGINT);
    restoresig(SIGQUIT);
    return ((w == -1)? w: status);
}
