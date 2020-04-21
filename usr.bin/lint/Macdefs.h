# define makecc(val,i)  lastcon = i ? (val<<8)|lastcon : val  

# define  ARGINIT 64
# define  AUTOINIT 0

# define  SZCHAR 8
# define  SZINT 32
# define  SZFLOAT 32
# define  SZDOUBLE 64
# define  SZLONG 32
# define  SZSHORT 16
# define  SZPOINT 32

# define ALCHAR 8
# define ALSHORT 16
#ifdef	FIXED_ALIGNMENT
# define ALINT 16
# define ALFLOAT 16
# define ALDOUBLE 16
# define ALLONG 16
# define ALPOINT 16
# define ALSTRUCT 16
# define ALSTACK 16
#else
/****
*****	(KIPP)	structure alignment is controlled via a compiler option.
*****		Because the compiler uses the actual member types to
*****		force alignment (hmm) we have to over-ride many of
*****		the alignment values.  The values we use are:
*****/
/****	    	structure members on short boundaries:
*****/
# define		AL16_ALINT 16
# define		AL16_ALFLOAT 16
# define		AL16_ALDOUBLE 16
# define		AL16_ALLONG 16
# define		AL16_ALPOINT 16
# define		AL16_ALSTRUCT 16
# define		AL16_ALSTACK 16
/****	    	structure members on short boundaries:
*****/
# define		AL32_ALINT 32
# define		AL32_ALFLOAT 32
# define		AL32_ALDOUBLE 32
# define		AL32_ALLONG 32
# define		AL32_ALPOINT 32
# define		AL32_ALSTRUCT 32
# define		AL32_ALSTACK 32
/****		These variables are used by the main code, and are set up
*****		via a switch to the compiler.  By default, we use our
*****		old fixed alignment values (align everything except chars on
*****		short boundaries)
*****/
extern	int	ALINT, ALFLOAT, ALDOUBLE, ALLONG, ALPOINT, ALSTRUCT, ALSTACK;
#endif

/*	size in which constants are converted */
/*	should be long if feasable */

# define CONSZ long
# define CONFMT "0x%lx"

/*	size in which offsets are kept
/*	should be large enough to cover address space in bits
*/

# define OFFSZ long

/* 	character set macro */

# define  CCTRANS(x) x

/* register cookie for stack poINTer */

# define  STKREG 14
# define ARGREG 14

/*	maximum and minimum register variables */

# define MAXRVAR 7
# define MINRVAR 2

	/* various standard pieces of code are used */
# define STDPRTREE
# define LABFMT ".L%d"

/* definition indicates automatics and/or temporaries
   are on a negative growing stack */

# define BACKAUTO
# define BACKTEMP

# ifndef FORT
# ifndef LINT
# define ONEPASS
# endif
# endif

# ifndef FORT
# define EXIT dexit
# endif

# define ENUMSIZE(high,low) INT
#define FIXDEF(p) outstab(p)
#define FIXARG(p) fixarg(p)
#define FIXSTRUCT outstruct

/** GB - Kipps hack **/
int reservea5;
