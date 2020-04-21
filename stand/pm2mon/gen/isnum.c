#
# define PROMSTATIC

/*
 * isnum.c --
 * convert (initial) string to number,
 * according to adb conventions (leading 0 or 0[otx]).
 * store it in user-supplied cell.
 */
char *
skipnum(sp,defradix,ip)
    register char *sp;
    int defradix;
    long *ip;
{
    register long radix,number;
    register unsigned digit;
    register int negflag;

    /*determine sign and radix*/
    number = 0;
    if( (radix = defradix) == 0 )
	radix = 0x10;
    if( (negflag = *sp == '-') || *sp == '+' )
	sp++;			/*skip over optional sign*/
    if( *sp == '0' )
    {				/*leading 0 ==> octal, decimal or hex*/
	if( defradix == 0 )
	    radix = 010;
	sp++;
	digit = *sp++;
	if( digit == 'x' || digit == 'X' )
	    radix = 0x10;
	else
	if( digit == 't' || digit == 'T' )
	    radix = 10;
	else
	if( digit == 'o' || digit == 'O' )
	    radix = 010;
	else
	    sp--;
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
	if( digit >= radix )
	    break;		/*illegal digit*/
	number = number*radix + digit;
	sp++;
    }

    /*now sp points past the last legal digit*/
    *ip = negflag?-number:number;
    return sp;
}

PROMSTATIC int def_in_radix;

int
isnum(src,ip)
    char *src;
    long *ip;
{
    return *skipnum(src,def_in_radix,ip) == 000;
}
