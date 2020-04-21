#include "defs.h"
# undef DEBUG adb_debug
short adb_debug = 1;
# ifdef DEBUG
# define dprintf(x) (DEBUG?printf x:0)
# define dinterp(x) (DEBUG?x:0)
# define IFDEBUG(x) x
# define ASSERT(c) if(!(c))_assert("c",__FILE__,__LINE__)
extern short DEBUG;
# else DEBUG
# define dprintf(x)
# define dinterp(x)
# define IFDEBUG(x)
# define ASSERT(c)
# endif DEBUG

/****************************************************************************

 DEBUGGER - sub process creation and setup

****************************************************************************/
#define BSR 0x6100 /* bsr <offset> */
#define BSR_MASK 0xFFFF
#define BSR_SIZE 4
#define BSRS 0x6100 /* bsrs <offset> */
#define BSRS_MASK 0xFF00
#define BSRS_SIZE 2
#define JSRA 0x4E90 /* jsr a?@ */
#define JSRA_MASK 0xFFF8
#define JSRA_SIZE 2
#define JSRW 0x4EB8 /* jsrw <adr> */
#define JSRW_MASK 0xFFFF
#define JSRW_SIZE 4
#define JSRL 0x4EB9 /* jsrl <adr> */
#define JSRL_MASK 0xFFFF
#define JSRL_SIZE 6
#define ADDQLSP 0x508F /* addql #?,sp */
#define ADDQLSP_MASK 0xF1FF
#define ADDQWSP 0x504F /* addqw #?,sp */
#define ADDQWSP_MASK 0xF1FF
#define ADDWSP 0xDEFC /* addw #?,sp */
#define ADDWSP_MASK 0xFFFF
#define ADDLSP 0xDFFC /* addl #?,sp */
#define ADDLSP_MASK 0xFFFF
#define BRA 0x6000 /* bra <offset> */
#define BRA_MASK 0xFF00
/* #define ISYM 2 */

MSG BADWAIT;
MSG NOPCS;
MSG ENDPCS;
char * signals[];
char * corfil;
int pid = 0;
int fcor;
long mainsval;
int signum;
int (*sigint)();
int (*sigqit)();
char *corhdr;
int regoff;
REGLIST reglist[];
int regloc[] = {
	R0,R1,R2,R3,R4,R5,R6,R7,AR0,AR1,
	AR2,AR3,AR4,AR5,AR6,SP,PC,RPS };

extern long fulong(),fuilong(),fuishort();

runwait(addr, statusp)
int addr;
int *statusp;
{
	ioctl(0, TCSETAW, &subtty);
	ptrace(CONTIN, pid, addr, signum);
	return(subwait(statusp));
}

subwait(statusp)
int *statusp;
{
	int code;

	signal(SIGINT, SIG_IGN);
	while (((code = wait(statusp)) != pid) && (code != -1));
	signal(SIGINT, sigint);
	ioctl(0, TCGETA, &subtty);
	ioctl(0, TCSETAW, &adbtty);
	if (code == -1)
	{
		pid = 0;
		error(BADWAIT);
	}
	else code = *statusp & 0177;
	if (code != 0177)
	{
		pid = 0;
		if (signum = code) prints(signals[signum]);
		if (*statusp & 0200)
		{
			prints(" - core dumped\n");
			close(fcor);
			corfil = "core";
			setcor();
		}
		error(ENDPCS);
	}
	else
	{
		signum = (*statusp >> 8) & 0377;
		if ((signum != SIGTRAP) && (signum != SIGIOT))
			prints(signals[signum]);
		else signum = 0;
		flushbuf();
	}
	return(code);
}

getreg(pid, offset)
{
	register int data;

	if (pid) data = ptrace(RUREGS, pid, offset, 0);
	else data = *((int *)(((char *)corhdr) + regoff +
	    regloc[offset]*sizeof(unsigned )));
	return(data);
}

putreg(pid, offset, data)
{
	if (pid) ptrace(WUREGS, pid, offset, data);
}

backtr(link, cnt, extra)
{
	extern long entarg();

	register long rtn, p, inst;
	register int n = 1, i, argn;
	long calladr, entadr;
	int indir;
	long olink;

	while(cnt--)
	{
		if (link == 0)
			break;
		olink = p = link;
		link = fulong(p);
		rtn = fulong(p += 4);

		/* determine (guess) address of call instruction */
		entadr = entarg(p,rtn,&calladr,&indir);

		/* determine # of bytes in args */
		argn = (incarg(rtn) + sizeof (long)-1) / sizeof (long);
		dprintf((" -nargs %d\n",argn));

		if (calladr != -1) psymoff(calladr, ISYM, ":");
		else printf("???%x-:",rtn);
		while (charpos() % 8) printc(' ');
		if (charpos() == 8) printc('\t');
		if (entadr != -1) {
			if (indir != -1) printf("(a%d@)", indir);
			else psymoff(entadr, ISYM, "");
		} else
			printf("???");
		while (charpos() % 8) printc(' ');
		printc('(');
		if (argn) printf("%X", fulong(p += 4));
		for(i = 1; i < argn; i++)
			printf(", %X", fulong(p += 4));
		printf(")\n");
		if(extra)printf("\t->%x\n",olink);
		if ((entadr == mainsval) || (++n > 20)) break;
	}
}

long
entarg(addr,radr,_cadr,_indir)
long addr;
register long radr;
register long *_cadr;
int *_indir;
{
	register short instword;

	*_indir = -1;

	*_cadr = radr-JSRL_SIZE;
	instword = fuishort(*_cadr);
	if( (instword&JSRL_MASK) == JSRL )
	{
		return fuilong(*_cadr+sizeof (short));
	}

	*_cadr = radr-JSRW_SIZE;
	instword = fuishort(*_cadr);
	if( (instword&JSRW_MASK) == JSRW )
	{
		return fuishort(*_cadr+sizeof (short));
	}

	*_cadr = radr-BSR_SIZE;
	instword = fuishort(*_cadr);
	if( (instword&BSR_MASK) == BSR )
	{
		return *_cadr + sizeof (short) + (short)
		    fuishort(*_cadr+sizeof (short));
	}

	*_cadr = radr-BSRS_SIZE;
	instword = fuishort(*_cadr);
	if( (instword&BSRS_MASK) == BSRS )
	{
		instword &= ~BSRS_MASK;
		instword = (char)instword; /*SIGNED chars*/
		return *_cadr + sizeof (short) + instword;
	}

	*_cadr = radr-JSRA_SIZE;
	instword = fuishort(*_cadr);
	if( (instword&JSRA_MASK) == JSRA )
	{
		instword &= ~JSRA_MASK;
		*_indir = instword;
		return 0;
	}

	dprintf((" -called_via %x %x %x\n"
	    fuishort(radr-6),fuishort(radr-4),fuishort(radr-2)));

	*_cadr = -1;
	return -1;
}

long
fuilong(addr)
long addr;
{
	register long data;
	data = itol68(get(addr,ISP),get(addr+2,ISP));
	return data;
}
long
fulong(addr)
long addr;
{
	register long data;
	data = itol68(get(addr,DSP),get(addr+2,DSP));
	return data;
}
long
fuishort(addr)
long addr;
{
	return get(addr,ISP,sizeof (short));
}

int
incarg(addr)
{
	register int incr;

	incr = incarg1(addr,1);
	dprintf((" -incr%d\n",incr));
	return incr;
}

int
incarg1(addr,level)
long addr;
int level;
{
	register short instword;

	if( --level < 0 )
	{
		dprintf((" -recurses\n"));
		return 0;
	}

	instword = fuishort(addr);
	dprintf((" incarg([%x])\n",instword));
	addr += sizeof instword;

	if( (instword&ADDQLSP_MASK) == ADDQLSP
	    || (instword&ADDQWSP_MASK) == ADDQWSP )
		return "\10\1\2\3\4\5\6\7"[(instword>>9)&0x7];

	if( (instword&ADDWSP_MASK) == ADDWSP )
		return (short)fuishort(addr);

	if( (instword&ADDLSP_MASK) == ADDLSP )
		return fuilong(addr);

	if( (instword&BRA_MASK) == BRA )
	{
		instword &= ~BRA_MASK;
		instword = (char)instword; /*SIGNED chars*/
		addr += instword;
		if( instword == 0 )
			addr += (short)fuishort(addr);
		dprintf((" -bra%x\n",addr));
		return incarg(addr,level);
	}

	dprintf((" -unknown_incr\n"));
	return 0;
}
