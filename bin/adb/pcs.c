#include "defs.h"
/****************************************************************************

 DEBUGGER - process control loop

****************************************************************************/
MSG NOBKPT;
MSG SZBKPT;
MSG EXBKPT;
MSG NOPCS;
MSG BADMOD;
MSG BADSYN;
struct bkpt * bkptr;
struct bkpt * bkpthead;
struct bkpt * runpcs();
struct bkpt * lbpt;
char *lp;
char *printptr;
char lastc;
REGLIST reglist[];
long int entrypt;
long int expv;
long int dot;
long int dotinc;
int pid;
int adrflg;
long int cntval;
int signum;

subpcs(modif)
{
	int xarg[10];
	register int check, cnt = cntval;
	char * comptr;
	register int *stkptr;
	extern char *malloc();

	switch(modif)
	{
		/* delete breakpoint */
	case 'd':
	case 'D':
		if (bkptr = scanbkpt(dot))
		{
			bkptr->flag = 0;
			return;
		}
		else error(NOBKPT);

		/* set breakpoint */
	case 'b':
	case 'B':
		if (bkptr = scanbkpt(dot)) bkptr->flag = 0;
		for (bkptr = bkpthead; bkptr; bkptr = bkptr->nxtbkpt)
			if (bkptr->flag == 0) break;
		if (bkptr == 0)
		{
			if ((bkptr = (struct bkpt *)malloc(sizeof *bkptr)) == NULL)
				error(SZBKPT);
			else
			{
				bkptr->nxtbkpt = bkpthead;
				bkpthead = bkptr;
			}
		}
		bkptr->loc = dot;
		bkptr->initcnt = bkptr->count = cntval;
		bkptr->flag = BKPTSET;

		check = MAXCOM - 1;
		comptr = bkptr->comm;
		rdc();
		lp--;
		do 
			*comptr++ = readchar();
		while (check-- && (lastc != EOR));
		*comptr = NULL;
		lp--;
		if (check) return;
		else error(EXBKPT);

		/* exit */
	case 'k' :
	case 'K':
		if (pid)
		{
			endpcs();
			return;
		}
		else error(NOPCS);

		/* set program */
	case 'e':
	case 'E':
		endpcs();
		setup(cnt);
		break;

		/* run program */
	case 'r':
	case 'R':
		endpcs();
		setup(cnt);
		reglist[ps].rval &= 0x7FFF;
		bkptr = runpcs(0);
		break;

		/* single step */
	case 's':
		signum = (expr(0) ? expv : signum);
		bkptr = runpcs(cnt);
		break;

		/* single step across subroutine */
	case 'S':
		signum = (expr(0) ? expv : signum);
		printins(0, ISP, get(dot, ISP));
		printptr -= charpos();
		dot += dotinc;
		subpcs('b');
		dot -= dotinc;
		bkptr->flag = P_SINGLE;
		reglist[ps].rval &= 0x7FFF;
		bkptr = runpcs(-1);
		break;

		/* execute subroutine */
	case 'x':
	case 'X':
		if (!pid) error(NOPCS);
		cnt = 0;
		check = dot;
		while (expr(0) && (cnt < 10)) xarg[cnt++] = expv;
		if ((!cnt) || (cnt >= 10)) error(BADSYN);

		dot = entrypt;
		stkptr = (int *)getreg(pid, reglist[sp].roffs);
		subpcs('b');
		bkptr->flag = getreg(pid, reglist[pc].roffs);
		bkptr->count = check;
		bkptr->initcnt = (int)stkptr;

		while (--cnt) ptrace(WDUSER, pid, --stkptr, xarg[cnt]);
		ptrace(WDUSER, pid, --stkptr, dot);
		putreg(pid, reglist[sp].roffs, stkptr);
		reglist[ps].rval &= 0x7FFF;
		dot = xarg[0];
		adrflg = TRUE;
		printf("\nrunning subroutine:\n");
		bkptr = runpcs(-2);
		break;

		/* continue */
	case 'c':
	case 'C':
	case 0:
		reglist[ps].rval &= 0x7FFF;
		signum = (expr(0) ? expv : signum);
		while (cnt--) bkptr = runpcs(0);
		break;

	default:
		error(BADMOD);
	}
	printc(EOR);
	if (bkptr && (*(bkptr->comm) != EOR))
	{
		printf("%s", bkptr->comm);
		command(bkptr->comm, ':');
	}
}

