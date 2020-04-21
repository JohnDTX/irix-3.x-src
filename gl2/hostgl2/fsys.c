/**************************************************************************
 *									  *
 * 		 Copyright (C) 1983, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/

/*
** System specific i/o stuff for Unix
**
*/
#include <stdio.h>
#include <signal.h>
#include <sgtty.h>
#include <setjmp.h>
#include <sys/ioctl.h>
#include <xns/Xnsioctl.h>

/* assume we start out without ethernet */
static short haveether = 0;
static int   irisfile = 1;	/* so dliris.f can use wrtbuf_ */
static struct sgttyb ttydat;
static short tty_save;


/*
**	nettst - see if we have a network to blast bits with
**
*/
nettst_()
{
	register short i;
	char *ifp, *getenv();

	if ( (ifp = getenv("IRISFILE")) == NULL)
	    ifp = "/dev/tty";
	if( (irisfile = open(ifp,2)) == -1){
	    fprintf(stderr,"error opening %s\n",ifp);
	    exit(1);
	}
	if(ioctl(irisfile, NXIOSLOW, 0) == -1 && (strcmp(ifp,"/dev/tty")==0))
	    haveether = 0;
	else
	    haveether = (strcmp(ifp,"/dev/tty") == 0);
	return( haveether );
}

/*
**	rdchar - read in a character
**
*/
rdchar_()
{
	return ( unsigned long )getc(stdin);
}


/*
**	wrtbuf - write out a buffer
**
*/

wrtbuf_(outbuf,nbytes)
char *outbuf;
int *nbytes;
{
    fflush(stdout);
    if(haveether)
	netwrite(irisfile,outbuf,*nbytes);
    else
	write(irisfile,outbuf,*nbytes);
}

netwrite(fd, buf, cc)
int fd;
char *buf;
register cc;
{
    struct xnsio io;

    io.addr = buf;
    io.count = cc;
    io.dtype = 0;
    io.control = 0;
    ioctl(fd, NXWRITE, &io);
    return(io.count);
}

cleane_()
{
    ttyres_();
    exit(0);
}
int echoon_();

/*
**	echoff - Turn local echoing off
**
*/
echoff_()
{
    ttydat.sg_flags = tty_save & ~ECHO;
    stty(0, &ttydat);
}


/*
**	echoon - Turn local echoing on
**
*/
echoon_()
{
    ttydat.sg_flags = tty_save | ECHO;
    stty(0, &ttydat);
}

/*
**	termsv - save our current terminal characteristics
**
*/
termsv_()
{
	ttysave();
	signal(SIGQUIT,cleane_);
	signal(SIGINT,cleane_);
	signal(SIGBUS,cleane_);
	signal(SIGTERM,cleane_);
}

/*
**	ttysave - save the state of the tty
**
*/
ttysave()
{
    short t_local = LLITOUT;

    gtty(0, &ttydat);
    tty_save = ttydat.sg_flags;
    if(!haveether)
	ioctl(1,TIOCLBIS,&t_local);	/* set literal output mode */
}

/*
**	ttres - restore the state of the tty
**
*/
ttyres_()
{
    short t_local = LLITOUT;

    ttydat.sg_flags = tty_save;
    stty(0, &ttydat);
    if(!haveether)
	ioctl(1,TIOCLBIC,&t_local);	/* clr literal output mode */
}
