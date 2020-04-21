/*
** NOTE: putchar is not called because it currently does not map a
**       <lf> to a <lf><cr>.
**	 If DBGTXT is defined during compilation the disk addresses
**	 associated with shared texts will be displayed also with
**	 display text command.
*/

#include "debug.h"
#if NDEBUG > 0

#include	"../h/param.h"
#include	"../h/systm.h"
#include	"../h/proc.h"
#include	"../h/dir.h"
#include	"../h/user.h"
#include	"../h/text.h"
#include	"../h/inode.h"
#include	"../h/fs.h"
#include	"../h/mount.h"
#include	"../h/map.h"
#include	"../h/cmap.h"
#include	"../vm/vm.h"
#include	"../h/setjmp.h"
#include	"../h/printf.h"
#include	"../h/buf.h"
#include	"../com/pipe_inode.h"
#include	"../ipII/cpureg.h"
#include	"../ipII/reg.h"
#include	"../ipII/pte.h"
#include	"../ipII/trap.h"
#include	"../ipII/cx.h"

#include	"efs.h"
#if NEFS > 0
#	include	"../efs/efs_inode.h"

extern short	efs_fstyp;
#endif

#include	"nfs.h"
#if NNFS > 0
#	include	"../nfs/nfs.h"
#	include	"../nfs/rnode.h"

extern short	nfs_fstyp;
#endif

#undef	TRACE

#define	BELL	7
#define	BS	8
#define CNTL_C	3
#define DEL	0x7f

#define	DISPLAY	0	/* display option	*/
#define	EDIT	1	/* edit option		*/

/*
** a nice macro to store info into a software pte
*/
#define	SETPTEINFO(p,pg,prt)	(*(u_long *)&(p) = ((pg)) | (prt))

/*
** routines declared
*/
extern int	pmemact(), kmemact(), udotact(), mapact(),
		pteact(),  regact(),  procact(), textact(),quact(),
		stkact(), inoact(), bufact(), traceact();
extern long	getnum();
extern char	procflgs(),
		*textflgs();

extern int	*nofault;	/* jump buffer pointer for traps	*/

/*
** these defines represent the index in the modifier structure a
** particular option resides at.  It is also used to address the
** help messages.
*/
#define	REGS_CMD	0
#define	PMEM_CMD	1
#define	KMEM_CMD	2
#define	PROC_CMD	3
#define	STK_CMD		4
#define	PTE_CMD		5
#define	TOG_CMD		6
#define	UDOT_CMD	7
#define	MAP_CMD		8
#define	TXT_CMD		9
#define	QU_CMD		10
#define	INO_CMD		11
#define	BUF_CMD		12
#define	TRACE_CMD	13

/*
** this structure contains the names we recognize as secondary arguments
** for displaying and editing.
*/
static struct modifier
{
	char	*mod_name;
	int	(*mod_func)();
} mods[] =
{
	{ "regs",		regact	},	/* registers		*/
	{ "pmem",		pmemact	},	/* physical memory	*/
	{ "kmem",		kmemact	},	/* kernel memory	*/
	{ "proc",		procact	},	/* processes		*/

	{ "stack",		stkact	},	/* stack becktrace	*/
	{ "pte",		pteact	},	/* ptes			*/
	{ "BARF",		(int (*)())NULL	},/* toggles (debugging)*/
	{ "udot",		udotact	},	/* udot			*/

	{ "map",		mapact	},	/* various maps		*/
	{ "text",		textact	},	/* shared texts		*/
	{ "queue",		quact	},	/* queues		*/
	{ "inode",		inoact	},	/* inodes		*/

	{ "buffers",		bufact	},	/* buffers		*/
	{ "trace",		traceact },	/* tracing package */
	{ NULL,			(int (*)())NULL }
};

#define	MODWSIZE	(sizeof (struct modifier)/sizeof (char *))

long		savessp;	/* sn saved here in locore assist	*/
long		savesfp;	/* kernel fp saved in locore		*/
static jmp_buf	_pconio;	/* jump buffer pointer for console i/o	*/
static char	lastchar;	/* holds last input char		*/
struct pte	upte;		/* holds the original udot pte		*/

#define	YES	1
#define	NO	0

/*ARGSUSED*/
kdb( mess, sr, regs )
char	*mess;
int	sr,
	regs;
{
	int	*save_nofault;
	jmp_buf	jb;
	char	c;
	int	i;
	int	fault_type;
	extern short kdebug;
	extern short kswitch;
	int s;

	s = spl7();			/* disable interrupts--just in case */

	if (!kdebug) {
		printf("kernel debugger disabled\n");
		splx(s);
		return;
	}
	if ( kswitch ) {
		setConsole(CONSOLE_ON_SERIAL);
	} else {
		setConsole(CONSOLE_NOT_ON_PTY);
	}
	resetConsole();

top:
	/*
	** announce ourself and print the saved stack value
	*/
	printf( "\nKernel Debugger:" );

	/*
	 * Make sure message is printable.
	 */
	save_nofault = nofault;
	if (setjmp(jb) == 0) {
		nofault = jb;
		if ( mess )
			printf( " %s", mess );
	}
	nofault = save_nofault;

	printf( "\n(kernel sp: %x, kernel fp: %x)\n", savessp, savesfp );

	getpte( UDOT_VBASE, KCX, &upte );	/* get the original udot pte */

	/*
	** setup the jump buffer for traps
	*/
	save_nofault = nofault;
	if ( fault_type = setjmp( jb ) )
		printf( "\nFault: type: %d  pc: %08x  addr: %08x\n",
			fault_type, u.u_ar0[ PC ], u.u_pcb.pcb_aaddr );
	nofault = jb;

	while ( 1 )
	{
		if ( setjmp( _pconio ) )
			printf( "\n" );

		printf( "Kdb: " );
		c = dbgetchar();

		switch ( c )
		{
		   case 'd':
			printf( "display " );
			i = match();
			(*mods[ i ].mod_func)( DISPLAY );
			break;

		   case 'e':
			printf( "edit " );
			i = match();
			(*mods[ i ].mod_func)( EDIT );
			break;

		   case 'c':
			printf( "continue\n" );
			if (yesno() == YES) {
				/*
				** if we switched consoles - switch them back
				*/
				if ( kswitch )
					setConsole(CONSOLE_NOT_ON_PTY);
				nofault = save_nofault;
				splx(s);
				return;
			}
			break;

		   case 'r':
			printf( "reboot\n" );
			if (yesno() == YES)
				doboot();
			break;

		   case 's':
			if (havegrconsole) {
				printf("switching to %s console\n",
					  con_putchar == duputchar ? "serial" :
					  "graphics");
				setConsole(con_putchar == duputchar ?
						       CONSOLE_ON_WIN :
						       CONSOLE_ON_SERIAL);
				printf("switched to %s console\n",
					  con_putchar == duputchar ? "serial" :
					  "graphics");
			}
			goto top;

		   case '?':
		   case 'h':
			printf( "help\n" );
			help( -1 );
			break;

		   case '\n':
			printf( "\n" );
			continue;

		   default:
			printf( "%c\n", BELL );
		}
	}
}

static
yesno()
{
	char b[50];

	printf("Are you sure (y or n)? ");
	gets(b);
	if (b[0] == 'y')
		return (YES);
	return (NO);
}

/* get an input character
 *	This is where one should do abominations like longjumps when
 *	DEL is received.  One should <<NOT>> put them into the depths of
 *	drivers.  In fact, it would be better to not use longjmp at all.
 */
static
dbgetchar()
{
	register char c;

	c = (getchar() & 0x7f);
	if (c == CNTL_C)		/* convert control-C to DEL */
		c = DEL;
	if (DEL == c)			/* this should not be done */
		longjmp(_pconio,1);	/* the callers should check results */
	return c;
}


static
dbgquit()
{
	printf(" (q/cr)? ");
	if (dbgetchar() == 'q')
		return (1);
	return (0);
}

/*
** regact
**   perform the actions necessary to display/edit registers.
**   The info is split into 2 parts:
**	1. Current process information (include registers)
**	2. Cpu board registers.
*/
static
regact( act )
int	act;	/* action to perform	*/
{
	register short		i;
	struct pte		apte;
	char			ch;

	getpte( (long)&u, KCX, &apte );

 switch ( act )
 {
    case DISPLAY:
	/*
	** null terminate the current executing command name
	*/
	ch = u.u_comm[ DIRSIZ - 1 ];
	u.u_comm[ DIRSIZ - 1 ] = 0;

	/*
	** current process information
	*/
	printf( "\nCurrent process:\n" );
	printf( "   pid: %d u_procp: %x exec: '%s'\n", u.u_procp->p_pid,
		u.u_procp, u.u_comm );

	/* reset the nulled character	*/
	u.u_comm[ DIRSIZ - 1 ] = ch;

	printf( "   active/current udot (%08x/%08x)\n", *(u_long *)&upte,
		*(u_long *)&apte );
	printf( "\nProcessor Registers (ssp: %08x):\n", savessp );
	printf( "   pc: %08x  sr: %04x\n", u.u_ar0[ PC ],
		u.u_ar0[ RPS ] & 0xffff );
	for ( i = 0; i < 16; i++ )
	{
		printf( "%08x ", u.u_ar0[ i ] );
		if ( i == 7 || i == 15 )
			printf( "\n" );
	}

	printf( "\nBoard Registers:\n" );
	printf( "   text base/limit (%04x/%04x)\n", *TDBASE_REG & 0xffff,
		*TDLMT_REG & 0xffff );
	printf( "   stk  base/limit (%04x/%04x)\n", *STKBASE_REG & 0xffff,
		*STKLMT_REG & 0xffff );
	printf( "   status: %04x parctl: %02x mbp: %02x\n",
		*STATUS_REG & 0xffff, *PARCTL_REG & 0xffff,
		*MBP_REG & 0xff );
	break;

    case EDIT:
	printf( "\nEditing of registers not implemented.\n" );
	break;

    default:
	help( REGS_CMD );
	break;
 }

}

/*
** pmemact
**   perform the actions necessary to display/edit physical memory
*/
static
pmemact( act )
int	act;	/* action to perform	*/
{
	long		val;
	struct pte	apte;
	u_short		*vaddr;

	/*
	** prompt for physical address; get reply: if invalid - bye!
	*/
	printf( "at " );
	if (  ( val = getnum( 16 ) ) == -1 )
	{
		printf( "Invalid physical address.\n" );
		help( PMEM_CMD );	/* output usage message	*/
		return;
	}


	/*
	** set virtual address to use and program the scratch pte
	** so we can access the physical memory.
	*/
	vaddr = (u_short *)( SCRPG0_VBASE + ( val & PGOFSET ) );
	SETPTEINFO( apte, btop( val ), PG_KW | PG_V );
	setpte( SCRPG0_VBASE, KCX, &apte );

 switch ( act )
 {
   case DISPLAY:
	/*
	** we always align display output to a 16 word boundary
	*/
	val &= ~0xf;
	vaddr = (u_short *)( (u_long)vaddr &  ~0xf );
	mdump( (u_short *)val, vaddr );	/* dump the memory		*/
	break;

   case EDIT:
	medit( (u_short *)val, vaddr );	/* edit the memory	*/
	break;

   default:
	help( PMEM_CMD );
	break;
 }
}

/*
** kmemact
**   perform the actions necessary to display/edit kernel memory
*/
static
kmemact( act )
int	act;	/* the action to perform	*/
{
	long		val;
	struct pte	apte1,
			apte2;

	/*
	** prompt for kernel address; get reply: if invalid - bye!
	*/
	printf( "at " );
	if (  ( val = getnum( 16 ) ) == -1 )
	{
		printf( "Invalid kernel address.\n" );
		help( KMEM_CMD );	/* output usage message	*/
		return;
	}

	/*
	** if kernel address does not have a segment number,
	** we supply the kernel segment number. If a segment is
	** given we just use that one - this allows access to other
	** "system" locations.
	*/
	if ( ( val & SEG_MSK ) == 0 )
		val |= SEG_OS;

 switch ( act )
 {
    case DISPLAY:
	/*
	** we always align display output to a 16 word boundary
	*/
	val &= ~0xf;
	mdump( (u_short *)val, (u_short *)val );  /* dump the stuff	*/
	break;

   case EDIT:
	/*
	** get the pte for this page, set it to be writable, and re-pgm it
	*/
	getpte( val, KCX, &apte1 );
	SETPTEINFO( apte2, apte1.pg_pfnum, PG_KW | PG_V );
	setpte( val, KCX, &apte2 );

	medit( (u_short *)val, (u_short *)val );	/* edit it	*/

	setpte( val, KCX, &apte1 );	/* restore original pte	*/
	break;

   default:
	help( KMEM_CMD );
	break;
 }
}

/*
** procact
**   perform the actions necessary to display/edit processes.
*/
static
procact( act )
int	act;
{
	register struct pte	*pte;
	register struct user	*up;
	register short		i;
	register struct proc	*p;
	long			val;
	struct pte		apte;
	char			pflgs();

	/*
	** prompt for pid; get reply: if no pid given - use current process.
	**			      if wildcard - print all processes.
	*/
	printf( "for Pid " );
	if ( ( val = getnum( 10 ) ) < 0 )
	{
		if ( lastchar == '\0' )	/* dump current proc?	*/
		{
			p = u.u_procp;
			goto skip;
		}
		if ( lastchar == '*' )	/* dump proc table?	*/
		{
			if ( act == EDIT )
			{
				printf( "A process must be specified for editing.\n" );
				return;
			}
			ptabledmp();
		}
		else
		{
			printf( "Invalid pid.\n" );
			help( PROC_CMD );
		}
		return;
	}

	/*
	** try to match the pid
	*/
	for ( p = &proc[ 0 ]; p < procNPROC; p++ )
		if ( p->p_pid == val )
			break;

	if ( p == procNPROC )
	{
		printf( "Process with given pid does not exist.\n" );
		return;
	}

skip:

 switch ( act )
 {
    case DISPLAY:
	printf( "procp: %x flag: %x stat: %c pri: %d time: %d cpu: %d nice: %d\n",
		 p, p->p_flag, procflgs( p->p_stat ),
		 p->p_pri, p->p_time, p->p_cpu, p->p_nice );

	printf( "slptime: %d uid: %d suid: %d pgrp: %d pid: %d ppid: %d\n",
		p->p_slptime, p->p_uid, p->p_suid,
		p->p_pgrp, p->p_pid, p->p_ppid );

	printf( "addr: %x poip: %d szpt: %d tsize: %d dsize: %d ssize: %d\n",
		p->p_addr, p->p_poip, p->p_szpt, p->p_tsize,
		p->p_dsize, p->p_ssize );

	printf( "rssize: %d maxrss: %d swrss: %d swaddr: %d p0br: %x\n",
		p->p_rssize, p->p_maxrss, p->p_swrss,
		p->p_swaddr, p->p_p0br);

	printf( "xlink: %x sig: %x wchan: %x textp: %x link: %x\n",
		p->p_xlink, p->p_sig,
		p->p_wchan, p->p_textp, p->p_link);

	printf( "clktim: %d ndx: %d loadaddr: %x\n",
		p->p_clktim, p->p_ndx, p->p_loadc);

	printf( "cxtdnum: %x cxbsize: %x cxsnum: %x cxssize: %x prev: %x next: %x\n",
		p->p_cxtdnum, p->p_cxbsize,
		p->p_cxsnum, p->p_cxssize, p->p_cxprev, p->p_cxnext);

	/*
	** if process is resident in memory: print the associated
	** text, data, stack and udot pages. Also the dmap and smap
	** info is printed.
	*/
	if ( p->p_flag & SLOAD )
	{
		pte = tptopte( p, 0 );
		if ( p->p_textp )
		{
			printf( "Text pages:" );
			for ( i = 0; i < p->p_tsize; i++ )
			{
				if ( ( i & 7 ) == 0 )
					printf( "\n" );
				printf( "%08x ", *(long *)pte );
				pte++;
			}
		}
		pte = dptopte( p, 0 );
		printf( "\nData pages:" );
		for ( i = 0; i < p->p_dsize; i++ )
		{
			if ( ( i & 7 ) == 0 )
				printf( "\n" );
			printf( "%08x ", *(long *)pte );
			pte++;
		}
		pte = sptopte( p, 0 );
		printf( "\nStack pages: (in reverse order!)" );
		for ( i = 0; i < p->p_ssize; i++ )
		{
			if ( ( i & 7 ) == 0 )
				printf( "\n" );
			printf( "%08x ", *(long *)pte );
			pte--;
		}
		printf( "\nUpage: %08x\n", *(long *)p->p_addr );

		up = (struct user *)SCRPG0_VBASE;
		apte = *(p->p_addr);
		*(long *)&apte |= PG_V;
		setpte( SCRPG0_VBASE, KCX, &apte );
		printf( "Dmap: (size: %d alloc: %d)",
			up->u_dmap.dm_size, up->u_dmap.dm_alloc );
		for ( i = 0; i < NDMAP; i++ )
		{
			if ( ( i & 7 ) == 0 )
				printf( "\n" );
			printf( "%5d ", up->u_dmap.dm_map[ i ] );
		}
		printf( "\nSmap: (size: %d alloc: %d)",
			up->u_smap.dm_size, up->u_smap.dm_alloc );
		for ( i = 0; i < NDMAP; i++ )
		{
			if ( ( i & 7 ) == 0 )
				printf( "\n" );
			printf( "%5d ", up->u_smap.dm_map[ i ] );
		}
		printf( "\n" );
	}
	break;

    case EDIT:
	printf( "Editing of a process is not implemented.\n" );
	break;

    default:
	help( PROC_CMD );
	break;
 }
}
/*
** stkact
**   perform the actions necessary to display a stack backtrace.
*/
/*ARGSUSED*/
static
stkact( act )
int	act;	/* the action to perform	*/
{
	register long	*vaddr,
			*ptr;
	register short	i;
	long		val;

	/*
	** prompt for starting address; get reply: if invalid - bye!
	*/
	printf( "starting at " );
	if ( ( val = getnum( 16 ) ) < 0 )
	{
		printf( "Invalid starting stack address.\n" );
		help( STK_CMD );
		return;
	}

	/*
	** if segment is not set, we set it to the kernel segment
	*/
	if ( ( val & SEG_MSK ) == 0 )
		val |= SEG_OS;

	/*
	** verify the given address is within the stack area.
	*/
	vaddr = (long *)val;
	if ( ( val < UDOT_VBASE ) || ( val >= UDOT_VLIMIT ) )
	{
		printf( "Address is not within the udot page.\n" );
		return;
	}

	printf( "......fp........pc...(args)\n" );
	for (;;)
	{
		/*
		** if frame pointer is out of range, we be done
		*/
		if ( ( vaddr < (long *)UDOT_VBASE ) ||
		     ( vaddr + 1 >= (long *)UDOT_VLIMIT ) )
			break;

		printf( "%08x  %08x  (", *vaddr, *(vaddr + 1) );

		/*
		** handle the arguments, up to 7
		*/
		for ( ptr = vaddr + 2, i = 0; i < 8; i++, ptr++ )
		{
			/*
			** if arg pointer is out of range, ending paren
			** and we be done
			*/
			if ( ( ptr < (long *)UDOT_VBASE ) ||
			     ( ptr >= (long *)UDOT_VLIMIT ) )
			{
				printf( ")" );
				goto done;
			}
			if ( i == 7 )
				printf( "%x)\n", *ptr );
			else
				printf( "%x, ", *ptr );
		}
		vaddr = (long *)*vaddr;
	}
done:
	printf( "\n" );
}

/*
** pteact
**   perform the actions necessary to display/edit hardware ptes.
*/
static
pteact( act )
int	act;	/* action to perform	*/
{
	long	val;
	char	tchar;
	short	dsplytype = 0;

	/*
	** prompt for value type (index or context); get reply:
	** and set flag if supplying context.
	*/
	printf( "for " );
	if ( ( tchar = dbgetchar() ) == 'i' )
	{
		printf( "index " );
	}
	else
	if ( tchar == 'c' )
	{
		printf( "context " );
		dsplytype = 1;
	}
	else
	if (tchar == 'v')
		printf("virtual address ");
	else
	{
		printf( "\nInvalid TYPE for pte display.\n" );
		help( PTE_CMD );
		return;
	}

	/*
	** get pte starting value; if invalid - bye!
	*/
	if ( ( val = getnum( 16 ) ) == -1 )
	{
		printf( "Invalid pte VALUE.\n" );
		help( PTE_CMD );
		return;
	}

 switch( act )
 {
    case DISPLAY:
    {
	register int	i,
			j;
	register u_long	*ppte;

	if (tchar == 'v') {
		/*
		 * Convert address into an index into the page map
		 */
		if ((val & SEG_MSK) == SEG_OS) {
			val &= ~SEG_MSK;
			val += (KCX << PPCXLOG2) * NBPG;
		}
		val /= NBPG;
	}

	/*
	** if starting pte was a context; convert it to an index
	*/
	if ( dsplytype )
	{
		printf( "Context: %04x => ", val );
		val <<= PPCXLOG2;
		printf( "Index: %04x\n", val );
	}

	/*
	** we round down to a 4 page boundary and begin displaying
	** ptes there
	*/
	i = val & ~0x3;
	if ( i >= PTMAP_SZ )
	{
		printf( "Juniper has a maximum of 0x%04x pte entries.\n", PTMAP_SZ );
		return;
	}

	ppte = (u_long *)PTMAP_BASE + i;

	for ( ; i < PTMAP_SZ; i+= 4 )
	{
		printf( "\n%04x: ", i );
		for ( j = 0; j < 4; j++, ppte++ )
			printf( "%08x ", *ppte );

		if (dbgquit())
			break;
	}
	printf( "\n" );
	break;
    }

    case EDIT:
	printf( "Editing of hardware ptes is not implemented\n" );
	break;

    default:
	help( PTE_CMD );
	break;
 }
}

#ifdef	NOTDEF
/*
** the various debugging flags we support
*/
char	traps,		buserrs,	syscalls,	pages,
	expnd,		procs,		cxes,		dflag1,
	dflag2,		dflag3,		dflag4,		dflag5,
	dflag6;

/*
** the sizes of a single entry of the following structures
*/
#define	TOGWSIZE	(sizeof (struct toginfo)/(sizeof (char *)))
#define	TOGMWSIZE	(sizeof (struct togmods)/(sizeof (char *)))
#define	TOGBWSIZE	(sizeof (struct togbits)/(sizeof (char *)))
#define	TOGVWSIZE	(sizeof (struct togvals)/(sizeof (char *)))

#define	MODALL	0	/* modify all values		*/
#define	MODINT	1	/* modify individual value	*/

/*
** this structure contains the toggle name and the variable that
** holds its value.
*/
struct toginfo
{
	char	*tg_name;
	char	*tg_flg;
} toginfo[] =
{
	{ "traps",	&traps		},
	{ "buserrs",	&buserrs	},
	{ "syscalls",	&syscalls	},
	{ "pages",	&pages		},
	{ "expnd",	&expnd		},
	{ "procs",	&procs		},
	{ "cxes",	&cxes		},
	{ "dflag1",	&dflag1		},
	{ "dflag2",	&dflag2		},
	{ "dflag3",	&dflag3		},
	{ "dflag4",	&dflag4		},
	{ "dflag5",	&dflag5		},
	{ "dflag6",	&dflag6		},
	{ (char *)NULL,	(char *)NULL	}
};

/*
** this structure holds the optional values we can set a toggle to
*/
struct togmods
{
	char	*tm_name;
	char	*tm_flg;
} togmods[] =
{
	{ "*",		(char *)NULL	},	/* set all togs to same     */
	{ "-",		(char *)NULL	},	/* set all togs individually*/
	{ (char *)NULL,	(char *)NULL	}
};

/*
** this structure contains the ascii strings that various toggle values
** represent
*/
struct togbits
{
	char	*tb_name;
	char	*tb_flg;
} togbits[] =
{
	{ "trace",	NULL	},
	{ "break",	NULL	},
	{ NULL,		NULL	}
};

/*
** the toggle values we understand
*/
struct togvals
{
	char	*tv_name;
	char	*tv_flg;
} togvals[] =
{
	{ "0",	NULL	},
	{ "1",	NULL	},
	{ NULL,	NULL	}
};

/*
** togact
**   perform the actions necessary to display/edit toggles.
*/
static
togact( act )
int	act;	/* the action to perform	*/
{
	char		buff[ 20 ];
	int		flag,
			mod;
	long		val;

	flag = mod = -1;

	/*
	** get the toggle name and try to match it to a real name
	** or a pseudo-name
	*/
	gets( buff );
	flag = checked( buff, &toginfo[ 0 ].tg_name, TOGWSIZE );
	if ( flag == -1 )
		mod = checked( buff, &togmods[ 0 ].tm_name, TOGMWSIZE );

	/*
	** see if it is a named flag
	*/
	if ( flag == -1 )
	{
		/*
		** maybe it is a modification to all flags
		*/
		if ( mod == -1 )
		{
			printf( "Say what?\n" );
			printf( " choices are: " );
			for ( flag = 0; toginfo[ flag ].tg_name; flag++ )
				printf( "%s ", toginfo[ flag ].tg_name );
			for ( mod = 0; togmods[ mod ].tm_name; mod++ )
				printf( "%s ", togmods[ mod ].tm_name );
			printf( "\n" );
			return;
		}
	}

 switch ( act )
 {
    case DISPLAY:
	if ( flag >= 0 )
	{
		togpr( flag );
		printf( "\n" );
	}
	else
		switch ( mod )
		{
		   case MODINT:
		   case MODALL:
			for ( flag = 0; toginfo[ flag ].tg_name; flag++ )
			{
				togpr( flag );
				printf( "\n" );
			}
			break;
		}
	break;

   case EDIT:
	if ( flag >= 0 )
	{
		togpr( flag );
		printf( "=> " );
		togset( flag );
	}
	else
	if ( mod == MODINT )
		for ( flag = 0; toginfo[ flag ].tg_name; flag ++ )
		{
			togpr( flag );
			printf( "=> " );
			togset( flag );
			printf( "\n" );
		}
	else
	{
		printf( "change all values to " );
		if ( ( val = getnum( 16 ) ) == -1 )
			return;
		for ( flag = 0; toginfo[ flag ].tg_name; flag ++ )
		{
			togpr( flag );
			printf( " => %x\n", val );
			*toginfo[ flag ].tg_flg = (char)val;
		}
	}
	break;

    default:
	help( TOG_CMD );
	break;
 }
}
#endif

/*
** udotact
**   perform the actions necessary to display/edit a udot area.
*/
static
udotact( act )
int	act;	/* the action to perform	*/
{
	long		val;
	struct proc	*p;
	struct pte	apte;

	printf( "for Pid " );
	if ( ( val = getnum( 10 ) ) < 0 )
	{
		if ( lastchar == '\0' )
		{
			val = u.u_procp->p_pid;
			goto skip;
		}
		printf( "Invalid pid.\n" );
		help( UDOT_CMD );
		return;
	}

	/*
	** try to match the pid
	*/
	for ( p = &proc[ 0 ]; p < procNPROC; p++ )
		if ( p->p_pid == val )
			break;

	if ( p == procNPROC )
	{
		printf( "Process with given pid does not exist.\n" );
		return;
	}

skip:
 switch ( act )
 {
    case DISPLAY:
	if ( ! ( p->p_flag & SLOAD ) )
	{
		printf( "Process is swapped out (udot not memory resident)\n" );
		return;
	}
	apte = *( p->p_addr );
	*(u_long *)&apte |= PG_V;
	setpte( UDOT_VBASE,KCX, &apte );
	printf( "Saved fp: %x pc: %x\n", u.u_rsave[ 10 ], u.u_rsave[ 12 ] );
	break;

    case EDIT:
	printf( "Editing of a udot area is not implemented.\n" );
	break;

    default:
	help( UDOT_CMD );
	break;
 }
}

/*
** this structure holds the map name and the variable which contains the
** map info.
*/
extern	struct map malloc_map[];
static struct mapinfo
{
	char		*mpi_name;
	struct map	**mpi_map;
} mapinfo[] =
{
	{ "cmap",	&argmap			},
	{ "swapmap",	&swapmap		},
	{ "argmap",	&argmap			},
	{ (char *)NULL,	(struct map **)NULL	}
};

/* size of a mapinfo entry */
#define	MAPWSIZE	((sizeof (struct mapinfo))/(sizeof (char *)))
#define	CMAPINDX	0

/*
** mapact
**   perform the actions necessary to display/edit various maps
*/
static
mapact( act )
int	act;	/* action to perform */
{
	char		b[ 20 ];
	register int	i;
	int		indx;
	struct mapent	*mp;
	struct map	*mapptr;

	/*
	** prompt for map name and get reply
	*/
	printf( "for " );
	gets( b );

	/*
	** check if a valid map name
	*/
	indx = checked( b, &mapinfo[ 0 ].mpi_name, MAPWSIZE );
	if ( indx == -1 )
	{
		printf( "Unknown map specified.\n" );
		printf( " choices are: " );
		for ( i = 0; mapinfo[ i ].mpi_name; i++ )
			printf( "%s ", mapinfo[ i ].mpi_name );
		printf( "\n" );
		return;

	}

	mapptr = *( mapinfo[ indx ].mpi_map );

 switch ( act )
 {
    case DISPLAY:
	/*
	** core map must be displayed in a different format
	*/
	if ( indx == CMAPINDX )
	{
		register struct cmap	*c = cmap;

printf( "maxmem/freemem: %d/%d  maxfree/desfree: %d/%d   firstfree: %d\n",
	maxmem, freemem, maxfree, desfree, firstfree );

	printf ( "cmap: %08x ncmap: %d  ecmap: %08x ecmx: %d\n",
		 cmap, ncmap, ecmap, ecmx );
printf("         NUM NEXT PREV LW PAGE IFGTY NDX   PF");

		for ( i = 0; i < ncmap; i++, c++ )
		{
printf( "\n%08x %3d %4d %4d %1d%1d %4x %1d%1d%1d%2d %3d %04x",
		c, i, c->c_next, c->c_prev, c->c_lock,
		c->c_want, c->c_page, c->c_intrans,
		c->c_free, c->c_gone, c->c_type,
		c->c_ndx, i ? cmtopg(i) : 0xffff );
			if ( ( ( i % 20 ) == 0 ) && i )
			{
				if (dbgquit())
					break;
			}
		}
		printf ( "\n" ) ;
		return;
	}

	mp = (struct mapent *)( mapptr + 1 );
	printf( "Map size: %d\n", i = ( mapptr->m_limit - mp ) );

	printf( " Size  @  Loc\n" );

	for ( ; i; i--, mp++ )
	{
		printf( "%5d    %04x\n", mp->m_size, mp->m_addr );
		if ( mp->m_size == 0 )
			break;
	}
	break;

    case EDIT:
	printf( "Editing of maps is not implemented.\n" );
	break;

    default:
	help( MAP_CMD );
	break;
 }
}

/*
** textact
**   perform the actions necessary to display info about shared texts
*/
static
textact( act )
int	act;	/* the action to perform	*/
{
	register struct text *xp;

	printf( "\n" );

 switch ( act )
 {
    case DISPLAY:
printf(
"NDX       XP    PT SIZ    CADDR INUM RSS SWRSS REF LOAD POIP SLP CMX FLG\n" );

#ifdef	DBGTXT
printf(
"  DADDR\n" );
#endif
	for ( xp = text; xp < textNTEXT; xp++ )
	{
		if (xp->x_iptr == NULL)
			continue;
printf(
"%3d %08x %5d %3d %08x %4d %3d %5d %3d %4d %4d %3d %3d %s\n",
		    xp - text, xp, xp->x_ptdaddr, xp->x_size,
		    xp->x_caddr, xp->x_iptr->i_number, xp->x_rssize,
		    xp->x_swrss, xp->x_count, xp->x_ccount,
		    xp->x_poip, xp->x_slptime,
		    xp->x_cmx, textflgs(xp->x_flag));
#ifdef	DBGTXT
		{
			register short i;

			for ( i = 0; i < NXDAD; i++ )
				printf( "%3d ", xp->x_daddr[ i ] );
			printf( "\n" );
		}
#endif
	}
	break;

    case EDIT:
	printf( "Editing of shared text entries is not implemented.\n" );
	break;

    default:
	help( TXT_CMD );
	break;
 }
}

/*
** the following taken from slp.c (referenced for display queues).
*/
#define	NHSQUE		64	/* must be power of 2 */
#define	sqhash(X)	(&hsque[(short) (((int)(X) >> 3) & (NHSQUE-1))])
extern struct proc 	*hsque[NHSQUE];
extern char		sched_sleeping,
			runrun,
			curpri;
extern struct proc	*runq;

/*
** quact
**   perform the actions necessary to display the run and sleep queues
*/
static
quact( act )
int	act;
{
	char		c;
	struct proc	*cursor,	/* traces q		*/
			*follower;	/* detects cycles	*/
	int		everyother;	/* counts everyother link*/
	register int	bucket;

	if ( act == EDIT )
	{
		printf( "Editing of queues is not implemented.\n" );
		return;
	}

	/*
	** prompt for which queue and get the reply
	*/
	printf( "of " );
	c = dbgetchar();
	printf( "%c", c );

 switch ( c )
 {
    case 'r':
	printf( "unning procs:\n" );
	printf( "\ncurproc: %x u_procp: %x (pid: %d) sched_sleeping: %d curpri:%d\n",
		0xffff, u.u_procp, u.u_procp->p_pid,
		sched_sleeping, curpri );
	cursor = follower = runq;
	everyother = 0;
	while ( cursor )
	{
		printf( " %d->", cursor - proc );
		cursor = cursor->p_link;
		if ( everyother & 1 )
			/*
			** follower pusues at half speed
			*/
			follower = follower->p_link;
		if ( follower == cursor )
			break;
		everyother++;
	}
	if ( cursor )
		printf( " CYCLE DETECTED\n" );
	else
		printf( "NULL\n" );
	break;

    case 's':
	printf( "leeping procs:\n" );
	printf( "\n" );
	for ( bucket = 0; bucket < NHSQUE; bucket++ )
	{
		if ( cursor = hsque[ bucket ] )
		{
			printf( "Bucket #%d: ", bucket );
			while ( cursor )
			{
				printf( " %d->", cursor - proc );
				cursor = cursor->p_link;
			}
			printf( "NULL\n" );
		}
	}
	break;

   default:
	printf( "\nInvalid queue TYPE.\n" );
	help( QU_CMD );
	break;
 }
}

#ifdef	NOTDEF
/*
** togpr
**   utility routine to symbolically toggle flags
*/
static
togpr( no )
int	no;
{
	char	needcomma = 0,
		needparen  = 0;

	printf( "%s (%x) ", toginfo[ no ].tg_name,
		(*toginfo[ no ].tg_flg ) & 0xf );

	if ( (*toginfo[ no ].tg_flg ) & TRACE_DBG )
	{
		needcomma = needparen = 1;
		printf( "(trace" );
	}
	if ( (*toginfo[ no ].tg_flg ) & BREAK_DBG )
	{
		if ( needcomma )
			printf( "," );
		else
			printf( "(" );
		printf( "break)" );
		needparen = 0;
	}
	if ( needparen )
		printf( ")" );
}

/*
** togset
**   utility routine to set a toggle
*/
static
togset( no )
int	no;
{
	long	val;

	if ( ( val = getnum( 16 ) ) == -1 )
		return;
	*toginfo[ no ].tg_flg = (char)val;
}
#endif

/*
** mdump
**   utility routine to display memory.  This routine will display
**   information past a page boundary.
*/
static
mdump( sadr, vadr )
u_short	*sadr,
	*vadr;
{
	register int	i,
			j,
			k;
	register char	c;
	char		buf[ 17 ];

	i = (u_long)sadr & PGOFSET;
    for ( j = 0;; j = 0 )
    {
	for (; j < 12 && i < NBPG; j++, i += 16, sadr += 8, vadr += 8 )
	{
		/*
		** convert the bytes to ascii printable characters
		*/
		for ( k = 0; k < 16; k++ )
		{
			c = *( ( (u_char *)vadr ) + k );
			if ( ( c < 32 ) || ( c >= 127 ) )
				buf[ k ] = '.';
			else
				buf[ k ] = c;
		}
		buf[ k ] = 0;

		/*
		** now print the words and ascii printable representation
		*/
		printf("\n%08x: %04x %04x %04x %04x %04x %04x %04x %04x %s",
				sadr,
				*(vadr + 0), *(vadr + 1),
				*(vadr + 2), *(vadr + 3),
				*(vadr + 4), *(vadr + 5),
				*(vadr + 6), *(vadr + 7),
				buf);
	}
	printf( " (q/<cr>)? " );
	buf[ 0 ] = dbgetchar();
	printf( "\n" );
	if ( buf[ 0 ] == 'q' || i == NBPG )
		break;
    }

}

/*
** medit
**   utility routine to edit memory.  This routine will not go past
**   a page boundary.
*/
static
medit( sadr, vadr )
u_short	*sadr,
	*vadr;
{
	register int	i;
	long		val;

	i = (u_long)sadr & PGOFSET;	/* calc i to be bytes within page  */
	while ( i >= 0 && i < NBPG )
	{
		printf( "%08x: %04x => ", sadr, *vadr );
		val = getnum ( 16 );
		if ( val == -1 )
		{
			switch ( lastchar )
			{
			   case 'q':
				return;

			   case '.':
				continue;

			   case '^':
				i -= 2;
				sadr -= 1; vadr -= 1;
				break;

			   case '\n':
			   default:
				i += 2;
				sadr += 1; vadr += 1;
				break;
			}
		}
		else
		{
			*vadr = (u_short)val;
			i += 2;
			sadr += 1; vadr += 1;
		}

	}
}

/*
** match
**   utility routine to the name against the secondary arguments for
**   display and edit. Trys to match the smallest subset of the name -
**   if no match rings the bell waiting for more input.
*/
static
match()
{
	register int	indx;
	register int	inpcnt;
	char		buf[ 20 ];
	register char	*ptr = buf;

	while ( 1 )
	{
		*ptr = dbgetchar();
		if ( *ptr == BS )
		{
			if ( ptr > &buf[ 0 ] )
			{
				printf( "\b \b" );
				ptr--;
			}
			continue;
		}
		printf( "%c", *ptr );
		*( ptr + 1 ) = '\0';
		if ( ( indx = checked( buf, &mods[ 0 ].mod_name, MODWSIZE ) ) >=
		     0 )
			break;
		printf( "%c", BELL );
		ptr++;
		if ( ptr >= &buf[ 19 ] )
		{
			printf( "\nInput string too long; try again: " );
			ptr = buf;
		}
	}
	inpcnt = ptr - buf + 1;
	printf( "%s ", &mods[ indx ].mod_name[ inpcnt ] );
	return ( indx );
}

/*
** checked
**   utility routine to check if the arg buf matches any of the strings
**   in the arg ptr.  Will try to match the smallest subset.
*/
static
checked( buf, ptr, size )
char	*buf,
	**ptr;
int	size;
{
	register int	i;
	register char	*ptr1,
			*ptr2;
	int		cand = 0;
	int		indx = 0;

	for ( i = 0; *ptr; ptr += size, i++ )
	{
		ptr2 = *ptr;
		for ( ptr1 = buf; ( *ptr1 == *ptr2 ) && *ptr1; ptr1++, ptr2++ )
			;
		if ( *ptr1 == '\0' )
		{
			cand++;
			indx = i;
		}
	}
	if ( cand == 1 )
		return ( indx );
	return ( -1 );
}

/*
** getnum
**   utility routine for reading in a number in the supplied base (10 or 16)
**   and converts it from ascii to a numeric value.
*/
static long
getnum( base )
int	base;
{
	char		buf[ 20 ];
	register char	*ptr = buf;
	register int	charval;
	long		v;
	int		grody;

	v = 0;
	grody = 0;
	if ( base == 16 )
		printf( "0x" );
	gets( buf );
	lastchar = buf[ 0 ];
	if ( buf[ 0 ] == '\0' )
		return ( -1 );

	for ( ; *ptr; ptr++ )
	{
		if ( base == 16 && ( *ptr >= 'A' && *ptr <= 'F' ) )
			charval = 10 + ( *ptr - 'A' );
		else
		if ( base == 16 && ( *ptr >= 'a' && *ptr <= 'f' ) )
			charval = 10 + ( *ptr - 'a' );
		else
		if ( *ptr >= '0' && *ptr <= '9' )
			charval = *ptr - '0';
		else
		{
			if ( grody )
				return ( v );
			else
				return ( -1 );
		}
		grody++;
		if ( base == 16 )
			v = ( v << 4 ) + charval;
		else
			v = ( v * 10 ) + charval;
	}
	return ( v );
}

/*
** procflgs
**   utility routine for printing symbolically the flags associated with
**   a process.
*/
static char
procflgs( state )
register char	state;
{
	switch ( state )
	{
	  case SSLEEP:
		return 'S';

	  case SWAIT:
		return 'W';

	  case SRUN:
		return 'R';

	  case SIDL:
		return 'I';

	  case SZOMB:
		return 'Z';

	  case SSTOP:
		return 'T';

	  default:
		if ( state < 10 )
			return state + '0';
		else
			return '?';
	}
}

/*
** textflgs
**   utility routine for printing symbolically the flags associated with
**   a shared text entry.
*/
static char *
textflgs( f )
register char f;
{
	static char	b[ 8 ];
	register char	*bp = b;

	if ( f & XTRC )
		*bp++ = 'T';
	if ( f & XWRIT )
		*bp++ = 'W';
	if ( f & XLOAD )
		*bp++ = 'L';
	if ( f & XLOCK )
		*bp++ = 'X';
	if ( f & XWANT )
		*bp++ = 'W';
	if ( f & XERROR )
		*bp++ = 'E';
	if ( f & XSAVE )
		*bp++ = 'S';
	*bp = 0;
	return b;
}

/*
** ptabledmp
**   utility routine for dumping the entire process table.
*/
static
ptabledmp()
{
	register struct proc	*p;

printf( "\nPROCP      FLAGS S UID   PID  PPID PRI NIC     ADDR SIZE    WCHAN     TEXT\n" );
printf( "        P0BR   CX:TEXT  CX:STACK\n" );

	for ( p = proc; p < procNPROC; p++ )
	{
		if ( p->p_stat == 0 )
			continue;
printf( "%08x %07x %c %3d %5d %5d %3d %3d %08x %4d %08x %08x\n", p,
	p->p_flag, procflgs( p->p_stat ), p->p_uid, p->p_pid, p->p_ppid,
	p->p_pri, p->p_nice, p->p_addr, p->p_dsize + p->p_ssize, p->p_wchan,
	p->p_textp );
printf( "    %08x %04x/%04x %04x/%04x\n\n", p->p_p0br, p->p_cxtdnum, p->p_cxbsize,
	p->p_cxsnum, p->p_cxssize );
	}

}

#define	HELPS_SZ	(sizeof helps/(sizeof (struct helps)))
#define	SHELPS_SZ	(sizeof shelps/(sizeof (struct helps)))

/*
** the struct that holds the help messages for usage strings as well as
** general help
*/
struct helps
{
	char	*h_msg;
	short	h_lines;
};

/*
** Contains all the help messages we print
*/
struct helps	helps[] = {
{ "\
e[dit]    r[egs]\n\
d[isplay] r[egs]		Edit or display information about the current\n\
				process (includes saved registers) and the\n\
				processor board registers.\n",
	5
},

{ "\
e[dit]    pm[em] ADDR\n\
d[isplay] pm[em] ADDR		Edit or display physical memory starting at\n\
				the given hex ADDR.\n",
	4
},

{ "\
e[dit]    k[mem] ADDR\n\
d[isplay] k[mem] ADDR		Edit or display kernel memory starting at\n\
				the given hex ADDR.  The kernel\n\
				segment does not need to be specified\n",
	5
},

{ "\
e[dit]    pr[oc] PID\n\
d[isplay] pr[oc] PID		Edit or display the process that has the\n\
				pid specified as decimal PID. If an '*'\n\
				is used instead of a pid then a quick dump\n\
				of the entire process table is printed; if\n\
				no PID is given then the current process\n\
				will be referenced.\n",
	8
},

{ "\
e[dit]    s[tack] ADDR\n\
d[isplay] s[tack] ADDR		Edit or display a stack backtrace starting\n\
				at the given hex ADDR. The kernel segment\n\
				does not need to be specified.\n",
	5
},

{ "\
e[dit]    pt[e] TYPE VALUE\n\
d[isplay  pt[e] TYPE VALUE	Edit or display a hardware page table entry.\n\
				The beginning pte is specified via TYPE:\n\
				i[ndex] or c[ontext]; and VALUE which is the\n\
				number of the pte in the specified TYPE.\n",
	6
},

{ "\
e[dit]    to[ggle] NAME\n\
d[isplay] to[ggle] NAME		Edit or display the toggle given by NAME.\n\
				If the toggle NAME is '*' or '-' the new\n\
				toggle value will be applied to all toggles.\n\
				Toggle values allow you to trace (1) or\n\
				break (2) a predefined locations in the code.\n\
				Legal toggle names can be found by requesting\n\
				an illegal toggle name.\n",
	9
},

{ "\
e[dit]    u[dot] PID\n\
d[isplay] u[dot] PID		Edit or display the udot area of the process\n\
				with the given pid. If no PID is given the\n\
				udot of the current process is referenced.\n",
	5
},

{ "\
e[dit]    m[ap] NAME\n\
d[isplay] m[ap] NAME		Edit or display the map given by NAME. Legal\n\
				map names can be found by specifying an\n\
				illegal one.\n",
	5
},

{ "\
e[dit]    te[xt]\n\
d[isplay] te[xt]		Edit or display the currently active shared\n\
				text processes.\n",
	4
},

{ "\
e[dit]    q[ueue] TYPE\n\
d[isplay] q[ueue] TYPE		Edit or display the queue of TYPE.  TYPE can\n\
				be r[unning] for the run queue or s[leeping]\n\
				for the sleep queue.\n",
	5
},

{ "\
d[isplay] i[node] a[t] ADDR	Display an inode at address ADDR.\n\
d[isplay] i[node] TYPE		Edit or display an inode table of TYPE.  TYPE\n\
				can be f[reelist] for the inode freelist or\n\
				t[able] for the inode table.\n",
	5
},

{ "\
h[elp]|?			Print these help messages.\n",
	2
},

{ "\
c[ontinue]			Continue after a toggle has indicated break.\n",
	2
},

{ "\
r[eboot]			Reboot the system.\n",
	2
}
};

/*
** help
**   print help messages.  If 'cmd' has a value, we print the string
**   as a usage message, otherwise we print the help info.
*/
help( cmd )
register short	cmd;
{
	register int	i;
	register int	lines;

	if ( cmd >= 0 )
	{
		printf( "Usage:\n" );
			printf( "%s", helps[ cmd ].h_msg );
	}
	else
	{
		lines = 2;
		printf( "Kernel Debugging Commands:\n" );
		for ( i = 0; i < HELPS_SZ; i++ )
		{
			lines += helps[ i ].h_lines;
			if ( lines > 20 )
			{
				printf( "q/<cr>? " );
				if ( dbgetchar() == 'q' )
				{
					printf( "\n" );
					return;
				}
				lines = 0;
				printf( "\n" );
			}
			printf( "%s", helps[ i ].h_msg );
			printf( "\n" ); lines++;
		}
	}
}

bufact(act)
	int act;
{
	register int i;
	register struct buf *bp;

	if (act == EDIT) {
		printf( "Editing of buffers is not implemented.\n" );
		return;
	}

printf("\n     buf flags  dev blkno len     addr      pte bcount iobase");
	bp = buf;
	for (i = 0; i < nbuf; i++, bp++) {
		printf("\n%8x %5x %4x %5d %3d %8x %8x %6d %6x",
			    bp, bp->b_flags, (unsigned short)bp->b_dev,
			    bp->b_blkno, bp->b_length, bp->b_un.b_addr,
			    bp->b_memaddr, bp->b_bcount, bp->b_iobase);
		if (i && ((i % 23) == 0)) {
			if (dbgquit())
				break;
		}
	}
	printf("\n");
}

inoact(act)
	int act;
{
	char c;
	register struct inode *ip;

	if (act == EDIT) {
		printf( "Editing of inodes is not implemented.\n" );
		return;
	}

	c = dbgetchar();
	switch (c) {
	  case 'a':
		printf("at ");
		ip = (struct inode *) getnum(16);
		if (ip != (struct inode *)-1)
			dump1_inode(ip);
		break;
	  case 'f':
		printf("freelist\n");
		dump_ifree();
		break;
	  case 't':
		printf("table\n");
		dump_i();
		break;
	}
}

dump_ifree()
{
    register int nfree;
    register struct inode *ip;

    nfree = 0;
    printf("ifreepool $%x\n",&ifreepool);
    for( ip = ifreepool.il_fforw; ip != (struct inode *) &ifreepool;
	 ip = ip->i_fforw )
    {
	if( !(ip->i_fforw->i_fback == ip && ip == ip->i_fback->i_fforw) )
	    printf("broken chain! $%x - $%x - $%x\n"
		    ,ip->i_fforw->i_fback,ip,ip->i_fback->i_fforw);
	nfree++;
	printf(" $%x: $%x",ip,ip->i_flag);
    }
    printf(" -- %d free inodes\n",nfree);
}

dump_i()
{
    register struct inode *ip;
    register int nused;
    register int i;

    nused = 0;
    ip = inode + 0;
    for( i = ninode; --i >= 0; ip++ )
    if( ip->i_count != 0 )
    {
	nused++;
	printf(" $%x: $%x",ip,ip->i_flag);
    }
    printf(" -- %d used inodes\n",nused);
}

dump1_inode(ip)
	register struct inode *ip;
{
	printf("$%x: hback=$%x hforw=$%x fback=$%x fforw=$%x\n",
	    ip, ip->i_hback, ip->i_hforw, ip->i_fback, ip->i_fforw);
	printf(" flag=$%x size=%d", ip->i_flag, ip->i_size);
	printf(" count=%d dev=$%x", ip->i_count, ip->i_dev);
	printf(" number %d",ip->i_number);
	printf(" fstyp %d\n",ip->i_fstyp);
	printf(" rdev $%x",ip->i_rdev);
	printf(" ftype %o",ip->i_ftype);
	printf(" nlink %d",ip->i_nlink);
	printf(" id %d.%d\n",ip->i_uid, ip->i_gid);
	printf(" fsptr $%x", ip->i_fsptr);
	printf("\n");
	if (ip->i_ftype == IFIFO) {
		register struct pipe_inode *pi = pipe_fsptr(ip);

		printf(" mode %o\n", pi->pi_com.ci_mode);
		printf(" rb={flags=%x ptr=%d ref=%d pq={len=%d,proc=$%x}}\n",
			pi->pi_rb.pb_flags,
			pi->pi_rb.pb_ptr,
			pi->pi_rb.pb_ref,
			pi->pi_rb.pb_pq.pq_length,
			pi->pi_rb.pb_pq.pq_proc);
		printf(" wb={flags=%x ptr=%d ref=%d pq={len=%d,proc=$%x}}\n",
			pi->pi_wb.pb_flags,
			pi->pi_wb.pb_ptr,
			pi->pi_wb.pb_ref,
			pi->pi_wb.pb_pq.pq_length,
			pi->pi_wb.pb_pq.pq_proc);
	}
#if NEFS > 0
	else if (ip->i_fstyp == efs_fstyp) {
		register int i;
		register struct efs_inode *iip;

		iip = efs_fsptr(ip);
		if (iip == NULL)
			return;
		printf(" mode %o",iip->ii_mode);
		if (ip->i_ftype == IFBLK || ip->i_ftype == IFCHR) {
			printf("\n");
			return;
		}
		printf(" #extents %d", iip->ii_numextents);
		if (iip->ii_numindirs > 0) {
			printf(" #indirect %d\n", iip->ii_numindirs);
			printf("indirect? ");
			if (dbgetchar() != 'y') {
				printf("\n");
			} else {
				printf("\n # bn     len");
				for (i = 0; i < iip->ii_numindirs; i++) {
					printf("\n%2d %6d %3d",
					    i, iip->ii_indir[i].ex_bn,
					    iip->ii_indir[i].ex_length);
				}
			}
		}
		if (iip->ii_numextents > 0) {
			printf("\ndirect? ");
			if (dbgetchar() != 'y') {
				printf("\n");
				return;
			}
			printf("\n# bn     len");
			for (i = 0; i < iip->ii_numextents; i++) {
				printf("\n%2d %6d %3d",
					    i, iip->ii_extents[i].ex_bn,
					    iip->ii_extents[i].ex_length);
				if (((i & 7) == 0) && i) {
					if (dbgquit())
						return;
				}
			}
		}
		printf("\n");
	}
#endif
#if NNFS > 0
	else if (ip->i_fstyp == nfs_fstyp) {
		register struct rnode *rp;
		register struct nfsfattr *ap;
		register int flags, i;
		static struct flagmap {
			u_short	bit;
			char	*name;
		} flagmap[] = {
			RLOCKED,	"RLOCKED",
			RWANT,		"RWANT",
			RIOWAIT,	"RIOWAIT",
			REOF,		"REOF",
			RDIRTY,		"RDIRTY",
		};

		rp = vtor(ip);
		if (rp == NULL) {
			return;
		}
		printf(" rnode %d forw=$%x back=$%x ip=$%x\n",
		    rp->r_number, rp->r_forw, rp->r_back, rp->r_ip);
		printf(" fh={fsid=%x fno=%d gen=%x}",
		    rp->r_fh.fh_fsid, rp->r_fh.fh_fno, rp->r_fh.fh_fgen);
		printf (" open=%d flags=", rp->r_open);
		flags = rp->r_flags;
		for (i = 0; i < sizeof(flagmap) / sizeof(flagmap[0]); i++) {
			if (flagmap[i].bit & flags) {
				printf(flagmap[i].name);
				flags &= ~flagmap[i].bit;
				if (flags == 0)
					break;;
				putchar('|');
			}
		}
		printf("\n error=%d lastr=%d cred=%d.%d unlcred=%d.%dn",
		    rp->r_error, rp->r_lastr,
		    rp->r_cred.cr_uid, rp->r_cred.cr_gid,
		    rp->r_unlcred.cr_uid, rp->r_unlcred.cr_gid);
		printf("\n unlname=%s unldip=$%x iocount=%d ncap=%d",
		    rp->r_unlname ? rp->r_unlname : "NULL",
		    rp->r_unldip, rp->r_iocount, rp->r_ncap);
		ap = &rp->r_nfsattr;
		printf("\nnfsattr={\n");
		printf("\t type=%d mode=%o nlink=%d id=%d.%d\n",
		    ap->na_type, ap->na_mode, ap->na_nlink,
		    ap->na_uid, ap->na_gid);
		printf("\t size=%d blocksize=%d rdev=%x blocks=%x\n",
		    ap->na_size, ap->na_blocksize,
		    ap->na_rdev, ap->na_blocks);
		printf("\t fsid=%x nodeid=%d atime=%d.%d\n",
		    ap->na_fsid, ap->na_nodeid,
		    ap->na_atime.tv_sec, ap->na_atime.tv_usec);
		printf("\t mtime=%d.%d ctime=%d.%d\n",
		    ap->na_mtime.tv_sec, ap->na_mtime.tv_usec,
		    ap->na_ctime.tv_sec, ap->na_ctime.tv_usec);
		printf("\t nfsattrtime=%d\n }\n",
		    rp->r_nfsattrtime);
	}
#endif
}
#endif

#ifndef	TRACE
/*ARGSUSED*/
traceact(act)
	int act;
{
	printf("tracing is compiled out\n");
}

#else

#define NTRACEBUF	1000

struct	tracebuf {
	char	*t_msg;				/* pointer to message */
	u_long	t_p0, t_p1, t_p2, t_p3, t_p4;	/* some values */
	time_t	t_time;				/* time stamp */
} tracebuf[NTRACEBUF];

short	tindex;				/* current index into xtbuf[] */

/*
 * Return non-zero if user wants more printout
 */
more(x)
	int x;
{
	if (x && ((x % 20) == 0)) {
		printf(" (q/cr)? ");
		if (dbgetchar() == 'q')
			return (0);
	}
	return (1);
}

/*
 * trace:
 *	- snapshot some values and a message into a circular trace buffer
 */
/*VARARGS1*/
trace(c, p0, p1, p2, p3, p4)
	char *c;
	long p0, p1, p2, p3, p4;
{
	register struct tracebuf *t;
	register int s;

	s = spl7();;
	t = &tracebuf[tindex++];
	if (tindex >= NTRACEBUF)
		tindex = 0;
	t->t_msg = c;
	t->t_p0 = p0;
	t->t_p1 = p1;
	t->t_p2 = p2;
	t->t_p3 = p3;
	t->t_p4 = p4;
	t->t_time = lbolt;
	splx(s);
}

/*
 * Given a set of single character choices, return the index of the matched
 * choice, or -1 if no match
 */
choice(choices)
	register char *choices;
{
	register char ch;
	register int i;

	ch = dbgetchar();
	i = 0;
	while (*choices) {
		if (*choices == ch)
			return (i);
		choices++;
		i++;
	}
	return (-1);
}

/*
 * traceact:
 *	- display results of trace
 */
traceact(act)
	int act;
{
	register struct tracebuf *t;
	register short i, n, nout;
	register int mode;
	char b[50];
	char traceobuf[100];

	if (act) {
		printf("Editing of trace buffers is not supported.\n");
		return;
	}

	printf(" how (search|dump) ");
	switch (choice("sd")) {
	  case 0:			/* search */
		printf("search pattern=");
		gets(b);
		if (b[0] == 0)
			return;
		mode = 0;
		break;
	  case 1:			/* dump */
		printf("dump\n");
		mode = 1;
		break;
	  default:
		printf("?\n");
		return;
	}

	i = tindex - 1;
	if (i < 0)
		i = NTRACEBUF - 1;

	nout = 0;
	printf("\nTrace buffer (in reverse order!) (current index=%d)", i);
	for (n = 0; n < NTRACEBUF; n++) {
		t = &tracebuf[i];
		if (t->t_msg) {
			sprintf(traceobuf, t->t_msg, t->t_p0, t->t_p1,
					   t->t_p2, t->t_p3, t->t_p4);
			if (mode || findpattern(b, traceobuf)) {
				printf("\n%8d\t%s", t->t_time, traceobuf);
				nout++;
				if (!more(nout))
					break;
			}
		}
		if (--i < 0)
			i = NTRACEBUF - 1;
	}
	printf("\n");
}

/*
 * Compare two strings, insuring that for at least the length of the shortest
 * string, that they match.  If the length of the pattern string is larger
 * than the text string, then assume no match.
 */
patmatch(text, pat)
	register char *text;
	register char *pat;
{
	if (strlen(text) < strlen(pat))
		return (0);

	while (*pat) {
		if (*text++ != *pat++)
			return (0);
	}
	return (1);
}

/*
 * Search for the given pattern in the given text, returning non-zero if
 * we have a match, zero otherwise.  Sorry, no regular expressions
 */
findpattern(pat, text)
	register char *pat;
	register char *text;
{
	register char *cp;
	extern char *strchr();

	while (*text) {
		cp = strchr(text, *pat);
		if (cp == NULL)
			return (0);		/* not even possibly there */
		if (patmatch(cp, pat))
			return (1);
		text = cp + 1;
	}
	return (0);
}
#endif	TRACE
