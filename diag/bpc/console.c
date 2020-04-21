/*
 *	Kurt Akeley			9/18/82
 *
 *	These routines are intended to be linked with the main routine
 *	"console.b".
 *
 *	Procedures:
 *		console ()
 *
 *	Updates:
 *		9/18/82	 KBA	Copied from console.c
 *		9/20/82  KBA	Added displaytest
 *		11/1/82  KBA	Readword and Writeword commands
 *		11/2/82	 KBA	Incorporates fast test routines
 *		11/3/82	 KBA	Stripetest added
 *		8/4/84   KBA	Getcmnd() added
 */

#include "uctest.h"
#include "console.h"
#include <dcdev.h>
#include <ucdev.h>
#include "getcmnd.h"
#include "commands.h"

#define a0	(cmnd->arg[0])
#define a1	(cmnd->arg[1])
#define a2	(cmnd->arg[2])
#define a3	(cmnd->arg[3])
#define a4	(cmnd->arg[4])
#define a5	(cmnd->arg[5])
#define a6	(cmnd->arg[6])
#define a7	(cmnd->arg[7])
#define a8	(cmnd->arg[8])
#define a9	(cmnd->arg[9])

#define MAXMACRONEST	5
#define MAXSAVE		10

short macronest = 0;
Save savearray[MAXSAVE];
long savesigplanes[MAXSAVE];
long savecolorcode[MAXSAVE];
long savewecode[MAXSAVE];
short savecolorindex[MAXSAVE];
short saveweindex[MAXSAVE];
boolean savevalid[MAXSAVE] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
extern long gferrors;

boolean doline (cmnd, interactive, cmndlist, macrolist)
Command *cmnd;
boolean interactive;
Cmnddef *cmndlist;
Command macrolist[][MACROSIZE+1];
{
    /*
     *	Executes a command, returns TRUE if successful, FALSE otherwise.
     */
    long	i;		/* temporary long integer variable	*/
    long	total, errors;	/* used for test loops			*/
    short	red,green,blue;	/* temporary short variables		*/
    short	stipple[16];	/* stores a single stipple pattern	*/
    long	pixels[16];	/* stores 16 pixels			*/
    long	gftotal;	/* total gf errors, this is a hack	*/
    
    switch (cmnd->number) {
	case C_MACRO:
	    macronest += 1;
	    if (macronest > MAXMACRONEST) {
		printf ("  macro nest limit of %d exceeded, aborted\n",
			MAXMACRONEST);
		macronest = 0;
		dobreak ();
		}
	    if (a0 >= MAXMACRO || a0 < 0) {
		printf ("  There is no macro %x.  Range is 0..%x\n",
			a0, MAXMACRO);
		return FALSE;
		}
	    for (i=0; i<a1; i++) {
		short j;
		for (j=0;
		     doline (&macrolist[a0][j], FALSE, cmndlist, macrolist);
		     j++);
		breakcheck ();
		}
	    macronest -= 1;
	    break;
	case C_SEEMACRO:
	    if (a0 >= MAXMACRO || a0 < 0) {
		printf ("  There is no macro %x.  Range is 0..%x\n",
			a0, MAXMACRO);
		return FALSE;
		}
	    for (i=0; macrolist[a0][i].number != C_NOTACOMMAND; i++) {
		printf ("  [see-%d,%d] -> ", a0, i+1);
		printcmnd (&macrolist[a0][i], cmndlist);
		printf ("\n");
		}
	    break;
	case C_DCR:
	    /************************************************************
	    printf ("  Assigns dcflags to the display controller reg\n");
	    ************************************************************/
	    if (cmnd->args > 0) {
		DCR (a0);
		}
	    if (interactive)
		printdcflags (dc_dcr);
	    break;
#ifdef UC4
	case C_UCR:
	    UCR (a0);
	    break;
#endif UC4
	case C_COLORCODE:
	    colorcode = a0;
	    setcodes (colorcode, wecode);
	    if (interactive) {
		printf ("  colorcode: ");
		printplanes (colorcode);
		printf ("\n");
		}
	    break;
	case C_WECODE:
	    wecode = a0;
	    setcodes (colorcode, wecode);
	    if (interactive) {
		printf ("  wecode: ");
		printplanes (wecode);
		printf ("\n");
		}
	    break;
	case C_LDCONFIG:
	    if (cmnd->args > 0) {
		LDCONFIG (a0);
		REQUEST (LOADXYADDR, 0)	/* force register update */
		}
	    if (interactive)
		printconfig (uc_cfb);
	    break;
	case C_LDED:
	    LDED (a0)
	    break;
	case C_LDEC:
	    LDEC (a0)
	    break;
	case C_LDXS:
	    LDXS (a0)
	    break;
	case C_LDXE:
	    LDXE (a0)
	    break;
	case C_LDYS:
	    LDYS (a0)
	    break;
	case C_LDYE:
	    LDYE (a0)
	    break;
	case C_LDFMADDR:
	    LDFMADDR (a0)
	    break;
#ifdef UC4
	case C_LDMODE:
	    if (cmnd->args > 0) {
		LDMODE (a0)
		REQUEST (LOADXYADDR, 0)
		}
	    if (interactive)
		printmode (uc_mdb);
	    break;
	case C_LDREPEAT:
	    LDREPEAT (a0)
	    break;
	case C_LDDDASAF:
	    LDDDASAF (a0)
	    break;
	case C_LDDDASAI:
	    LDDDASAI (a0)
	    break;
	case C_LDDDASDF:
	    LDDDASDF (a0)
	    break;
	case C_LDDDASDI:
	    LDDDASDI (a0)
	    break;
	case C_LDDDAEAF:
	    LDDDAEAF (a0)
	    break;
	case C_LDDDAEAI:
	    LDDDAEAI (a0)
	    break;
	case C_LDDDAEDF:
	    LDDDAEDF (a0)
	    break;
	case C_LDDDAEDI:
	    LDDDAEDI (a0)
	    break;
#endif UC4
	case C_REQUEST:
	    if (isreadcommand (a0)) {
#ifdef UC4
		printf ("    got %x\n", (*UCCommandAddr (a0))&0xffff);
#endif UC4
		}
	    else {
		REQUEST (a0, a1)
		}
	    break;
	case C_LOOP:
	    if (isreadcommand (a0)) {
		printf ("  reading ... ");
		while (TRUE) {
		    short temp;
		    breakcheck ();
#ifdef UC4
		    temp = *UCCommandAddr (a0);
#endif UC4
		    }
		}
	    else {
		printf ("  writing ... ");
		while (TRUE) {
		    breakcheck ();
		    REQUEST (a0, a1);
		    }
		}
	    break;
	case C_ROTATELOOP:
	    if (isreadcommand (a0)) {
		printf ("  rotate loop is inappropriate for read commands\n");
		}
	    else {
		printf ("  writing rotating data ... ");
		while (TRUE) {
		    breakcheck ();
		    REQUEST (a0, a1);
		    a1 <<= 1;
		    if (a1&0x10000)
			a1 |= 1;
		    }
		}
	    break;
	case C_WRITE: {
	    long address;
	    address = DCMBM (a0 & 0xfffe);
	    if (!safewrite (address, a1))
		printf ("  timed out\n\007");
	    }
	    break;
	case C_WRITELOOP: {
	    long address;
	    address = DCMBM (a0 & 0xfffe);
	    if (!safewrite (address, a1)) {
		printf ("  timed out.  looping through interrupts ... ");
		while (TRUE)
		    safewrite (address, a1);
		}
	    else {
		printf ("  writing ... ");
		while (TRUE) {
		    breakcheck ();
		    *(short*)address = a1;
		    }
		}
	    }
	    break;
	case C_READ: {
	    long temp, address;
	    address = DCMBM (a0 & 0xfffe);
	    if ((temp=saferead (address)) == -1)
		printf ("  timed out\n\007");
	    else
		printf ("  %x (hex)\n", temp);
	    }
	    break;
	case C_READLOOP: {
	    long temp, address;
	    address = DCMBM (a0 & 0xfffe);
	    if (saferead (address) == -1) {
		printf ("  timed out.  looping through interrupts ... ");
		while (TRUE)
		    saferead (address);
		}
	    else {
		printf ("  reading ... ");
		while (TRUE) {
		    breakcheck ();
		    temp = *(short*)address;
		    }
		}
	    }
	    break;
	case C_READWORD:
	    readpixels (pixels, a0, a1, 0xffffffff);
	    printf ("absolute:\n");
	    if (sigplanes & 0xffff0000) {
		for (i=15; i>=0; i--) {
		    printf ("%4x", (pixels[i]>>16) & 0xffff);
		    if (i > 0)
			printf (" ");
		    }
		printf ("\n");
		}
	    for (i=15; i>=0; i--) {
		printf ("%4x", pixels[i] & 0xffff);
		if (i > 0)
		    printf (" ");
		}
	    printf ("\n");
	    printf ("colorindex:\n");
	    for (i=15; i>=0; i--) {
		printf ("%4x", getindex (pixels[i], sigplanes, uc_cfb));
		if (i > 0)
		    printf (" ");
		}
	    printf ("\n");
	    for (i=0; i<32; i++) {
		if (sigplanes & (1<<i)) {
		    printf ("    plane %1x: %4x    ",
			 i, planeword (i, pixels));
		    binprint (planeword (i, pixels), '0', '1',16);
		    printf ("\n");
		    }
		}
	    break;
	case C_WRITEWORD:
	    setcodes (0x00000000, 0xffffffff);
	    writeword (a0, a1, 0xffffffff);
	    setcodes (0xffffffff, 0xffffffff);
	    writeword (a0, a1, a2);
	    LDXS (a0<<4)
	    LDYS (a1)
	    REQUEST (READWORD, 0)
	    break;
	case C_DISPLAY:
	    display ((a0<<4), a1, 20);
	    break;
	case C_INIT:
	    init (interactive, a0);
	    break;
	case C_COLOR:
	    if (cmnd->args > 0) {
		i = planecode (a0, sigplanes, uc_cfb);
		if (i == -1) {
		    printf ("  code out of range, cannot be mapped\007\n");
		    }
		else {
		    colorindex = a0;
		    colorcode = i;
		    setcodes (colorcode, wecode);
		    }
		}
	    if (interactive) {
		printf ("  colorcode: ");
		printplanes (colorcode);
		printf ("\n  index: %x\n", colorindex);
		}
	    break;
	case C_WE:
	    if (cmnd->args > 0) {
		i = planecode (a0, sigplanes, uc_cfb);
		if (i == -1) {
		    printf ("  code out of range, cannot be mapped\007\n");
		    return FALSE;
		    }
		else {
		    weindex = a0;
		    wecode = i;
		    setcodes (colorcode, wecode);
		    }
		}
	    if (interactive) {
		printf ("  wecode: ");
		printplanes (wecode);
		printf ("\n  index: %x\n", weindex);
		}
	    break;
	case C_RGBCOLOR:
	    colorcode = rgbcode (a0, a1, a2);
		printf ("  colorcode: %x\n", colorcode);
	    setcodes (colorcode, wecode);
	    if (interactive) {
		printf ("  colorcode: ");
		printplanes (colorcode);
		printf ("\n  index is invalid\n");
		}
	    break;
	case C_MAPCOLOR:
	    i = planecode (a0, sigplanes, uc_cfb);
	    if (i == -1) {
		printf ("  colorindex out of range, cannot map\007\n");
		return FALSE;
		}
	    else {
		mapcolor (mapcode (i, dc_dcr, uc_cfb),
		      mapcode (sigplanes, dc_dcr, uc_cfb),
		      a1, a2, a3);
		}
	    break;
	case C_SIGPLANES:
	    if (cmnd->args > 0)
		sigplanes = a0;
	    if (interactive) {
		printf ("  significant planes: ");
		printplanes (sigplanes);
		printf ("\n");
		}
	    break;
	case C_DRAWCHAR:
	    writefont (STIPADDR+0, 0xffff);
	    writefont (STIPADDR+1, 0xfefe);
	    writefont (STIPADDR+2, 0xfcfc);
	    writefont (STIPADDR+3, 0xf8f8);
	    writefont (STIPADDR+4, 0xf0f0);
	    writefont (STIPADDR+5, 0xe0e0);
	    writefont (STIPADDR+6, 0xc0c0);
	    writefont (STIPADDR+7, 0x8080);
	    LDXS (a0)
	    LDYS (a1)
	    LDYE (a1+7)
#ifdef UC3
	    LDXE (a0+7)
#endif UC3
#ifdef UC4
	    LDXE (a0+15)
#endif UC4
	    LDFMADDR (STIPADDR)
	    REQUEST (DRAWCHAR, 0)
	    break;
	case C_FILLRECT:
	    for (i=0; i<16; i++) {
		if (a5 & (1<<i))
		    stipple[i] = 0xffff;
		else
		    stipple[i] = a4;
		}
	    setstipple (STIPADDR, stipple);
	    drawrect (a0, a1, a2, a3, STIPADDR);
	    break;
#ifdef UC4
	case C_FILLTRAP:
	    for (i=0; i<16; i++) {
		if (a7 & (1<<i))
		    stipple[i] = 0xffff;
		else
		    stipple[i] = a6;
		}
	    setstipple (STIPADDR, stipple);
	    filltrap (a0, a1, a2, a3, a4, a5, STIPADDR);
	    break;
#endif UC4
	case C_LOOPCLEARWORD:
	    for (i=0; i<16; i++) {
		stipple[i] = a2;
		}
	    setstipple (STIPADDR, stipple);
	    LDXS (a0<<4);
	    LDXE (a0<<4);
	    LDYS (a1);
	    LDYE (a1);
	    LDFMADDR (STIPADDR);
	    while (TRUE) {
		breakcheck ();
		REQUEST (CLEAR, 0);
		}
	    break;
	case C_CLEAR:
	    for (i=0; i<16; i++) {
		if (a1 & (1<<i))
		    stipple[i] = 0xffff;
		else
		    stipple[i] = a0;
		}
	    setstipple (STIPADDR, stipple);
	    clear (STIPADDR);
	    break;
	case C_DRAWLINE:
	    drawline (a0, a1, a2, a3, a4);
	    break;
#ifdef UC4
	case C_DEPTHCUE:
	    depthcue (a0, a1, a2, a3, a4, a5, a6);
	    break;
#endif UC4
	case C_DCPAL:
	    if (cmnd->args == 0)
		dcpal (FALSE, 0);
	    else
		dcpal (TRUE, a0);
	    break;
	case C_STRIPE:
	    for (i=0; i<1024; i+=a1) {
		drawrect (i, 0, a0, 1024, ONESTIPADDR);
		}
	    break;
	case C_STRIPETEST:
	    total = 0;
	    for (i=1; i<=a0; i++) {
		printf ("  pass %3d: ", i);
		errors = stripetest (sigplanes, a1, a2, a3);
		printf ("badplanes:%8x,  ", errors);
		total |= errors;
		printf ("cumulative:%8x\n", total);
		breakcheck ();
		}
	    break;
	case C_SCREENMASK:
	    LDXS (a0)
	    LDXE (a0+a2-1)
	    LDYS (a1)
	    LDYE (a1+a3-1)
#ifdef UC3
	    LDCONFIG (uc_cfb | VIEWPORTMASK)
	    REQUEST (LOADVIEWPORT, 0)
#endif UC3
#ifdef UC4
	    LDCONFIG (uc_cfb | UC_SCREENMASK)
	    REQUEST (UC_SETSCRMASKX, 0)
	    REQUEST (UC_SETSCRMASKY, 0)
#endif UC4
	    break;
	case C_BALL:
	    ball (a0, a1, a2, a3, a4, a5);
	    break;
	case C_SWAP:
	    while (TRUE) {
		for (i=0; i<a0; i++);
		LDCONFIG ((uc_cfb & ~ DISPLAYB) | DISPLAYA)
		REQUEST (LOADXYADDR, 0)
		for (i=0; i<a0; i++);
		LDCONFIG ((uc_cfb & ~ DISPLAYA) | DISPLAYB)
		REQUEST (LOADXYADDR, 0)
		}
	    break;
	case C_RANDRECT:
	    randrect (a0);
	    break;
	case C_RANDLINES:
	    randlines (a0, a1, a2);
	    break;
	case C_SCROLL:
	    scrolldemo (a0, a1, a2, a3, a4);
	    break;
#ifdef UC4
	case C_READDDA:
	    printf ("    ddasaf %3x\n", *UCBufferAddr(UC_DDASAF));
	    printf ("    ddasai %3x\n", *UCBufferAddr(UC_DDASAI));
	    printf ("    ddaeaf %3x\n", *UCBufferAddr(UC_DDAEAF));
	    printf ("    ddaeai %3x\n", *UCBufferAddr(UC_DDAEAI));
	    break;
	case C_DDATEST:
    	    for (i=0, total=0; i<a0; i++) {
		printf ("  pass %4d: ", i+1);
		errors = ddatest (a1, a2);
		total += errors;
		printf (", errors this pass = %4d, ", errors);
		printf ("total errors = %4d\n", total);
		breakcheck ();
		}
	    break;
	case C_PIXELTEST:
    	    for (i=0, total=0; i<a0; i++) {
		printf ("  pass %4d: ", i+1);
		errors = pixeltest (a1, a2);
		total += errors;
		printf (", errors this pass = %4d, ", errors);
		printf ("total errors = %4d\n", total);
		breakcheck ();
		}
	    break;
#endif UC4
	case C_WRITEFM:
	    writefont (a0, a1);
	    break;
	case C_READFM:
	    for (i=a0+a1-1; i >= a0; i--) {
		printf ("  %4x %4x\n", i, readfont (i) & 0xffff);
		breakcheck ();
		}
	    break;
	case C_READMAP:
	    for (i=a0+a1-1; i >= a0; i--) {
		if (readmap (i, &red, &green, &blue))
		    printf ("  %4x: %2x %2x %2x\n",
			i,red&0xff,green&0xff,blue&0xff);
		else
		    printf ("  %4x: index out of range\n\007", i);
		breakcheck ();
		}
	    break;
	case C_WRITEMAP:
	    mapcolor (a0, ~0, a1, a2, a3);
	    break;
	case C_QUIT:
	    printf ("  type 'y' to confirm: ");
	    if (negetch () == 'y') {
		printf ("\n");
		exit (0);
		}
	    printf ("\n");
	    break;
	case C_BPTEST:
	    for (i=0, total=0; i<a0; i++) {
		printf ("  pass %4d: ", i+1);
		errors = testplanes (sigplanes, a1, a4, a2, a3);
		total += errors;
		printf (", errors this pass = %4d, ", errors);
		printf ("total errors = %4d\n", total);
		breakcheck ();
		}
	    break;
	case C_FMTEST:
	    for (i=0, total=0; i<a0; i++) {
		printf ("  pass %4d: ", i+1);
		errors = fmtest (a1, a2, a3);
		total += errors;
		printf (", errors this pass = %4d, ", errors);
		printf ("total errors = %4d\n", total);
		breakcheck ();
		}
	    break;
	case C_GFTEST:
	    for (i=0, total=0; i<a0; i++) {
		printf ("  pass %4d: ", i+1);
		errors = testall (a1, a2);
		total += errors;
		printf (", errors this pass = %4d, ", errors);
		printf ("total errors = %4d\n", total);
		breakcheck ();
		}
	    break;
	case C_MAPTEST:
	    for (i=0, total=0; i<a0; i++) {
		printf ("  pass %4d: ", i+1);
		errors = maptest (a1, a2, a3);
		total += errors;
		printf (", errors this pass = %4d, ", errors);
		printf ("total errors = %4d\n", total);
		breakcheck ();
		}
	    break;
	case C_LINETEST:
	    for (i=0, total=0; i<a0; i++) {
		printf ("  pass %4d: ", i+1);
		errors = linetest (a1, a2);
		total += errors;
		printf (", errors this pass = %4d, ", errors);
		printf ("total errors = %4d\n", total);
		breakcheck ();
		}
	    break;
	case C_BPCTEST:
	    gftotal = 0;
	    for (i=0, total=0; i<a0; i++) {
		printf ("  pass %4d: ", i+1);
		errors = bpctest (a1, a2);
		total += errors;
		printf ("errors this pass = %4d, ", errors);
		printf ("total errors = %4d\n", total);
		if (gferrors || gftotal) {
		    gftotal += gferrors;
		    printf ("  %d gf errors this pass, %d total\n",
			gferrors, gftotal);
		    }
		breakcheck ();
		}
	    break;
	case C_COLORWETEST:
	    for (i=0, total=0; i<a0; i++) {
		printf ("  pass %4d: ", i+1);
		errors = colorwetest (a1);
		total += errors;
		printf (", errors this pass = %4d, ", errors);
		printf ("total errors = %4d\n", total);
		breakcheck ();
		}
	    break;
	case C_DCRTEST:
	    for (i=0, total=0; i<a0; i++) {
		printf ("  pass %4d: ", i+1);
		errors = dcrtest (a1);
		total += errors;
		printf ("errors this pass = %4d, ", errors);
		printf ("total errors = %4d\n", total);
		breakcheck ();
		}
	    break;
	case C_SCRMSKTEST:
	    for (i=0, total=0; i<a0; i++) {
		printf ("  pass %4d: ", i+1);
		errors = viewporttest (a1);
		total += errors;
		printf ("errors this pass = %4d, ", errors);
		printf ("total errors = %4d\n", total);
		breakcheck ();
		}
	    break;
	case C_RECTTEST:
	    for (i=0, total=0; i<a0; i++) {
		printf ("  pass %4d: ", i+1);
		errors = recttest (a1);
		total += errors;
		printf (", errors this pass = %4d, ", errors);
		printf ("total errors = %4d\n", total);
		breakcheck ();
		}
	    break;
#ifdef UC4
	case C_TRAPTEST:
	    for (i=0, total=0; i<a0; i++) {
		printf ("  pass %4d: ", i+1);
		errors = traptest (a1, a2);
		total += errors;
		printf (", errors this pass = %4d, ", errors);
		printf ("total errors = %4d\n", total);
		breakcheck ();
		}
	    break;
#endif UC4
	case C_CHARTEST:
	    for (i=0, total=0; i<a0; i++) {
		printf ("  pass %4d: ", i+1);
		errors = chartest (a1);
		total += errors;
		printf ("errors this pass = %4d, ", errors);
		printf ("total errors = %4d\n", total);
		breakcheck ();
		}
	    break;
	case C_TIME:
	    time (a0);
	    break;
	case C_DCLAMPTEST:
	    dclamptest (a0);
	    break;
	case C_UCLAMPTEST:
	    uclamptest (a0);
	    break;
	case C_BPLAMPTEST:
	    bplamptest (a0);
	    break;
	case C_RAMPTEST:
	    ramptest (a0, a1, a2, a3);
	    break;
	case C_STEPTEST:
	    steptest (a0, a1, a2, a3);
	    break;
	case C_JOESCALE:
	    joescale ();
	    break;
	case C_MEMTEST:
#ifdef PM1
	    printf ("  Memtest not available for PM1 version\n");
#endif PM1
#ifdef PM2
	    memtest (a0);
#endif PM2
#ifdef PM3
	    ip2memtest (a2, a1, a0);
#endif PM3
	    break;
	case C_FPTEST:
#ifdef PM1
	    printf ("  FPtest not available for PM1 version\n");
#endif PM1
#ifdef PM2
	    printf ("  FPtest not available for PM2 version\n");
#endif PM2
#ifdef PM3
	    fptest (1, 3, a1, a0, a2);
#endif PM3
	    break;
	case C_PRINT:
#ifdef PM2
	    seecircbuf ();
#endif PM2
	    break;
	case C_CMNDLIST:
	    alphalist (cmndlist, 2);
	    break;
	case C_SAVE:
	    if ((a0) >= 0 && (a0) < MAXSAVE) {
		save (&savearray[a0]);
		savecolorindex[a0] = colorindex;
		saveweindex[a0] = weindex;
		savecolorcode[a0] = colorcode;
		savewecode[a0] = wecode;
		savesigplanes[a0] = sigplanes;
		savevalid[a0] = TRUE;
		}
	    else if (interactive)
		printf ("  save: index out of range 0,%x\n", MAXSAVE-1);
	    break;
	case C_RESTORE:
	    if (cmnd->args > 0) {
		if ((a0) >= 0 && (a0) < MAXSAVE && savevalid[a0]) {
		    restore (&savearray[a0]);
		    sigplanes = savesigplanes[a0];
		    colorindex = savecolorindex[a0];
		    weindex = saveweindex[a0];
		    colorcode = savecolorcode[a0];
		    wecode = savewecode[a0];
		    setcodes (colorcode, wecode);
		    }
		else {
		    if (interactive) {
			if ((a0) < 0 || (a0) >= MAXSAVE)
			    printf ("  restore: index out of range 0,%x\n",
				    MAXSAVE-1);
			else
			    printf ("  restore: reg %x was never saved\n",a0);
			}
		    return FALSE;
		    }
		}
	    if (interactive) {
		printrestore ();
		printf ("  color index: %x, code: ", colorindex);
		printplanes (colorcode);
		printf ("\n");
		printf ("  we index: %x, code: ", weindex);
		printplanes (wecode);
		printf ("\n");
		printf ("  sigplanes: ");
		printplanes (sigplanes);
		printf ("\n");
		}
	    break;
	default:
	    return FALSE;
	    break;
	}
    return (TRUE);
    }

