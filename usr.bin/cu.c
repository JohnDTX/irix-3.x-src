char _Origin_[] = "System V";
/* @(#)cu.c	1.4 */
/* $Source: /d2/3.7/src/usr.bin/RCS/cu.c,v $ */
static	char	*Sccsid = "@(#)$Revision: 1.1 $";
/* $Date: 89/03/27 17:39:52 $ */

#define ddt
/***************************************************************
 *	cu [-s baud] [-l line] [-h] [-t] [-o | -e] telno
 *
 *	legal baud rates: 110,134,150,300,600,1200,2400,4800,9600.
 *
 *	-l is for specifying a line unit from the file whose
 *		name is defined under LDEVS below.
 *	-h is for half-duplex (local echoing).
 *	-t is for adding CR to LF on output to remote (for terminals).
 *	-d can be used (with ddt) to get some tracing & diagnostics.
 *	-o or -e is for odd or even parity on transmission to remote.
 *	Telno is a telephone number with `=' for secondary dial-tone.
 *	If "-l dev" is used, speed is taken from LDEVS.
 *
 *	Escape with `~' at beginning of line.
 *	Silent output diversions are ~>:filename and ~>>:filename.
 *	Terminate output diversion with ~> alone.
 *	~. is quit, and ~![cmd] gives local shell [or command].
 *	Also ~$ for canned local procedure pumping remote.
 *	Both ~%put from [to]  and  ~%take from [to] invoke built-ins.
 *	Also, ~%break or just ~%b will transmit a BREAK to remote.
 *	~%nostop toggles on/off the DC3/DC1 input control from remote,
 *		(certain remote systems cannot cope with DC3 or DC1).
 *
 *	As a device-lockout semaphore mechanism, create an entry
 *	in the directory #defined as LOCK whose name is LCK..dev
 *	where dev is the device name taken from the "line" column
 *	in the file #defined as LDEVS.  Be sure to trap every possible
 *	way out of cu execution in order to "release" the device.
 *	This entry is `touched' from the dial() library routine
 *	every hour in order to keep uucp from removing it on
 *	its 90 minute rounds.  Also, have the system start-up
 *	procedure clean all such entries from the LOCK directory.
 ***************************************************************/
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sys/termio.h>
#include <sys/errno.h>
#include "dial.h"

#define MID	BUFSIZ/2	/* mnemonic */
#define	RUB	'\177'		/* mnemonic */
#define	XON	'\21'		/* mnemonic */
#define	XOFF	'\23'		/* mnemonic */
#define	TTYIN	0		/* mnemonic */
#define	TTYOUT	1		/* mnemonic */
#define	TTYERR	2		/* mnemonic */
#define	YES	1		/* mnemonic */
#define	NO	0		/* mnemonic */
#define	EQUALS	!strcmp		/* mnemonic */
#define EXIT	0		/* exit code */
#define CMDERR	1		/* exit code */
#define NODIAL	2		/* exit code */
#define HUNGUP	3		/* exit code */
#define IOERR	4		/* exit code */
#define SIGQIT	5		/* exit code */
#define NOFORK	6		/* exit code */
#define R_ACC	04		/* read access */
#define W_ACC	02		/* write access */

extern int
	_debug,			/* flag for more diagnostics */
	errno,			/* supplied by system interface */
	optind,			/* variable in getopt() */
	strcmp();		/* c-lib routine */

extern unsigned
	sleep();		/* c-lib routine */

extern char
	*optarg;		/* variable in getopt() */

static struct termio tv, tv0;	/* for saving, changing TTY atributes */
static struct termio lv;	/* attributes for the line to remote */
static CALL call;		/* from dial.h */

static char
	cxc,			/* place into which we do character io*/
	tintr,			/* current input INTR */
	tquit,			/* current input QUIT */
	terase,			/* current input ERASE */
	tkill,			/* current input KILL */
	myeol,			/* current input EOL */
	teol,			/* current sencondary input EOL */
	myeof;			/* current input EOF */

static int
	terminal=NO,		/* flag; remote is a terminal */
	echoe,			/* save users ECHOE bit */
	echok,			/* save users ECHOK bit */
	rlfd,			/* fd for remote comm line */
	child,			/* pid for recieve proccess */
	intrupt=NO,		/* interrupt indicator */
	duplex=YES,		/* half(NO), or full(YES) duplex */
	sstop=YES,		/* NO means remote can't XON/XOFF */
	rtn_code=0,		/* default return code */
	takeflag=NO,		/* indicates a ~%take is in progress */
	w_char(),		/* local io routine */
	r_char();		/* local io routine */

static
	onintrpt(),		/* interrupt routine */
	rcvdead(),		/* interrupt routine */
	hangup(),		/* interrupt routine */
	quit(),			/* interrupt routine */
	bye();			/* interrupt routine */

static void
	flush(),
	shell(),
	dopercen(),
	receive(),
	mode(),
	say(),
#ifdef ddt
	tdmp(),
#endif
	w_str();

char	*msg[]= {
/*  0*/	"usage: %s [-s baud] [-l line] [-h] [-t] [-d] [-m] [-o|-e] telno\n",
/*  1*/	"interrupt",
/*  2*/	"dialer hung",
/*  3*/	"no answer",
/*  4*/	"illegal baud-rate",
/*  5*/	"acu problem",
/*  6*/	"line problem",
/*  7*/	"can't open L-devices file",
/*  8*/	"Requested device not available\r\n",
/*  9*/	"Requested device not known\r\n",
/* 10*/	"No device available at %d baud\r\n",
/* 11*/	"No device known at %d baud\r\n",
/* 12*/	"Connect failed: %s\r\n",
/* 13*/	"Can not open: %s\r\n",
/* 14*/	"Line gone\r\n",
/* 15*/	"Can't execute shell\r\n",
/* 16*/	"Can't divert ",
/* 17*/	"Use `~~' to start line with `~'\r\n",
/* 18*/	"character missed\r\n",
/* 19*/	"after %ld bytes\r\n",
/* 20*/	"%d lines/%ld characters\r\n",
/* 21*/	"Only digits & '-'s or '='s in telno\n",
/* 22*/	"File transmission interrupted\r\n",
/* 23*/	"Cannot fork -- try later\r\n",
/* 24*/	"\r\nCan't transmit special character `%#o'\r\n",
/* 25*/	"\nLine too long\r\n",
/* 26*/	"r\nIO error\r\n",
/* 27*/ "Use `~$'cmd \r\n",
};

/***************************************************************
 *	main: get command line args, establish connection, and fork.
 *	Child invokes "receive" to read from remote & write to TTY.
 *	Main line invokes "transmit" to read TTY & write to remote.
 ***************************************************************/

main(argc, argv)
char *argv[];
{
	char *string;
	int i, errflag=0;

	lv.c_iflag = (IGNPAR | IGNBRK | ISTRIP | IXON | IXOFF);
	lv.c_cc[VMIN] = '\1';

	call.attr = &lv;
	call.baud = 300;
	call.speed = 300;
	call.line = NULL;
	call.telno = NULL;
	call.modem = 0;

	while((i = getopt(argc, argv, "dhteoms:l:")) != EOF)
		switch(i) {
			case 'd':
#ifdef	ddt
				_debug = YES;
#else
				++errflag;
#endif
				break;
			case 'h':
				duplex ^= YES;
				lv.c_iflag &= ~(IXON | IXOFF);
				sstop = NO;
				break;
			case 't':
				terminal = YES;
				lv.c_oflag |= (OPOST | ONLCR);
				break;
			case 'e':
				if(lv.c_cflag & PARENB)
					++errflag;
				else
					goto PAROUT;
				break;
			case 'o':
				if(lv.c_cflag & PARENB)
					++errflag;
				else
					lv.c_cflag = PARODD;
			PAROUT:
					lv.c_cflag |= (CS7 | PARENB);
				break;
			case 's':
				call.baud = atoi(optarg);
				call.speed = call.baud;
			  	break;
			case 'l':
				call.line = optarg;
			  	break;
			case 'm':
				call.modem = 1;		/* override modem control for direct lines */
				break;
			case '?':
				++errflag;
		}

	if(optind < argc && optind > 0) {
		string = argv[optind];
		if(strlen(string) == strspn(string, "0123456789=-"))
			call.telno = string;
		else {
			if(EQUALS(string, "dir")) {
				if(call.line == NULL)
					++errflag;
			} else
				++errflag;
		}
	} else
		if(call.line == NULL)
			++errflag;

	if(errflag) {
		say(msg[0], argv[0]);
		exit(1);
	}

	(void)ioctl(TTYIN, TCGETA, &tv0); /* save initial tty state */
	tintr = tv0.c_cc[VINTR]? tv0.c_cc[VINTR]: '\377';
	tquit = tv0.c_cc[VQUIT]? tv0.c_cc[VQUIT]: '\377';
	terase = tv0.c_cc[VERASE]? tv0.c_cc[VERASE]: '\377';
	tkill = tv0.c_cc[VKILL]? tv0.c_cc[VKILL]: '\377';
	teol = tv0.c_cc[VEOL]? tv0.c_cc[VEOL]: '\377';
	myeol = (tv0.c_iflag & ICRNL)? '\r': '\r';
	myeof = tv0.c_cc[VEOF]? tv0.c_cc[VEOF]: '\04';
	echoe = tv0.c_lflag & ECHOE;
	echok = tv0.c_lflag & ECHOK;

	(void)signal(SIGHUP, hangup);
	(void)signal(SIGQUIT, hangup);
	(void)signal(SIGINT, onintrpt);

	if((rlfd = dial(call)) < 0) {
		if(rlfd == NO_BD_A || rlfd == NO_BD_K)
			say(msg[-rlfd], call.baud);
		else
			say(msg[12], msg[-rlfd]);
		exit(NODIAL);
	}

	/* When we get this far we have an open communication line */

	mode(1);			/* put terminal in `raw' mode */

	say("Connected\r\n");

	child = dofork();
	if(child == 0) {
		(void)signal(SIGHUP, rcvdead);
		(void)signal(SIGQUIT, SIG_IGN);
		(void)signal(SIGINT, SIG_IGN);
		receive();	/* This should run until killed */
		/*NOTREACHED*/
	} else if(child > 0) {
		(void)signal(SIGUSR1, bye);
		(void)signal(SIGHUP, SIG_IGN);
		(void)signal(SIGQUIT, onintrpt);
		rtn_code = transmit();
		quit(rtn_code);
	} else {
		hangup(rlfd);
		exit(NOFORK);
	}
}

/***************************************************************
 *	transmit: copy stdin to rlfd, except:
 *	~.	terminate
 *	~!	local login-style shell
 *	~!cmd	execute cmd locally
 *	~$proc	execute proc locally, send output to line
 *	~%cmd	execute builtin cmd (put, take, or break)
 ****************************************************************/

int
transmit()
{
	char b[BUFSIZ];
	register char *p;
	register int escape;

#ifdef	ddt
	if(_debug == YES) say("transmit started\n\r");
#endif
	while(1) {
		p = b;
		while(r_char(TTYIN) == YES) {
			if(p == b)	/* Escape on leading  ~    */
				escape = (cxc == '~');
			if(p == b+1)	/* But not on leading ~~   */
				escape &= (cxc != '~');
			if(escape) {
				if(cxc == myeol || cxc == teol || cxc == '\n') {
					*p = '\0';
					if(tilda(b+1) == YES)
						return(EXIT);
					break;
				}
				if(cxc == tintr || cxc == tkill || cxc == tquit ||
					    (intrupt && cxc == '\0')) {
					if(!(cxc == tkill) || echok)
						say("\r\n");
					break;
				}
				if(cxc == terase) {
					p = (--p < b)? b:p;
					if(p > b)
						if(echoe)
							say("\b \b");
						else
							(void)w_char(TTYOUT);
				} else {
					(void)w_char(TTYOUT);
					if(p-b < BUFSIZ)
						*p++ = cxc;
					else {
						say(msg[25]);
						break;
					}
				}
			} else {
				if(intrupt && cxc == '\0') {
#ifdef	ddt
					if(_debug == YES)
						say("got break in transmit\n\r");
#endif
					intrupt = NO;
					(void)ioctl(rlfd, TCSBRK, 0);
					flush();
					break;
				}
				if(w_char(rlfd) == NO) {
					say(msg[14]);
					return(IOERR);
				}
				if(duplex == NO)
					if(w_char(TTYERR) == NO)
						return(IOERR);
				if ( (cxc == tintr) || (cxc == tquit) || ( (p==b) && (cxc == myeof) ) ) {
#ifdef	ddt
					if(_debug == YES) say("got a tintr\n\r");
#endif
					flush();
					break;
				}
				if(cxc == myeol || cxc == teol ||
							cxc == tkill) {
					takeflag = NO;
					break;
				}
				p = (char*)0;
			}
		}
	}
}

/***************************************************************
 *	routine to halt input from remote and flush buffers
 ***************************************************************/
static void
flush()
{
	(void)ioctl(TTYOUT, TCXONC, 0);	/* stop tty output */
	(void)ioctl(rlfd, TCFLSH, 0);		/* flush remote input */
	(void)ioctl(TTYOUT, TCFLSH, 1);	/* flush tty output */
	(void)ioctl(TTYOUT, TCXONC, 1);	/* restart tty output */
	if(takeflag == NO) {
		return;		/* didn't interupt file transmission */
	}
	say(msg[22]);
	(void)sleep(3);
	w_str("echo '\n~>\n';mesg y;stty echo\n");
	takeflag = NO;
}

/**************************************************************
 *	command interpreter for escape lines
 **************************************************************/
int
tilda(cmd)
char	*cmd;
{
	say("\r\n");
#ifdef	ddt
	if(_debug == YES) say("call tilda(%s)\r\n", cmd);
#endif
	switch(cmd[0]) {
		case '.':
			if(call.telno == NULL)
				if(cmd[1] != '.')
					w_str("\04\04\04\04\04");
			return(YES);
		case '!':
			shell(cmd);	/* local shell */
			if (cmd[1] == '\0')
				say("\n");
			say("\r%c\r\n", *cmd);
			break;
		case '$':
			if(cmd[1] == '\0')
				say(msg[27]);
			else {
				shell(cmd);	/* Local shell */
				say("\r%c\r\n", *cmd);
			}
			break;
		case '%':
			dopercen(++cmd);
			break;
#ifdef ddt
		case 't':
			tdmp(TTYIN);
			break;
		case 'l':
			tdmp(rlfd);
			break;
#endif
		default:
			say(msg[17]);
	}
	return(NO);
}

/***************************************************************
 *	The routine "shell" takes an argument starting with
 *	either "!" or "$", and terminated with '\0'.
 *	If $arg, arg is the name of a local shell file which
 *	is executed and its output is passed to the remote.
 *	If !arg, we escape to a local shell to execute arg
 *	with output to TTY, and if arg is null, escape to
 *	a local shell and blind the remote line.  In either
 *	case, RUBout or '^D' will kill the escape status.
 **************************************************************/

static void
shell(str)
char	*str;
{
	int	fk, (*xx)(), (*yy)();
	char	*sh;
	char	*getenv();

#ifdef	ddt
	if(_debug == YES) say("call shell(%s)\r\n", str);
#endif
	fk = dofork();
	if(fk < 0)
		return;
	mode(0);	/* restore normal tty attributes */
	xx = signal(SIGINT, SIG_IGN);
	yy = signal(SIGQUIT, SIG_IGN);
	if(fk == 0) {
		/***********************************************
		 * Hook-up our "standard output"
		 * to either the tty or the line
		 * as appropriate for '!' or '$'
		 ***********************************************/
		(void)close(TTYOUT);
		(void)fcntl((*str == '$')? rlfd:TTYERR,F_DUPFD,TTYOUT);
		(void)close(rlfd);
		(void)signal(SIGINT, SIG_DFL);
		(void)signal(SIGHUP, SIG_DFL);
		(void)signal(SIGQUIT, SIG_DFL);
		(void)signal(SIGUSR1, SIG_DFL);
		setuid(getuid());
		setgid(getgid());
		sh = getenv("SHELL");
		if (!sh || !*sh)
			sh = "/bin/sh";
		if(*++str == '\0')
			(void)execl(sh,sh,(char*)0,(char*)0,0);
		else
			(void)execl(sh,sh,"-c",str,0);
		say(msg[15]);
		exit(0);
	}
	/* our hourly alarm clock (for uucp) can interupt this wait */
	while(wait((int*)0) != fk);
	(void)signal(SIGINT, xx);
	(void)signal(SIGQUIT, yy);
	mode(1);
}


/***************************************************************
 *	This function implements the 'put', 'take', 'break', and
 *	'nostop' commands which are internal to cu.
 ***************************************************************/

static void
dopercen(cmd)
register char *cmd;
{
	char	*arg[5];
	int	narg;

#ifdef	ddt
	if(_debug == YES) say("call dopercen(\"%s\")\r\n", cmd);
#endif
	arg[narg=0] = strtok(cmd, " \t\n");
	/* following loop breaks out the command and args */
	while((arg[++narg] = strtok((char*) NULL, " \t\n")) != NULL) {
		if(narg < 5)
			continue;
		else
			break;
	}

	if(EQUALS(arg[0], "take")) {
		if(narg < 2 || narg > 3) {
			say("usage: ~%%take from [to]\r\n");
			return;
		}
		if(narg == 2)
			arg[2] = arg[1];
		w_str("stty -echo;mesg n;echo '~>':");
		w_str(arg[2]);
		w_str(";cat ");
		w_str(arg[1]);
		w_str(";echo '~>';mesg y;stty echo\n");
		takeflag = YES;
		return;
	}
	else if(EQUALS(arg[0], "put")) {
		FILE	*file;
		char	ch, buf[BUFSIZ], spec[NCC+1], *b, *p, *q;
		int	i, j, len, tc=0, lines=0;
		long	chars=0L;

		if(narg < 2 || narg > 3) {
			say("usage: ~%%put from [to]\r\n");
			goto R;
		}
		if(narg == 2)
			arg[2] = arg[1];
		if (access(arg[1],R_ACC) < 0 ||
		  (file = fopen(arg[1], "r")) == NULL) {
			say(msg[13], arg[1]);
R:
			w_str("\n");
			return;
		}
		w_str("stty -echo; cat - > ");
		w_str(arg[2]);
		w_str("; stty echo\n");
		intrupt = NO;
		for(i=0,j=0; i < NCC; ++i)
			if((ch=tv0.c_cc[i]) != '\0')
				spec[j++] = ch;
		spec[j] = '\0';
		mode(2);
		(void)sleep(5);
		while(intrupt == NO &&
				fgets(b= &buf[MID],MID,file) != NULL) {
			len = strlen(b);
			chars += len;		/* character count */
			p = b;
			while(q = strpbrk(p, spec)) {
				if(*q == tintr || *q == tquit ||
							*q == teol) {
					say(msg[24], *q);
					(void)strcpy(q, q+1);
					intrupt = YES;
				}
				b = strncpy(b-1, b, q-b);
				*(q-1) = '\\';
				p = q+1;
			}
			if((tc += len) >= MID) {
				(void)sleep(1);
				tc = len;
			}
			if(write(rlfd, b, (unsigned)strlen(b)) < 0) {
				say(msg[26]);
				intrupt = YES;
				break;
			}
			++lines;		/* line count */
		}
		mode(1);
		(void)fclose(file);
		if(intrupt == YES) {
			intrupt = NO;
			say(msg[22]);
			w_str("\n");
			say(msg[19], ++chars);
		} else
			say(msg[20], lines, chars);
		w_str("\04");
		(void)sleep(3);
		return;
	}
	else if(EQUALS(arg[0], "b") || EQUALS(arg[0], "break")) {
		(void)ioctl(rlfd, TCSBRK, 0);
		return;
	}
	else if(EQUALS(arg[0], "nostop")) {
		(void)ioctl(rlfd, TCGETA, &tv);
		if(sstop == NO)
			tv.c_iflag |= IXOFF;
		else
			tv.c_iflag &= ~IXOFF;
		(void)ioctl(rlfd, TCSETAW, &tv);
		sstop = !sstop;
		mode(1);
		return;
	}
	say("~%%%s unknown to cu\r\n", arg[0]);
}

/***************************************************************
 *	receive: read from remote line, write to fd=1 (TTYOUT)
 *	catch:
 *	~>[>]:file
 *	.
 *	. stuff for file
 *	.
 *	~>	(ends diversion)
 ***************************************************************/

static void
receive()
{
	register silent=NO, file;
	register char *p;
	int	tic;
	char	b[BUFSIZ];
	long	lseek(), count;

#ifdef	ddt
	if(_debug == YES) say("receive started\r\n");
#endif
	file = -1;
	p = b;
	while(r_char(rlfd) == YES) {
		if(silent == NO)
			if(w_char(TTYOUT) == NO)
				rcvdead(IOERR);	/* this will exit */
		/* remove CR's and fill inserted by remote */
		if(cxc == '\0' || cxc == '\177' || cxc == '\r')
			continue;
		*p++ = cxc;
		if(cxc != '\n' && (p-b) < BUFSIZ)
			continue;
		/***********************************************
		 * The rest of this code is to deal with what
		 * happens at the beginning, middle or end of
		 * a diversion to a file.
		 ************************************************/
		if(b[0] == '~' && b[1] == '>') {
			/****************************************
			 * The line is the beginning or
			 * end of a diversion to a file.
			 ****************************************/
			if((file < 0) && (b[2] == ':' || b[2] == '>')) {
				/**********************************
				 * Beginning of a diversion
				 *********************************/
				int	append;

				*(p-1) = NULL; /* terminate file name */
				append = (b[2] == '>')? 1:0;
				p = b + 3 + append;
					/* strip parity bit */
				while (*p = (*p&0177))
					p++;
				p = b + 3 + append;
				file = -1;
				if(append && access(p,W_ACC) >= 0
				  && (file=open(p,O_WRONLY)) > 0)
					(void)lseek(file, 0L, 2);
				else {
					if (!fork()) {
						setuid(getuid());
						setgid(getgid());
						close(creat(p, 0666));
						exit(0);
					}
					wait((int *)0);
					if (access(p,W_ACC) >= 0
					  && (file=open(p,O_WRONLY)) > 0)
						(void)lseek(file, 0L, 2);
				}
				if(file < 0) {
					say(msg[16]);
					perror(p);
					fflush(stderr);
				}
				silent = YES; 
				count = tic = 0;
			} else {
				/*******************************
				 * End of a diversion (or queer data)
				 *******************************/
				if(b[2] != '\n')
					goto D;		/* queer data */
				if (file >= 0 && close(file)) {
					say(msg[16]);
					perror(p);
					fflush(stderr);
					file = -1;
				}
				silent = NO;
				say("~>\r\n");
				if (file >= 0)
					say(msg[20], tic, count);
				fflush(stderr);
				file = -1;
			}
		} else {
			/***************************************
			 * This line is not an escape line.
			 * Either no diversion; or else yes, and
			 * we've got to divert the line to the file.
			 ***************************************/
D:
			if(file > 0) {
				(void)write(file, b, (unsigned)(p-b));
				count += p-b;	/* tally char count */
				++tic;		/* tally lines */
			}
		}
		p = b;
	}
	rcvdead(IOERR);
}

/***************************************************************
 *	change the TTY attributes of the users terminal:
 *	0 means restore attributes to pre-cu status.
 *	1 means set `raw' mode for use during cu session.
 *	2 means like 1 but accept interrupts from the keyboard.
 ***************************************************************/
static void
mode(arg)
{
#ifdef	ddt
	if(_debug == YES) say("call mode(%d)\r\n", arg);
#endif
	if(arg == 0) {
		(void)ioctl(TTYIN, TCSETAW, &tv0);
	} else {
		(void)ioctl(TTYIN, TCGETA, &tv);
		if(arg == 1) {
			tv.c_iflag &= ~(INLCR | ICRNL | IGNCR |
						IXOFF | IUCLC);
			tv.c_iflag |= ISTRIP;
			tv.c_oflag |= OPOST;
			tv.c_oflag &= ~(OLCUC | ONLCR | OCRNL | ONOCR
						| ONLRET);
			tv.c_lflag &= ~(ICANON | ISIG | ECHO);
			if(sstop == NO)
				tv.c_iflag &= ~IXON;
			else
				tv.c_iflag |= IXON;
			if(terminal) {
				tv.c_oflag |= ONLCR;
				tv.c_iflag |= ICRNL;
			}
			tv.c_cc[VEOF] = '\01';
			tv.c_cc[VEOL] = '\0';
		}
		if(arg == 2) {
			tv.c_iflag |= IXON;
			tv.c_lflag |= ISIG;
		}
		(void)ioctl(TTYIN, TCSETAW, &tv);
	}
}

static int
dofork()
{
	register int x, i=0;

	while(++i < 6)
		if((x = fork()) >= 0)
			return(x);
#ifdef	ddt
	if(_debug == YES) perror("dofork");
#endif
	say(msg[23]);
	return(x);
}

static int
r_char(fd)
{
	static char abuf[1024];
	static short nchars = 0;
	static char *cp = &abuf[0]; 

	if (nchars == 0) {
		while ((nchars = read(fd, &abuf[0], sizeof(abuf))) <= 0) {
			if(errno == EINTR) {
				if (intrupt == YES) {
					cxc = '\0';	/* got a BREAK */
					return(YES);
				} else
					continue;	/* alarm went off */
			} else {
#ifdef	ddt
				if(_debug == YES)
					say("got read error, not EINTR\n\r");
#endif
				abort();		/* something worng */
			}
		}
		cp = &abuf[0];
	}
	cxc = *cp++;
	nchars--;
	return YES;
}

static int
w_char(fd)
{
	int rtn;

	while((rtn = write(fd, &cxc, 1)) < 0)
		if(errno == EINTR)
			if(intrupt == YES) {
				say("\ncu: Output blocked\r\n");
				quit(IOERR);
			} else
				continue;	/* alarm went off */
		else
			break;			/* bad news */
	return(rtn == 1? YES: NO);
}

/*VARARGS1*/
static void
say(fmt, arg1, arg2, arg3, arg4, arg5)
char	*fmt;
{
	(void)fprintf(stderr, fmt, arg1, arg2, arg3, arg4, arg5);
}

static void
w_str(string)
register char *string;
{
	int len;

	len = strlen(string);
	if(write(rlfd, string, (unsigned)len) != len)
		say(msg[14]);
}

static
onintrpt()
{
	(void)signal(SIGINT, onintrpt);
	(void)signal(SIGQUIT, onintrpt);
	intrupt = YES;
}

static
rcvdead(arg)	/* this is executed only in the receive proccess */
int arg;
{
#ifdef ddt
	if(_debug == YES) say("call rcvdead(%d)\r\n", arg);
#endif
	(void)kill(getppid(), SIGUSR1);
	exit((arg == SIGHUP)? SIGHUP: arg);
	/*NOTREACHED*/
}

static
quit(arg)	/* this is executed only in the parent proccess */
int arg;
{
#ifdef ddt
	if(_debug == YES) say("call quit(%d)\r\n", arg);
#endif
	(void)kill(child, SIGKILL);
	bye(arg);
	/*NOTREACHED*/
}

static
bye(arg)	/* this is executed only in the parent proccess */
int arg;
{
	int status;
#ifdef ddt
	if(_debug == YES) say("call bye(%d)\r\n", arg);
#endif
	(void)wait(&status);
	say("\r\nDisconnected\r\n");
	hangup((arg == SIGUSR1)? (status >>= 8): arg);
	/*NOTREACHED*/
}

static
hangup(arg)	/* this is executed only in the parent proccess */
int arg;
{
#ifdef ddt
	if(_debug == YES) say("call hangup(%d)\r\n", arg);
#endif
	undial(rlfd);
	mode(0);	/* restore users prior tty status */
	exit(arg);
	/*NOTREACHED*/
}

#ifdef ddt
static void
tdmp(arg)
{
	struct termio xv;
	int i;

	say("\rdevice status for fd=%d\n", arg);
	say("\rF_GETFL=%o,", fcntl(arg, F_GETFL,1));
	if(ioctl(arg, TCGETA, &xv) < 0) {
		char	buf[100];

		i = errno;
		(void)sprintf(buf, "\rtdmp for fd=%d", arg);
		errno = i;
		perror(buf);
		return;
	}
	say("iflag=`%o',", xv.c_iflag);
	say("oflag=`%o',", xv.c_oflag);
	say("cflag=`%o',", xv.c_cflag);
	say("lflag=`%o',", xv.c_lflag);
	say("line=`%o'\r\n", xv.c_line);
	say("cc[0]=`%o',", xv.c_cc[0]);
	for(i=1; i<8; ++i)
		say("[%d]=`%o',", i, xv.c_cc[i]);
	say("\r\n");
}
#endif
