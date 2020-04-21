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

/* One of `gl1' or `gl2' must be defined. */

#ifdef gl1
#define GL1
#undef GL2
#endif

#ifdef gl2
#define GL2
#undef GL1
#endif

#ifndef FALSE
#define	FALSE		0
#endif
#ifndef TRUE
#define TRUE		(!FALSE)
#endif

#define NULLP(type)	((type *)0)
#define MAXHOSTLEN	31		    /* maximum length of a hostname */
#define DEFAULT_SERIAL_LINE \
			"/dev/ttyd2"	    /* default serial line to host */
#define I488_RDEV	"/dev/ib1"	    /* IEEE-488 read from host dev */
#define I488_WDEV	"/dev/ib2"	    /* IEEE-488 write to host dev */
#define I488_DEV	I488_RDEV	    /* a full-duplex IEEE-488 device
					     * that is used when logins can
					     * done over the IEEE-488
					     */
#define TCPINIT		"/etc/rc.tcp"	    /* TCP/IP initialization script */
#define HOSTS		"/etc/hosts"	    /* TCP/IP hosts file */
#define DEFAULT_SHELL   "/bin/sh"	    /* shell used if no SHELL 
					     * environment variable */
#define REBOOT		"/etc/reboot"	    /* reboot command */

#define COLORD		"/usr/lib/colord"   /* color hardcopy daemon */
#define SPOOL		"/usr/spool/colord" /* spool dir for COLORD */
#define PREFIX		"ldata"		    /* COLORD only processes files
					     * with this prefix */

/* the graphics contexts */

#define TEXT		1
#define GRAPHICS	2

/* the pipe commands */

#define SWITCHCONTEXT	1
#define SETMONITOR	2
#define TOGGLEPFLAG	3
#define TOGGLEXFLAG	4
#define RESETDISPLAY	5
#define INSUBSHELL	6
#define TOGGLETP	7
#define CLEAR4010	8
#define HARDCOPY4010	9
#define KILLCHILD	10		/* to be used later */
#define TERMINATE	11
#define TOGGLEDFLAG	12
#define TOGGLEZFLAG	13
#define LOGFILENAME	14

#ifdef GL1
#define LAMP_KBDLOCKED	(1<<2)
#define LAMP_LOCAL	(1<<1)
#else GL2
#define LAMP_KBDLOCKED	(1<<5)
#define LAMP_LOCAL	(1<<4)
#endif GL2

#define putscreenchar(c) putc(c, stdout)
#define flushscreen()	 fflush(stdout)

#define GetBuflenAheadOfTime \
			gl_GetBuflenAheadOfTime

#define ttyfd		0	    /* file descriptor for keybd (stdin) */

typedef int (*FUNPTR)();

extern int     	hosttype;
extern int     	host;
extern int	grhost_r;
extern int	grhost_w;
extern int	have488;
extern int	escsub4010;
extern char 	cmdbuf[];
extern char 	serialline[];
extern unsigned	maxcom;
extern int 	xstat;

extern int     	infile;
extern int     	outfile;

extern int     	fromreader;
extern int     	towriter;

extern int     	toreader;
extern int     	fromwriter;

extern int 	writehostpid;
extern int 	netinputpid;
extern int 	i488inputpid;
extern int 	pipereaderpid;

extern int 	insubshell;
extern int 	graphinited;
extern int 	ingraphprog;
extern int 	ignoretext;
extern int 	pipeready;
extern int 	haveconnection;
extern int 	context;
extern int 	kbdlocked;
extern int 	tpison;

extern int 	dflag[];
extern int 	n_dflag;
extern int 	noesc;
extern char 	escchar;
extern int 	fflag;
extern int 	hflag;
extern int 	iflag;
extern int 	pflag;
extern int 	speed;
extern int 	speedcode;
extern int 	xflag;
extern int 	yflag;
extern int 	zflag[];
extern int 	n_zflag;

extern char	logfile[];
extern FILE	*lf;
extern int 	gcmdcnt;

extern unsigned curcmd;
extern long	a_retval;

extern int	fastmode;
