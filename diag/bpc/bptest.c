/*
 *	Kurt Akeley			7/17/82
 *
 *	This program is designed to test the entire functionality
 *	  of the BPM memory boards in the IRIS system.  However, they
 *	  do not accomplish this goal.  In particular, the actual display
 *	  output signals are not confirmed.  I.e., the memory may contain
 *	  (and read back) correct data, but still send incorrect video
 *	  output to the dc.
 *
 *	Procedures:
 *		loadmem (mode)
 *		complement ()
 *		adderrors (addr)
 *		binprint (fp, val, zerochar, onechar, numofbits)
 *		bplamptest ()
 *
 *	Functions:
 *		long	testplanes (planes, verbose, testmode,
 *				    teststorun, chipreport)
 *		long	comparemem (planes, constant, mode,
 *				    complement, verbose, badchips)
 *
 *	Updated:
 *		9/18/82  KBA	Subroutine to console
 *		10/19/82 KBA	Error total output added
 *		10/21/82 KBA	Error messages changed
 *		10/25/82 KBA	Stop every four n messages
 *		11/2/82  KBA	Incorporates fast test routines
 *		11/16/82 KBA	Loop test added to debug new test board
 *		2/21/83  KBA	32-bit capability added
 *		4/11/83  KBA	bplamptest added
 *		5/13/83	 KBA	random mode added to bptest
 */

#include "console.h"
#include "uctest.h"
#include "ucdev.h"
#include "bpccodes.h"

#define NORMALMODE	0
#define ADDRESSMODE	1
#define RANDOMMODE	2
#define COMPLEMENT	TRUE
#define VERBOSE		TRUE

#define RASCAS(x,y)	(((y<<8) | (y>>8) | (x<<2)) & 0xffff)
#define RANDOMIZE(r)	r = ((r<<2) + r + 17623) & 0xffff
#define SEED		433



long	testplanes (planes, verbose, testmode, teststorun, chipreport)
long	planes;		/* planes of memory to be tested.  0..ffff */
boolean	verbose;	/* boolean.  forces verbose error reporting */
boolean	testmode;	/* boolean.  forces errors into memory */
long	teststorun;	/* low order bits determine whether each test
			   test is run */
boolean	chipreport;	/* boolean.  forces report of bad chips */
{
    /*
     *	The specified planes are put through six complete tests.  They are
     *    1.  All zero write and readback
     *    2.  All one write and readback
     *    3.  Cell address write and read back
     *    4.  Cell address write, complement on board, and read back(UC4 only)
     *	  5.  Random write and read back
     *	  6.  Exercise word save and draw
     *  If verbose is true, errors are reported as they occur.  The total
     *    number of errors is reported on completion in any case.
     */

    short	badchips [32];	/* one integer for each of 32 planes */
    long	errortotal;
    short	i;
    long	pixels[16];
    Save	s;

    save (&s);
    errortotal = 0;
    for (i=0; i<32; i++)
	badchips [i] = 0;

    /* all zero test */
    if (teststorun & 1) {
	if (verbose)
	    putchar ('0');
	setcodes (0x00000000, planes);
	setstipple (STIPADDR, onestipple);
	clear (STIPADDR);
	if (testmode) {
	    adderrors (10);
	    printf ("*");
	    }
	errortotal += comparemem (planes, 0x00000000, NORMALMODE,
				 !COMPLEMENT, verbose, badchips);
	restore (&s);
	}

    /* all one test */
    if (teststorun & 2) {
	if (verbose)
	    putchar ('1');
	setcodes (0xffffffff, planes);
	setstipple (STIPADDR, onestipple);
	clear (STIPADDR);
	if (testmode) {
	    adderrors (100);
	    printf ("*");
	    }
	errortotal += comparemem (planes, 0xffffffff, NORMALMODE,
				 !COMPLEMENT, verbose, badchips);
	restore (&s);
	}

    /* address test */
    if (teststorun & 4) {
	if (verbose)
	    putchar ('2');
	setcodes (0x00000000, planes);
	setstipple (STIPADDR, onestipple);
	clear (STIPADDR);
	setcodes (0xffffffff, planes);
	loadmem (ADDRESSMODE);
	errortotal += comparemem (planes, 0x00000000, ADDRESSMODE,
				 !COMPLEMENT, verbose, badchips);
	restore (&s);
	}

    /* complement address test */
#ifdef UC3
    if (teststorun & 8) {
	if (verbose)
	    printf ("\nComplement address test not available for UC3\n");
	}
#endif UC3
#ifdef UC4
    if (teststorun & 8) {
	if (verbose)
	    putchar ('3');
	setcodes (0x00000000, planes);
	setstipple (STIPADDR, onestipple);
	clear (STIPADDR);
	setcodes (0xffffffff, planes);
	loadmem (ADDRESSMODE);
	complement ();
	errortotal += comparemem (planes, 0x00000000, ADDRESSMODE, COMPLEMENT,
				  verbose, badchips);
	restore (&s);
	}
#endif UC4

    /* random data test */
    if (teststorun & 16) {
	if (verbose)
	    putchar ('4');
	setcodes (0x00000000, planes);
	setstipple (STIPADDR, onestipple);
	clear (STIPADDR);
	setcodes (0xffffffff, planes);
	loadmem (RANDOMMODE);
	errortotal += comparemem (planes, 0x00000000, RANDOMMODE, !COMPLEMENT,
				  verbose, badchips);
	restore (&s);
	}

    /* save and draw */
    if (teststorun & 32) {
	short x, pixel;
	long pixels[16];
	initerror (5);
	if (verbose)
	    putchar ('5');
	setcodes (0x00000000, planes);
	setstipple (STIPADDR, onestipple);
	clear (STIPADDR);
	setcodes (0xffffffff, planes);
	for (x=0; x<16; x++) {
	    writeword (x, 0, 1<<x);
	    REQUEST (READWORD, 0);
	    LDYS (1);
	    REQUEST (WRITEWORD, 0xffff);
	    }
	for (x=0; x<16; x++) {
	    readpixels (pixels, x, 1, planes);
	    for (pixel=0; pixel<16; pixel++) {
		if (pixels[pixel] != ((pixel==x) ? planes : 0)) {
		    if (verbose) {
			printf ("savedraw: error on pixel %x, ", pixel);
			printf ("expected %x, got %x\n",
				(pixel==x) ? planes : 0, pixels[pixel]);
			}
		    adderror (verbose);
		    }
		}
	    }
	errortotal += geterrors ();
	}

    if (chipreport) {
	printf ("\n     ************* Bad Chip Report ************\n");
	printf ("     Schematic assignments: fedc ba98 7654 3210\n");
	for (i=0; i<32; i++)
	    if (badchips[i] != 0) {
		printf ("   Plane %2x: Badchips       ", i);
		binprint (badchips[i], '.', '*', 16);
		printf ("\n");
		}
	}
    return (errortotal);
    }



loadmem (mode)
short	mode;
{
    /*
     *	Loads each word in the bitplane memory with a value that is
     *    determined by the mode argument.  The modes are:
     *      NORMALMODE
     *		Simply clear the memory to the current colorcode
     *	    ADRESSMODE
     *		Loads each word with its address as a value.  The address
     *		is computed so that its 8 high-order bits correspond to RAS,
     *		and its 8 low-order bits correspond to CAS.  Pixels which
     *		correspond to ones are written in the current color; those
     *		corresponding to zeros are left alone.
     *	    RANDOMMODE
     *		A simple generator with fixed seed, multiplier, and offset,
     *		is used to load the memory.
     */
    register long x, y;
    register long rand;

    if (mode == NORMALMODE) {
	setstipple (STIPADDR, onestipple);
	clear (STIPADDR);
	}
    else if (mode == ADDRESSMODE) {
	for (y=0; y < 1024; y++) {
	    breakcheck ();
	    for (x=0; x < 64; x++)
	        writeword (x, y, RASCAS (x, y));
	    }
	}
    else if (mode == RANDOMMODE) {
	rand = SEED;
	for (y=0; y < 1024; y++) {
	    breakcheck ();
	    for (x=0; x < 64; x++) {
		writeword (x, y, rand);
		RANDOMIZE (rand);
		}
	    }
	}
    }



long	comparemem (planes, constant, mode, complement, verbose,
		    badchips)
long	planes;
long	constant;
short	mode;
boolean	complement;
boolean	verbose;
short	badchips[];
{
    /*
     *	The contents of memory are read back and compared with either
     *	  a constant value or with the address of the cell.  In either
     *    case the complement of the cell contents is taken before the
     *    comparison if <complement> is true.
     *  A distinction is made between read errors and write errors.
     *    An error is considered to be a write error if it reads back
     *	  consistently incorrectly.  It is a read error if it reads back
     *    first incorrectly, then once correctly.  Otherwise the error
     *	  is a "wierd" error, and is so reported.
     *  If verbose is true, errors are reported as they are encountered.
     *    Otherwise no reporting is done.
     *  Returns the total number of errors encountered.
     *  Update - added random readback mode
     */
    long x, y;
    register long bit;
    short	rand;			/* 16-bit random variable	*/
    boolean	error;
    register short pixel;		/* range 0..f			*/
    register short plane;		/* range 0..32			*/
    register long value;		/* range 0..ffff		*/
    short	dots;			/* number of dots printed	*/
    long	pixels1[16];		/* first pixel readback		*/
    long	pixels2[16];		/* second pixel readback	*/
    long	return1, return2;	/* rotated pixel values		*/
    long	i;

    rand = SEED;
    initerror (4);
    for (y=0, dots=0; y < 1024; y++) {
	breakcheck ();
	if ((verbose) && (!(y&0xff))) {
	    printf (".");
	    dots += 1;
	    }
	for (x=0; x < 64; x++) {
	    switch (mode) {
		case NORMALMODE:	value = constant; break;
		case ADDRESSMODE:	value = RASCAS (x, y); break;
		case RANDOMMODE:	value = rand; RANDOMIZE (rand); break;
		}
	    if (complement)
		value = (~value);
	    value &= 0xffff;
	    readpixels (pixels1, x, y, planes);
	    for (pixel=0, bit=1, error=FALSE; pixel<16; pixel++, bit<<=1)
		if (pixels1[pixel] != ((value&bit)?planes:0)) {
		    error = TRUE;
		    break;
		    }
	    if (!error)
		continue;
	    readpixels (pixels2, x, y, planes);	/* take another sample */
	    for (plane=0; plane<32; plane++) {
		return1 = planeword (plane, pixels1);
		if ((planes & (1<<plane)) && (return1 != value)) {
		    badchips[plane] |= (value ^ return1) & 0xffff;
		    if (verbose) {
			return2 = planeword (plane, pixels2);
			if (return2 == value)		/* read error */
			    printf ("plane %1x: read error,  ", plane);
			else if (return1 == return2)	/* write error */
			    printf ("plane %1x: write error, ", plane);
			else
			    printf ("plane %1x: wierd error, ", plane);
			printf ("ERROR: x=%x, y=%x, ", x, y);
			printf ("ras=%x, cas=%x\n", y&0xff, (x<<2)|(y>>8));
			printf ("    value:         %4x,    ", value);
			binprint (value, '0', '1', 16);
			printf ("\n    first return:  %4x,    ", return1);
			binprint (return1, '0', '1', 16);
			printf ("\n    second return: %4x,    ", return2);
			binprint (return2, '0', '1', 16);
			printf ("\n");
			}
		    adderror (verbose);
		    }
		}
	    }
	}
    for (x=0; x<dots; x++)
	printf ("\010 \010");
    return geterrors ();
    }



complement ()
{
    /*
     *	The entire contents of the memory plane are bit-complemented
     *    one word at a time.  This is accomplished by first reading
     *	  into the on plane buffer, then rotating 16 steps, then writing
     *    into the same location.
     */
    register long x, y;

    for (y=0; y < 1024; y++) {
	breakcheck ();
	for (x=0; x < 64; x++)
	    cmplword (x, y);
	}
    }



adderrors (addr)
long	addr;
{
    /*
     *	Each plane is written individually with the value addr+p, where
     *	  p is the plane number 0..1f.
     */

    short	i;

    printf ("\nWARNING: adding errors !!!\n\007");
    for (i=0; i<32; i++) {
	setcodes (0x00000000, (1<<i));
	writeword (0, i, 0xffffffff);
	setcodes (0xffffffff, (1<<i));
	writeword (0, i, addr+i);
	}
    }



binprint (val, zerochar, onechar, numofbits)
long		val;
char		zerochar, onechar;
short		numofbits;
{
    /*
     *  Outputs the 'numofbits' low order bits of val as a string of zerochars
     *    and onechars to the specified stream.
     *  A single space is left between each set of four characters.
     *    **** **** **** ****
     */
    register long i;
    for (i=numofbits-1; i>=0; i--) {
	printf ("%c", (val&(1<<i)) ? onechar : zerochar);
	if (((i&3)==0) && (i!=0))
	    printf (" ");
	}
    }



bplamptest (cycles)
short	cycles;
{
    /*
     *	Rotates the 8 LEDs on each bitplane board that is installed
    *    (all boards rotate together).  Timing depends on vertical
     *	  pulse hardware.
     */

    short	i, j;
    short	wait;
    Save	s;

    save (&s);
    for (j=0; j<cycles; j++) {
	breakcheck ();
	for (i=0; i<8; i++) {
	    switch (i) {
	        case 0:
		    setcodes (0, A0 | A2 | A4 | A6 | C0 | C2 | C4 | C6);
		    break;
	        case 1:
		    setcodes (0, A1 | A3 | A5 | A7 | C1 | C3 | C5 | C7);
		    break;
	        case 2:
		    setcodes (0, B0 | B2 | B4 | B6 | D0 | D2 | D4 | D6);
		    break;
	        case 3:
		    setcodes (0, B1 | B3 | B5 | B7 | D1 | D3 | D5 | D7);
		    break;
	        case 4:
		    setcodes (A0 | A2 | A4 | A6 | C0 | C2 | C4 | C6, 0);
		    break;
	        case 5: 
		    setcodes (A1 | A3 | A5 | A7 | C1 | C3 | C5 | C7, 0);
		    break;
	        case 6:
		    setcodes (B0 | B2 | B4 | B6 | D0 | D2 | D4 | D6, 0);
		    break;
	        case 7:
		    setcodes (B1 | B3 | B5 | B7 | D1 | D3 | D5 | D7, 0);
		    break;
		}
	    for (wait=0; wait<60; wait++)
	        waitvert (TRUE);
	    }
	}
    setcodes (colorcode, wecode);
    restore (&s);
    }



