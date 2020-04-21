#ifndef lint
static char Notice[] = "Copyright (c) 1985 Adobe Systems Incorporated";
static char *RCSID="$Header: /d2/3.7/src/usr.bin/print/trscript/src/RCS/pscomm.c,v 1.1 89/03/27 18:20:50 root Exp $";
#endif
/* pscomm.c
 *
 * Copyright (C) 1985 Adobe Systems Incorporated
 *
 * System V lp/lpsched communications filter for PostScript printers
 *
 * pscomm is the general communications filter for
 * sending files to a PostScript printer (e.g., an Apple LaserWriter,
 * QMS PostScript printer, or Linotype PostScript typesetter)
 * via RS232 lines.  It does error handling/reporting,
 * job logging, etc.
 * It observes (parts of) the PostScript file structuring conventions.
 * In particular, it distinguishes between PostScript files (beginning
 * with the "%!" magic number) -- which are shipped to the printer --
 * and text files (no magic number) which are formatted and listed
 * on the printer.  Files which begin with "%!PS-Adobe-" may be
 * page-reversed if the target printer has that option specified.
 *
 *
 * pscomm gets called with:
 *	stdin	== the file to print (may be a pipe!)
 *	stdout	== the printer
 *	stderr	== the printer log file
 *	cwd	== the spool directory ???
 *	argv	== varies depending on how invoked, but one of: ???
 *	  filtername -p filtername -n login -h host
 *	environ	== various environment variable effect behavior
 *		JOBOUTPUT	- file for actual printer stream
 *				  output (if defined)
 *		VERBOSELOG	- do job verbose logging
 *
 * pscomm depends on certain additional features of the System V spooling
 * architecture.
 *
 * Edit History:
 * Andrew Shore: Fri Oct 25 10:29:01 1985
 * End Edit History.
 *
 * RCSLOG:
 * $Log:	pscomm.c,v $
 * Revision 1.1  89/03/27  18:20:50  root
 * Initial check-in for 3.7
 * 
 * Revision 1.1  86/11/17  20:23:25  root
 * Initial revision
 * 
 * Revision 2.1  85/11/24  11:50:32  shore
 * Product Release 2.0
 * 
 * Revision 1.2  85/11/20  10:11:40  shore
 * Alarm kept ringing, reset signal handler
 * 
 *
 */

#include <ctype.h>
#include <errno.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <termio.h>
#include <fcntl.h>
#include <time.h>

#include "transcript.h"
#include "psspool.h"

#ifdef BDEBUG
#define debugp(x) {fprintf x ; (void) fflush(stderr);}
#else
#define debugp(x)
#endif BDEBUG

private jmp_buf startstatus, dwait, sendint;

private char	*prog;			/* invoking program name */
private int	progress, oldprogress;	/* finite progress counts */
private int	getstatus = FALSE;
private int	goahead = FALSE;	/* got initial status back */
private int	gotemt = FALSE;		/* got ^D ack from listener */
private int	sendend = TRUE;		/* send an ^D */

private int	cpid = 0;	/* listener pid */

private int	intrup = FALSE;	/* interrupt flag */

private char abortbuf[] = "\003";	/* ^C abort */
private char statusbuf[] = "\024";	/* ^T status */
private char eofbuf[] = "\004";		/* ^D end of file */

private char EOFerr[] = "%s: unexpected EOF from printer (%s)!\n";

/* global file descriptors (avoid stdio buffering!) */
private int fdsend;		/* to printer (from stdout) */
private int fdlisten;		/* from printer (same tty line) */
private int fdinput;		/* file to print (from stdin) */

private FILE *jobout;		/* special printer output log */
private char *verboselog;		/* VERBOSELOG set */

private VOID	intinit();
private VOID	intsend();
private VOID	intwait();
private VOID	ialarm();
private VOID	walarm();
private VOID	falarm();
private VOID	readynow();
private VOID	emtdone();
private char 	*FindPattern();

#define INITALARM 90
#define WAITALARM 30

main(argc,argv)
	int argc;
	char *argv[];
{
    register int cnt, wc;
    register char *mbp;

    char  **av;
    long clock;		/* for log timestamp */
    char magic[11];	/* first few bytes of stdin ?magic number and type */
    FILE *streamin;

    char mybuf[BUFSIZ];
    int wpid;
    int status;
    int i;

    VOIDC signal(SIGTERM, intinit);
    VOIDC signal(SIGINT, intinit);
    VOIDC signal(SIGHUP, intinit);
    VOIDC signal(SIGQUIT, intinit);

    /* parse command-line arguments */

    av = argv;
    if (prog = strrchr(*av,'/')) prog++;
    else prog = *av;

    /* close all file descriptors >= 4 */
    for (i = 4; i < _NFILE ; i++) VOIDC close(i);

    verboselog = envget("VERBOSELOG");

    debugp((stderr,"%s: pid %d ppid %d\n",prog,getpid(),getppid()));

    /* IMPORTANT: in the case of cascaded filters, */ 
    /* stdin may be a pipe! (and hence we cannot seek!) */

    if ((cnt = read(fileno(stdin),magic,11)) != 11) goto badfile;
    debugp((stderr,"%s: magic number is %11.11s\n",prog,magic));
    streamin = stdin;

    if (strncmp(magic,"%!",2) == 0) goto go_ahead;

    /* here is where you might test for other file type
     * e.g., PRESS, imPRESS, DVI, Mac-generated, etc.
     */

    badfile:
        fprintf(stderr,"%s: bad magic number, EOF\n", prog);
	VOIDC fflush(stderr);
	exit(THROW_AWAY);

    go_ahead:;

    fdinput = fileno(streamin); /* the file to print */
    fdsend = fileno(stdout);	/* the printer (write) */
    fdlisten = 3;		/* the printer (read) see interface */

    if ((cpid = fork()) < 0) pexit(prog, THROW_AWAY);
    else if (cpid) {/* parent - sender */
	VOIDC setjmp(sendint);

	if (intrup) {
	    /* we only get here if there was an interrupt */

	    fprintf(stderr,"%s: abort (sending)\n",prog);
	    VOIDC fflush(stderr);

	    /* flush and restart output to printer,
	     * send an abort (^C) request and wait for the job to end
	     */
	    if (ioctl(fdsend, TCFLSH, 1) || ioctl(fdsend, TCXONC, 1)
	    || (write(fdsend, abortbuf, (unsigned) 1) != 1)) {
		RestoreStatus();
		pexit(prog,THROW_AWAY);
	    }
	    debugp((stderr,"%s: sent interrupt - waiting\n",prog));
	    goto donefile; /* sorry ewd! */
	}

        VOIDC signal(SIGTERM, intsend);
        VOIDC signal(SIGINT, intsend);
        VOIDC signal(SIGHUP, intsend);
        VOIDC signal(SIGQUIT, intsend);
	VOIDC signal(SIGEMT, readynow);

	progress = oldprogress = 0; /* finite progress on sender */
	getstatus = FALSE; /* prime the pump for fun FALSE; */

	VOIDC signal(SIGALRM, ialarm); /* initial phase alarm */
	VOIDC alarm(INITALARM); /* schedule an alarm/timeout */

	/* loop, trying to send a ^T to get printer status
	 * We will hang here (and post a message) if the printer
	 * is unreachable.  Eventually, we will succeed, the listener
	 * will see the status report, signal us, and we will proceed
	 */

	cnt = 1;
	VOIDC setjmp(startstatus);

	while (TRUE) {
	    if (goahead) break;
	    debugp((stderr,"%s: get start status\n",prog));
	    VOIDC write(fdsend, statusbuf, (unsigned) 1);
	    VOIDC pause();
	    if (goahead) break; 
	    /* if we get here, we got an alarm */
	    VOIDC ioctl(fdsend, TCFLSH, 1);
	    VOIDC ioctl(fdsend, TCXONC, 1);
	    VOIDC ioctl(fdsend, TCFLSH, 1);
	    sprintf(mybuf, "Not Responding for %d minutes",
	    	(cnt * INITALARM+30)/60);
	    Status(mybuf);
	    VOIDC alarm(INITALARM);
	    cnt++;
	}

	VOIDC signal(SIGEMT, SIG_IGN); /* now ignore EMTs */
	RestoreStatus();
	debugp((stderr,"%s: printer responding\n",prog));

	/* ship the magic number! */
	VOIDC write(fdsend,magic, (unsigned) 11);

	/* now ship the rest of the file */

	VOIDC signal(SIGALRM, SIG_IGN); /* now ignore alarms */
	VOIDC alarm(0); /* avoid alarms altogether, they screw up writes */

	while ((cnt = read(fdinput, mybuf, sizeof mybuf)) > 0) {
	    if (intrup == TRUE) break;

	    if (getstatus) {
		VOIDC write(fdsend, statusbuf, 1);
		getstatus = FALSE;
		progress++;
	    }
	    mbp = mybuf;
	    while ((cnt > 0) && ((wc = write(fdsend, mbp, (unsigned) cnt)) != cnt)) {
		/* this seems necessary but not sure why */
		if (wc < 0) {
		    fprintf(stderr,"%s: error writing to printer:\n",prog);
		    perror(prog);
		    RestoreStatus();
		    VOIDC kill(cpid,SIGKILL);
		    VOIDC sleep(10);
		    exit(TRY_AGAIN);
		}
		mbp += wc;
		cnt -= wc;
		progress++;
	    }
	    progress++;
	    if (progress > (oldprogress + 20)) {
		getstatus = TRUE;
		oldprogress = progress;
	    }
	}
	if (cnt < 0) {
	    fprintf(stderr,"%s: error reading from stdin: \n", prog);
	    perror(prog);
	    RestoreStatus();
	    VOIDC sleep(10);
	    exit(TRY_AGAIN);	/* kill the listener? */
	}


	donefile:;

	sendend = 1;

	VOIDC setjmp(dwait);

	if (sendend && !gotemt) {

	    VOIDC signal(SIGEMT, emtdone);

	    debugp((stderr,"%s: done sending\n",prog));

	    /* now send the PostScript EOF character */
	    VOIDC write(fdsend, eofbuf, 1);
	    sendend = 0;
	    progress++;

	    if (!intrup) {
		VOIDC signal(SIGTERM, intwait);
		VOIDC signal(SIGINT, intwait);
		VOIDC signal(SIGHUP, intwait);
		VOIDC signal(SIGQUIT, intwait);
	    }

	    VOIDC signal(SIGALRM, walarm);
	    VOIDC alarm(WAITALARM);
	    getstatus = FALSE;
	}

	/* wait to sync with listener EMT signal
	 * to indicate it got an EOF from the printer
	 */
	while (TRUE) {
	    debugp((stderr,"wait e%d i%d %d %d\n",gotemt,intrup,wpid,status));
	    if (gotemt) break;
	    if (getstatus) {
		VOIDC write(fdsend, statusbuf, 1);
		getstatus = FALSE;
	    }
	    wpid = wait(&status);
	    if (wpid == -1) break;
	}
	debugp((stderr,"out of wait e%d i%d %d %d en%d\n",
		gotemt,intrup,wpid,status,errno));

	VOIDC signal(SIGALRM, falarm);
	VOIDC alarm(WAITALARM);

	/* wait for listener to die */
	VOIDC setjmp(dwait);
        while ((wpid = wait(&status)) > 0);
	VOIDC alarm(0);
	VOIDC signal(SIGTERM, SIG_IGN);
	VOIDC signal(SIGINT, SIG_IGN);
	VOIDC signal(SIGHUP, SIG_IGN);
	VOIDC signal(SIGQUIT, SIG_IGN);
	VOIDC signal(SIGEMT, SIG_IGN);
        if (intrup) {
	    fprintf(stderr,"%s: end - %s", prog,(time(&clock),ctime(&clock)));
	    VOIDC fflush(stderr);
	}
	debugp((stderr,"w2: s%lo p%d = p%d\n", status, wpid, cpid));

	RestoreStatus();
	exit(0);
    }
    else {/* child - listener */
      register FILE *psin;
      register int r;

      char pbuf[BUFSIZ]; /* buffer for pagecount info */
      char *pb;		/* pointer for above */
      char *outname;	/* file name for job output */
      int havejobout = FALSE; /* flag if jobout != stderr */

      VOIDC signal(SIGTERM, SIG_IGN);
      VOIDC signal(SIGINT, SIG_IGN);
      VOIDC signal(SIGHUP, SIG_IGN);
      VOIDC signal(SIGQUIT, SIG_IGN);
      VOIDC signal(SIGALRM, SIG_IGN);

      /* get jobout from environment if there, otherwise use stderr */
      if (((outname = envget("JOBOUTPUT")) == NULL)
      || ((jobout = fopen(outname,"a")) == NULL)) {
	  jobout = stderr;
      }
      else havejobout = TRUE;

      if ((psin = fdopen(fdlisten, "r")) == NULL) {
	  RestoreStatus();
	  pexit(prog, THROW_AWAY);
      }

      /* listen for first status (idle?) */
      pb = pbuf;
      *pb = '\0';
      while (TRUE) {
	  r = getc(psin);
	  if (r == EOF) {
	      fprintf(stderr, EOFerr, prog, "startup");
	      VOIDC fflush(stderr);
	      RestoreStatus();
	      VOIDC sleep(10);
	      exit(TRY_AGAIN);
	  }
	  if ((r & 0377) == '\n') break; /* newline */
	  *pb++ = r;
      }
      *pb = 0;
      if (strcmp(pbuf, "%%[ status: idle ]%%\r") != 0) {
	  fprintf(stderr,"%s: initial status - %s\n",prog,pbuf);
	  VOIDC fflush(stderr);
      }

      /* flush input state and signal sender that we heard something */
      VOIDC ioctl(fdlisten, TCFLSH, 1);

      VOIDC kill(getppid(),SIGEMT);

      /* listen for the user job */
      while (TRUE) {
	r = getc(psin);
	if ((r&0377) == 004) break; /* PS_EOF */
	else if (r == EOF) {
	    VOIDC fclose(psin);
	    fprintf(stderr, EOFerr, prog, "job");
	    VOIDC fflush(stderr);
	    RestoreStatus();
	    VOIDC sleep(10);
	    /* may have been the job's fault, so don't retry */
	    exit(THROW_AWAY); 
	}
	GotChar(r);
      }

      /* let sender know we saw the end of the job */

      debugp((stderr,"%s: listener saw eof, signaling %d\n",prog,getppid()));

      VOIDC kill(getppid(),SIGEMT);

      /* all done -- exit */
      if (havejobout) VOIDC fclose(jobout);
      VOIDC fclose(psin);
      exit(0); /* to parent */
    }
}

/* search backwards from p in start for patt */
private char *FindPattern(p, start, patt)
char *p;
char *start;
char *patt;
{
    int patlen;
    patlen = strlen(patt);
    
    p -= patlen;
    for (; p >= start; p--) {
	if (strncmp(p, patt, patlen) == 0) return(p);
    }
    return ((char *)NULL);
}

private GotChar(c)
register int c;
{
    static char linebuf[BUFSIZ];
    static char *cp = linebuf;
    static enum State {normal, onep, twop, inmessage,
    			close1, close2, close3, close4} st = normal;
    char *match, *last;

    switch ((int) st) { /* this cast makes by 3b2 C compiler happy */
	case normal:
	    if (c == '%') {
		st = onep;
		cp = linebuf;
		*cp++ = c;
		break;
	    }
	    putc(c,jobout);
	    VOIDC fflush(jobout);
	    break;
	case onep:
	    if (c == '%') {
		st = twop;
		*cp++ = c;
		break;
	    }
	    putc('%',jobout);
	    putc(c,jobout);
	    VOIDC fflush(jobout);
	    st = normal;
	    break;
	case twop:
	    if (c == '\[') {
		st = inmessage;
		*cp++ = c;
		break;
	    }
	    if (c == '\%') {
		putc('%',jobout);
		VOIDC fflush(jobout);
		/* don't do anything to cp */
		break;
	    }
	    putc('%',jobout);
	    putc('%',jobout);
	    VOIDC fflush(jobout);
	    st = normal;
	    break;
	case inmessage:
	    *cp++ = c;
	    if (c == '\]') st = close1;
	    break;
	case close1:
	    *cp++ = c;
	    switch (c) {
		case '%': st = close2; break;
		case '\]': st = close1; break;
		default: st = inmessage; break;
	    }
	    break;
	case close2:
	    *cp++ = c;
	    switch (c) {
		case '%': st = close3; break;
		case '\]': st = close1; break;
		default: st = inmessage; break;
	    }
	    break;
	case close3:
	    *cp++ = c;
	    switch (c) {
		case '\r': st = close4; break;
		case '\]': st = close1; break;
		default: st = inmessage; break;
	    }
	    break;
	case close4:
	    *cp++ = c;
	    switch(c) {
		case '\n': st = normal; break;
		case '\]': st = close1; break;
		default: st = inmessage; break;
	    }
	    if (st == normal) {
		/* parse complete message */
		last = cp;
		*cp = 0;
		debugp((stderr,">>%s",linebuf));
		if (match = FindPattern(cp, linebuf, " PrinterError: ")) {
		    if (*(match-1) != ':') {
			fprintf(stderr,"%s",linebuf);
			VOIDC fflush(stderr);
			*(last-6) = 0;
			Status(match+15);
		    }
		    else {
			last = INDEX(match,';');
			*last = 0;
			Status(match+15);
		    }
		}
		else if (match = FindPattern(cp, linebuf, " status: ")) {
		    match += 9;
		    if (strncmp(match,"idle",4) == 0) {
			/* we are hopelessly lost, get everyone to quit */
			fprintf(stderr,"%s: ERROR: printer is idle, giving up!\n",prog);
			VOIDC fflush(stderr);
			VOIDC kill(getppid(),SIGKILL); /* will this work */
			exit(THROW_AWAY);
		    }
		    else {
			/* one of: busy, waiting, printing, initializing */
			/* clear status message */
			RestoreStatus();
		    }
		}
		else {
		    /* message not for us */
		    fprintf(jobout,"%s",linebuf);
		    VOIDC fflush(jobout);
		    st = normal;
		    break;
		}
	    }
	    break;
	default:
	    fprintf(stderr,"%s: bad case;\n",prog);
    }
    return;
}

/* restore the "status" message from the backed-up ".status" copy */
private RestoreStatus() {
    /* BackupStatus("status",".status"); */
}

/* report PrinterError via "status" message file */
private Status(msg)
register char *msg;
{
/* fprintf(stderr,"Printer Error: may need attention (%s)\n",msg); */
}

/* initial phase alarm handler for sender */

private VOID ialarm() {
    /* reset the alarm and return */
    VOIDC signal(SIGALRM, ialarm);
    VOIDC alarm(INITALARM);
    return;
}

/* waiting phase alarm handler for sender */

private VOID walarm() {
    static int acount = 0;

    debugp((stderr,"%s: WA %d %d %d %d\n",
    	prog,acount,oldprogress,progress,getstatus));

    if ((oldprogress != progress) || (acount == 4)) {
	getstatus = TRUE;
	acount = 0;
	oldprogress = progress;
    }
    else acount++;

    /* reset alarm */
    VOIDC signal(SIGALRM, walarm);
    VOIDC alarm(WAITALARM);

    /* return to wait loop */
    longjmp(dwait, 0);
}

/* final phase alarm handler for sender */

private VOID falarm() {

    debugp((stderr,"%s: FA %d %d %d\n",prog,oldprogress,progress,getstatus));

    /* no reason to count progress, just get status */
    if (!intrup) {
	VOIDC write(fdsend, statusbuf, 1);
    }
    getstatus = FALSE;

    /* reset alarm */
    VOIDC signal(SIGALRM, falarm);
    VOIDC alarm(WAITALARM);
    return;
}

/* initial interrupt handler - before communications begin, so
 * nothing to be sent to printer
 */
private VOID intinit(sig) int sig; {
    long clock;

    VOIDC signal(SIGTERM,SIG_IGN);
    VOIDC signal(SIGINT,SIG_IGN);
    VOIDC signal(SIGHUP,SIG_IGN);
    VOIDC signal(SIGQUIT,SIG_IGN);
    fprintf(stderr,"%s: abort (during setup)\n",prog);
    VOIDC fflush(stderr);

    /* these next two may be too cautious */
    /* VOIDC kill(0,SIGINT); */
    /* while (wait((int *) 0) > 0); */

    if (verboselog) {
	fprintf (stderr, "%s: end - %s", prog, (time(&clock), ctime(&clock)));
	VOIDC fflush(stderr);
    }

    exit(THROW_AWAY);
}

/* interrupt during sending phase to sender process */

private VOID intsend(sig) int sig;{
    /* set flag */
    VOIDC signal(SIGTERM,SIG_IGN);
    VOIDC signal(SIGINT,SIG_IGN);
    VOIDC signal(SIGHUP,SIG_IGN);
    VOIDC signal(SIGQUIT,SIG_IGN);
    intrup = TRUE;
    debugp((stderr,"intsend sig %d\n",sig));
    longjmp(sendint, 0);
}

/* interrupt during waiting phase to sender process */

private VOID intwait(sig) int sig; {

    intrup = TRUE;
    VOIDC signal(SIGTERM,SIG_IGN);
    VOIDC signal(SIGINT,SIG_IGN);
    VOIDC signal(SIGHUP,SIG_IGN);
    VOIDC signal(SIGQUIT,SIG_IGN);

    fprintf(stderr,"%s: abort (waiting)\n",prog);
    VOIDC fflush(stderr);
    if (ioctl(fdsend, TCFLSH, 1) || ioctl(fdsend, TCXONC, 1)
    || (write(fdsend, abortbuf, 1) != 1)) {
	fprintf(stderr, "%s: error in ioctl(fdsend):\n", prog);
	perror(prog);
    }

    longjmp(dwait, 0);
}

/* EMT on startup to sender -- signalled by listener after first status
 * message received
 */

private VOID readynow() {
    VOIDC signal(SIGEMT,SIG_IGN);
    goahead = TRUE;
    longjmp(startstatus, 0);
}

/* EMT during waiting phase -- listener saw an EOF (^D) from printer */

private VOID emtdone() {
    VOIDC signal(SIGEMT,SIG_IGN);
    VOIDC alarm(0);
    gotemt = TRUE;
    debugp((stderr,"emtdone\n"));
    longjmp(dwait, 0);
}
