#include "pmII.h"
#include "pmIImacros.h"
#include "ndbg.h"
#include "mem.h"
#include "Qglobals.h"
#include "Qdevices.h"
#include "common.h"
#include "duart.h"
# include "ctype.h"

# define PROMSTATIC

PROMSTATIC	long lastmemwrote;



/* flags */
# define NONCONTIG	01
# define WRITEABLE	02
# define RELATIVE	04

# define AREGOFF	ADDRREGOFFSET
# define DREGOFF	DATAREGOFFSET



display_regs(frameptr)
register unsigned long *frameptr;
{
	register unsigned long *ptr;
	register int i;
	ptr = frameptr;
	printf("\nREGISTERS AT TIME OF <BREAK>:\n ");
	for (i=0;(i<8);i++) printf("  -- %1d --",i);
	printf("\nD ");
	for (i=8;--i>=0;) printf("%8x ",*ptr++);
	printf("\nA ");
	for (i=8;--i>=0;) printf("%8x ",*ptr++);
	printf("\n\tPC = %8x\tSR = %8x\n\n",ptr[1],ptr[0]);

}


mempoke(addr,len,single)
	register long addr;
	register int len;
	int single;
{
	/* begin editing memory locations at addr of length len.
	   catch bus errors cleanly */

	long oaddr;
	long data;
	char **argv; int argc;
	register char *cp; char c;

	oaddr = addr;

	for( ;; )
	{
	    /* print logical offset */
	    data = (addr-oaddr)/len;
	    if( data < 0 )
		printf("-%2x: ",-data);
	    else
		printf("+%2x: ",data);

	    /* print address */
	    printf("%6x -> ",addr);

	    if (memread(addr,&data,len))
	    {
		/* no bus error...print the value */
	        printf("%8x ",data);

		if( single ) { newline(); return; }

		/* get new data if any */
	    }
	    else
	    {
		/* bus error at read....inform user */
		printf("\
BUS ERROR; STATUS REG = 0x%04x   EXCEPTION REG = 0x%04x\n",
			STATUS_REG, EXCEPTION_REG);
	    }
	    readargs(&argc,&argv);
	    if( argc > 1 )
	    {
		argcnt();
		continue;
	    }
	    if( argc <= 0 || *(cp = *argv) == 000 )
		goto increment;
	    if( !isnum(cp,&data) )
	    {
		if( cp[1] != 000 )
		{
		    illegalnum(cp);
		    continue;
		}

		c = *cp;
		if( isupper(c) )
		    c = tolower(c);
		switch(c)
		{
		case 'q':
		    return;
		case ',':
		    addr -= len;
		    continue;
		case 000:
		    goto increment;
		case 'h':
		    exam_help();
		    continue;
		case '.':
		    data = lastmemwrote;
		    break;
		default:		
		    illegalnum(cp);
		    continue;
		}
	    }

	    lastmemwrote = data;
	    memwrite(addr,data,len);

increment:
	    addr += len;
	}
}




dump_memory(addr,count)
    long *addr;
    int count;
{
    wdumpmem(addr,2,count+1>>1);
}




struct regdesc
{
    char name[2];
    long addr;
    short size;
    short flags;
};

struct regdesc regswitch[] =
{
	{"a0",AREGOFF+0,sizeof (int),RELATIVE|WRITEABLE|NONCONTIG},
	{"a1",AREGOFF+1,sizeof (int),RELATIVE|WRITEABLE|NONCONTIG},
	{"a2",AREGOFF+2,sizeof (int),RELATIVE|WRITEABLE|NONCONTIG},
	{"a3",AREGOFF+3,sizeof (int),RELATIVE|WRITEABLE|NONCONTIG},
	{"a4",AREGOFF+4,sizeof (int),RELATIVE|WRITEABLE|NONCONTIG},
	{"a5",AREGOFF+5,sizeof (int),RELATIVE|WRITEABLE|NONCONTIG},
	{"a6",AREGOFF+6,sizeof (int),RELATIVE|WRITEABLE|NONCONTIG},
	{"a7",AREGOFF+7,sizeof (int),RELATIVE|WRITEABLE|NONCONTIG},

	{"d0",DREGOFF+0,sizeof (int),RELATIVE|WRITEABLE|NONCONTIG},
	{"d1",DREGOFF+1,sizeof (int),RELATIVE|WRITEABLE|NONCONTIG},
	{"d2",DREGOFF+2,sizeof (int),RELATIVE|WRITEABLE|NONCONTIG},
	{"d3",DREGOFF+3,sizeof (int),RELATIVE|WRITEABLE|NONCONTIG},
	{"d4",DREGOFF+4,sizeof (int),RELATIVE|WRITEABLE|NONCONTIG},
	{"d5",DREGOFF+5,sizeof (int),RELATIVE|WRITEABLE|NONCONTIG},
	{"d6",DREGOFF+6,sizeof (int),RELATIVE|WRITEABLE|NONCONTIG},
	{"d7",DREGOFF+7,sizeof (int),RELATIVE|WRITEABLE|NONCONTIG},

	{"fp",AREGOFF+6,sizeof (int),RELATIVE|WRITEABLE|NONCONTIG},
	{"sp",AREGOFF+7,sizeof (int),RELATIVE|WRITEABLE|NONCONTIG},
	{"sr",SROFFSET,sizeof (short),RELATIVE|WRITEABLE|NONCONTIG},
	{"pc",PCOFFSET,sizeof (int),RELATIVE|WRITEABLE|NONCONTIG},

	{"xx",0xFC0001,sizeof (char),WRITEABLE|NONCONTIG},
	{"xs",0xFC9000,sizeof (short),WRITEABLE|NONCONTIG},
	{"xe",0xFCA000,sizeof (short),NONCONTIG},
	{"xq",0xFCC000,sizeof (short),NONCONTIG},
	{"xb",0xFCE000,sizeof (short),NONCONTIG},
	{"xc",0xFD0000,sizeof (short),NONCONTIG},
	{0}
};


struct regdesc *
reglookup(s)
    register char *s;
{
    register struct regdesc *rp;

    for( rp = regswitch; *rp->name != 0; rp++ )
	if( strncmp(s,rp->name,2) == 0 && s[2] == 000 )
	    return rp;

    return 0;
}

int
isreg(s)
    register char *s;
{
    return reglookup(s) != 0;
}

regcmd(fp,regname,wflag,value)
    long fp;
    char *regname;
    long value;
{
    register long addr;
    register struct regdesc *rp;

    rp = reglookup(regname);

    addr = rp->addr;
    if( rp->flags & RELATIVE )
    {
	if( fp == 0 )
	{
	    nosaved();
	    return;
	}
	addr = addr * sizeof (int) + fp;
    }

    if( wflag )
    {
	if( !(rp->flags & WRITEABLE) )
	{
	    printf("\"%s\" is readonly\n",regname);
	    return;
	}
	lastmemwrote = value;
	memwrite(addr,value,rp->size);
    }
    else
    {
	mempoke(addr,rp->size,1);
    }
}

uartcmd(n,wflag)
    int n;
{
    extern duart *dad[];

    register unsigned long addr;
    register char *cp;

    if( n >= 4 )
    {
	printf("Illegal uart number\n");
	return;
    }

    addr = (unsigned long)dad[n];
    cp = "\4\10\4\4";
    while( *cp != 000 )
    {
	dump_memory(addr,*cp++);
	addr += 8;
    }
}

int
nosaved()
{
    printf("? No saved state\n");
    return 0;
}

exam_help()
{
    printf("Edit mode commands:\n");
    colprint(".","store the previous data value here");
    colprint(",","decrement addr without changing the data here");
    colprint("q","quit (back to monitor)");
    colprint("<CR>","increment addr without changing the data here");
    colprint("NUMBER","store the value here (default radix is hex)");
    colprint("h","print this help message");
}
