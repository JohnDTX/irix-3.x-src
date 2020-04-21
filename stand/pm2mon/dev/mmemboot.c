# include "pmII.h"

# undef  DEBUG do_debug
# include "dprintf.h"

# define PROMSTATIC
# define MEMPAGE	ptoa(1)



PROMSTATIC	long mmemoff;

# define DEFMBMEMBOOTADDR	0x300000

int
mmem_open(ext,file)
    char *ext,*file;
{
    mmemoff = DEFMBMEMBOOTADDR;
    if( memstring(ext,file,&mmemoff) < 0 )
	return -1;
    printf("(MBmembooting: 0x%x)\n",mmemoff);
}

int
mmem_close()
{
}

int
mmem_read(_ptr,len)
    char **(_ptr);
    int len;
{
    register short magicpage,mbpage,pageoff,pagerem;

dprintf((" mmemoff $%x",mmemoff));
    /* truncate request to 1 page max */
    pageoff = (unsigned)mmemoff % MEMPAGE;
    pagerem = MEMPAGE - pageoff;
    if( pagerem > len )
	pagerem = len;

    /* map in current page and copy it out */
    magicpage = mapmagic();

    mapin(magicpage,atop(mmemoff),PROT_MBMEM|PROT_RWX___,1);
dprintf((" mapin($%x,$%x...)",magicpage,atop(mmemoff)));
    bcopy(ptoa(magicpage)+pageoff,*_ptr,pagerem);
dprintf((" bcopy($%x,$%x,$%x)"
,ptoa(magicpage)+pageoff,*_ptr,pagerem));

    unmapmagic();

    mmemoff += pagerem;
    return pagerem;
}
