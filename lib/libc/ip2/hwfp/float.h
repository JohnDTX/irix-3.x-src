/*
 *
 *	float.h  -  file of definitions and macros for assembling
 *		    addresses which are really floating point operations
 *		    in disguise.
 *
 *
 *	there are four types of operations:
 *
 *	   internal operations:  These perform an operation on two of
 *		the internal fp registers.  The result goes to f15.
 *	   internal operations with update: These perform an operation
 *		on two of the internal fp registers and update the
 *		first operand with the result as well as f15.
 *	   data bus operations:  These perform an operation where the
 *		first operand is the data on the bus and the second is
 *		an internal register.  If the operation is double precision,
 *		the data on the bus is the most significant long of the
 *		first operand.  This is combined with the least significatn
 *		long of f0 to form the first operand.
 *	   data bus reverse operations:  These perform an operation where
 *		the first operand is an internal register and the second
 *		is data on the bus.  Double operand handled similarly to above.
 *	   
 *	   data bus operations and data bus reverse operations also have
 *	   versions which update the first operand with the result.
 *
 *	the operations are coded into the macros as follows:
 *
 *	   the first character in the macro indicates the precision: 'f' or 'd'.
 *
 *	   if the next character is an 'r', the reverse form of the operation
 *		is indicated.
 *
 *	   the next three or five characters in the macro name indicate 
 *		the operation.  Three characters are used except in convert,
 *		in which case the source and dest type are indicated (idcvt ->
 *		integer to double convert)
 *
 *		add, sub, mul, cmp, div 
 *
 *	   if the next character is an 'x', an external data value is
 *		one of the operands.  The x is followed by a '0' if the
 *		external value is the first operand, by a '1' if it is the
 *		second operand.
 *
 *	   if the macro ends in 'upd', the update version of the operation
 *		is indicated.
 *
 *	thus:
 *
 *		frdivxupd(4,d1)
 *
 *		    divide d1/f4, where d1 is the contents of data reg 1 (off
 *		    the bus), and update f4 with the result (as well as f15).
 *		    single precision.
 *
 *		dmul(2,3)
 *
 *		    mul f2*f3, and place the result in f15 (only).  double
 *		    precision.
 *	   
 *	The macros simply generate an address.
 *
 */

#define NOEXTERNALDIV

/* special registers */
#define FPA_OR		0x8000+0x600:w
#define FPA_CCR		0x8000+0x700:w
#define FPA_ER		0x8000+0x800:w
#define FPA_MR		0x8000+0x900:w


/* operations */
#define SADD 	 0xa00
#define SADDUPD  0xb00
#define DADD	 0xc00
#define DADDUPD	 0xd00
#define SSUB	 0xe00
#define SSUBUPD  0xf00
#define DSUB	 0x1000
#define DSUBUPD	 0x1100
#define SRSUB	 0x1200
#define SRSUBUPD 0x1300
#define DRSUB	 0x1400
#define DRSUPUPD 0x1500
#define SMUL 	 0x1600
#define SMULUPD	 0x1700
#define DMUL	 0x1800
#define DMULUPD  0x1900
#define SDIV 	 0x1a00
#define SDIVUPD	 0x1b00
#define DDIV	 0x1c00
#define DDIVUPD	 0x1d00
#define SRDIV	 0x1e00
#define SRDIVUPD 0x1f00
#define DRDIV	 0x2000
#define DRDIVUPD 0x2100

#define DSCVT	 0x2400
#define DSCVTUPD 0x2500
#define DICVT	 0x2c00
#define DICVTUPD 0x2d00

#define SDCVT	 0x2200
#define SDCVTUPD 0x2300
#define SICVT	 0x2800
#define SICVTUPD 0x2900

#define IDCVT	 0x2a00
#define IDCVTUPD 0x2b00
#define ISCVT	 0x2600
#define ISCVTUPD 0x2700

#define SNEG	 0x2e00
#define SNEGUPD	 0x2f00
#define DNEG	 0x3000
#define DNEGUPD	 0x3100

#define SCMP	 0x3200
#define DCMP	 0x3300
#define STST	 0x3400
#define DTST	 0x3500

#define COPY	 0x0500
#define DWRITEHI 0x0300
#define DWRITELO 0x0400
#define SWRITE	 0x0100
#define DREADHI  0x0300
#define DREADLO  0x0400
#define SREAD	 0x0100

/* register operations */
#define fwritefpa(a,b)	movl a,0x8000+SWRITE+0x/**/b/**/0:w
#define freadfpa(a,b)	movl 0x8000+SREAD+0x/**/a/**/0:w,b
#define dwritefpahi(a,b)	movl a,0x8000+DWRITEHI+0x/**/b/**/0:w
#define dwritefpalo(a,b)	movl a,0x8000+DWRITELO+0x/**/b/**/0:w
#define dreadfpahi(a,b)	movl 0x8000+DREADHI+0x/**/a/**/0:w,b
#define dreadfpalo(a,b)	movl 0x8000+DREADLO+0x/**/a/**/0:w,b
#define dcopyfpa(a,b)	movb d0,0x8000+COPY+0x/**/a/**/0+0x/**/b/**/:w

/* single precision add operations */
#define fadd(a,b)	movb d0,0x8000+SADD+0x/**/a/**/0+0x/**/b:w
#define faddx1(a,b)	movl b,0x8000+SADD+0x/**/a/**/0:w
#define faddupd(a,b)	movb d0,0x8000+SADDUPD+0x/**/a/**/0+0x/**/b:w
#define faddx1upd(a,b)	movl b,0x8000+SADDUPD+0x/**/a/**/0:w

/* single precision subtract operations */
#define fsub(a,b)	movb d0,0x8000+SSUB+0x/**/a/**/0+0x/**/b:w
#define frsub(a,b)	movb d0,0x8000+SRSUB+0x/**/a/**/0+0x/**/b:w
#define fsubx1(a,b)	movl b,0x8000+SSUB+0x/**/a/**/0:w
#define frsubx1(a,b)	movl b,0x8000+SRSUB+0x/**/a/**/0:w
#define fsubupd(a,b)	movb d0,0x8000+SSUBUPD+0x/**/a/**/0+0x/**/b:w
#define frsubupd(a,b)	movb d0,0x8000+SRSUBUPD+0x/**/a/**/0+0x/**/b:w
#define fsubx1upd(a,b)	movl b,0x8000+SSUBUPD+0x/**/a/**/0:w
#define frsubx1upd(a,b)	movl b,0x8000+SRSUBUPD+0x/**/a/**/0:w

/* single precision multiply operations */
#define fmul(a,b)	movb d0,0x8000+SMUL+0x/**/a/**/0+0x/**/b:w
#define fmulx1(a,b)	movl b,0x8000+SMUL+0x/**/a/**/0:w
#define fmulupd(a,b)	movb d0,0x8000+SMULUPD+0x/**/a/**/0+0x/**/b:w
#define fmulx1upd(a,b)	movl b,0x8000+SMULUPD+0x/**/a/**/0:w

/* single precision divide operations */
#define fdiv(a,b)	movb d0,0x8000+SDIV+0x/**/a/**/0+0x/**/b:w
#define frdiv(a,b)	movb d0,0x8000+SRDIV+0x/**/a/**/0+0x/**/b:w
#define fdivupd(a,b)	movb d0,0x8000+SDIVUPD+0x/**/a/**/0+0x/**/b:w
#define frdivupd(a,b)	movb d0,0x8000+SRDIVUPD+0x/**/a/**/0+0x/**/b:w
#ifdef NOEXTERNALDIV
#define fdivx1(a,b)	movl b,0x8000+SDIV+0x/**/a/**/0:w
#define frdivx1(a,b)	movl b,0x8000+SRDIV+0x/**/a/**/0:w
#define fdivx1upd(a,b)	movl b,0x8000+SDIVUPD+0x/**/a/**/0:w
#define frdivx1upd(a,b)	movl b,0x8000+SRDIVUPD+0x/**/a/**/0:w
#endif

/* double precision add operations */
#define dadd(a,b)	movb d0,0x8000+DADD+0x/**/a/**/0+0x/**/b:w
#define daddx1(a,b)	movl b,0x8000+DADD+0x/**/a/**/0:w
#define daddupd(a,b)	movb d0,0x8000+DADDUPD+0x/**/a/**/0+0x/**/b:w
#define daddx1upd(a,b)	movl b,0x8000+DADDUPD+0x/**/a/**/0:w

/* double precision subtract operations */
#define dsub(a,b)	movb d0,0x8000+DSUB+0x/**/a/**/0+0x/**/b:w
#define drsub(a,b)	movb d0,0x8000+DRSUB+0x/**/a/**/0+0x/**/b:w
#define dsubx1(a,b)	movl b,0x8000+DSUB+0x/**/a/**/0:w
#define drsubx1(a,b)	movl b,0x8000+DRSUB+0x/**/a/**/0:w
#define dsubupd(a,b)	movb d0,0x8000+DSUBUPD+0x/**/a/**/0+0x/**/b:w
#define drsubupd(a,b)	movb d0,0x8000+DRSUBUPD+0x/**/a/**/0+0x/**/b:w
#define dsubx1upd(a,b)	movl b,0x8000+DSUBUPD+0x/**/a/**/0:w
#define drsubx1upd(a,b)	movl b,0x8000+DRSUBUPD+0x/**/a/**/0:w

/* double precision multiply operations */
#define dmul(a,b)	movb d0,0x8000+DMUL+0x/**/a/**/0+0x/**/b:w
#define dmulx1(a,b)	movl b,0x8000+DMUL+0x/**/a/**/0:w
#define dmulupd(a,b)	movb d0,0x8000+DMULUPD+0x/**/a/**/0+0x/**/b:w
#define dmulx1upd(a,b)	movl b,0x8000+DMULUPD+0x/**/a/**/0:w

/* double precision divide operations */
#define ddiv(a,b)	movb d0,0x8000+DDIV+0x/**/a/**/0+0x/**/b:w
#define drdiv(a,b)	movb d0,0x8000+DRDIV+0x/**/a/**/0+0x/**/b:w
#define ddivupd(a,b)	movb d0,0x8000+DDIVUPD+0x/**/a/**/0+0x/**/b:w
#define drdivupd(a,b)	movb d0,0x8000+DRDIVUPD+0x/**/a/**/0+0x/**/b:w
#ifdef NOEXTERNALDIV
#define ddivx1(a,b)	movl b,0x8000+DDIV+0x/**/a/**/0:w
#define drdivx1(a,b)	movl b,0x8000+DRDIV+0x/**/a/**/0:w
#define ddivx1upd(a,b)	movl b,0x8000+DDIVUPD+0x/**/a/**/0:w
#define drdivx1upd(a,b)	movl b,0x8000+DRDIVUPD+0x/**/a/**/0:w
#endif

/* single precision compare operations */
#define fcmp(a,b)	movb d0,0x8000+SCMP+0x/**/a/**/0+0x/**/b:w
#define fcmpx1(a,b)    	movl b,0x8000+SCMP+0x/**/a/**/0:w

/* double precision compare operations */
#define dcmp(a,b)	movb d0,0x8000+DCMP+0x/**/a/**/0+0x/**/b:w
#define dcmpx1(a,b)    	movl b,0x8000+DCMP+0x/**/a/**/0:w

/* convert from double operations */
#define dfcvt(a)	movb d0,0x8000+DSCVT+0x/**/a:w
#define dfcvtx0(a)	movl a,0x8000+DSCVT:w
#define dfcvtupd(a,b)	movb d0,0x8000+DSCVTUPD+0x/**/b/**/0+0x/**/a:w
#define dfcvtx0upd(a,b) movl a,0x8000+DSCVT+0x/**/b/**/0:w
#define dicvt(a)	movb d0,0x8000+DICVT+0x/**/a:w
#define dicvtupd(a,b)	movb d0,0x8000+DICVTUPD+0x/**/b/**/0+0x/**/a:w
#define dicvtx0(a)	movl a,0x8000+DICVT:w
#define dicvtx0upd(a,b) movl a,0x8000+DICVT+0x/**/b/**/0:w

/* convert from single operations */
#define fdcvt(a)	movb d0,0x8000+SDCVT+0x/**/a:w
#define fdcvtupd(a,b)	movb d0,0x8000+SDCVTUPD+0x/**/b/**/0+0x/**/a:w
#define fdcvtx0(a)	movl a,0x8000+SDCVT:w
#define fdcvtx0upd(a,b) movl a,0x8000+SDCVT+0x/**/b/**/0:w
#define ficvt(a)	movb d0,0x8000+SICVT+0x/**/a:w
#define ficvtupd(a,b)	movb d0,0x8000+SICVTUPD+0x/**/b/**/0+0x/**/a:w
#define ficvtx0(a)	movl a,0x8000+SICVT:w
#define ficvtx0upd(a,b) movl a,0x8000+SICVT+0x/**/b/**/0:w

/* convert from integer operations */
#define idcvt(a)	movb d0,0x8000+IDCVT+0x/**/a:w
#define idcvtupd(a,b)	movb d0,0x8000+IDCVTUPD+0x/**/b/**/0+0x/**/a:w
#define idcvtx0(a)	movl a,0x8000+IDCVT:w
#define idcvtx0upd(a,b) movl a,0x8000+IDCVT+0x/**/b/**/0:w
#define ifcvt(a)	movb d0,0x8000+ISCVT+0x/**/a:w
#define ifcvtupd(a,b)	movb d0,0x8000+ISCVTUPD+0x/**/b/**/0+0x/**/a:w
#define ifcvtx0(a)	movl a,0x8000+ISCVT:w
#define ifcvtx0upd(a,b) movl a,0x8000+ISCVT+0x/**/b/**/0:w

