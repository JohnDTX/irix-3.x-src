/*
 *	Kurt Akeley			3/9/83
 *
 *	Attempts to completely test an IRIS bpc board set.
 *	Calls init () before each test; the value of sigplanes restored by
 *	  init () is therefore used.
 *	Config is set individually for each test.
 *
 *	Routines:
 *		long	bpctest (verbose, boardstotest)
 *		short	bitcount (i)
 *
 *	Updates:
 */

#include "console.h"
#include "uctest.h"
#include "ucdev.h"

#define DCTEST		1
#define UCTEST		2
#define BPTEST		4
#define GMEMTEST	8	/* must be true to allow bitplane test */
#define PMEMTEST	16	/* must be true to allow processor mem test */
#define GFTEST		32	/* must be true to allow gf board test */
#define FPTEST		64	/* must be true to allow fpa board test */

extern long globerrors;
long	gferrors;

long	bpctest (verbose, boardstotest)
long	verbose;
long	boardstotest;
{
    /*
     *	Tests are selected based on the board(s) which is(are) being tested.
     */

    long	errors;
    long	errortotal;
    Save	s;

    if (boardstotest & (DCTEST | UCTEST | BPTEST | GFTEST | GMEMTEST))
	save (&s);
    errortotal = 0;
    gferrors = 0;

    if (boardstotest & PMEMTEST) {
#ifdef PM1
	if (verbose)
	    printf ("  Processor memory test not available for PM1\n");
#endif PM1
#ifdef PM2
	errors = memtest (0);
	errortotal += errors;
	if (verbose)
	    printf ("  memtest complete, %d errors\n", errors);
#endif PM2
#ifdef PM3
	errors = ip2memtest (0xFFFF, 0, 1);
	errortotal += errors;
	if (verbose)
	    printf ("  memtest complete, %d errors\n", errors);
#endif PM3
	}

    if (boardstotest & FPTEST) {
#ifdef PM1
	if (verbose)
	    printf ("  Floating-point test not available for PM1\n");
#endif PM1
#ifdef PM2
	if (verbose)
	    printf ("  Floating-point test not available for PM2\n");
#endif PM2
#ifdef PM3
	globerrors = 0;
	fptest (1, 3, 0, 0x800, 0x01);
	errortotal += globerrors;
	if (verbose)
	    printf ("  fptest complete, %d errors\n", globerrors);
	globerrors = 0;
#endif PM3
	}

    if (boardstotest & GFTEST) {
	init (0, 0);
	errors = testall (0, 0xFFFFFFFF);
	/********************
	errortotal += errors;
	********************/
	gferrors += errors;
	if (verbose)
	    printf ("  gftest complete, %d errors\n", errors);
	}

    if ((boardstotest & (DCTEST|UCTEST|BPTEST)) && (boardstotest & GMEMTEST)){
	init (0, 0);
	LDCONFIG (UPDATEA | UPDATEB | DISPLAYA | DISPLAYB);
	errors = testplanes (sigplanes, 0, 0, 0x3f, 0);
	errortotal += errors;
	if (verbose)
	    printf (" bptest complete, display ab, %d errors\n", errors);
	init (0, 0);
	LDCONFIG (UPDATEA | UPDATEB);
	errors = testplanes (sigplanes, 0, 0, 0x3f, 0);
	errortotal += errors;
	if (verbose)
	    printf (" bptest complete, no display, %d errors\n", errors);
	}

    if (boardstotest & (DCTEST | UCTEST | BPTEST)) {
	init (0, 0);
	LDCONFIG (DISPLAYA | DISPLAYB);
	errors = bitcount (stripetest (sigplanes, 0, 40, 0));
	errortotal += errors;
	if (verbose)
	    printf (" stripetest complete, display ab, %d errors\n", errors);
	}

    if (boardstotest & (DCTEST | UCTEST | BPTEST)) {
	init (0, 0);
	LDCONFIG (DISPLAYA | DISPLAYB);
	errors = bitcount (stripetest (sigplanes, 0, 40, 1));
	errortotal += errors;
	if (verbose)
	    printf (" stripetest complete, display ab, sbuf, %d errors\n",
		    errors);
	init (0, 0);
	LDCONFIG (UPDATEA | UPDATEB | DISPLAYA | DISPLAYB);
	errors = colorwetest (0);
	errortotal += errors;
	if (verbose)
	    printf (" color/we test complete, %d errors\n", errors);
	}

    if (boardstotest & (DCTEST | UCTEST)) {
	init (0, 0);
	LDCONFIG (UPDATEA | UPDATEB | DISPLAYA | DISPLAYB);
	errors = fmtest (0, 0x1f, 0);
	errortotal += errors;
	if (verbose)
	    printf (" fmtest complete, %d errors\n", errors);
	}

    if (boardstotest & (DCTEST)) {
	init (0, 0);
	LDCONFIG (UPDATEA | UPDATEB | DISPLAYA | DISPLAYB);
	errors = dcrtest (0);
	errortotal += errors;
	if (verbose)
	    printf (" dcrtest complete, %d errors\n", errors);
	}

    if (boardstotest & (DCTEST)) {
	init (0, 0);
	LDCONFIG (UPDATEA | UPDATEB | DISPLAYA | DISPLAYB);
	errors = maptest (0, 0x1f, 0);
	errortotal += errors;
	if (verbose)
	    printf (" maptest complete, %d errors\n", errors);
	}

    if (boardstotest & (UCTEST)) {
	init (0, 0);
	LDCONFIG (UPDATEA | UPDATEB | DISPLAYA | DISPLAYB);
	errors = linetest (0, 0xffff);
	errortotal += errors;
	if (verbose)
	    printf (" linetest complete, %d errors\n", errors);
	}

    if (boardstotest & (UCTEST)) {
	init (0, 0);
	LDCONFIG (UPDATEA | UPDATEB | DISPLAYA | DISPLAYB);
	errors = viewporttest (0);
	errortotal += errors;
	if (verbose)
	    printf (" viewporttest complete, %d errors\n", errors);
	}

    if (boardstotest & (UCTEST)) {
	init (0, 0);
	LDCONFIG (UPDATEA | UPDATEB | DISPLAYA | DISPLAYB);
	errors = recttest (0);
	errortotal += errors;
	if (verbose)
	    printf (" rectangle test complete, %d errors\n", errors);
	}

    if (boardstotest & (UCTEST)) {
	init (0, 0);
	LDCONFIG (UPDATEA | UPDATEB | DISPLAYA | DISPLAYB);
	errors = chartest (0);
	errortotal += errors;
	if (verbose)
	    printf (" character test complete, %d errors\n", errors);
	}

#ifdef UC4
    if (boardstotest & (UCTEST)) {
	init (0, 0);
	LDCONFIG (UPDATEA | UPDATEB | DISPLAYA | DISPLAYB);
	errors = ddatest (0, 0xffff);
	errortotal += errors;
	if (verbose)
	    printf (" dda test complete, %d errors\n", errors);
	}

    if (boardstotest & (UCTEST|BPTEST)) {
	init (0, 0);
	LDCONFIG (UPDATEA | UPDATEB | DISPLAYA | DISPLAYB);
	errors = pixeltest (0, 0xffff);
	errortotal += errors;
	if (verbose)
	    printf (" pixel test complete, %d errors\n", errors);
	}

    if (boardstotest & (UCTEST)) {
	init (0, 0);
	LDCONFIG (UPDATEA | UPDATEB | DISPLAYA | DISPLAYB);
	errors = traptest (0, 0xffff);
	errortotal += errors;
	if (verbose)
	    printf (" trapezoid test complete, %d errors\n", errors);
	}
#endif UC4

    if (boardstotest & (DCTEST | UCTEST | BPTEST | GFTEST | GMEMTEST))
	restore (&s);
    return (errortotal);
    }



short	bitcount (i)
long	i;
{
    /* Returns the number of '1' bits in i */
    long	bit;
    short	count;

    for (bit=1, count=0; bit != 0; bit<<=1)
	if (i & bit)
	    count += 1;
    return (count);
    }



