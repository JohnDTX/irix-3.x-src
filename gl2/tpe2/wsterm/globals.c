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

#include <stdio.h>
#include "term.h"

int     hosttype;		/* type of text communications == type
				 * of graphics communications except when
				 * using IEEE-488 with serial.
				 */
int     host;			/* file descriptor for line to host */
int	grhost_r;		/* fd for graphics line from host -- only
				 * time it's different from host is when
				 * IEEE-488 with serial is used.
				 */
int	grhost_w;		/* fd for graphics line to host.  The 
				 * IEEE-488 bus is only half-duplex,
				 * and since the reading from it and
				 * the writing to it are done by
				 * separate processes, separate IEEE-488
				 * devices must be used for each in order to
				 * prevent deadlock.
				 */
int	have488 = 0;		/* IEEE-488 board is installed */
int	escsub4010 = 0;		/* in non-blocking read mode */

char    cmdbuf[16 + MAXHOSTLEN + 1] = "CMDNAME=wsiris:";
char 	serialline[50] = DEFAULT_SERIAL_LINE;
unsigned maxcom = 0;		/* number of commands in dispatch table */
int	xstat = 0;		/* exit status */

/* pipe 1 */
int     infile;			/* readhost() reads from this */
int     outfile;		/* writehost() writes half-duplex echo
				 * and netread() writes net input to this 
				 */
/* pipe 2 */
int     fromreader;		/* pipe from readehost() to writehost() */
int     towriter;

/* pipe 3 */
int     toreader;		/* pipe from writehost() to readhost() */
int     fromwriter;

int     writehostpid = 0;	/* pid of writehost() */
int     netinputpid = 0;	/* pid of netinput() */
int	i488inputpid = 0;	/* pid of i488input() */
int     pipereaderpid = 0;	/* who to signal after writing to the pipe */

int     graphinited = 0;	/* ginit (or equiv) has been called */
int     ingraphprog = 0;	/* this is different from graphinited */
int     ignoretext = 0;		/* if 1, throw text from host away */
int     pipeready = 0;		/* one or more pipe cmds are awaiting */
int     insubshell = 0;		/* if 1, writehost() has spawned a subshell
				 * and readhost() will wait before killing
				 * everyone off
				 */
int     haveconnection = 0;	/* readhost() maintains this */
int     context = TEXT;		/* readhost() keeps writehost() value up
				 * to date
				 */
int	kbdlocked = 0;		/* if 1, keyboard is locked */
int	tpison = 1;		/* textport is on */

	/* command line flags */
int     dflag[6] = { 0 };	/* only 5 so far ([0] isn't used for
				 * simplicity):
				 *   [1] - create a logfile
				 *   [2] - don't catch SIGBUS or SIGSEGV
				 *   [3] - logfile name is next argument
				 *   [5] - kill writehost() when initializing
				 *	   the graphics -- useful when
				 *	   debugging readhost().  
				 *	   
				 */
int	n_dflag = (sizeof dflag) / sizeof(int);
int  	noesc = 0;		/* if 1, then no escape char */
char 	escchar = '~';		/* the escape char */
int  	fflag = 0;		/* don't read ~/.wsirisrc */
int  	hflag = 0;		/* Half duplex flag. */
int  	iflag = 0;		/* try TCP/IP protocol first instead of
				 * XNS
				 */
int     pflag = 0;		/* don't ignore text when the textport is
				 * off
				 */
int     speed;			/* serial port speed */
int	speedcode;		/* bits to give ioctl */
int     xflag = 0;		/* do not turn on IXOFF */
int  	yflag = 0;		/* do not turn on IXON */
int     zflag[5] = { 0 };	/* only 4 so far ([0] isn't used for
				 * simplicity):
				 *   [1] - convert ginit's into gbegin's
				 *   [2] - do a ginit immediately
				 *   [3] - only try one of TCP/IP (-i flag
				 *	   given) or XNS (no -i)
				 *   [4] - emulate a Tektronics 4010
				 */
int	n_zflag = (sizeof zflag) / sizeof(int);

char	logfile[100] = "LOGFILE.0";
				/* pathname of logfile */
FILE	*lf = NULLP(FILE);	/* file pointer of log file */
int     gcmdcnt = 0;		/* count of graphics commands logged */

unsigned curcmd;		/* current graphics cmd -- used to lookup
				 * cmd name for error messages
				 */
long    a_retval;		/* value returned for graphics cmd */

int	fastmode = 0;		/* if 1 then in fast lane */
