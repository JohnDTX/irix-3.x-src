/*
 *	Kurt Akeley			11/3/82
 *
 *	This file contains routines which interract with
 *	  the PAL on the Display control card.
 *
 *	Functions:
 *		long	stripetest (planes, verbose, fields, singlebuftest)
 *		long	dcpaltest (mode, scans)
 *
 *	Procedures:
 *		dcpal (change, command)
 *
 *	Updates:
 *		5/16/83	 KBA	Incorporated new PAL definition
 */

#include "uctest.h"
#include "console.h"
#include "ucdev.h"
#include "dcdev.h"
#include "bpccodes.h"
#include "vfsm.h"

#define DCPAL_SAMPLE_COUNT	4



dcpal (change, command)
boolean		change;		/* True if register contents are to be	*/
				/*    changed, false otherwise		*/
short		command;	/* Number of pal control 		*/
{
    short	regval[DCPAL_SAMPLE_COUNT];
    short	cmnd;
    short	i;

    if (change) {
	short dcr;
	dcr = dc_dcr;
	dcr &= ~VFSM_CMND_MASK;
	if (command & 1) dcr |= DCPALCTRL0;
	if (command & 2) dcr |= DCPALCTRL1;
	if (command & 4) dcr |= DCPALCTRL2;
	waitvert (TRUE);
	DCR (dcr);
	}

    for (i=0; i<DCPAL_SAMPLE_COUNT; i++) {
	regval[i] = VFSM_ADJUST (DCflags);
	waitvert (TRUE);
	}

#ifdef DC3
    cmnd = dc_dcr & VFSM_CMND_MASK;
    if (cmnd == VFSM_EVEN_CHECKSUM) {
	printf ("    Sample   Checksum\n");
	for (i=0; i<DCPAL_SAMPLE_COUNT; i++)
	    printf ("     %3d       %4x\n", i,
			regval[i] & VFSM_EVEN_CHECKSUM_MASK);
	}
    else if (cmnd == VFSM_ODD_CHECKSUM) {
	printf ("    Sample   Checksum\n");
	for (i=0; i<DCPAL_SAMPLE_COUNT; i++)
	    printf ("     %3d       %4x\n", i,
			regval[i] & VFSM_ODD_CHECKSUM_MASK);
	}
    else if (cmnd == VFSM_SET) {
	printf ("    Sample   Return\n");
	for (i=0; i<DCPAL_SAMPLE_COUNT; i++)
	    printf ("     %3d       %2x\n", i,
			regval[i] & VFSM_SET_MASK);
	}
    else if (cmnd == VFSM_CLEAR) {
	printf ("    Sample   Return\n");
	for (i=0; i<DCPAL_SAMPLE_COUNT; i++)
	    printf ("     %3d       %2x\n", i,
			regval[i] & VFSM_CLEAR_MASK);
	}
    else if ((cmnd == VFSM_CYCLE) || (cmnd == VFSM_STICKY_CYCLE)) {
	printf ("    Sample       Event          Cycle\n");
	printf ("              Even    Odd    Even    Odd\n");
	for (i=0; i<DCPAL_SAMPLE_COUNT; i++)
	    printf ("     %3d      %3x    %3x     %3x    %3x\n", i,
			(regval[i] & VFSM_EVEN_EVENT_MASK) ? 1 : 0,
			(regval[i] & VFSM_ODD_EVENT_MASK) ? 1 : 0,
			(regval[i] & VFSM_EVEN_CYCLE_MASK) ? 1 : 0,
			(regval[i] & VFSM_ODD_CYCLE_MASK) ? 1 : 0);
	}
#endif DC3
#ifdef DC4
    printf ("sample  7-4 3 2 1 0\n");
    for (i=0; i<DCPAL_SAMPLE_COUNT; i++) {
	printf ("   %d     %x  %x %x %x %x\n", i, regval[i]>>4,
		(regval[i]&8)?1:0, (regval[i]&4)?1:0,
		(regval[i]&2)?1:0, (regval[i]&1)?1:0);
	}
#endif DC4
    }

long		dcpaltest (mode, scans)
short		mode;
short		scans;
{
    /*
     *	Waits for the next vertical retrace, then changes the dcpal mode
     *    to the specified value and waits for <scans> retraces to go by.
     *    Then resets the dcpal register to its original value and
     *    returns the register contents.
     */

    short	dcr;
    short	dcval;
    short	i;
    short	s_dcr;

    s_dcr = dc_dcr;
    dcr = dc_dcr & (~VFSM_CMND_MASK);
    waitvert (TRUE);
    DCR (dcr | VFSM_CLEAR);
    DCR (dcr | mode);
    for (i=0; i<scans; i++) {
	waitvert (TRUE);
	}
    dcval = VFSM_ADJUST (DCflags);
    DCR (s_dcr);
    return (dcval);
    }




static char planename[4] = {'a', 'b', 'c', 'd'};

long		stripetest (planes, verbose, fields, singlebuftest)
long		planes;		/* planes to be tested, 0..f0fffff	    */
boolean		verbose;	/* true causes verbose error reporting	    */
short		fields;		/* number of fields (vert sweeps) per plane */
boolean		singlebuftest;	/* true if single buffer operation is desire*/
{
    /*
     *	Each of the specified planes is loaded with a stripe pattern
     *    and then tested with the VFSM stripe mode for bit errors.
     *    Because the stripe width and repeat are not powers of 2, the
     *    test detects all stuck-at and shift-register faults in the
     *    video path (except the DACs themselves).
     */

    short	i;
    short	mapbit;
    short	dcval;
    short	p;
    long	badplanes;
    long	testplanes;
    long	pmask;
    Save	s;

    save (&s);
    badplanes = 0;

    if (singlebuftest)
	testplanes = A0|A1|A2|A3|A4|A5|B0|B1|B2|B3|B4|B5;
    else
	testplanes = A0|A1|A2|A3|A4|A5|A6|A7|B0|B1|B2|B3|B4|B5|B6|B7 |
		     C0|C1|C2|C3|D0|D1|D2|D3;
    if (planes & ~testplanes) {
	if (verbose) {
	    printf ("Cannot test planes ");
	    printplanes (planes & ~testplanes);
	    printf ("\n");
	    }
	planes &= testplanes;
	}

    for (p=0; p<32; p++) {
	breakcheck ();
	pmask = (1<<p);
	if (!(pmask & planes))
	    continue;

	/* zero all planes except pmask, which is striped */
	LDCONFIG (UPDATEA | UPDATEB | s.cfb);
	setcodes (0, 0xffffffff);
	drawrect (0, 0, 1024, 1024, ONESTIPADDR);
	setcodes (pmask, pmask);
	drawrect (0, 0, 1024, 1024, ONESTIPADDR);
	setcodes (0, pmask);
	for (i=0; i<1024; i+=6)
	    drawrect (i, 0, 3, 1024, ONESTIPADDR);

	if (singlebuftest) {
	    LDCONFIG (DISPLAYA | DISPLAYB | s.cfb);
	    mapbit = mapcode (pmask, 0, DISPLAYA|DISPLAYB);
	    }
	else {
	    if (pmask&(ACODE|CCODE)) {		/* A or C plane */
	        LDCONFIG (DISPLAYA | (s.cfb & ~DISPLAYB));
	        mapbit = mapcode (pmask, 0, DISPLAYA);
	        }
	    else if (pmask&(BCODE|DCODE)) {	/* B or D plane */
	        LDCONFIG (DISPLAYB | (s.cfb & ~DISPLAYA));
	        mapbit = mapcode (pmask, 0, DISPLAYB);
	        }
	    }
	REQUEST (LOADXYADDR, 0);
	mapcolor (mapbit, mapbit, 0xff, 0xff, 0xff);
	mapcolor (     0, mapbit, 0, 0, 0);
#ifdef DC3
	dcval = dcpaltest (VFSM_STICKY_CYCLE, fields);
	if (verbose)
	    printf ("%c%1x", planename[p>>3], p&7);
	if (dcval & VFSM_CYCLE_MASK) {
	    badplanes |= pmask;
	    if (verbose) {
		printf ("(");
		if (dcval & VFSM_EVEN_CYCLE_MASK)
		    printf ("e");
		if (dcval & VFSM_ODD_CYCLE_MASK)
		    printf ("o");
		printf (")");
		}
	    }
#endif DC3
#ifdef DC4
	if (dc_dcr & DCPIPE4) 
	    dcval = dcpaltest (VFSM_STICKY_CYCLE_4, fields);
	else
	    dcval = dcpaltest (VFSM_STICKY_CYCLE_2, fields); 
	if (verbose)
	    printf ("%c%1x", planename[p>>3], p&7);
/*	if (dcval & VFSM_CYCLE_MASK) { */
	if (dcval & VFSM_CYCLE_MASK_4) {
	    badplanes |= pmask;
	    if (verbose) {
		printf ("(");
		if (dcval & VFSM_0_CYCLE_MASK)
		    printf ("0");
		if (dcval & VFSM_1_CYCLE_MASK)
		    printf ("1");
		if (dcval & VFSM_2_CYCLE_MASK)
		    printf ("2");
		if (dcval & VFSM_3_CYCLE_MASK)
		    printf ("3");
		printf (")");
		}
	    }
#endif DC4
	}
    restore (&s);
    if (verbose)
	printf ("    ");
    return badplanes;
    }
