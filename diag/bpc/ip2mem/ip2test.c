/*
 *	Kurt Akeley
 *	12 July 1985
 *
 *	Call ip2diag memtest routines in such a way as to
 *	    1.	Require no changes to those routines, and
 *	    2.	Test all memory, regardless of configuration
 */

#define MEGBITS		((long*)0x33000000)	/* magic SRAM location */
#define PTE(page)	((long*)(0x3B000000 | ((page)<<2)))

extern long globerrors;
extern long lowmem;

long ip2memtest (teststorun, verbose, count)
long teststorun, verbose, count;
{
    long i;
    long megabyte;

    initmemsize ();
    for (megabyte=0; megabyte<32; megabyte+=2) {
	if (*MEGBITS & (1<<megabyte)) {
	    if (megabyte != 0)
		setlowtest (0, 1, megabyte << 20);
	    else
		setlowtest (0, 1, lowmem);
	    sethightest (0, 1, (megabyte << 20) + 0x1FFFFF);
	    memtest (0, 3, teststorun, verbose, count);
	    }
	}
    setgloberrors (1, 0, 0);
    return globerrors;
    }

freshline () {
    printf ("\n");
    }

setpte (pteindex, physpage, protection)
long pteindex, physpage, protection;
{
    *PTE (pteindex) = physpage | (protection << 28);
    }

int getptepage (pteindex)
long pteindex;
{
    return *PTE (pteindex) & 0x1FFF;
    }

