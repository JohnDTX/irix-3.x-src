/*
 * $Source: /d2/3.7/src/stand/lib/dev/RCS/prf.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:14:44 $
 */

#include	"types.h"
#include	"termio.h"
#include	"cpureg.h"
#include	"common.h"

char *sprintf_cp;		/* a pointer used by sprintf	*/

/*
** prf.c
**   This module contains the high level tty routines:
**   printf, fprintf, putchar, getchar, gets
**
**   The all map down into calls to getc/putc routines for the current
**   console.
**   The version of printf implemented here is a scaled down version.
*/

/*
** printf
**   print on the console
*/
/*VARARGS1*/
printf( fmt, x1 )
u_int	x1;
{
	doprnt( _commdat->c_putc, fmt, &x1 );
}

/*
** fprintf
**   like printf, except the putchar routine is specified instead
**   of being assumed.
*/
/*VARARGS1*/
fprintf( putc, fmt, x1 )
int		(*putc)();
char		*fmt;
u_int	x1;
{
	doprnt( putc, fmt, &x1 );
}

sprintf( buf, fmt, x1 )
char *buf;
u_int	x1;
{
	int shoveit();

	sprintf_cp = buf;
	doprnt( shoveit, fmt, &x1 );
	/* Make sure result is null terminated */
	shoveit('\0');
}

/* A simple hack	*/
shoveit( c )
char c;
{
	*sprintf_cp++ = c;
}

/*
** doprnt
**   do the actual printf processing, using the given character output
**   routine - "putc".
**   This scaled down version only support the following format items:
**	%s %u %d (==%u) %o %x %D
**   If an unknown format item is used, the resulting output will just
**   be the format item followed by a question mark.
*/
doprnt( putc, fmt, adx )
int		(*putc)();
register char	*fmt;
register u_int	*adx;
{
	register u_char	 c;
	register char		*s;
	register short		width,
				zero_fill;

	for (;;)
	{
		while ( ( c = *fmt++ ) != '%' )
		{
			if ( c == '\0' )
				return;
			(*putc)( c );
		}
		c = *fmt++;
		zero_fill = width = 0;
		if ( c == '0' )
			zero_fill = 1;
		while ( ( c >= '0' ) && ( c <= '9' ) )
		{
			width = width * 10 + ( c - '0' );
			c = *fmt++;
		}
		if ( width > 11 )
			width = 11;
		switch ( c )
		{
		  case 0:
			(*putc)( '%' );
			return;

		  case '%':
			(*putc)( '%' );
			continue;			/* skip adx++ */

		  case 'd':
		  case 'u':
			printn( (u_long)*adx, 10, width, zero_fill, putc );
			break;

		  case 'U':
		  case 'D':
			printn( *(u_long *)adx, 10, width, zero_fill, putc );
			break;

		  case 'x':
			printn( (u_long)*adx, 16, width, zero_fill, putc );
			break;

		  case 'X':
			printn( *(u_long *)adx, 16, width, zero_fill, putc );
			break;

		  case 'o':
			printn( (u_long)*adx, 8, width, zero_fill, putc );
			break;

		  case 'O':
			printn( *(u_long *)adx, 8, width, zero_fill, putc );
			break;

		  case 's':
			if ( (s = (char *)*adx) == (char *)0 )
				s = "(NULL)";
			while ( c = *s++ )
			{
				if ( c == '\n' )
					(*putc)( '\r' );
				(*putc)( c );
			}
			break;

		  case 'c':
			(*putc)( (u_char)*adx );
			break;

		  default:
			(*putc)( '%' );
			(*putc)( c );
			(*putc)( '?' );
			break;
		}
		adx++;
	}
}

/*
** printn
**	print out a number
*/
printn( num, base, width, zero_fill, putc )
register u_long	 num;
register short	base,
		width,
		zero_fill;
int		(*putc)();
{
	register short	digit;
	char		digit_buf[ 20 ];

	digit = 0;
	if ( num )
	{
		while ( num )
		{
			digit_buf[ digit++ ] = "0123456789abcdef"[ num % base ];
			num /= base;
		}
	}
	else
		digit_buf[ digit++ ] = '0';

	if ( width )
	{
		while ( digit < width )
		{
			if ( zero_fill )
				digit_buf[ digit++ ] = '0';
			else
				digit_buf[ digit++ ] = ' ';
		}
	}
	while ( digit )
		(*putc)( digit_buf[ --digit ] );
}

/*
** putchar
**	output a character to the current console
*/
putchar( ch )
char	ch;
{
	(*_commdat->c_putc)( ch & 0xFF );

	if ( ch == '\n' )
		(*_commdat->c_putc)( '\r' );
}

/*
** getchar
**	get a character from the current console
**	We ISTRIP and ICRNL on the character.
*/
getchar()
{
	int	c;

	c = (*_commdat->c_getc)(0);

	if ( c != -1 ) {
		c &= 0xFF;
		if ( c == '\r' )
			c = '\n';
		if ( _commdat->c_lflag & ECHO )
			putchar( c );
	}
	return c;
}

/* XXX get rid of these two */
negetchar()
{
	int lflag, c;

	lflag = _commdat->c_lflag;
	_commdat->c_lflag &= ~ECHO;
	c = getchar();
	_commdat->c_lflag = lflag;
	return(c);
	

}

nwgetchar()
{
	int	c;

	if ( (c = (*_commdat->c_getc)(1)) == -1 )
		return(c);

#ifdef notdef
	c &= 0x7F;
#endif
	if ( c == '\r' )
		c = '\n';
	if ( _commdat->c_lflag & ECHO )
		putchar( c );
	return(c);
}

#define	CNTRLU	025	/* control-u: kill a line	*/
#define	CNTRLX	030	/* control-x: kill a line	*/
#define	CNTRLC	03	/* control-c: interrupt (kill line) */
#define BRK	0176	/* break key 			*/
#define	BS	010	/* backspace			*/
#define DEL    0177	/* delete			*/
#define	NL	012	/* newline			*/

/*
** gets
**   read a line of input from the current console, while doing
**   a limited amount of cannonization.
*/
gets( buf )
u_char	*buf;
{
	register u_char	*cp;
	register u_char	ch;

	cp = buf;
	for (;;)
	{
		ch = getchar();
		switch ( ch )
		{
		  case BS:
		  case DEL:
			if ( cp == buf )
				break;
			if ( ch == DEL )
				putchar( '\b' );
			printf(" \b");
			cp--;
			break;

		  case CNTRLU:		/* damn we're nice	*/
		  case CNTRLX:
			while ( cp != buf )
			{
				printf( "\b \b" );
				cp--;
			}
			break;


		  case NL:
			*cp = 0;
			return;

		  default:
			if ( isprint( ch ) )
			{
				*cp++ = ch;
			}
			break;
		}
	}
}
