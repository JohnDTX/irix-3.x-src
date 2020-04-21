/*
 *	Kurt Akeley
 *	15 May 1985
 *
 *	August 1985	Dave P. - fptest now tests all arithmetic and
 *				  conditional fp operations.
 *
 *	'fptest' selects operands, opcodes and register numbers for fpa
 *		operations in a random manner with the intent of generating
 *		a cross section of combinations in order to excercise the fpa.
 *		For each opcode selected, 'fptest' calls 'dofpop' to perform
 *		error checking and error reporting on the results.
 *		'fptest' prints a summary of errors for all opcodes upon
 *		completion.
 */

#include <fpa.h>

#ifdef UNIX
#include <fperr.h>
#endif UNIX


#define	QTYSGLSGL  18
#define	QTYDBLDBL  14
#define QTYSGLDBL  4
#define QTYDBLSGL  4
#define QTYSGLCR   2
#define QTYDBLCR   2
#define NUMOPS	QTYSGLSGL+QTYDBLDBL+QTYSGLDBL+QTYDBLSGL+QTYSGLCR+QTYDBLCR

#define MAXOP	128

#ifdef	IP2DIAG
extern	/* long globerrors */
#endif	IP2DIAG
long globerrors;

long left, right;
long leftlo, lefthi, rightlo, righthi;
long databus;
long operror[MAXOP];
long oproundnear[MAXOP];
long oproundfar[MAXOP];

long erdivbyzero;
long eroverflow;
long erunderflow;
long erdenormalize;
long ernan;
long erillegalop;
long erinexact;

static float nextsingle ();
static float nextsingle2 ();

int randreg;
long randmant;
int randexp;
int randsign;
long randmant2;
int randexp2;
int randsign2;

char optable[NUMOPS] = {	/* The order of listing these categories of */
				/* opcodes must remain so that the operand  */
				/* and result data types can be computed.   */
	ADDSGL, ADDSGLUPD,		/* QTYSGLSGL means		    */
	REVSUBSGL, REVSUBSGLUPD,	/* single operands => single results*/
	SUBSGL, SUBSGLUPD,
	MULSGL, MULSGLUPD,
	DIVSGL, DIVSGLUPD,
	REVDIVSGL, REVDIVSGLUPD,
	INTSGL,INTSGLUPD,
	SGLINT,SGLINTUPD,
	NEGSGL,NEGSGLUPD,		
	INTDBL,INTDBLUPD,		/* QTYSGLDBL means single => double */
	SGLDBL,SGLDBLUPD,
	CMPSGL,TSTSGL,			/* QTYSGLCR means single operand,   */
					/* result is in Condition Register. */
	ADDDBL, ADDDBLUPD,		/* QTYDBLDBL means		    */
	REVSUBDBL,REVSUBDBLUPD,		/* double operands => double results*/
	SUBDBL, SUBDBLUPD,
	MULDBL, MULDBLUPD,
	DIVDBL, DIVDBLUPD,
	REVDIVDBL, REVDIVDBLUPD,
	NEGDBL, NEGDBLUPD,
	DBLINT,DBLINTUPD,		/* QTYDBLSGL means double => single */
	DBLSGL,DBLSGLUPD,
	CMPDBL,TSTDBL			/* QTYDBLCR means double operand,   */
					/* result is in Condition Register. */
	};

/*  *****  Add capability to correctly generate Double Precision and NANS ***/
#define RANDREG		(randreg = ((9*randreg+3)&0xF))
#define RANDEXP		(randexp = ((13*randexp+9)&0xFF))
#define RANDMANT	(randmant = ((13*randmant+9)&0x7FFFFF))
#define RANDSIGN	(randsign = (21*randsign+13))
#define RANDEXP2	(randexp2 = ((25*randexp2+15)&0xFF))
#define RANDMANT2	(randmant2 = ((9*randmant2+3)&0x7FFFFF))
#define RANDSIGN2	(randsign2 = (33*randsign2+19))

#ifdef	UNIX
main (argc, argv)
int argc;
char *argv[];
#endif	UNIX

#ifdef	IP2DIAG
fptest (interactive, args, verbose, count, OptReg)
long interactive, args, verbose, count, OptReg;
#endif IP2DIAG

{
    int opcode, leftreg, rightreg;
    int optype, restype;
    int opindex = 0;
    long index;

#ifdef UNIX
    long verbose;
    long count = 1;
    long OptReg = 1;			/* Fast Mode */

    fpsigset(0,CONTINUE_AFTER_FPERROR | INHIBIT_FPMESSAGE);

    if (argc >= 2)
	sscanf(argv[1],"%lx",&verbose);
    if (argc >= 3)
	sscanf(argv[2],"%lx",&count);
    if (argc >= 4)
	sscanf(argv[3],"%lx",&OptReg);	/* Option Register */
    if ((argc == 1) || (count<1)) {
	printf("fptest exercises all numerical floating point operations.\n");
	printf("If any errors occur, fptest returns a non-zero exit code \n");
	printf("and displays a summary of errors.\n");
	printf("USAGE: fptest  <verbose>  <# of operations>  <Option Reg>\n");
	printf("       (arguments shown in <> are in hex) \n");
	printf("       If verbose is set to 1, each error is reported in detail\n");
	printf("       in addition to the summary.\n");
	printf("       If verbose is set to 2, an extended summary is displayed.\n");
	printf("       Default value for Option Reg is hex '01'.\n");
	exit(0);
        }
#endif UNIX

randreg = 5;
randmant = 0x87657;
randexp = 0x92;
randsign = 0x2534;
randmant2 = 0x56789;
randexp2 = 0x54;
randsign2 = 0x7654;

#ifdef IP2DIAG
    initerror (24);			/* Stop scroll every 20 messages    */
#endif IP2DIAG

    fpinit (OptReg);
    for (index=1; index <= (count*NUMOPS); index++) {
	optype = getoptype(opindex);
	restype= getrestype(opindex);
	opcode = optable[opindex++];
	if (opindex >= NUMOPS)
	    opindex = 0;
	leftreg = nextreg ();
	rightreg = nextreg ();

	Instrb (ER, 0, 0) = 0xFF;	/* Reset ER (Error Register)	    */

	if (optype == SINGLESIZE) {
	    left =  nextsingle ();
	    right = nextsingle2 ();

	    Instrl (SINGLE, leftreg) = left;
	    if (rightreg == 0) {
		databus = right;
		Instrl (opcode, leftreg) = databus;
	        }
	    else {
		Instrl (SINGLE, rightreg) = right;
		Instrb (opcode, leftreg, rightreg) = 0;
	        }
	    }
	else {				/* Double - rewrite !!!		    */
	    lefthi = nextsingle ();
	    righthi = nextsingle2 ();
	    leftlo = nextsingle ();
	    rightlo = nextsingle2 ();

	    Instrl (DOUBLELO, leftreg) = leftlo;
	    Instrl (DOUBLEHI, leftreg) = lefthi;
	    Instrl (DOUBLELO, rightreg) = rightlo;
	    if (rightreg == 0) {
		databus = righthi;
		Instrl (opcode, leftreg) = databus;
	        }
	    else {
		Instrl (DOUBLEHI, rightreg) = righthi;
		Instrb (opcode, leftreg, rightreg) = 0;
	        }
	    }

	dofpop (verbose, opcode, leftreg, rightreg, optype, restype);

        }
    if (globerrors > 0) {		/* Exit with error status */
	printf ("\nERROR REPORT - %x (hex) FP OPERATIONS COMPLETED:\n\n", index-1);
	printerrors(verbose);

#ifdef	UNIX
	exit(1);
#endif	UNIX
        }
    else {				/* Exit with no errors    */
	if (verbose) {
	    printf ("\nERROR REPORT - %x (hex) FP OPERATIONS COMPLETED:\n\n", 
					index-1);
	    printerrors(verbose);
	    }

#ifdef	UNIX
	exit(0);
#endif	UNIX
        }
    }


fpinit (OptReg) 
long	OptReg;
{
    int i;
    Instrb (MR, 0, 0) = 0xFF;
    Instrb (OR, 0, 0) = OptReg;
    Instrb (ER, 0, 0) = 0xFF;
    globerrors = 0;
    erdivbyzero = 0;
    eroverflow = 0;
    erunderflow = 0;
    erdenormalize = 0;
    ernan = 0;
    erillegalop = 0;
    erinexact = 0;
    for (i=0; i<MAXOP; i++) {
	operror[i] = 0;
	oproundnear[i] = 0;
	oproundfar[i] = 0;
	}
    }

int getoptype (opindex) 	/* Return data type of opcode's operands.   */
int opindex;
{
    if (opindex < (QTYSGLSGL + QTYSGLDBL + QTYSGLCR))
	return SINGLESIZE;
    else
	return DOUBLESIZE;
    }

int getrestype (opindex) 	/* Return data type of opcode's results.    */
int opindex;
{
    if (opindex < QTYSGLSGL)
	return SINGLESIZE;
    else if (opindex < (QTYSGLDBL + QTYSGLSGL))
	return DOUBLESIZE;
    else if (opindex < (QTYSGLCR + QTYSGLDBL + QTYSGLSGL))
	return CONDREG;
    else if (opindex < (QTYDBLDBL + QTYSGLCR + QTYSGLDBL + QTYSGLSGL))
	return DOUBLESIZE;
    else if (opindex < 
		(QTYDBLSGL + QTYDBLDBL + QTYSGLCR + QTYSGLDBL + QTYSGLSGL))
	return SINGLESIZE;
    else
	return CONDREG;
    }

int nextreg () {
    return RANDREG;
    }

static float nextsingle () {
    long exp, mant, sign;
    exp = RANDEXP;
    if (exp == 0)
	exp = 1;
    else if (exp == 0xFF)
	exp = 0xFE;
    mant = RANDMANT;
    sign = (RANDSIGN>>4) & 1;
    return (sign << 31) | (exp << 23) | mant;
    }

static float nextsingle2 () {
    long exp, mant, sign;
    exp = RANDEXP2;
    if (exp == 0)
	exp = 1;
    else if (exp == 0xFF)
	exp = 0xFE;
    mant = RANDMANT2;
    sign = (RANDSIGN2>>6) & 1;
    return (sign << 31) | (exp << 23) | mant;
    }

printerrors (verbose) 
long verbose;
{

#ifdef IP2DIAG
	addmessage(verbose);
	addmessage(verbose);
	addmessage(verbose);
	addmessage(verbose);
	addmessage(verbose);
#endif IP2DIAG
    printf ("            SGL     SGLUPD  DBL     DBLUPD");
    if (verbose & 2)
	printf("  round   round");
    printf ("\nopcode      errors  errors  errors  errors");
    if (verbose & 2)
	printf("  by 1,2  by 3 bits");
    printf ("\n---------------------------------------------------------\n");
#ifdef IP2DIAG
	addmessage(verbose);
	addmessage(verbose);
#endif IP2DIAG
    ptotal ("ADD      ", ADDSGL, ADDSGLUPD,ADDDBL, ADDDBLUPD, verbose);
    ptotal ("SUB      ", SUBSGL, SUBSGLUPD, SUBDBL, SUBDBLUPD, verbose);
#ifdef IP2DIAG
	addmessage(verbose);
	addmessage(verbose);
#endif IP2DIAG
    ptotal ("REVSUB   ", REVSUBSGL, REVSUBSGLUPD, REVSUBDBL, REVSUBDBLUPD, verbose);
    ptotal ("MUL      ", MULSGL, MULSGLUPD, MULDBL, MULDBLUPD, verbose);
#ifdef IP2DIAG
	addmessage(verbose);
	addmessage(verbose);
#endif IP2DIAG
    ptotal ("DIV      ", DIVSGL, DIVSGLUPD, DIVDBL, DIVDBLUPD, verbose);
    ptotal ("REVDIV   ", REVDIVSGL, REVDIVSGLUPD, REVDIVDBL, REVDIVDBLUPD, verbose);
#ifdef IP2DIAG
	addmessage(verbose);
	addmessage(verbose);
#endif IP2DIAG
    ptotal ("INT-FLOAT", INTSGL, INTSGLUPD, INTDBL, INTDBLUPD, verbose);
    ptotal ("FLOAT-INT", SGLINT, SGLINTUPD, DBLINT, DBLINTUPD, verbose);
#ifdef IP2DIAG
	addmessage(verbose);
	addmessage(verbose);
#endif IP2DIAG
    ptotal ("SGL<->DBL", SGLDBL, SGLDBLUPD, DBLSGL, DBLSGLUPD, verbose);
    ptotal ("NEGATE   ", NEGSGL, NEGSGLUPD, NEGDBL, NEGDBLUPD, verbose);
#ifdef IP2DIAG
	addmessage(verbose);
	addmessage(verbose);
#endif IP2DIAG
    printf ("TST       %7x         %7x\n", operror[TSTSGL], operror[TSTDBL]);
    printf ("CMP       %7x         %7x\n", operror[CMPSGL], operror[CMPDBL]);
#ifdef IP2DIAG
	addmessage(verbose);
	addmessage(verbose);
#endif IP2DIAG
    if (verbose & 2) {
	printf ("\ndivbyzero = %x, overflow = %x, underflow = %x, nan = %x\n",
		erdivbyzero, eroverflow, erunderflow, ernan);
	printf ("denormalize = %x, inexact = %x, illegalop = %x\n",
	    erdenormalize, erinexact, erillegalop);
        }
    }

ptotal (s, op1, op2, op3, op4, verbose)
char *s;
long verbose;
int op1, op2, op3, op4;
{
    printf ("%s %7x %7x %7x %7x",s,
	operror[op1], operror[op2], operror[op3], operror[op4]);
    if (verbose & 2)
	printf (" %7x %7x\n", 
	oproundnear[op1]+oproundnear[op2]+oproundnear[op3]+oproundnear[op4],
	oproundfar[op1] +oproundfar[op2] +oproundfar[op3] +oproundfar[op4]);
    else
	printf ("\n");
    }

addmessage () {
    }
