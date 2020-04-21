/*
 *	Kurt Akeley			9/20/82
 *
 *	These routines are intended to be linked with the main routine
 *	"console.b".
 *
 *	Functions:
 *		long	fmtest (verbose, teststorun, diagnostic)
 *		long	comparefm (data, mode, verbose)
 *
 *	Procedures:
 *		loadfm (data, mode)
 *		addfmerrors ()
 *
 *	Updates:
 *		10/25/82 KBA	Changed error information printed
 *		5/13/83  KBA	Added random test
 */

#include "uctest.h"
#include "ucdev.h"
#include "console.h"

#define DATAMODE	0
#define LOWADDRMODE	1
#define HIGHADDRMODE	2
#define RANDOMMODE	3

#define SEED		433
#define RANDOMIZE(r)	r = ((r<<2) + r + 17623) & 0xffff

#ifdef UC3
#define MAXFONT		0x10000
#endif UC3
#ifdef UC4
#define MAXFONT		0x04000
#endif UC4

long		fmtest (verbose, teststorun, diagnostic)
boolean		verbose;	/* forces verbose error reporting	*/
short		teststorun;	/* enable various tests			*/
boolean		diagnostic;	/* tests the code itself		*/
{
    /*
     *	A sequence of 5 tests of the font memory is performed.  These are:
     *	  1.  All zero write and readback
     *	  2.  All one write and readback
     *	  3.  Lower 8 address bit write and readback
     *    4.  Upper 8 address bit write and readback
     *	  5.  Random data write and readback
     *  Individual errors are reported as they occur if verbose if true.
     *  The total number of errors is returned.
     */

    long errortotal;
    errortotal = 0;

    /* all zero test */
    if (teststorun & 1) {
	if (verbose)
	    putchar ('0');
	breakcheck ();
	loadfm (0, DATAMODE);
	if (diagnostic) {
	    addfmerrors ();
	    }
	breakcheck ();
	errortotal += comparefm (0, DATAMODE, verbose);
	}

    /* all one test */
    if (teststorun & 2) {
	if (verbose)
	    putchar ('1');
	breakcheck ();
	loadfm (0xffff, DATAMODE);
	if (diagnostic) {
	    printf ("introducing errors ...\n");
	    addfmerrors ();
	    }
	breakcheck ();
	errortotal += comparefm (0xffff, DATAMODE, verbose);
	}

    /* low address test */
    if (teststorun & 4) {
	if (verbose)
	    putchar ('2');
	breakcheck ();
	loadfm (0, LOWADDRMODE);
	breakcheck ();
	errortotal += comparefm (0, LOWADDRMODE, verbose);
	}

    /* high address test */
    if (teststorun & 8) {
	if (verbose)
	    putchar ('3');
	breakcheck ();
	loadfm (0, HIGHADDRMODE);
	breakcheck ();
	errortotal += comparefm (0, HIGHADDRMODE, verbose);
	}

    /* random data test */
    if (teststorun & 16) {
	if (verbose)
	    putchar ('4');
	breakcheck ();
	loadfm (0, RANDOMMODE);
	breakcheck ();
	errortotal += comparefm (0, RANDOMMODE, verbose);
	}

    return (errortotal);
    }


loadfm (data, mode)
short		data;		/* written if mode is DATAMODE		*/
short		mode;		/* controls data			*/
{
    /*
     *	Writes the entire font memory with either the data provided
     *    (DATAMODE) or with either the 8 low or high address bits of
     *    each cell.
     */

    long	i;		/* fm address index			*/
    short	cellvalue;	/* value to be written into each byte	*/
    short	rand;

#ifdef UC4
    LDFMADDR (0)
    REQUEST (UC_SETADDRS, 0)
#endif UC4
    rand = SEED;
    for (i=0; i<MAXFONT; i++) {
	switch (mode) {
	    case DATAMODE:	cellvalue = (data);	    	    break;
#ifdef UC3
	    case LOWADDRMODE:	cellvalue = (i&0xff);		    break;
	    case HIGHADDRMODE:	cellvalue = ((i>>8)&0xff);	    break;
#endif UC3
#ifdef UC4
	    case LOWADDRMODE:
	    case HIGHADDRMODE:	cellvalue = i;			    break;
#endif UC4
	    case RANDOMMODE:	cellvalue = rand; RANDOMIZE (rand); break;
	    }
#ifdef UC3
	LDFMADDR (i)
#endif UC3
	REQUEST (WRITEFONT, cellvalue)
	}
    }



long 		comparefm (data, mode, verbose)
short		data;
short		mode;
boolean		verbose;
{
    /*
     *  Compares the contents of the entire font memory with with
     *    the (presumed) desired contents.  Errors are reported as
     *    as they occur if verbose is true.  The total number of
     *    errors is returned as a long integer.
     */

    long	i;		/* font memory address index		*/
    short	expectedvalue;
    short	firstreturn;	/* first value read from the fm		*/
    short	secondreturn;	/* second value read from the fm	*/
    short	rand;

#ifdef UC4
    LDFMADDR (0)
    REQUEST (UC_SETADDRS, 0)
#endif UC4
    initerror (10);
    rand = SEED;
    for (i=0; i<MAXFONT; i++) {
	switch (mode) {
#ifdef UC3
	    case DATAMODE:	expectedvalue = (data&0xff);	break;
	    case LOWADDRMODE:	expectedvalue = (i&0xff);	break;
	    case HIGHADDRMODE:	expectedvalue = ((i>>8)&0xff);  break;
	    case RANDOMMODE:	expectedvalue = (rand&0xff);
				RANDOMIZE (rand); break;
#endif UC3
#ifdef UC4
	    case DATAMODE:	expectedvalue = (data&0xffff);	break;
	    case LOWADDRMODE:
	    case HIGHADDRMODE:	expectedvalue = i;		break;
	    case RANDOMMODE:	expectedvalue = rand;
				RANDOMIZE (rand); break;
#endif UC4
	    }
	firstreturn = readfont (i);
	if (firstreturn != expectedvalue) {
	    secondreturn = readfont (i);
	    if ((secondreturn == expectedvalue) && verbose) {
		printf ("READ ERROR:  addr=%4x value=%4x ",i,expectedvalue);
		printf ("first=%4x second=%4x\n", firstreturn, secondreturn);
		}
	    else if ((firstreturn == secondreturn) && verbose) {
		printf ("WRITE ERROR: addr=%4x value=%4x ",i,expectedvalue);
		printf ("first=%4x second=%4x\n", firstreturn, secondreturn);
		}
	    else if (verbose) {
		printf ("WIERD ERROR: addr=%4x value=%4x ",i,expectedvalue);
		printf ("first=%4x second=%4x\n", firstreturn, secondreturn);
		}
	    adderror (verbose);
	    }
	}
    return geterrors ();
    }



addfmerrors ()
{
    short	i;
    printf ("\nWARNING: adding errors !!!\n\007");
    for (i=0; i<8; i++) {
	LDFMADDR (i)
	REQUEST (WRITEFONT, i)
	}
    }
