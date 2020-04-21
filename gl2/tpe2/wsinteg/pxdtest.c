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

/**************************** T E R M 3 2 7 9 *****************************
*
*	3279 terminal emulator through the CXI coax interface card
*
*********************************************************************/

#include <sys/types.h>
#include <sys/termio.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <gl.h>
#include "pxw.h"

#define READ		    0
#define WRITE		    1
#define MAXHOSTLEN	    31		    /* maximum length of a hostname */
/*
**	Externals
*/

extern int 	errno;
extern int	fd;
extern outft	frcv;
extern u_char	rbuf[];
extern char	*sys_errlist[];
extern int	sys_nerr;
extern u_char	ttytype;
extern u_char	Breaksent;


/*
**	Globals
*/

u_char		Manuflag = 1;
u_char		Msg_proc = 0;	/* [1] - MXFER for message counting
				 * [2] - RGLXFER for RGL operation
				 * [4] - FXFER for file transfer */
static char ident[] = "@(#) Pxdtest Version 1.4, GL2 WS";
int		context = 1;
short		maxcom = 0; 
int		graphinited = 0;
int		ingraphprog = 0;
u_char		sbuf[4];
int		rc;
u_char		*rp;
FILE		*mlf, *rlf;
int		ttyd;			/* stdin,0  is keyboard */
char    cmdbuf[16 + MAXHOSTLEN + 1] = "CMDNAME=t3279:";
	/* command line flags */
int     dflag[4] = { 0 };	/* only 3 so far ([0] isn't used for
				 * simplicity):
				 *   [1] - create RGL command logfile
				 *   [2] - create RGL ascii logfile
				 *   [3] - don't catch SIGBUS or SIGSEGV
				 */
int     pflag = 0;		/* don't ignore text when the textport is
				 * off */
int     replay[5] = { 0 };	/*   [4] - doing a replay
				 *   [1] - replay RAWINP file
				 *   [2] - replay RGL RDDATA logfile
				 *   [3] - initialize ttytype and execute RGL
				 */
int	zflag[5] = { 0 };
int		gcmdcnt = 0;
FILE * lf = NULL;
int     ignoretext = 0;
int	tpison = 1;
int     xstat = 0;
int     subshellpid = 0;
char	*inputfile;




main(argc, argv)
int argc;
char *argv[];
{
	register i;
	int trace_flag = 0;

	if (argc > 1) {
		if (argv[1][0] == '-') {
			inputfile = argv[2];
			for (i=1; argv[1][i]!='\0'; ++i) {
				switch (argv[1][i])
				{
				case 'm':
					Manuflag = 1;	/* minimal open */
					break;
				case 't':
					ttytype = 2;	/* x.4 textport */
					replay[3] = 1;
					break;
				case 'v':
					ttytype = 1;	/* x.3 textport */
					replay[3] = 1;
					break;
				default:
					usage();
					break;
				}
			}
			if (i==1)
				usage();
			else
				if (trace_flag)
					tr_init();
		} else
			usage();
	}
	usropen();
/*	irisinit();
	initcom(I3270_COM);*/
	if (pxdopen() <= 0) {
		(void)printf("Cannot open '/dev/pxd':  errno = %d\n", errno);
		exit(1);
	}
	do_conopen();
	setbuf (stdout, (char *)0);
	i = f13_loop(1,0);
	i = f14_diag();

	if (trace_flag)
		tr_close();
	if (pxdclose()) {
		perror("pxd close error ");
		exit(1);
	}
	exit(0);
}

readhost() {	}


/*
**	Trace this module
**	Dummy to keep lint happy, no tracing (DT) present
*/
tr_main(flag)
{
	trace = flag;
}

oops(fmt, args)
char   *fmt;
{
    _doprnt(fmt, &args, stderr);
    xstat = 1;

}

/*
**	Display usage message and exit program
*/
usage()
{
	(void)printf("\007\nUsage:  pxdtest [-mtv]\n");
	exit(1);
}
