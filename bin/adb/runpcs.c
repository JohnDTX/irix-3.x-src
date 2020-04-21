#include "defs.h"
/****************************************************************************

 DEBUGGER

****************************************************************************/
#define BKPTI 0x4E420000

MSG NOFORK;
MSG NOPCS;
MSG ENDPCS;
MSG BADWAIT;
FILE *outfile;
char *lp;
int errno;
int (*sigint)();
int (*sigqit)();
char * signals[];
struct bkpt * lbpt;
struct bkpt * bkpthead;
REGLIST reglist[];
char lastc;
int fcor;
int fsym;
char * errflg;
int signum;
long int dot;
char * symfil;
int wtflag;
int pid;
long int expv;
int adrflg;

struct bkpt *
runpcs(goval)
{
	register struct bkpt * bkpt;
	int code, status, rc = 1;

	if (!pid) error(NOPCS);
	if (adrflg)
	{
		lbpt = NULL;
		putreg(pid, reglist[pc].roffs, dot);
		reglist[pc].rval = dot;
	}
	if (goval <= 0) setbp();
	putreg(pid, reglist[ps].roffs, reglist[ps].rval);
	if (goval == -2) goval = 0;
	else printf("\nrunning sub-process: %s\n", symfil);
	if (goval > 0)
	{
		if (lbpt) lbpt = NULL;
		for (; goval > 0; --goval)
		{
			ioctl(0, TCSETAW, &subtty);
			ptrace(P_SINGLE, pid, reglist[pc].rval, 0);
			if ((code = subwait(&status)) != 0177) break;
			reglist[pc].rval = getreg(pid, reglist[pc].roffs);
		}
	}
	else while (rc--)
	{
		if (lbpt && lbpt->flag)
		{
			code = execbkpt(lbpt);
			rc++;
		}
		else code = runwait(reglist[pc].rval, &status);
		reglist[pc].rval = getreg(pid, reglist[pc].roffs);
		if ((code == 0177) && (bkpt = scanbkpt(reglist[pc].rval-2))
		    && (bkpt != lbpt))
		{
			/*stopped at bkpt*/
			lbpt = bkpt;
			reglist[pc].rval = bkpt->loc;
			putreg(pid, reglist[pc].roffs, bkpt->loc);
			if (bkpt->flag == BKPTSET)
			{
				if (--bkpt->count) rc++;
				else bkpt->count = bkpt->initcnt;
			}
			else rc = 0;
		}
		else lbpt = NULL;
	}
	readregs();
	if (rc < 0) delbp();
	if (!lbpt) printf("stopped at location->\t");
	else if (lbpt->flag == BKPTSET) printf("stopped at breakpoint->\t");
	else if (lbpt->flag == P_SINGLE)
	{
		printf(":S stopped at location->\t");
		lbpt->flag = 0;
		lbpt = 0;
	}
	else
	{
		printf("subroutine call completed");
		printc(EOR);
		putreg(pid, reglist[pc].roffs, lbpt->flag);
		putreg(pid, reglist[sp].roffs, lbpt->initcnt);
		reglist[pc].rval = lbpt->flag;
		reglist[sp].rval = lbpt->initcnt;
		dot = lbpt->count;
		lbpt->flag = 0;
		lbpt = 0;
		return(lbpt);
	}
	printpc();
	return(lbpt);
}

endpcs()
{
	register struct bkpt * bkptr;

	if (pid)
	{
		printf("sub-process %d killed\n", pid);
		ptrace(EXIT, pid, 0, 0);
		pid = 0;
		for(bkptr = bkpthead; bkptr; bkptr = bkptr->nxtbkpt)
			if (bkptr->flag != BKPTSET) bkptr->flag = 0;
	}
}

setup(psize)
{
	int status;

	close(fsym);
	fsym = -1;
	if ((pid = fork()) == 0)
	{
		ptrace(SETTRC, 0, 0, 0);
		signal(SIGINT, sigint);
		signal(SIGQUIT, sigqit);
		doexec();
		exit(0);
	}
	else if (pid == -1)
	{
		pid = 0;
		error(NOFORK);
	}
	subwait(&status);
	lbpt = NULL;
	readregs(pid);
	fsym = open(symfil, wtflag);
	lp[0] = EOR;
	lp[1] = NULL;
	printf("sub-process %d created\n", pid);
}

execbkpt(bkptr)
struct bkpt * bkptr;
{
	int saveps, status, code;

	put(bkptr->loc, ISP, bkptr->ins);
	ioctl(0, TCSETAW, &subtty);
	ptrace(P_SINGLE, pid, bkptr->loc, signum);
	code = subwait(&status);
	put(bkptr->loc, ISP, BKPTI);
	return(code);
}

doexec()
{
	char *argl[MAXARG];
	char args[LINSIZ];
	register char *p, **ap, *fname;

	ap = argl;
	p = args;
	*ap++ = symfil;
	do
	{
		if (rdc() == EOR) break;
		*ap = p;
		while ((lastc != EOR) && (lastc != SPACE) && (lastc != TB))
		{
			*p++ = lastc;
			readchar();
		}
		*p++ = NULL;
		fname = *ap + 1;
		if (**ap == '<')
		{
			close(0);
			if (open(fname, 0) < 0)
			{
				printf("cannot open %s\n", fname);
				exit(0);
			}
		}
		else if (**ap == '>')
		{
			close(1);
			if (creat(fname, 0666) < 0)
			{
				printf("cannot create %s\n", fname);
				exit(0);
			}
		}
		else ap++;
	} while (lastc != EOR);

	*ap++ = NULL;
	execv(symfil, argl);
}

struct bkpt *
scanbkpt(adr)
{
	register struct bkpt * bkptr;

	for(bkptr = bkpthead; bkptr; bkptr = bkptr->nxtbkpt)
		if (bkptr->flag && (bkptr->loc == adr)) break;
	return(bkptr);
}

delbp()
{
	register struct bkpt * bkptr;

	if (pid)
		for(bkptr = bkpthead; bkptr; bkptr = bkptr->nxtbkpt)
			if (bkptr->flag) put(bkptr->loc, ISP, bkptr->ins);
}

setbp()
{
	register struct bkpt * bkptr;

	if (pid)
		for(bkptr = bkpthead; bkptr; bkptr = bkptr->nxtbkpt)
			if (bkptr->flag)
			{
				bkptr->ins = (get(bkptr->loc, ISP)<<16) & 0xFFFF0000;
				put(bkptr->loc, ISP, BKPTI);
				if (errno)
				{
					prints("cannot set breakpoint: ");
					psymoff(bkptr->loc, ISYM, "\n");
				}
			}
}

readregs(fid)
int fid;
{
	register REGPTR regptr;

	for(regptr = reglist; regptr <= &reglist[ps]; regptr++)
		regptr->rval = getreg(pid, regptr->roffs);
}
