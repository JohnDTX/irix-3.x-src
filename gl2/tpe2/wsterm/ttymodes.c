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
** 	ttymodes.c - switch tty modes between cooked and raw
**
** 	Same as routines in libxns.a, except TCSETAF is used instead of
** 	TCSETA to prevent garbled output, use of xflag to set IXON,
** 	and test for saved in restoremode().
*/

#include <termio.h>
#include "term.h"

static struct termio    old;
static struct termio    new;
static int		saved = 0;

rawmode(fd)
int     fd;
{
    if (saved == 0) {
	ioctl(fd, TCGETA, &old);
	saved = 1;
    }
    ioctl(fd, TCGETA, &new);
    new.c_cflag &= ~(CSIZE);
    new.c_cflag |= CS8;
    new.c_iflag = xflag ? IXON : 0;
    new.c_lflag &= ~(ISIG | ICANON | XCASE | ECHO);
    new.c_oflag &= ~(OPOST);
    new.c_cc[VMIN] = 1;
    new.c_cc[VTIME] = 1;
    ioctl(fd, TCSETAF, &new);
}


restoremode(fd)
int     fd;
{
    if (saved)
	ioctl(fd, TCSETAF, &old);
}
