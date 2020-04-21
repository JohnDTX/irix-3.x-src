/*
 *	Kurt Akeley
 *	22 October 1984
 *
 *	Test the dda hardware.  These tests are intended to find faults in
 *	  the dda bitslice.  They are NOT indended to find errors in the
 *	  trapezoid fill algorithm or hardware.  It is impossible, however,
 *	  to do the necessary dda addition without invoking the FILLTRAP
 *	  command.  This is done only in the add() subroutine, and is done
 *	  only for a degenerate trapezoid.
 *
 *	Functions:
 *		long ddatest (verbose, teststorun)
 *
 *	Static functions:
 *		int db (i)
 *
 *	Static procedures:
 *		testaddress (verbose, saf, sai, eaf, eai)
 *		testaddition (verbose, saf, sai, eaf, eai, sdf, sdi, edf, edi)
 *		add ()
 */

#ifdef UC4

#include "uctest.h"
#include "ucdev.h"
#include "console.h"

long ddatest (verbose, teststorun)
boolean verbose;	/* enables error reporting */
short teststorun;	/* individual bits enable various tests */
{
    short i;
    Save s;

    initerror (2);
    save (&s);

    /* address register 0 test */
    if (teststorun & 1) {
	if (verbose)
	    putchar ('0');
	restore (&s);
	breakcheck ();
	testaddress (verbose, 0, 0, 0, 0);
	}

    /* address retister 1 test */
    if (teststorun & 2) {
	if (verbose)
	    putchar ('1');
	restore (&s);
	breakcheck ();
	testaddress (verbose, 0xfff, 0xfff, 0xfff, 0xfff);
	}

    /* rotate 1s through the address registers */
    if (teststorun & 4) {
	if (verbose)
	    putchar ('2');
	restore (&s);
	breakcheck ();
	for (i=0; i<12; i++) {
	    testaddress (verbose, db(i), db(i+1), db(i+2), db(i+3));
	    }
	}

    /* delta register 0 test */
    if (teststorun & 8) {
	if (verbose)
	    putchar ('3');
	restore (&s);
	breakcheck ();
	testaddition (verbose, 0, 0, 0, 0, 0, 0, 0, 0);
	}

    /* delta register 1 test */
    if (teststorun & 16) {
	if (verbose)
	    putchar ('4');
	restore (&s);
	breakcheck ();
	testaddition (verbose, 0, 0, 0, 0, 0xfff, 0xfff, 0xfff, 0xfff);
	}

    /* rotate 1s through delta registers */
    if (teststorun & 32) {
	if (verbose)
	    putchar ('5');
	restore (&s);
	breakcheck ();
	for (i=0; i<12; i++) {
	    testaddition (verbose,0,0,0,0,db(i),db(i+1),db(i+2),db(i+3));
	    }
	}

    /* test carry overflow */
    if (teststorun & 64) {
	if (verbose)
	    putchar ('6');
	restore (&s);
	testaddition (verbose, 1, 1, 1, 1, 0xfff, 0xfff, 0xfff, 0xfff);
	}
    restore (&s);
    return geterrors ();
    }

static testaddress (verbose, saf, sai, eaf, eai)
boolean verbose;
short saf, sai, eaf, eai;
{
    short gotsaf, gotsai, goteaf, goteai;
    /* mask the arguments */
    saf &= 0xfff;
    sai &= 0xfff;
    eaf &= 0xfff;
    eai &= 0xfff;
    /* load the hardware address registers */
    LDDDASAF (saf)
    LDDDASAI (sai)
    LDDDAEAF (eaf)
    LDDDAEAI (eai)
    /* read the hardware address registers */
    gotsaf = (*UCBufferAddr(UC_DDASAF)) & 0xfff;
    gotsai = (*UCBufferAddr(UC_DDASAI)) & 0xfff;
    goteaf = (*UCBufferAddr(UC_DDAEAF)) & 0xfff;
    goteai = (*UCBufferAddr(UC_DDAEAI)) & 0xfff;
    /* report any errors */
    if (saf != gotsaf || sai != gotsai || eaf != goteaf || eai != goteai) {
	if (verbose) {
	    printf ("dda address register test\n");
	    printf ("            saf sai eaf eai\n");
	    printf ("  written   %3x %3x %3x %3x\n", saf, sai, eaf, eai);
	    printf ("  received  %3x %3x %3x %3x\n", gotsaf, gotsai, goteaf,
		goteai);
	    printf ("  xor       %3x %3x %3x %3x\n", saf^gotsaf, sai^gotsai,
		eaf^goteaf, eai^goteai);
	    }
	adderror (verbose);
	}
    }

static testaddition (verbose, saf, sai, eaf, eai, sdf, sdi, edf, edi)
boolean verbose;
short saf, sai, eaf, eai;
short sdf, sdi, edf, edi;
{
    short gotsaf, gotsai, goteaf, goteai;
    short sumsaf, sumsai, sumeaf, sumeai;
    /* mask the arguments */
    saf &= 0xfff;
    sai &= 0xfff;
    eaf &= 0xfff;
    eai &= 0xfff;
    sdf &= 0xfff;
    sdi &= 0xfff;
    edf &= 0xfff;
    edi &= 0xfff;
    /* load the hardware address registers */
    LDDDASAF (saf)
    LDDDASAI (sai)
    LDDDAEAF (eaf)
    LDDDAEAI (eai)
    /* load the hardware delta registers */
    LDDDASDF (sdf)
    LDDDASDI (sdi)
    LDDDAEDF (edf)
    LDDDAEDI (edi)
    /* add the deltas to the addresses */
    add ();
    /* read the sums (in the address registers) back */
    gotsaf = (*UCBufferAddr(UC_DDASAF)) & 0xfff;
    gotsai = (*UCBufferAddr(UC_DDASAI)) & 0xfff;
    goteaf = (*UCBufferAddr(UC_DDAEAF)) & 0xfff;
    goteai = (*UCBufferAddr(UC_DDAEAI)) & 0xfff;
    /* compute the expected sums - handle overflow correctly */
    sumsaf = (saf + sdf) & 0xfff;
    sumsai = (sai + sdi + (((saf+sdf)&0x1000) ? 1 : 0)) & 0xfff;
    sumeaf = (eaf + edf) & 0xfff;
    sumeai = (eai + edi + (((eaf+edf)&0x1000) ? 1 : 0)) & 0xfff;
    /* report any errors */
    if (sumsaf != gotsaf || sumsai != gotsai ||
	    sumeaf != goteaf || sumeai != goteai) {
	if (verbose) {
	    printf ("dda addition test\n");
	    printf ("            sxf sxi exf exi\n");
	    printf ("  address   %3x %3x %3x %3x\n", saf, sai, eaf, eai);
	    printf ("  delta     %3x %3x %3x %3x\n", sdf, sdi, edf, edi);
	    printf ("  sum       %3x %3x %3x %3x\n", sumsaf, sumsai, sumeaf,
		sumeai);
	    printf ("  received  %3x %3x %3x %3x\n", gotsaf, gotsai, goteaf,
		goteai);
	    printf ("  xor       %3x %3x %3x %3x\n", sumsaf^gotsaf,
		sumsai^gotsai, sumeaf^goteaf, sumeai^goteai);
	    }
	adderror (verbose);
	}
    }

static add () {
    /* force the dda engine to do 1 addition by filling a small trapezoid */
    LDXS (0)
    LDXE (0)
    LDYS (0)
    LDYE (0)
    REQUEST (UC_FILLTRAP, 0)
    }

static db (i)
short i;
{
    while (i >= 12)
	i -= 12;
    return 1 << i;
    }

#endif UC4
