/*
 *	Kurt Akeley
 *	2 November 1984
 *
 *	Do multibus read/write cycles without aborting due to timeout.
 */

#define label(lab)	{{extern lab();} asm("	.globl	lab"); asm("lab:");}
#define BusErrorVector	8

static long BusErrorReturnAddress;
static long *BerrVector;		/* pointer to the berr vector */
static long vectorbase;			/* vector base from vbr */

static nullfunction () {
    /*
     *	Assumes that a bus error has just occurred.  The stack pointer is
     *    pointed to the pc portion of the bus error frame.  This address is
     *    replaced with the value stored in the global BusErrorReturnAddress.
     *    The stack pointer is than pointed to the status register field of
     *    the bus error frame, and ReTurn from Exception is called.
     *  Entry should always be at the label BusErrorHandler.
     */

    label (BusErrorHandler);
#ifdef PM1
    asm ("	addl	#10,sp");
    asm ("	movl	BusErrorReturnAddress,sp@");
    asm ("	subql	#2,sp");
#endif PM1
#ifdef PM2
    /* replace the long stack frame with a short frame */
    asm ("	movw	sp@,sp@(50)");
    asm ("	addl	#52,sp");
    asm ("	movl	BusErrorReturnAddress,sp@");
    asm ("	addl	#4,sp");
    asm ("	movw	#0,sp@");
    asm ("	subql	#6,sp");    
#endif PM2
#ifdef PM3
    /* replace either 16- or 44-word bus error frame with 4-word frame */
    asm ("	andw	#4096,sp@(6)");		/* test bit 12 */
    asm ("	beq	shortframe");
    /* long frame */
    asm ("	movw	sp@,sp@(80)");
    asm ("	addl	#82,sp");
    asm ("	bra	cleanup");
    /* short frame */
    asm ("shortframe:	movw	sp@,sp@(24)");
    asm ("	addl	#26,sp");
    /* finish the job */
    asm ("cleanup:	movl	BusErrorReturnAddress,sp@");
    asm ("	addl	#4,sp");
    asm ("	movw	#0,sp@");
    asm ("	subql	#6,sp");    
#endif PM3
    asm ("	rte");
    }



extern ReadNotThere ();		/* use an external function as a label */

long saferead (address)
short *address;
{
    /*
     *	Reads the word at the specified address.  Returns a 16-bit value
     *	  if the read is successful, -1 (long) otherwise.
     *  BusErrorVector is restored to its original value after the test is
     *	  complete.
     *	Branching on foo fools the compiler and avoids "unreachable code"
     *	  warnings.
     */

    short	temp;
    long	BEVsave;	/* vector is stored as a number, not address*/
    short	foo;

    foo = 1;
#ifdef PM3
    BerrVector = (long*)((char*)getvbr () + BusErrorVector);
#else
    BerrVector = (long*)BusErrorVector;
#endif PM3
    BEVsave = *BerrVector;
    *BerrVector = (long)BusErrorHandler;
    BusErrorReturnAddress = (long)ReadNotThere;

    temp = *address;
    *BerrVector = BEVsave;
    if (foo)
	return temp & 0xFFFF;

    label (ReadNotThere);
    *BerrVector = BEVsave;
    return -1;
    }



extern WriteNotThere ();

safewrite (address, data)
short *address;
short data;
{
    /*
     *	Writes the word at the specified address.  Returns 1 if the
     *	  the write is successful, 0 if it times out.
     *  BusErrorVector is restored to its original value after the test is
     *	  complete.
     *	Branching on foo fools the compiler and avoids "unreachable code"
     *	  warnings.
     */
    long	BEVsave;	/* vector is stored as a number, not address*/
    short	foo;

    foo = 1;
#ifdef PM3
    BerrVector = (long*)((char*)getvbr () + BusErrorVector);
#else
    BerrVector = (long*)BusErrorVector;
#endif PM3
    BEVsave = *BerrVector;
    *BerrVector = (long)BusErrorHandler;
    BusErrorReturnAddress = (long)WriteNotThere;

    *address = data;
    *BerrVector = BEVsave;
    if (foo)
	return 1;

    label (WriteNotThere);
    *BerrVector = BEVsave;
    return 0;
    }
