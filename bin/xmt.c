#
/*
 * nmt.c --
 * new mt program, Clover version
 *
 * currently works only with xm driver
 */

# define TOYMT

# include "sys/types.h"
# include "sys/ioctl.h"
# ifndef TOYMT
# include "sys/mtio.h"
# else   TOYMT
# include "./mtio.h"
# endif  TOYMT
# include "fcntl.h"

# include "stdio.h"
# include "ctype.h"


/* default tape name */
char *deftape = "/dev/rmt/xmt0d0nrw1600";


/*
 * command descriptor
 */
struct cmd
{
	char *name;
	char *help;
	int (*func)();
};


/*
 * bit field descriptor
 */
struct bitfield
{
	int mask, val;
	char *name;
};


/* vocabulary of commands */
extern struct cmd cmds[];

/* lookup function for commands */
extern struct cmd *lookup();


char *thetape;
char *usage = "usage: mt [-t tape] [cmd ...]";
char *progname = "mt";

main(argc, argv)
	int argc;
	char **argv;
{
	extern char *getenv();
	register char *ap;

	thetape = deftape;
	if ((ap = getenv("TAPE")) != 0)
		thetape = ap;

	argc--; progname = *argv++;
	while (argc > 0 && *(ap = *argv) == '-') {
		argc--; argv++;

		while (*++ap)
		switch (*ap) {
		case 't':
			if (--argc < 0)
				errexit("missing -t {tapefile}");
			thetape = *argv++;
			break;
		default:
			errwarn("unknown flag %c", *ap);
			errexit(usage);
			break;
		}
	}

	if (argc <= 0)
		interactive();
	else
		domtcmd(argc, argv);
	exit(0);
}

interactive()
{
	int ac; char **av;
	for (;;) {
		printf("mt> "); fflush(stdout);
		readargs(&ac, &av);
		if (ac <= 0)
			continue;
		domtcmd(ac, av);
	}
}

domtcmd(ac, av)
	int ac; char **av;
{
	register struct cmd *cp;

	if ((cp = lookup(*av)) == 0) {
		help();
		return;
	}

	(*cp->func)(ac, av);
}

help_func(ac, av)
	int ac; char **av;
{
	ac--; av++;
	if (ac > 0)
		while (--ac >= 0)
			cmdhelp(*av++);
	else
		help();
}

quit_func(ac, av)
	int ac; char **av;
{
	exit(0);
}

help()
{
	printf("* * * * * C O M M A N D S * * * * *\n");
	cmdhelp("");
}

cmdhelp(s)
	char *s;
{
	register struct cmd *cp;

	for (cp = cmds; cp->name != 0; cp++)
		if (*s == '\0' || submatch(s, cp->name))
			helpstring(cp->help);
}

helpstring(s)
	register char *s;
{
	extern char *index();
	register char *sp;
	char namebuf[15+1];
	char *f, *name;

	f = "%-15.15s - %s\n",

	sp = index(s, '$');
	if (sp == 0) {
		printf("%s\n", s);
		return;
	}
	*sp = '\0';
	strncpy(namebuf, s, sizeof namebuf - 1);
	*sp++ = '$';
	s = sp;
	name = namebuf;

	for (;;) {
		sp = index(s, '$');
		if (sp != 0)
			*sp = '\0';
		printf(f, name, s);
		if (sp == 0)
			return;
		*sp++ = '$';
		s = sp;
		name = "";
	}
}


/* ----- command functions ----- */
# ifdef MTFSF
fsf_func(ac, av) int ac; char **av; { op(ac, av, 1, MTFSF, 0); }
# endif MTFSF
# ifdef MTBSF
bsf_func(ac, av) int ac; char **av; { op(ac, av, 1, MTBSF, 0); }
# endif MTBSF
# ifdef MTFSR
fsr_func(ac, av) int ac; char **av; { op(ac, av, 1, MTFSR, 0); }
# endif MTFSR
# ifdef MTBSR
bsr_func(ac, av) int ac; char **av; { op(ac, av, 1, MTBSR, 0); }
# endif MTBSR
# ifdef MTREW
rew_func(ac, av) int ac; char **av; { op(ac, av, 0, MTREW, 0); }
# endif MTREW
# ifdef MTOFFL
offl_func(ac, av) int ac; char **av; { op(ac, av, 0, MTOFFL, 0); }
# endif MTOFFL
# ifdef MTNOP
nop_func(ac, av) int ac; char **av; { op(ac, av, 0, MTNOP, 0); }
# endif MTNOP
# ifdef MTWEOF
weof_func(ac, av) int ac; char **av; { op(ac, av, 1, MTWEOF, 1); }
# endif MTWEOF
# ifdef MTRET
ret_func(ac, av) int ac; char **av; { op(ac, av, 0, MTRET, 0); }
# endif MTRET
# ifdef MTRST
rst_func(ac, av) int ac; char **av; { op(ac, av, 0, MTRST, 0); }
# endif MTRST
# ifdef MTERASE
era_func(ac, av) int ac; char **av; { op(ac, av, 0, MTERASE, 1); }
# endif MTERASE

sta_func(ac, av)
	int ac; char **av;
{
	long a[2];
	struct mtget m;
	int b;
	if (argnum(ac, av, a, 0, 0) < 0)
		return;
	mtioctl(MTIOCGET, (char *)&m, 0);
	devstats(&m);
	printf("resid=  %u\n", m.mt_resid);
	printf("fileno= %ld\n", m.mt_fileno);
	printf("blkno=  %ld\n", m.mt_blkno);
# ifdef MTIOCGETBLKSIZE
	mtioctl(MTIOCGETBLKSIZE, (char *)&b, 0);
	printf("blksize=%d\n", b);
# endif MTIOCGETBLKSIZE
}

struct cmd cmds[] =
{
	{ "fsf", "fsf [N]$space forward N files", fsf_func },
	{ "bsf", "bsf [N]$space backward N files", bsf_func },
	{ "fsr", "fsr [N]$space forward N records", fsr_func },
	{ "bsr", "bsr [N]$space backward N records", fsr_func },
	{ "rewind", "rew[ind]$rewind tape", rew_func },
	{ "retension", "ret[ension]$retension the tape", ret_func },
	{ "status", "stat[us]$print tape status", sta_func },
	{ "reset", "res[et]$reset drive", rst_func },
	{ "weof", "weof [N]$write N eofs", weof_func },
# ifdef MTERASE
	{ "erase", "era[se]$erase the tape", era_func },
# endif MTERASE
	{ "help", "h[elp]$print command list", help_func },
	{ "quit", "q[uit]$quit this program", quit_func },
	{ 0 }
};

op(ac, av, maxargs, mtfunc, wflag)
	int ac; char **av; int maxargs;
	int mtfunc, wflag;
{
	long a[1];
	struct mtop m;
	if (argnum(ac, av, a, 0, maxargs) < 0)
		return;
	m.mt_op = mtfunc;
	mtioctl(MTIOCTOP, (char *)&m, wflag);
}

mtioctl(code, m, wflag)
	int code; char *m; int wflag;
{
	register int fd;
	if ((fd = open(thetape, wflag?2:0)) < 0) {
		scerrwarn("can't open %s", thetape);
		return;
	}
	if (ioctl(fd, code, m) < 0)
		scerrwarn("ioctl failed");
	close(fd);
}

dsd_statfunc(m)
	register struct mtget *m;
{
	printf("status= 0x%04x\n", (unsigned short)m->mt_dsreg);
	printf("error=  0x%04x", (unsigned short)m->mt_erreg);
}

ip_statfunc(m)
	register struct mtget *m;
{
	printf("status= 0x%04x\n", (unsigned short)m->mt_dsreg);
	printf("error=  0x%04x", (unsigned short)m->mt_erreg);
}

tmt_statfunc(m)
	register struct mtget *m;
{
	printf("status= 0x%04x\n", (unsigned short)m->mt_dsreg);
	printf("error=  0x%04x", (unsigned short)m->mt_erreg);
}

struct bitfield xm_bits[] =
{
# define STAT3B(b)	((b)<<8)
# define STAT2B(b)	((b)<<0)
	{ STAT2B(1<<7), STAT2B(1<<7), "HardErr" },
	{ STAT2B(1<<6), STAT2B(1<<6), "CtlrErr" },
	{ STAT2B(1<<5), STAT2B(1<<5), "RecShort" },
	{ STAT2B(1<<4), STAT2B(1<<4), "RecLong" },
	{ STAT2B(1<<3), STAT2B(1<<3), "AtFileMark" },
	{ STAT2B(1<<2), STAT2B(1<<2), "PhaseEncoded" },
	{ STAT2B(1<<1), STAT2B(1<<1), "EndOfTape" },
	{ STAT2B(1<<0), STAT2B(1<<0), "WriteProtect" },
	{ STAT3B(1<<7), STAT3B(1<<7), "GcNrz" },
	{ STAT3B(1<<6), STAT3B(1<<6), "HiSpeed" },
	{ STAT3B(1<<5), STAT3B(1<<5), "BeginOfTape" },
	{ STAT3B(1<<4), STAT3B(1<<4), "Rewinding" },
	{ STAT3B(1<<3), STAT3B(1<<3), "DriveBusy" },
	{ STAT3B(1<<2), STAT3B(1<<2), "FmtrBusy" },
	{ STAT3B(1<<1), STAT3B(1<<1), "DriveReady" },
	{ STAT3B(1<<0), STAT3B(1<<0), "OnLine" },
	{ 0 }
};

xm_statfunc(m)
	register struct mtget *m;
{
	printf("status= ");
	symbolic((int)m->mt_dsreg, xm_bits);
	printf("\n");
	printf("error=  0x%04x", (unsigned short)m->mt_erreg);
}


struct tapedesc
{
	short type;
	int (*statfunc)();
};

struct tapedesc tapedesc[] =
{
# ifdef MT_ISTS
	{ MT_ISTS, dsd_statfunc },
# endif MT_ISTS
# ifdef MT_ISHT
	{ MT_ISHT, ip_statfunc },
# endif MT_ISHT
# ifdef MT_ISTMT
	{ MT_ISTMT, tmt_statfunc }, 
# endif MT_ISTMT
# ifdef MT_ISXM
	{ MT_ISXM, xm_statfunc },
# endif MT_ISXM
	{ 0 }
};

struct bitfield mttypes[] =
{
# ifdef MT_ISTS
	{ ~0, MT_ISTS, "Qic-02 Drive, DSD 5217 Ctlr" },
# endif MT_ISTS
# ifdef MT_ISHT
	{ ~0, MT_ISHT, "Qic-02 Drive, Interphase Storager Ctlr" },
# endif MT_ISHT
# ifdef MT_ISTMT
	{ ~0, MT_ISTMT, "1/2\" Drive, Tapemaster X000 Ctlr" },
# endif MT_ISTMT
# ifdef MT_ISXM
	{ ~0, MT_ISXM,  "1/2\" Drive, Xylogics 772 Ctlr" },
# endif MT_ISXM
	{ 0 }
};

devstats(m)
	struct mtget *m;
{
	register struct tapedesc *tp;
	printf("type=   ");
	symbolic((int)m->mt_type, mttypes);
	printf("\n");
	for (tp = tapedesc; tp->statfunc != 0; tp++)
		if (tp->type == m->mt_type)
			(*tp->statfunc)(m);
}

/* ----- utility funcs ----- */
int
argnum(ac, av, arr, minargs, maxargs)
	int ac; char **av;
	long *arr; int minargs, maxargs;
{
	char *name;
	register char *ap;
	long n;

	ac--; name = *av++;
	if (!(minargs <= ac && ac <= maxargs)) {
		errwarn("arg count");
		cmdhelp(name);
		return -1;
	}
	while (--ac >= 0) {
		ap = *av++;
		if (!isnumber(ap, &n)) {
			errwarn("unrecognizable number %s", ap);
			return -1;
		}
		*arr++ = n;
	}
	return 0;
}

struct cmd *
lookup(s)
	char *s;
{
	register struct cmd *cp, *mcp;
	register int matches;

	matches = 0;
	for (cp = cmds; cp->name != 0; cp++)
		if (submatch(s, cp->name)) {
			matches++;
			mcp = cp;
		}

	if (matches == 1) {
		return mcp;
	}
	else
	if (matches == 0) {
		errwarn("unknown command %s", s);
		return 0;
	}
	else {
		errwarn("ambiguous command %s", s);
		return 0;
	}
}

int
submatch(str, name, n)
	register char *str, *name;
{
	while (*str == *name) {
		str++, name++;
		if (*str == '\0')
			return 1;
	}
	return 0;
}

readargs(_argc, _argv)
	int (*_argc); char **(*_argv);
{
# define MAXLINE	512
# define MAXVEC		100
	static char buf[MAXLINE];
	static char *vec[MAXVEC];
	register char *cp;
	int nvec;

	gets(buf, MAXLINE);

	cp = buf;
	nvec = 0;
	for (;;) {
		while (isspace(*cp))
			cp++;
		if (*cp == '\0')
			break;
		if (nvec >= MAXVEC)
			break;

		vec[nvec++] = cp;
		while (*cp != '\0' && !isspace(*cp))
			cp++;
		if (*cp == '\0')
			break;
		*cp++ = '\0';
	}

	*_argc = nvec;
	*_argv = vec;
}

symbolic(val, tab)
	register int val;
	register struct bitfield *tab;
{
	register char *f;

	f = "";
	while (tab->name != 0) {
		if ((val & tab->mask) == tab->val) {
			printf("%s%s", f, tab->name);
			f = "|";
			val &= ~tab->mask;
		}
		tab++;
	}
	if (*f == '\0' || val != 0)
		printf("%s0x%x", f, val);
}

int
isnumber(s, _l)
	char *s;
	long (*_l);
{
	extern long strtol();
	*_l = strtol(s, &s, 0);
	return *s == '\0';
}


/* ----- error handling routines ----- */
typedef struct { int x[6]; } ARGS;


/* VARARGS0 */
scerrwarn(a)
	ARGS a;
{
	register int xerrno;
	extern int errno;
	extern int sys_nerr;
	extern int sys_errlist[];

	fflush(stdout);
	xerrno = errno;
	if (progname != 0)
		fprintf(stderr, "%s: ", progname);
	if ((unsigned)xerrno >= sys_nerr)
		fprintf(stderr, "Error %u -- ", xerrno);
	else
		fprintf(stderr, "%s -- ", sys_errlist[xerrno]);
	fprintf(stderr, a);
	fprintf(stderr, "\n");
	fflush(stderr);
	errno = xerrno;
}

/* VARARGS0 */
errwarn(a)
	ARGS a;
{
	fflush(stdout);
	if (progname != 0)
		fprintf(stderr, "%s: ", progname);
	fprintf(stderr, a);
	fprintf(stderr, "\n");
	fflush(stderr);
}

/* VARARGS0 */
errexit(a)
	ARGS a;
{
	errwarn(a);
	exit(-1);
}
