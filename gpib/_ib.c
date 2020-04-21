#
# include "stdio.h"
# define reg	register

/*
 * atoci.c --
 * convert (initial) string to number,
 * according to c conventions (leading 0 or 0x).
 */
int
atoci(src)
    char *src;
{
    static int nn;
    cnum(src,&nn);
    return nn;
}
#
/*
 * cnum.c --
 * convert (initial) string to number,
 * according to c conventions (leading 0 or 0x).
 * store it in user-supplied cell.
 * return the number of chrs used.
 */
int
cnum(src,ip)
    char *src;
    int *ip;
{
    reg char *sp;
    reg int radix,number;
    unsigned digit;
    char negflag;
    /*examine initial part of src*/
    sp = src;
    number = 0;
    radix = 10;			/*default radix is decimal*/
    if( (negflag = *sp=='-') || *sp=='+' )
	sp++;			/*skip over optional sign*/
    if( *sp=='0' )
    {				/*leading 0 ==> octal or hex*/
	radix = 010;
	sp++;
	if( *sp=='x' || *sp=='X' )
	{			/*leading 0x ==> hex*/
	    radix = 0x10;
	    sp++;
	}
    }
    for( ;; )
    {				/*build number a digit at a time*/
	digit = *sp;
	if( (digit += 0-'0') <= ('9'-'0') )
	    digit += 0;		/*digit in '0'..'9'*/
	else
	if( (digit += '0'-'a') <= ('f'-'a') )
	    digit += 0xa;	/*digit in 'a'..'f'*/
	else
	if( (digit += 'a'-'A') <= ('F'-'A') )
	    digit += 0xA;	/*digit in 'A'..'F'*/
	else
	    break;		/*not a digit*/
	if( digit>=radix )
	    break;		/*illegal digit*/
	number = number*radix + digit;
	sp++;
    }
    /*now sp points past the last legal digit*/
    *ip = negflag?-number:number;
    return sp-src;
}
#
    extern char *malloc();
/*
 * newstr() --
 * returns a new (malloc()ed) copy of
 * string s, or 0 if no space.
 */
char *newstr(src)
    char *src;
{
    register char *s,*m,*t;
    s = src;
    while( *s++ != 000 )
	;
    if( (m = malloc(s - src)) == 0 )
	return 0;
    s = src;
    t = m;
    while( (*t++ = *s++) != 000 )
	;
    return m;
}
#



# define BYTESPERLINE 16
# define LINESPERPAGE 16
/*
 * prdata() --
 * print arbitrary data in readable format.
 XX XX XX XX XX XX XX XX  XX XX XX XX XX XX XX XX  cccccccc cccccccc
 */
prdata(src,len,F)
    char *src;
    int len;
    FILE *F;
{
    register int iii,bbb;
    int pgoff;
    char *digits;
    register char *s;
    digits = "0123456789ABCDEF";

    pgoff = LINESPERPAGE;
    while( len>0 )
    {
	if( pgoff>=LINESPERPAGE )
	{
	    putc('>',F);
	    pgoff = 0;
	}
	else
	{
	    putc(' ',F);
	}
	pgoff ++;
	s = src;
	for( iii = 0; iii<BYTESPERLINE; iii++ )
	{
	    if( iii==BYTESPERLINE/2 )
		putc(' ',F);
	    putc(' ',F);
	    if( iii>=len )
	    {
		putc(' ',F); putc(' ',F);
	    }
	    else
	    {
		bbb = *s++;
		putc(digits[ (bbb >> 4)&0xF ],F);
		putc(digits[ (bbb >> 0)&0xF ],F);
	    }
	}
	putc(' ',F); putc(' ',F);
	s = src;
	for( iii = 0; iii<BYTESPERLINE; iii++ )
	{
	    if( iii==BYTESPERLINE/2 )
		putc(' ',F);
	    putc(iii>=len?' '
		:(040<*s&&*s<0177)?*s:'.',F);
	    s++;
	}
	putc('\n',F);
	len -= BYTESPERLINE;
	src += BYTESPERLINE;
    }
    fflush(F);
}
#
/*
 * error.c --
 * routines for printing error messages.
 * all print
 *	the program name if available (ie progname!=0),
 *	up to 5 printf-style args,
 *	a newline and flush,
 * to `stderr .
 */





/*things defined here...*/
extern int errwarn(),scerrwarn(),errexit(),scerrexit();
extern char *progname;
extern int warned;

/*things defined elsewhere...*/
extern int fprintf(),fflush(),_exit();
extern int errno,sys_nerr;
extern char *sys_errlist[];

char *progname;		/*global name of program if specified*/
int warned;		/*global error flag set when err package is used*/




static char warnfmt[] = "%s:  ";
static char fatalfmt[] = "(fatal) ";

/*
 * errwarn() --
 * non-fatal error message.
 */
int
errwarn(args)
    int args;
{
    printprog();
    return print5(&args);
}

/*
 * scerrwarn() --
 * non-fatal error message, combined with
 * the standard system error message for
 * a failed system call.
 */
int
scerrwarn(args)
    int args;
{
    printprog();
    printsc();
    return print5(&args);
}

/*
 * errexit() --
 * fatal error message.
 */
errexit(args)
    int args;
{
    printprog();
    fprintf(stderr,fatalfmt);
    _exit(print5(&args));
}

/*
 * scerrexit() --
 * fatal error message, combined with
 * the standard system message for
 * a failed system call.
 */
scerrexit(args)
    int args;
{
    printprog();
    printsc();
    fprintf(stderr,fatalfmt);
    _exit(print5(&args));
}

/*
 * print5() --
 * called with a pointer to up to 5 printf-style args,
 * uses `fprintf() to print them to `stderr.
 * adds newline and flush.
 * sets the global error flag `warned .
 * returns -1 .
 */
static int
print5(argp)
    reg int *argp;
{
    fprintf(stderr,argp[0],argp[1],argp[2],argp[3],argp[4]);
    fprintf(stderr,"\n");
    fflush(stderr);
    warned = -1;
    return-1;
}

/*
 * printsc() --
 * prints the standard system error message
 * corresponding to global error number `errno ,
 * followed by dashes.  prints to `stderr .
 */
static
printsc()
{
    reg int xerrno;
    extern int errno,sys_nerr;
    extern char *sys_errlist[];
    xerrno = errno;
    if( (unsigned)xerrno>=sys_nerr )
	fprintf(stderr,"error %u -- ",xerrno);
    else
	fprintf(stderr,"%s -- ",sys_errlist[xerrno]);
    errno = xerrno;
}

/*
 * printprog() --
 * if the program name is available
 * (ie, `progname!=0) print it followed
 * by a colon and spaces.
 */
static
printprog()
{
    extern int errno;
    reg int xerrno;
    xerrno = errno;
    fflush(stdout);
    if( progname!=0 )
	fprintf(stderr,warnfmt,progname);
    errno = xerrno;
}
