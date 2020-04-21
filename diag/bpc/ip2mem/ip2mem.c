/*
 *	Kurt Akeley
 *	3 February 1985
 *	Memory test routines for the IP2
 */

#define MEGBITS		((long*)0x33000000)	/* magic SRAM location */
#define FALSE		0
#define TRUE		1

extern _end;
long lowmem, highmem;		/* always hold correct values */
long lowtest, hightest;		/* specify test range, can be changed */
long globerrors;		/* total errors of all tests */
long readerrors;		/* keep track of error type totals too */
long writeerrors;
long wierderrors;
long rounderrors;

initmemsize () {
    long i, megs;
    for (i=0, megs=0; i<32; i++)
	if (*MEGBITS & (1<<i))
	    megs += 1;
    highmem = (megs << 20) - 1;
    hightest = highmem;
    lowmem = ((long)&_end) & 0xFFFFFFF;
    lowmem = (lowmem | 0xFFC) + 0x1004;	/* save a page for the stack */
    lowtest = lowmem;
    globerrors = 0;
    readerrors = 0;
    writeerrors = 0;
    wierderrors = 0;
    rounderrors = 0;
    }

setlowtest (interactive, args, value)
long interactive, args, value;
{
    if (args == 1)
	lowtest = value;
    if (interactive) {
	printf ("  lowtest: %08x\n", lowtest);
	printf ("  lowmem:  %08x\n", lowmem);
	}
    }

sethightest (interactive, args, value)
long interactive, args, value;
{
    if (args == 1)
	hightest = value;
    if (interactive) {
	printf ("  hightest: %08x\n", hightest);
	printf ("  highmem:  %08x\n", highmem);
	}
    }

setgloberrors (interactive, args, value)
long interactive, args, value;
{
    if (args == 1) {
	globerrors = value;
	readerrors = 0;
	writeerrors = 0;
	wierderrors = 0;
	rounderrors =0;
	}
    printf ("  global errors: %5x\n", globerrors);
    printf ("  read errors:   %5x\n", readerrors);
    printf ("  write errors:  %5x\n", writeerrors);
    printf ("  weird errors:  %5x\n", wierderrors);
    printf ("( round errors:  %5x)\n", rounderrors);
    }

long memtest (interactive, args, teststorun, verbose, count)
long interactive, args, teststorun, verbose, count;
{
    long errors;
    long errortotal;
    short i;
    short test;

    initmap ();
    if (verbose)
	printf ("  Testing memory from %08x to %08x\n", lowtest, hightest);
    for (i=0, errortotal=0; i<count; i++) {
	if (verbose)
	    printf ("  pass %d: ", i+1);
	for (test=0, errors=0; test<32; test++) {
	    if (teststorun & (1<<test))
		errors += domemtest (test, verbose);
	    breakcheck ();
	    }
	errortotal += errors;
	if (verbose)
	    printf (", %d errors, %d total\n", errors, errortotal);
	}
    return errortotal;
    }

long domemtest (test, verbose)
short test;
short verbose;
{
    initerror (10);
    switch (test) {
	case 0:
	    putchar ('0');
	    fillconstant (0);
	    return compconstant (verbose, 0);
	    break;
	case 1:
	    putchar ('1');
	    fillconstant (0xFFFFFFFF);
	    return compconstant (verbose, 0xFFFFFFFF);
	    break;
	case 2:
	    putchar ('2');
	    fillalternate (0, 0xFFFFFFFF);
	    return compalternate (verbose, 0, 0xFFFFFFFF);
	    break;
	case 3:
	    putchar ('3');
	    fillalternate (0x55555555, 0xAAAAAAAA);
	    return compalternate (verbose, 0x55555555, 0xAAAAAAAA);
	    break;
	case 4:
	    putchar ('4');
	    filladdress (FALSE);
	    return compaddress (verbose, FALSE);
	    break;
	case 5:
	    putchar ('5');
	    filladdress (TRUE);
	    return compaddress (verbose, TRUE);
	    break;
	case 6:
	    putchar ('6');
	    fillrotate (1);
	    return comprotate (verbose, 1);
	    break;
	case 7:
	    putchar ('7');
	    fillrotate (0xFFFFFFFE);
	    return comprotate (verbose, 0xFFFFFFFE);
	    break;
	default:
	    return 0;
	    break;
	}
    }

fillconstant (value)
register long value;
{
    register long count;
    register long *addr;
    count = (hightest - lowtest + 1) / 4;
    addr = (long*)lowtest;
    while (count--)
	*addr++ = value;
    }

fillalternate (value0, value1)
register long value0, value1;
{
    register long count;
    register long *addr;
    register long value;
    count = (hightest - lowtest + 1) / 4;
    addr = (long*)lowtest;
    value = value0;
    while (count--) {
	*addr++ = value;
	if (value == value0)
	    value = value1;
	else
	    value = value0;
	}
    }

filladdress (invert)
long invert;
{
    register long count;
    register long *addr;
    count = (hightest - lowtest + 1) / 4;
    addr = (long*)lowtest;
    if (invert)
	while (count--) {
	    *addr = -((long)addr);
	    addr++;
	    }
    else
	while (count--) {
	    *addr = (long)addr;
	    addr++;
	    }
    }
    
fillrotate (value)
register long value;
{
    register long *addr;
    register long count;
    count = (hightest - lowtest + 1) / 4;
    addr = (long*)lowtest;
    while (count--) {
	*addr++ = value;
	if (value & (1<<31))
	    value = (value << 1) | 1;
	else
	    value = value << 1;
	}
    }

long compconstant (verbose, value)
short verbose;
register long value;
{
    register long count;
    register long *addr;
    register long got;
    count = (hightest - lowtest + 1) / 4;
    addr = (long*)lowtest;
    while (count--)
	if ((got = *addr++) != value)
	    printerror (verbose, addr, got, value);
    return geterrors ();
    }

long compalternate (verbose, value0, value1)
short verbose;
register long value0, value1;
{
    register long count;
    register long *addr;
    register long expect;
    register long got;
    count = (hightest - lowtest + 1) / 4;
    addr = (long*)lowtest;
    expect = value0;
    while (count--) {
	if ((got = *addr++) != expect)
	    printerror (verbose, addr, got, expect);
	if (expect == value0)
	    expect = value1;
	else
	    expect = value0;
	}
    return geterrors ();
    }

long compaddress (verbose, invert)
short verbose;
short invert;
{
    register long count;
    register long *addr;
    register long got;
    count = (hightest - lowtest + 1) / 4;
    addr = (long*)lowtest;
    if (invert)
	while (count--) {
	    if ((got = *addr) != -((long)addr))
		printerror (verbose, addr, got, -((long)addr));
	    addr++;
	    }
    else
	while (count--) {
	    if ((got = *addr) != (long)addr)
		printerror (verbose, addr, got, (long)addr);
	    addr++;
	    }
    return geterrors ();
    }

long comprotate (verbose, value)
short verbose;
register long value;
{
    register long count;
    register long *addr;
    register long got;
    count = (hightest - lowtest + 1) / 4;
    addr = (long*)lowtest;
    while (count--) {
	if ((got = *addr++) != value)
	    printerror (verbose, addr, got, value);
	if (value & (1<<31))
	    value = (value << 1) | 1;
	else
	    value = value << 1;
	}
    return geterrors ();
    }

printerror (verbose, address, got, expect)
short verbose;
long address, got, expect;
{
    long retry;
    retry = *(long*)address;
    adderror (verbose);
    if (verbose) {
	freshline ();
	printf ("  ERROR at %08x: got=%08x, expected=%08x, xor=%08x\n",
	    address, got, expect, got^expect);
	printf ("                   retry=%08x, ", retry);
	}
    if (retry == expect) {
	readerrors += 1;
	if (verbose)
	    printf ("READ ERROR\n");
	}
    else if (retry == got) {
	writeerrors += 1;
	if (verbose)
	    printf ("WRITE ERROR\n");
	}
    else {
	wierderrors += 1;
	if (verbose)
	    printf ("WEIRD ERROR\n");
	}
    if (verbose) {
	long virtpage, physpage;
	virtpage = address >> 12;
	physpage = getptepage (virtpage);
	if (virtpage != physpage) {
	    printf ("                   Physical address is %08x\n",
		(address & 0xF0000FFF) | (physpage <<12));
	    }
	}
    }

initmap () {
    /* The 0x200 top map pages are now used to map kernel space to
     *   2 meg of physical memory.  This routine uses the bottom half
     *   of the map to directly map the entire physical memory space
     *   (32meg).
     * The stack is currently kept in physical memory page 0x1FF.
     *   We alter our direct map so that the entry that would point
     *   to this physical page points instead to the page above the
     *   top of the text space.  This page was reserved by mucking
     *   with the lowmem value in initmem().
     */
    long count;
    for (count=0; count < 0x2000; count++)
	setpte (count, count, 3);
    setpte (0x1FF, (lowmem>>12)-1, 3);
    }
