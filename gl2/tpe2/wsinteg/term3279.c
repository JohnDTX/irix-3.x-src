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
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <gl.h>
#include "rpc.h"
#include "term.h"
#include "hostio.h"
#include "pxw.h"

#define READ		0
#define WRITE		1
#define MAXHOSTLEN	31		    /* maximum length of a hostname */
/*
**	Externals
*/

extern int 		errno;
extern int		fd;
extern outft		frcv;
extern u_char		rbuf[];
extern char		*sys_errlist[];
extern int		sys_nerr;
extern u_char		ttytype;
extern u_char		Breaksent;
extern u_char		F3174;
extern u_char		Keyboard;


/*
**	Globals
*/

u_char			Manuflag = 0;
u_char			Msg_proc = 0;	/* [1] - MXFER for message counting
					 * [2] - RGLXFER for RGL operation
					 * [4] - FXFER for file transfer */
static char		ident[] = "@(#) T3279 Version 1.8, GL2 WS";
int			context = TEXT;
int			kbdlocked = 0;
short			maxcom = 0; 
int			graphinited = 0;
int			ingraphprog = 0;
int			ttyd;			/* stdin,0  is keyboard */
int			writehostpid;
char    cmdbuf[16 + MAXHOSTLEN + 1] = "CMDNAME=t3279:";
	/* command line flags */
int     dflag[5] = { 0 };	/* only 3 so far ([0] isn't used for
				 * simplicity):
				 *   [1] - create RGL command logfile
				 *   [2] - create RGL ascii logfile
				 *   [3] - don't catch SIGBUS or SIGSEGV
				 *   [4] - doing R snapshots
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
	register int i;
	int trace_flag = 0,uxx;

	if (argc > 1) {
		if (argv[1][0] == '-') {
			inputfile = argv[2];
			for (i=1; argv[1][i]!='\0'; ++i) {
				switch (argv[1][i])
				{
				case '1': 
					F3174 = TRUE;	/* 3174 controller */
					break;
				case 'g':
					dflag[1] = 1;	/* RGL command log */
					break;
				case 'i':
					replay[2] = 1;	/* RDDATA input mode */
					replay[4] = 1;
					break;
				case 'm':
					Manuflag = TRUE; /* minimal open */
					break;
				case 'o':
					dflag[2] = 1;	/* ascii RGL logs */
					break;
				case 'r':
					logenable(2); /* creates RAWINP file */
					break;
				case 's':
					logenable(3);	/* log 3274 snaps */
					if (argc == 3) { /* used for 2300's */
						uxx = strlen(inputfile);
						dflag[4] = uxx;
					} else
						dflag[4] = 0;
					break;
				case 't':
					ttytype = 2;	/* textport */
					break;
				case 'z':
					zflag[1] = 1;	/* xginit -> gbegin */
					break;  /* if mex is up use for RGL */
#ifdef DEBUG
				case 'c':
					tr_conio(1);	/* trace conio */
					trace_flag++;
					break;
				case 'e':
					tr_emul(1);	/* trace emulint */
					trace_flag++;
					break;
				case 'E':
					tr_emulator(1);	/* trace emulator */
					trace_flag++;
					break;
				case 'f':
					tr_fnct(1);	/* trace functions */
					trace_flag++;
					break;
				case 'h':
					tr_host(1);	/* trace hostio3279 */
					tr_iris(1);	/* trace irisio3279 */
					trace_flag++;
					break;
				case 'l':
					tr_conio(1);	/* trace conio */
					tr_emulator(1);	/* trace emulator */
					tr_fnct(1);	/* trace functions */
					tr_host(1);	/* trace hostio3279 */
					tr_iris(1);	/* trace irisio3279 */
					tr_main(1);	/* trace main */
					tr_pxdio(1);	/* trace pxdio */
					trace_flag++;
					break;
				case 'p':
					tr_pxdio(1);	/* trace pxdio */
					trace_flag++;
					break;
				case 'q':
					replay[1] = 1;	/* RAWINP mode */
					replay[4] = 1;
					logenable(1); /* don't write raw log */
					break;
#endif /* DEBUG */
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
	if (replay[4] && argc < 3) {
		usage();
		exit(0);
	}
	usropen();
	irisinit();
	initcom(I3270_COM);
	if (replay[2]) {		/* RDDATA RGL input mode */
		context = TEXT;
		if (!Keyboard)
			qkeys(1);
		else
			conopen(1);
		if (!replay[1])
			dflag[2] = 0;	/* no ascii in, ascii out */
		rc = read(fd, rbuf, 3);
		if (*rbuf == 0x05) {
			i = *(rbuf+1);
			i <<= 8;
			i += *(rbuf+2);
			rc = read(fd, rbuf, i);
			errno = 0;
		} else {
			printf("Bad RGL inputfile, try another\r\n");
			usage();
			if (!Keyboard)
				unqkeys();
			else
				conclose(1);
			exit(1);
		}
		rp = rbuf;
#ifdef DEBUG
		DT("%d-read ",rc);
#endif /* DEBUG */
		if (!Keyboard)
			unqkeys();
		else
			conclose(1);
		readhost();
	} else {		/* raw 3274 input */
		dbgmenu();
	}

	if (trace_flag)
		tr_close();
	if (pxdclose()) {
		perror("pxd close error ");
		exit(1);
	}
	exit(0);
}

/*
**	readhost - interpret the character stream from the host 
**
*/
readhost()
{	
	register u_char onechar;
	register u_char scrtouched;

	scrtouched = 0;
	if (dflag[1])
		tadelay(-10);	/* this routine is used to open log files */
	if ((ttyd = open("/dev/tty", O_RDWR | O_NDELAY, 0))<=0) {
		if (errno < sys_nerr)
			(void)perror("pxd:readhost '/dev/tty' error ");
		else
			(void)printf("Cannot open '/dev/tty' - errno = %d\n", errno);
	}
	rawmode(ttyd);
	while(1) {
		onechar = gethostchar();
		if (Breaksent) {
			DT("Breaksent ");
			break;
		}
		if ((onechar & 0x7f) == TESC) {/* graphic escape 0x10 */
			if (scrtouched) {
				flushscreen();
				scrtouched = 0;
			}
			doprimitive();
		} else if ((char)onechar == ERROR) {
			DT("-1onechar ");
			break;
		} else if (lf)
			putexpc(onechar, lf);
		if(scrtouched) {
			flushscreen();
			scrtouched = 0;
		}
	}
	cleanup();
	restoremode(ttyd);
	close(ttyd);
	errno = 0;
}


cleanup()
{
    kblamp(LAMP_LOCAL,0);
    kblamp(LAMP_KBDLOCKED,0);
    Breaksent = 0;
}

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
    cleanup();

}

/*
**	Display usage message and exit program
*/
usage()
{
#ifdef DEBUG
	(void)printf("\007\nUsage:  t3279 [-1cefghilmnopqrstz] [filename]\n");
#else
	(void)printf("\007\nUsage:  t3279 [-1giostz] [filename]\n");
#endif
	exit(1);
}
