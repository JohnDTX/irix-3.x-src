# include "common.h"
# include "pmII.h"

/*
 * globals used by standalone library,
 * hopefully left correct by proms!
 */
long MBMemSize = ptoa(N_MBMA_PAGES);
long MBioSize = ptoa(N_MBIO_PAGES);
char *MBMemVA = (char *)ptoa(VIRT_LAST_MBMEM_PAGE-MBMEM_LAST_PAGE);
char *MBMemArea = (char *)ptoa(VIRT_FIRST_MBMA_PAGE);
char *MBioVA = (char *)ptoa(VIRT_FIRST_MBIO_PAGE);

/*
 * globals used by standalone library,
 * initialized prom-style.
 */
long LocalMemSize;
long MappedMemSize;
long StackPA;
unsigned short switches;


int errno;

_c_startup()
{
    switches = _commdat->config;

    mapin(VIRT_FIRST_MBIO_PAGE,MBIO_FIRST_PAGE
	    ,PROT_MBIO|PROT_RWX___,N_MBIO_PAGES);
    setmbmap(_commdat->mblog,_commdat->nmbpages,_commdat->mbphys);
    mapmem(VIRT_FIRST_MBMEM_PAGE,_commdat->mbphys,_commdat->nmbpages);

    calibuzz();

    sa_memchk();

    setkbd();

    main(0,0);
}

/*
 * sa_memchk() --
 * intialize for mbmalloc().
 * see initmap() in the proms.
 */
static
sa_memchk()
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

	    mappedpages += chpages;
	    totalpages += MEM_CHUNK_PAGES;
	}

    /*
     * physpage has been set to the page# of
     * the last chunk; totalpages has been set to
     * the #pages present; mappedpages to the #pages
     * mapped contiguously.
     */

    /*
     * share what is known.
     */
    LocalMemSize = ptoa(totalpages);
    MappedMemSize = ptoa(mappedpages);
    StackPA = ptoa(physpage+CHUNK_FIRST_STACK_PAGE);
}

/* drag in standalone versions for sure */
static
sa_dragin()
{
    berr();

    getchar(); putchar();
}

user_trapF()
{
    if( ISMICROSW )
    {
	TermComm();
	ScreenComm();
    }

    asm("	trap #0xF	");

    if( ISMICROSW )
    {
	extern short DumberTerm;

	DumberTerm = 0;
	glcursor(1);
    }
}
