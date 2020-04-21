# undef DEBUG do_debug
# include "pmII.h"
# include "mem.h"
# include "IrisConf.h"
# include "Qglobals.h"
# include "common.h"
# include "duart.h"


# include "dprintf.h"


/*
 * Global system configuration variables
 */
extern char *promversion;
extern short millibuzz;
char InProm = 1;

# ifdef ROOTHACK
char ForceBoot = 1;
# endif ROOTHACK

long MBMemSize = ptoa(N_MBMA_PAGES);
long MBioSize = ptoa(N_MBIO_PAGES);
char *MBMemVA = (char *)ptoa(VIRT_LAST_MBMEM_PAGE-MBMEM_LAST_PAGE);
char *MBMemArea = (char *)ptoa(VIRT_FIRST_MBMA_PAGE);
char *MBioVA = (char *)ptoa(VIRT_FIRST_MBIO_PAGE);
long LocalMemSize;
long MappedMemSize;
long StackPA;
unsigned short switches;


short HSpeeds[] = { 300, 1200, 19200, 9600 };

/*
 * qprom() --
 * called from pm2 startup code.
 * use real switches.  do_debug off.
 */
qprom()
{
    c_qprom((unsigned char)CONFIG_REG,0);
}

/*
 * soft_restart() --
 * for testing.
 * call c_qprom() with the given values for
 * switches and do_debug.
 */
soft_restart(switval,debugval)
    int switval;
    int debugval;
{
    extern char bstart[];

    extern int c_qprom();

    /* these *MUST* be in registers! */
    register long *ip;
    register int zero;
    register int s,d;

    s = switval;
    d = debugval;
    zero = 0;

    /* zero last page of stack+bss *INCLUDING OUR STACK* */
    ip = (long *)ptoa(VIRT_STACKBSS_PAGE);
    while( ip != (long *)ptoa(VIRT_LAST_STACK_PAGE) )
	*ip++ = zero;

    /* call c_qprom() */
    LaunchStack(c_qprom,bstart,3,zero,s,d);
}

static
c_qprom(switval,debugval)
    int switval;
    int debugval;
{
    register int hspeed;

# ifdef ROOTHACK
    if( ForceBoot )
    {
	switval &= ~VERBOSEMASK;
	_commdat->reboot = MAGIC_REBOOT_VALUE;
    }
# endif ROOTHACK

    /* set switches and debugging */
    do_debug = debugval;
    _commdat->config = switches = switval;

    DIAG_DISPLAY(9);
    spl7();

    calibuzz();

    /* if we are NOT a slave processor, reset the multibus */
    if( BOOTENV(switches) != ENV_SLAVE )
	wag();

    if( &InProm < (char *)ptoa(VIRT_LAST_PAGE) )
	InProm = 0;

    /* reset parity, just in case */
    STATUS_REG &= ~ENABLE_PARITY;

    DIAG_DISPLAY(10);

    /* *(unsigned long *)&_commdat->screenx = 0;	XXX */
    *(unsigned short *)&_commdat->imr = 0;
    _commdat->flags = 0;

    /*
     * special hack here.  The autoboot switch being ON means
     * that the default mode for the DC4 board is HIGH
     */
    if( BOOTENV(switches) == (ENV_MONITOR|ENV_NOBOOT) )
	_commdat->flags |= DC_HIGH;

    /*
     * set up keyboard parameter.
     * do as little as possible to configure.
     */
    mapin(VIRT_FIRST_MBIO_PAGE,MBIO_FIRST_PAGE
	    ,PROT_MBIO|PROT_RWX___,N_MBIO_PAGES);
    setkbd();

    /* sign-on message */
    printf("\
PM2/GL%c prom monitor %s\n",ISGL1?'1':ISGL2?'2':'?',promversion);

    hspeed = HSpeeds[HOSTSPEED(switches)];

    DIAG_DISPLAY(11);

    /* Kipp mode */
    if( DONTTOUCH(switches) )
    {
	printf("\
DEBUG MODE\n");
    }
    else
    {
	if( ISMICROSW )
	    TermInit(5);/* 5 to start off - more after initmap() */

	if( VERBOSE(switches) )
	{
	    newline();
	    ScreenNoise();
	    printf("\
switches = 0x%x (hostspeed %d, bootenv %d) buzzcount %d\n",
		    (unsigned short)switches,
		    hspeed,BOOTENV(switches),
		    millibuzz);
	}

	/* size memory */
	DIAG_DISPLAY(12);
	check_memory();
    
	find_multibus_window();

	/*
	 * we may just have destroyed the bss segment,	XXX
	 * so reset the switches			XXX
	/* switches = _commdat->config;			XXX */
    
	DIAG_DISPLAY(13);
	initmap();

	if( ISMICROSW )
	    TermInit(-1);/* now can allocate enough for whole screen */

	/* report the values to the user */
	if( VERBOSE(switches) )
	{
	    printf("\
0x%08x: physical %-10s; MB i/o    [0x%x - 0x%x)\n",
		ptoa(VIRT_FIRST_MBIO_PAGE),
		"-",
		ptoa(MBIO_FIRST_PAGE),
		ptoa(MBIO_LAST_PAGE));
	    printf("\
0x%08x: physical 0x%08x; stack+bss [0x%x - 0x%x)\n",
		ptoa(VIRT_FIRST_STACK_PAGE),
		StackPA,
		ptoa(VIRT_FIRST_STACK_PAGE),
		ptoa(VIRT_LAST_STACK_PAGE));
	    printf("\
0x%08x: physical 0x%08x; MB memory [0x%x - 0x%x)\n",
		MBMemVA+ptoa(_commdat->mblog),
		ptoa(_commdat->mbphys),
		ptoa(_commdat->mblog),
		ptoa(_commdat->mblog+_commdat->nmbpages));
	    printf("\
0x%x bytes of memory, ",LocalMemSize);
	    if( LocalMemSize != MappedMemSize )
		printf("0x%x ",MappedMemSize);
	    printf("mapped contiguous, rwx\n\
bootable memory [0x%x - 0x%x)\n",
		_commend,
		LocalMemSize-ptoa(MEM_CHUNK_PAGES-CHUNK_FIRST_MBMA_PAGE));
	}
    }

    /* enable parity */
    DIAG_DISPLAY(1);
    STATUS_REG |= ENABLE_PARITY;

    setbaud(HOST,hspeed);

    /* init devices */
    DIAG_DISPLAY(2);
    qdevinit( DONTTOUCH(switches) );
    
    /* call main prom routine */
    DIAG_DISPLAY(0);
    main();

    warmboot();
}

/*
 * initmap() --
 * initialize the memory and multibus maps.
 * other code plays with the map as part of
 * startup, but
 *	THIS HERE IS THE DEFINITIVE SETUP FOR NORMAL OPERATION
 * mappings not done here are UNDEFINED.
 *
 * on entry
	- mbio is partially mapped in
	  (enough for printf())
	- text + data + proms etc are mapped in
	- bss + half a page of stack is mapped
	  to a temporary phys page
 *
 * on return
	- mbio is fully mapped in
	- text + data + proms etc unchanged
	- bss + full stack are mapped to their
	  rightful place in the last physical
	  mem chunk (half-megabyte).
	- physical memory is mapped contiguous
	- mbmem is mapped in (mbmalloc() works)
	- the following globals are set:
		LocalMemSize
		MappedMemSize
		StackPA
 *
 * BUG / COMPROMISE
 *	if there is maximal physical memory, the
 *	"contiguous" mapping may conflict with various
 *	temporary uses of the VIRT_MAGIC mapping.
 */
static
initmap()
{
    register short physpage,i;
    register short mappedpages;
    short totalpages,chpages;

    /*
     * 1) map mem as virtually contiguous
     * (assuming mem has already been scanned).
     * find the highest address physical (and
     * logical) chunk for mbio, mbmem, and
     * stack+bss.  careful not to remap it
     * beforehand if there's maximal phys mem.
     */
    physpage = 0;
    mappedpages = 0;
    totalpages = 0;

    for( i = 0; i < MAX_MEM_CHUNKS; i++ )
	if( _commdat->memfound[i] )
	{
	    physpage = i * MEM_CHUNK_PAGES;

	    chpages = MEM_CHUNK_PAGES;
	    if( i == MAX_MEM_CHUNKS-1 )
		chpages = VIRT_FIRST_SPECIAL_PAGE%MEM_CHUNK_PAGES;

dprintf((" mapmem(%x,%x,%x)",mappedpages,physpage,chpages));
	    mapmem(mappedpages,physpage,chpages);

	    mappedpages += chpages;
	    totalpages += MEM_CHUNK_PAGES;
	}

dprintf((" mapout(%x,...%x)",mappedpages,VIRT_FIRST_SPECIAL_PAGE-mappedpages));
    /* unmap remaining virtual space */
    mapin(mappedpages,0,PROT_LOCMEM|PROT_NOACCESS
	    ,VIRT_FIRST_SPECIAL_PAGE-mappedpages);

    /*
     * physpage has been set to the page# of
     * the last chunk; totalpages has been set to
     * the #pages present; mappedpages to the #pages
     * mapped contiguously.
     */

    /*
     * 2) relocate physical stack to the end
     * of the last chunk.
     */
dprintf((" movestackbss(%x)",physpage+CHUNK_STACKBSS_PAGE));
    movestackbss(physpage+CHUNK_STACKBSS_PAGE);

    /* allow access to the whole of the stack+bss */
    mapmem(VIRT_FIRST_STACK_PAGE,physpage+CHUNK_FIRST_STACK_PAGE
	    ,N_STACKBSS_PAGES);

    /*
     * 3) map in some multibus mem.
     * this may involve correcting some of
     * what find_multibus_window() did.
     */
    if( BOOTENV(switches) != ENV_SLAVE )
    {
	_commdat->mbw = physpage;
	_commdat->mbphys = physpage+CHUNK_FIRST_MBMEM_PAGE;
    }
dprintf((" setmbmap(%x,%x,%x)"
,_commdat->mblog,_commdat->nmbpages,_commdat->mbphys));
    setmbmap(_commdat->mblog,_commdat->nmbpages,_commdat->mbphys);
    mapmem(VIRT_FIRST_MBMEM_PAGE,_commdat->mbphys,_commdat->nmbpages);

    /*
     * 4) map in mbio space.
     */
    mapin(VIRT_FIRST_MBIO_PAGE,MBIO_FIRST_PAGE,PROT_MBIO|PROT_RWX___
	    ,N_MBIO_PAGES);

    /*
     * share what is known.
     */
    LocalMemSize = ptoa(totalpages);
    MappedMemSize = ptoa(mappedpages);
    StackPA = ptoa(physpage+CHUNK_FIRST_STACK_PAGE);
}

/*
 * movestackbss() --
 * relocate the stack+bss page to physical newpage.
 * where newpage is the new STACKBSS page.
 * assume only this page is being used at
 * the time of the call.  see BUG / COMPROMISE above.
 * this routine is a good place to look for BUGS!
 */
static
movestackbss(new_phys_stackbss)
    register short new_phys_stackbss;
{
    register short old_phys_stackbss;
    register short magicpage;

    magicpage = mapmagic();

    old_phys_stackbss = PAGEMAP[VIRT_STACKBSS_PAGE];
dprintf((" NEWSTACK($%x) $%x",new_phys_stackbss,VIRT_STACKBSS_PAGE)); 

    /* copy active page of stack+bss */
    mapmem(magicpage,new_phys_stackbss,1);
    bcopy(ptoa(VIRT_STACKBSS_PAGE),ptoa(magicpage),ptoa(1));

    /* stack changes here! */
    PAGEMAP[VIRT_STACKBSS_PAGE] = new_phys_stackbss;

    /* zero out old active page of stack+bss */
    if( old_phys_stackbss != new_phys_stackbss )
    {
	mapmem(magicpage,old_phys_stackbss,1);
	bzero(ptoa(magicpage),ptoa(1));
    }

    unmapmagic();
}
