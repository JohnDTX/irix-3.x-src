/*
 *	Kurt Akeley
 *	14 May 1985
 *
 *	Aug 8 - Dave P. - added double precision and coversion/conditional ops
 *
 *	Code for a complete arithmetic test of the fpa.
 */

#include <fpa.h>

extern long erunderflow;
extern long eroverflow;
extern long erdivbyzero;
extern long erdenormalize;
extern long ernan;
extern long erillegalop;
extern long erinexact;

extern unsigned long left, right;
extern unsigned long leftlo, lefthi, rightlo, righthi;
extern long databus;
extern long globerrors;
extern long operror[];
extern long oproundnear[];
extern long oproundfar[];

#define makefloat(l)	(* ((float *) &(l)))
static long float makedbl ();
#define makelong(f)	(* ((long * ) &(f)))
static long makelonglo ();
static long makelonghi ();
static long bitsoff ();
static long bitsoffdbl ();

static int oops;
static    unsigned long expect, got, gotupd;
static    unsigned long expectlo, expecthi, gotlo, gothi, gotupdlo, gotupdhi;
static    int update;
static    int gotER;	/* complement of the ER (error register) contents  */
static    int expectER;	/* bits set for expected errors in ER (error reg)  */
static    int gotCR;	/* CR (condition register) contents		   */
static    int expectCR;	/* expected contents of CR.			   */


dofpop (verbose, opcode, leftreg, rightreg, optype, restype)
int verbose;
int opcode, leftreg, rightreg;
int optype, restype;		/* operand and result data types, either:   */
				/*	1. single, 2. double, or 3. neither */
				/*	(neither implies Condition Register)*/
{
    /*
     *	Executes the specified operation with the contents of the specified
     *	  registers using the fpa board.  If rightreg
     *    is zero, the data are taken from the global variable
     *	  databus and are presented with the opcode as a single
     *	  instruction.  If the opcode is UPD type, the result register
     *	  contents are compared with the left register contents after
     *	  the operation is complete.
     *	The expected result of the operation is computed by reading
     *	  the contents of the specified registers before the operation
     *	  is performed, then performing the operation in software.
     *	  The actual result is compared with the expected
     *	  result, and an error is reported if they do not match closely
     *	  enough (not yet specified).
     */

    float	singlevalue;
    long float	doublevalue;
    int ignoredata;	/* flag set when no valid result expected from fpa */

    long longvalue;
    int intvalue;
 
    update = FALSE;
    oops = FALSE;

    expectER = 0;
    ignoredata = FALSE;

    /* Simulate Floating Point opcode in software			   */

    switch (opcode) {
	case ADDSGLUPD:
	    update = TRUE;
	case ADDSGL:
	    singlevalue = makefloat (left) + makefloat (right);
	    break;
	case ADDDBLUPD:
	    update = TRUE;
	case ADDDBL:
	    doublevalue = makedbl (lefthi,leftlo) + makedbl (righthi,rightlo);
	    break;
	case SUBSGLUPD:
	    update = TRUE;
	case SUBSGL:
	    singlevalue = makefloat (left) - makefloat (right);
	    break;
	case SUBDBLUPD:
	    update = TRUE;
	case SUBDBL:
	    doublevalue = makedbl (lefthi,leftlo) - makedbl (righthi,rightlo);
	    break;
	case REVSUBSGLUPD:
	    update = TRUE;
	case REVSUBSGL:
	    singlevalue = makefloat (right) - makefloat (left);
	    break;
	case REVSUBDBLUPD:
	    update = TRUE;
	case REVSUBDBL:
	    doublevalue = makedbl (righthi,rightlo) - makedbl (lefthi,leftlo);
	    break;
	case MULSGLUPD:
	    update = TRUE;
	case MULSGL:
	    singlevalue = makefloat (left) * makefloat (right);
	    break;
	case MULDBLUPD:
	    update = TRUE;
	case MULDBL:
	    doublevalue = makedbl (lefthi,leftlo) * makedbl (righthi,rightlo);
	    break;
	case DIVSGLUPD:
	    update = TRUE;
	case DIVSGL:
	    singlevalue = makefloat (left) / makefloat (right);
/*************
Add Capabilities to detect Overflow and underflow for Division
**************/
	    if (right == 0)
		expectER |= ER_DIVBYZERO; /* **** Add Special Cases **** */
	    break;
	case DIVDBLUPD:
	    update = TRUE;
	case DIVDBL:
	    doublevalue = makedbl (lefthi,leftlo) / makedbl (righthi,rightlo);
	    if ((righthi == 0) && (rightlo == 0))
		expectER |= ER_DIVBYZERO;
	    break;
	case REVDIVSGLUPD:
	    update = TRUE;
	case REVDIVSGL:
	    singlevalue = makefloat (right) / makefloat (left);
	    if (left == 0)
		expectER |= ER_DIVBYZERO;
	    break;
	case REVDIVDBLUPD:
	    update = TRUE;
	case REVDIVDBL:
	    doublevalue = makedbl (righthi,rightlo) / makedbl (lefthi,leftlo);
	    if ((lefthi == 0) && (leftlo == 0))
		expectER |= ER_DIVBYZERO;
	    break;
	case SGLINTUPD:
	    update = TRUE;
	case SGLINT:
	    intvalue = ((right & 0x7f800000) >> 23) - 127; /* unbiased exp  */
	    if (intvalue >=32)			/* too large for long int   */
		expectER |= ER_OVERFLOW;
	    longvalue = makefloat (right);	/* single to integer conv.  */
	    singlevalue = makefloat (longvalue);
	    break;
	case DBLINTUPD:
	    update = TRUE;
	case DBLINT:
	    intvalue = ((righthi & 0x7ff00000) >> 20) - 1023;/* unbiased exp*/
	    if (intvalue >=32)			/* too large for long int   */
		expectER |= ER_OVERFLOW;
	    longvalue = makedbl (righthi,rightlo); /* single to integer conv*/
	    singlevalue = makefloat (longvalue);
	    break;
	case INTSGLUPD:
	    update = TRUE;
	case INTSGL:
	    singlevalue = right;
	    break;
	case INTDBLUPD:
	    update = TRUE;
	case INTDBL:
	    doublevalue = right;
	    break;
	case SGLDBLUPD:
	    update = TRUE;
	case SGLDBL:
	    doublevalue = makefloat (right);
	    break;
	case DBLSGLUPD:
	    update = TRUE;
	case DBLSGL:
	    intvalue = ((righthi & 0x7ff00000) >> 20) - 1023;/* unbiased exp*/
	    if ((intvalue + 127) >= 255)	/* too large for single prec*/
		expectER |= ER_OVERFLOW;
	    singlevalue = makedbl (righthi,rightlo);
	    break;
	case NEGSGLUPD:
	    update = TRUE;
	case NEGSGL:
	    longvalue = right ^ 0x80000000;
	    singlevalue = makefloat (longvalue);
	    break;
	case NEGDBLUPD:
	    update = TRUE;
	case NEGDBL:
	    doublevalue = makedbl ((righthi ^ 0x80000000),rightlo);
	    break;
	case TSTSGL:
	    if (right == 0)
		expectCR = 0;
	    else if ((right & 0x80000000) == 0)
		expectCR = 1;
	    else
	        expectCR = 0x80;
	    break;
	case TSTDBL:
	    if ((righthi == 0) && (rightlo == 0))
		expectCR = 0;
	    else if ((righthi & 0x80000000) == 0)
		expectCR = 1;
	    else
	        expectCR = 0x80;
	    break;
	case CMPSGL:
	    if (right == left)
		expectCR = 0;
	    else if (makefloat(left) > makefloat(right))
		expectCR = 1;
	    else
		expectCR = 0x80;
	    break;
	case CMPDBL:
	    if ((righthi == lefthi) && (rightlo == leftlo))
		expectCR = 0;
	    else if (makedbl(lefthi,leftlo) > makedbl(righthi,rightlo))
		expectCR = 1;
	    else
		expectCR = 0x80;
	    break;
	default:
	    if (verbose)
		printf ("opcode %x not supported\n", opcode);
	    break;
	}

    if (restype == SINGLESIZE) {		/* Single precision result */
	got = Instrl (SINGLE, RESULT);
/* DEBUG got ^= 0x2;   */
	expect = makelong (singlevalue);
        }
    else {				/* Double prec. or Cond reg result */
	gotlo = Instrl (DOUBLELO, RESULT);
/* DEBUG  gotlo ^= 0x3;  */
	gothi = Instrl (DOUBLEHI, RESULT);
	expectlo = makelonglo (doublevalue);
	expecthi = makelonghi (doublevalue);
        }
    gotER = (~(Instrb (ER, 0, 0))) & 0x7F;	/* get Error Reg contents  */

    if (expectER & ER_NAN) {
	ernan += 1;
	if ((gotER & ER_NAN) == 0) {
	    globerrors += 1;
	    operror[opcode] += 1;
	    printf("ERROR: Expected Not A Number (NAN) exception\n");
	    expectER &= ER_NAN;		/* remove lower priority errors */
	    errormsg(verbose,opcode,leftreg,rightreg,optype,restype);
	    }
        }
    else if (expectER & ER_ILLEGALOP) {
	erillegalop += 1;
	if ((gotER & ER_ILLEGALOP) == 0) {
	    globerrors += 1;
	    operror[opcode] += 1;
	    printf("ERROR: Expected Illegal Operation exception\n");
	    expectER &= ER_ILLEGALOP;	/* remove lower priority errors */
	    errormsg(verbose,opcode,leftreg,rightreg,optype,restype);
	    }
        }
    else if (expectER & ER_DENORMALIZE) {
	erdenormalize += 1;
	if ((gotER & ER_DENORMALIZE) == 0) {
	    globerrors += 1;
	    operror[opcode] += 1;
	    printf("ERROR: Expected Denormalized Operands exception\n");
	    expectER &= ER_DENORMALIZE;	/* remove lower priority errors */
	    errormsg(verbose,opcode,leftreg,rightreg,optype,restype);
	    }
        }
    else if (expectER & ER_DIVBYZERO) {
	erdivbyzero += 1;
	if ((gotER & ER_DIVBYZERO) == 0) {
	    globerrors += 1;
	    operror[opcode] += 1;
	    printf("ERROR: Expected Divide by Zero exception\n");
	    expectER &= ER_DIVBYZERO;	/* remove lower priority errors */
	    errormsg(verbose,opcode,leftreg,rightreg,optype,restype);
	    }
        }
    else if (expectER & ER_OVERFLOW) {
	eroverflow += 1;
	if ((gotER & ER_OVERFLOW) == 0) {
	    globerrors += 1;
	    operror[opcode] += 1;
	    printf("ERROR: Expected result Overflow exception\n");
	    expectER &= ER_OVERFLOW;	/* remove lower priority errors */
	    errormsg(verbose,opcode,leftreg,rightreg,optype,restype);
	    }
        }

    else if (restype == SINGLESIZE) {	/* Single precision results	    */
	if (update) {
	    gotupd = Instrl (SINGLE, leftreg);
	    if (got != gotupd) {
		ignoredata = TRUE;
		globerrors += 1;
		operror[opcode] += 1;
		if (verbose) {
		  printf ("ERROR, update reg value not equal to result reg\n");
		  errormsg(verbose,opcode,leftreg,rightreg,optype,restype);
		  }
	        }
	    }
	if ( (((expect >> 23) & 0xff) == 0) /* Underflow or Zero Result     */
		&& (opcode != SGLINT) && (opcode != SGLINTUPD)
		&& (opcode != DBLINT) && (opcode != DBLINTUPD) )
	    {
            ignoredata = TRUE;		   /* indicated by zero exponent.   */
	    expectER |= ER_UNDERFLOW;
	    if (gotER & ER_UNDERFLOW)
		erunderflow += 1;
	    else if ((got & 0x7fffffff) !=0) {
	    	globerrors += 1;
	    	operror[opcode] += 1;
	    	if (verbose) {
		    printf ("ERROR: Expected Underflow or Zero Result\n");
		    expect = 0;
		    errormsg(verbose,opcode,leftreg,rightreg,optype,restype);
		    }
	        }
	    }
	if (ignoredata == FALSE) {
	    long bitsdif;

	    bitsdif = bitsoff(got,expect);
	    if ((1 <= bitsdif) && (bitsdif <= 2))
	        oproundnear[opcode] += 1;
	    else if (3 == bitsdif)
		oproundfar[opcode] +=1;
	    else if (0 != bitsdif) {
		globerrors += 1;
		operror[opcode] += 1;
		if (verbose)
    		    errormsg(verbose,opcode,leftreg,rightreg,optype,restype);
	        }
	    }
       }

    else if (restype == DOUBLESIZE) {
	if (update) {
	    gotupdhi = Instrl (DOUBLEHI, leftreg);
	    gotupdlo = Instrl (DOUBLELO, leftreg);
	    if ((gothi != gotupdhi) || (gotlo != gotupdlo)) {
		ignoredata = TRUE;
		globerrors += 1;
		operror[opcode] += 1;
		if (verbose) {
		  printf ("ERROR, update reg value not equal to result reg\n");
		  errormsg(verbose,opcode,leftreg,rightreg,optype,restype);
		  }
		}
	    }
	if ( ((expecthi >> 20) & 0x7ff) == 0) { /* Underflow or Zero Result */
            ignoredata = TRUE;		   /* indicated by zero exponent.   */
	    expectER |= ER_UNDERFLOW;
	    if (gotER & ER_UNDERFLOW)
	        erunderflow += 1;
	    else if ((gothi & 0x7fffffff) | (gotlo)) {
	    	globerrors += 1;
	    	operror[opcode] += 1;
	    	if (verbose) {
		    printf ("ERROR: Expected Underflow or Zero Result\n");
		    expect = 0;
		    errormsg(verbose,opcode,leftreg,rightreg,optype,restype);
		    }
	        }
            }
	if (ignoredata == FALSE) {
	    long bitsdif;

	    bitsdif = bitsoffdbl (expecthi,expectlo,gothi,gotlo);
	    if ((1 <= bitsdif) && (bitsdif <= 2))
	        oproundnear[opcode] += 1;
	    else if (3 == bitsdif)
		oproundfar[opcode] +=1;
	    else if (0 != bitsdif) {
		globerrors += 1;
		operror[opcode] += 1;
		if (verbose)
    		    errormsg(verbose,opcode,leftreg,rightreg,optype,restype);
		}
	    }
        }
    else {			/* restype = CONDREG */
	gotCR = Instrb (CR, 0 , 0);
	if (expectCR != gotCR) {
	    globerrors += 1;
	    operror[opcode] +=1;
	    if (verbose) {
		printf("ERROR: Condition Register returns wrong value\n");
		errormsg(verbose,opcode,leftreg,rightreg,optype,restype);
	        }
	    }
        }

    if (gotER & ER_INEXACT)
	erinexact += 1;
    }



static long float makedbl (hi,lo)
long hi,lo;
{
    long m[2];
    m[0] = hi;
    m[1] = lo;
    return( *((long float *)m) );
    }

static long makelonglo (dbl)
long float dbl;
{
    long *p = (long *)&dbl;
    return( p[1] );
    }

static long makelonghi (dbl)
long float dbl;
{
    long *p = (long *)&dbl;
    return( p[0] );
    }

static long bitsoff (a, b)
long a, b;
{
    long dif;

    if ( (a & 0x80000000) != (b & 0x80000000) )	/* Are Signs equal ?        */
	return -1;
    a = a & 0x7fffffff;				/* Exponent and Mantissa    */
    b = b & 0x7fffffff;
    dif = a - b;
    if (dif < 0)
	dif = -dif;
    return dif;
    }

static long bitsoffdbl (ahi,alo,bhi,blo)
unsigned long ahi, alo, bhi, blo;
{
    long dif;

    if ( (ahi & 0x80000000) != (bhi & 0x80000000) ) /* Are Signs equal ?    */
	return -1;
    dif = ahi - bhi;
    if (dif & 0x80000000)		/* Abs value of difference */
	dif = -dif;
    if (dif > 1)		/* Most signicant halves should not differ  */
	return -1;		/* by more than one for roundoff error	    */
    dif = alo - blo;
    if (dif & 0x80000000)		/* Abs value of difference */
	dif = -dif;
    return dif;
    }

#ifdef IP2DIAG
_raise_fperror () {
    /* called whenever a software floating point error is detected */
    oops = TRUE;
    }
#endif IP2DIAG

errormsg(verbose,opcode,leftreg,rightreg,optype,restype)
long verbose,opcode,leftreg,rightreg;
int optype,restype;
{
if (verbose) {
#ifdef IP2DIAG
	addmessage(verbose);
	addmessage(verbose);
	addmessage(verbose);
	addmessage(verbose);
	addmessage(verbose);
    if (update)
	addmessage(verbose);
#endif IP2DIAG
    printf ("ERROR: opcode=%02x\n", opcode);
    printf ("    error reg (complemented): expect=%02x, got=%02x\n", expectER,
						    		gotER);
    if (restype == SINGLESIZE) {
	printf ("    result reg: expect=%08x, got=%08x\n", expect, got);
	if (update) {
	    printf ("    update value in Reg %01x now is %08x\n",
							leftreg,gotupd);
	    }
        }
    else if (restype == DOUBLESIZE) {
	printf ("    result reg: expect=%08x %08x, got=%08x %08x\n", 
			expecthi, expectlo, gothi, gotlo);
	if (update)
	    printf ("    update value in Reg %01x now is %08x %08x\n",
			leftreg, gotupdhi, gotupdlo);
        }
    else if (restype == CONDREG)
	printf ("    condition reg: expect=%02x, got=%02x\n",expectCR,gotCR);


  if (optype == SINGLESIZE) {
	if (update)
	    printf ("    left operand formerly in Reg %01x was %08x\n",
							leftreg,left);
	else
    	    printf ("    leftop: Reg %01x = %08x",leftreg,
						Instrl(SINGLE,leftreg));
	printf ("    rightop: Reg %01x = %08x\n",rightreg,
						Instrl(SINGLE,rightreg));
        }
  else {				/* optype == DOUBLESIZE		    */
	if (update)
	    printf ("    left operand formerly in Reg %01x was %08x %08x\n",
						leftreg,lefthi,leftlo);
	else
    	    printf ("    leftop: Reg %01x = %08x %08x",leftreg,
			Instrl(DOUBLEHI,leftreg),Instrl(DOUBLELO,leftreg));
	printf ("    rightop: Reg %01x = %08x %08x\n",rightreg,
			Instrl(DOUBLEHI,rightreg),Instrl(DOUBLELO,rightreg));
        }
    }
    }













