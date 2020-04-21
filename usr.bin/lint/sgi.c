#include "mfile1.h"
static inwd;
static long word;

dexit(arg) {
	exit(arg);
}

bccode() 
{
}

int verbose=0;
int retnovallab=0;
prcstab() {}
plcstab() {}
pfstab() {}
outstab() {}
genswitch() {}

char *getftnnm() {
	return((char *)0);
}
vfdzero( n ){ /* define n bits of zeros in a vfd */

	if( n <= 0 ) return;

	inwd += n;
	inoff += n;
	while (inwd >= 16) {
	  word <<= 16;
	  inwd -= 16;
	}
}
cendarg() {} 		
#define sw(x) ((x>>16)&0xFFFF | (x<<16) & 0xFFFF0000)

fincode( d, sz ) long float d; {
	/* output code to initialize space of size sz to the value d */
	/* the proper alignment has been obtained */
	/* inoff is updated to have the proper final value */
	/* on the target machine, write it out in hex(GB)! */

/*****	GB altered fincode to write out data in hex 8/12/83 ******/

	register long *mi = (long *)&d;

#ifdef IEEE
/***** 	A note here is appropriate on IEEE format.  SGI uses
	FULL IEEE format.  We have gone to some lengths to ensure
	that floating point using IEEE is standard and is as efficient
	as possible.  Thus, in our compiler, floats are 32-bits,
	even across calls, doubles are 32-bits, and 'long floats'
	are 64-bit floats in FULL IEEE format. Compiler output of
	our IEEE format is controlled by the IEEE definition. (IF
	it is NOT defined, the compiler will use standard Vax format,
	without any of the alterations above (i.e., floats are extended
	to doubles across calls, etc.).)
	
	The representation of the incoming floating point value
	in this routine has, of course, a drastic effect, as
	we want to dump the correct floating point value in full
	IEEE format in hex to the .s file.  There are three possibilities:
	
		1. The host is a VAX, or uses Vax floating point format.
		   this is true if HOST_DEC_FLOAT is defined.
		   
		2. The host also uses full IEEE format.  In this case, the
		   binary representation does not have to be parsed and
		   the host's double to float conversion routine can be
		   used. In this case HOST_FULL_IEEE is defined.
		   
		3. The host uses a non-standard IEEE format.  This is
		   currently unsupported, but the hooks are in.


	GB (SGI) 9/26/83.
***********/
#ifndef HOST_FULL_IEEE
	/* the binary floating point representation must be parsed. */
	register unsigned long exp;
#ifdef HOST_DEC_FLOAT
#ifdef ONVAX
	mi[0]=sw(mi[0]);
	mi[1]=sw(mi[1]);
#endif
	exp = (mi[0]&0x7F800000)>>23;
	if (exp<3) /* this loses for small double prec. values*/
	{	mi[0]=0;
		if (sz==SZDOUBLE) mi[1]=0;
	}
	else
	{	
		if (sz==SZDOUBLE)
		{	exp += 894;
			mi[1]=
			 ((mi[1]>>3)&0x1FFFFFFF)|((mi[0]<<29)&0xE0000000);
			mi[0]=
			 ((exp<<20)|((mi[0]>>3)&0x000FFFFF))
				|(mi[0]<0 ?0x80000000:0);
		}
		else
		{	exp -= 2;
			mi[0]=(exp<<23)|(mi[0]&0x807FFFFF);
		}
	}

#else HOST_DEC_FLOAT
#ifdef HOST_IEEE_EXTENDED
	/* the host is using an extended floating point format.
	   the parameters of the extended floating point format are:
	   
	   IEEE_SEXP	- number of bits in the single precision exp.
	   IEEE_DEXP	- number of bits in the double precision exp.
	   IEEE_J	- integer part of the format.
	   IEEE_SBIAS	- bias of the single precision exponent.
	   IEEE_DBIAS	- bias of the double precision exponent.

	If it is ever necessary to support this format, these defines can
	be used to take apart the floating point numbers.
	   */
	   
	  
#endif HOST_IEEE_EXTENDED
#endif /* else HOST_DEC_FLOAT*/
#endif /* ifndef HOST_FULL_IEEE */
if( sz==SZDOUBLE )
	;
	else
#ifndef HOST_FULL_IEEE
	/* the conversion to single-precision was done above. */
	;
#else
	/* convert the double (64-bit) floating point number to
	   single precision using native conversion
	*/
	{	float f;
		f = d;
		*mi = *(long *)&f;
		;
	}
#endif
	inoff += sz;
	}
#else IEEE

	if( sz==SZDOUBLE )
		;
	else
		;
	inoff += sz;
	}
#endif

incode( p, sz ) register NODE *p; {

	/* generate initialization code for assigning a constant c
		to a field of width sz */
	/* we assume that the proper alignment has been obtained */
	/* inoff is updated to have the proper final value */
	/* we also assume sz  < SZINT */

	if((sz+inwd) > SZINT) cerror("incode: field > int");
	word |= (p->tn.lval & ((1 << sz) -1)) << (SZINT - sz - inwd);
	inwd += sz;
	inoff += sz;
	while (inwd >= 16) {
	  word <<= 16;
	  inwd -= 16;
	}
}

getlab() {return(0);}
psline() {}
pstab() {}
outstruct() {}
char ititle[500] = "";
int labelno;
locctr() {}
aobeg() {}
aoend() {}
rbusy() {}
p2init() {}
fixarg() {}

/* these should be fixed! **/
int strftn=0;

int nbytes() {}
