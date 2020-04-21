#include "defs.h"
/****************************************************************************

 DEBUGGER

****************************************************************************/
MSG LONGFIL;
MSG NOTOPEN;
MSG A68BAD;
MSG A68LNK;
MSG BADMOD;
struct map txtmap;
struct map datmap;
SYMPTR symbol;
int lastframe;
int callpc;
FILE *infile;
FILE *outfile;
int fcor;
char *lp;
int maxoff;
int maxpos;
int hexf = TRUE; /* 0-decimal, TRUE-hex */
int hexa = FALSE; /* 0-noadjust, TRUE-adjust */

/* breakpoints */
struct bkpt * bkpthead;

REGLIST reglist[] =
{
	"d0", 0, 0,
	    "d1", 1, 0,
	    "d2", 2, 0,
	    "d3", 3, 0,
	    "d4", 4, 0,
	    "d5", 5, 0,
	    "d6", 6, 0,
	    "d7", 7, 0,
	    "a0", 8, 0,
	    "a1", 9, 0,
	    "a2", 10, 0,
	    "a3", 11, 0,
	    "a4", 12, 0,
	    "a5", 13, 0,
	    "a6", 14, 0,
	    "sp", 15, 0,
	    "pc", 16, 0,
	    "ps", 17, 0,
};

char lastc;
int fcor;
char * errflg;
long int dot;
long int var[];
char * symfil;
char * corfil;
int pid;
long int adrval;
int adrflg;
long int cntval;
int cntflg;
int signum;
char * signals[NSIG] = 
{
	"signal 0",				/* non existant signal */
	"hangup",				/* SIGHUP */
	"interrupt",				/* SIGINT */
	"quit",					/* SIGQUIT */
	"illegal instruction",			/* SIGILL */
	"trace trap",				/* SIGTRAP */
	"IOT",					/* SIGIOT */
	"EMT",					/* SIGEMT */
	"floating exception",			/* SIGFPE */
	"killed",				/* SIGKILL */
	"bus error",				/* SIGBUS */
	"segmentation violation",		/* SIGSEGV */
	"bad syscall argument",			/* SIGSYS */
	"broken pipe",				/* SIGPIPE */
	"alarm",				/* SIGALRM */
	"terminated",				/* SIGTERM */
	"SIGUSR1",				/* SIGUSR1 */
	"SIGUSR2",				/* SIGUSR2 */
	"child death (SIGCLD)",			/* SIGCLD */
	"power fail",				/* SIGPWR */
	"signal 20",
	"signal 21",
	"signal 22",
	"signal 23",
	"signal 24",
	"window change (SIGWINCH)",		/* SIGWINCH */
	"input/output possible (SIGIO)",	/* SIGIO */
	"urgent condition on I/O channel (SIGURG)", /* SIGURG */
	"poll ready (SIGPOLL)",			/* SIGPOLL */
	"signal 29",
	"signal 30",
	"signal 31",
};

printtrace(modif)
{
	int narg, i, stat, name, limit;
	unsigned dynam;
	register struct bkpt * bkptr;
	char hi, lo;
	int word;
	char * comptr;
	long int argp, w1, w2;
	SYMPTR symp;

	if( cntflg==0 ){
		cntval = -1;
	}

	switch (modif)
	{
	case '<':
	case '>':
		{
			char file[64];
			int index;

			index=0;
			if( modif=='<')
			{
				iclose();
			} else {
				oclose();
			}
			if( rdc()!=EOR)
			{
				do{
					file[index++]=lastc;
					if( index>=63 ){
						error(LONGFIL);
					}
				}while( readchar()!=EOR );
				file[index]=0;
				if( modif=='<')
				{
					infile=fopen(file,"r");
					if( infile == NULL)
					{
						error(NOTOPEN);
					}
				} else {
					outfile=fopen(file,"w");
					if( outfile == NULL)
					{
						error(NOTOPEN);
					} else {
						fseek(outfile,0L,2);
					}
				}

			}
			lp--;
		}
		break;

	case 'x':
		hexf = TRUE;
		break;

	case 'H':
		hexa = TRUE;
		break;

	case 'd':
		hexf = FALSE;
		break;

	case 'q':
	case 'Q':
	case '%':
		done();

	case 'w':
	case 'W':
		maxpos = (adrflg ? adrval : MAXPOS);
		break;

	case 's':
	case 'S':
		maxoff = (adrflg ? adrval : 32768);
		break;

	case 'v':
	case 'V':
		prints("variables\n");
		for( i=0;i<=35;i++)
		{
			if( var[i])
			{
				printc((i<=9 ? '0' : 'a'-10) + i);
				printf(" = %X\n",var[i]);
			}
		}
		break;

	case 'm':
	case 'M':
		printmap("? map",&txtmap);
		printmap("/ map",&datmap);
		break;

	case 0:
	case '?':
		if (pid) printf("sub-process id = %d\n", pid);
		prints(signals[signum]);
		printf("\n");

	case 'r':
	case 'R':
		if (pid || (fcor != -1)) printregs();
		else prints("no process or core image\n");
		return;

	case 'c':
	case 'C':
		if (pid)
			backtr(adrflg
			    ? adrval
			    : getreg(pid,reglist[a6].roffs)
			    , cntval, modif=='C');
		else if (fcor != -1)
			backtr(adrflg
			    ? adrval
			    : reglist[a6].rval
			    , cntval, modif=='C');
		else
			prints("no process or core image\n");
		break;

		/*print externals*/
	case 'e':
	case 'E':
		symset();
		while (symp = symget())
			if ((symp->symf == 043) || (symp->symf == 044))
			{
				w1 = get(symp->vals, DSP);
				w2 = get(symp->vals + 2, DSP);
				printf("%s:\t%X\n", symp->symc, itol68(w1,w2));
			}
		break;

		/*print breakpoints*/
	case 'b':
	case 'B':
		printf("breakpoints\ncount%8tbkpt%24tcommand\n");
		for (bkptr = bkpthead; bkptr; bkptr=bkptr->nxtbkpt)
			if (bkptr->flag)
			{
				printf("%-8.8d", bkptr->count);
				psymoff(leng(bkptr->loc), ISYM, "%24t");
				comptr = bkptr->comm;
				while (*comptr) printc(*comptr++);
			}
		break;

	default:
		error(BADMOD);
	}
}

printmap(s, amap)
char * s;
struct map *amap;
{
	int file = amap->ufd;
	printf("%s\t`%s'\n", s,
	    (file < 0 ? "-" : (file == fcor ? corfil : symfil)));
	printf("b1 = %X\t", amap->b1);
	printf("e1 = %X\t", amap->e1);
	printf("f1 = %X\t", amap->f1);
	printf("\nb2 = %X\t", amap->b2);
	printf("e2 = %X\t", amap->e2);
	printf("f2 = %X", amap->f2);
	printc(EOR);
}

printregs()
{
	register int i;
	register long v;

	for (i = 0; i <= 7; i++)
	{
		printf("%s\t%X", reglist[i].rname, reglist[i].rval);
		while (charpos() % 32) printc(' ');
		printf("%s\t%X\n", reglist[i+8].rname, reglist[i+8].rval);
	}
	printf("%s\t%x\n", "ps", (reglist[ps].rval & 0xFFFF));
	printf("%s\t%X\t", "pc", reglist[pc].rval);
	printpc();
	printf("\n");
	/* printf("%s\t%X\n\n", "sp", reglist[sp].rval); */
}

getroffs(regnam)
{
	register REGPTR p;
	register char * regptr;
	char regnxt;

	regnxt = readchar();
	for (p = reglist; p <= &reglist[ps]; p++)
	{
		regptr = p->rname;
		if ((regnam == *regptr++) && (regnxt == *regptr))
			return(p->roffs);
	}
	lp--;
	return(16);
}

printpc()
{
	dot = reglist[pc].rval;
	printc(EOR);
	psymoff(dot, ISYM, ":");
	printins(0, ISP, chkget(dot, ISP));
	printc(EOR);
}
