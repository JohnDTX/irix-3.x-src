/*
* $Source: /d2/3.7/src/stand/mon/RCS/subr.c,v $
* $Revision: 1.1 $
* $Date: 89/03/27 17:15:48 $
*/

#include	"sys/types.h"
#include	"setjmp.h"
#include	"cpureg.h"
#include	"trap.h"
#include	"common.h"
#include	"ctype.h"
#include	"parse.h"
#include	"strs.h"

/*
** mdump
**   dump memory from 'addr' for 'cnt' items.  Each item of size 'sz'.
*/
mdump( addr, sz, cnt )
u_long	addr;
int		sz,
		cnt;
{
	u_char	*cptr1,
			*cptr2;
	union
	{
	    u_long	un_long;
	    u_short	un_short[ 2 ];
	    u_char	un_char[ 4 ];
	} data;
	int	count  = cnt;
	int	nlines = 0;
	int	ncol;
	int	i;
	u_char	buf[ 17 ];

	buf[ 0 ] = Null;
	cptr1 = buf;

	while ( 1 )
	{
		printf( Addrfmt, addr );
		for ( ncol = 16 / sz ;
		      ncol && count;
		      ncol--, count-- )
		{
			if ( ! ( i = mread( addr, &data.un_long, sz ) ) )
			{
				if ( sz == sizeof (char) )
				{
					printf( Charfmt, data.un_char[ 3 ] );
					cptr2 = &data.un_char[ 3 ];
				}
				else
				if ( sz == sizeof (short) )
				{
					printf( Shrtfmt, data.un_short[ 1 ] );
					cptr2 = &data.un_char[ 2 ];
				}
				else
				{
					printf( Longfmt, data.un_long );
					cptr2 = &data.un_char[ 0 ];
				}
				for ( i = 0; i < sz; i++ )
					if ( isprint( cptr2[ i ] ) )
						*cptr1++ = cptr2[ i ];
					else
						*cptr1++ = '.';
			}
			else
			{
				if ( i == T_BUSERR )
					printf( " %sBERR",
					( sz == sizeof (long) ) ? "BERR" : "" );
				else
				if ( i == T_PARERR )
					printf( " %sPERR",
					( sz == sizeof (long) ) ? "PERR" : "" );
				else
					trpprt( i );
					

				for ( i = 0; i < sz; i++ )
					*cptr1++ = '.';
			}
			*cptr1 = Null;
			addr += sz;
		}

		/* Have we fufulled the request?? */
		if ( ! count )
			break;

		printf( Fmtstr3, buf );	/* print the ascii representation */
		cptr1 = buf;		/* reset the pointer to the buffer*/
	
		nlines++;		/* incr line count */

		/*
		** if we have printed a screenful, let's ask if we
		** should continue
		*/
		if ( nlines > 20 )
			if ( !yes( Contfmt ) )
				return;
			else
				nlines = 0;
	}
	/*
	** at this point we have finished printing the given amount,
	** we just need to finish up an print the ascii representation
	*/
	printf( Fmtstr3, buf );
}

/*
** medit
**   edit memory starting at address 'addr', referencing memory as
**   size 'len'.  'single' determines if we are just changing 1 location
**   or until the user says stop
*/
medit( addr, len, single )
register long	addr;
register int	len;
register int	single;
{
	long	val1;
	int	trpval;

do
{
	printf( "%08x ->", addr );
	/*
	** try to read the memory
	*/
	if ( ! ( trpval = mread( addr, &val1, len ) ) )
	{
		if ( len == sizeof (char) )
			printf( Charfmt, val1 );
		else
		if ( len == sizeof (short) )
			printf( Shrtfmt, val1 );
		else
			printf( Longfmt, val1 );

		getargv( " ? " );

		if ( argc == 0 )
			addr += len;
		else
		if ( argv[ 0 ][ 0 ] == Ed_prev && argv[ 0 ][ 1 ] == Null )
				addr -= len;
		else
		if ( argv[ 0 ][ 0 ] == Ed_quit && argv[ 0 ][ 1 ] == Null )
			return 0;
		else
		if ( argv[ 0 ][ 0 ] == Ed_cur && argv[ 0 ][ 1 ] == Null )
			continue;
		else
		if ( ( argv[ 0 ][ 0 ] == Ed_hlp1 || 
		     argv[ 0 ][ 0 ] == Ed_hlp2 ) && argv[ 0 ][ 1 ] == Null )
		{
printf( "Legal Memory Editing commands are:\n\n" );
printf (
"-		decrement current address (current location not modified).\n" );
printf (
".		re-edit the current location.\n" );
printf (
"<cr>		increment current address (current location not modified).\n" );
printf (
"q		exit from editing memory.\n" );
printf (
"VALUE		this hex value will be stored at the displayed address.\n\n" );
		}
		else
		{
			if ( ( val1 = aton( argv[ 0 ], 16 ) ) == -1 )
			{
				printf( "%s%s", argv[ 0 ], Einval );
				continue;
			}
			if ( ( trpval = mwrite( addr, val1, len ) ) )
				trpprt( trpval );
			addr += len;
		}
	}
	else
	{
		trpprt ( trpval );
		return 1;
	}

} while ( !single );

	return 0;

}

/*
** mread
**   read into 'data' and item of size 'len' from 'addr'
*/
mread( addr, data, len )
u_long	addr,
	*data;
int	len;
{
	int	i;
	jmp_buf	jb;		/* my jump buffer	*/
	int	*saved_jb;	
	union
	{
		u_char	un_char[ 4 ];
		u_short	un_short[ 2 ];
		u_long	un_long;
	} maplong;

	saved_jb = _commdat->c_nofault;
	_commdat->c_nofault = jb;

	/*
	** set up to handle possible bus error
	*/
	if ( i = setjmp( jb ) )
	{
errout:
		_commdat->c_nofault = saved_jb;	/* restore default handling	*/
		return i;
	}

	/*
	** must access clock data in a different manner
	** byte by bite!
	*/
	if ( ( addr & (u_long)CLK_DATA ) == (u_long)CLK_DATA )
	{
		if ( todread( addr, maplong.un_char, len ) < 0 )
		{
			i = T_BUSERR;
			goto errout;
		}
		if ( len == sizeof (char) )
			*data = maplong.un_char[ 0 ];
		else
		if ( len == sizeof (short) )
			*data = maplong.un_short[ 0 ];
		else
			*data = maplong.un_long;
		goto out;
	}
	/* get the data */
	if ( len == sizeof (char) )
		*data = *(u_char *)addr;
	else
	if ( len == sizeof (short) )
		*data = *(u_short *)addr;
	else
		*data = *(u_long *)addr;

out:
	_commdat->c_nofault = saved_jb;	/* restore default handling	*/
	return 0;
}

/*
** mwrite
**   write the value 'data', at memory location 'addr' of size 'len'
*/
mwrite( addr, data, len )
register long	addr;
register u_long	data;
register int	len;
{
	int	i;
	jmp_buf	jb;
	int	*saved_jb;
	union
	{
		u_char	un_char[ 4 ];
		u_short un_short[ 2 ];
		u_long  un_long;
	} maplong;

	saved_jb = _commdat->c_nofault;
	_commdat->c_nofault = jb;

	if ( i = setjmp( jb ) )
	{
errout:
		_commdat->c_nofault = saved_jb;
		return i;
	}

	/*
	** must access clock data in a different manner
	** byte by bite!
	*/
	if ( ( addr & (u_long)CLK_DATA ) == (u_long)CLK_DATA )
	{
		if ( len == sizeof (char) )
			maplong.un_char[ 0 ] = (u_char)data;
		else
		if ( len == sizeof (short) )
			maplong.un_short[ 0 ] = (u_short)data;
		else
			maplong.un_long = data;

		if ( todwrite( addr, maplong.un_char, len ) < 0 )
		{
			i = T_BUSERR;
			goto errout;
		}
		goto out;
	}
	if ( len == sizeof (char) )
		*(u_char *)addr = (u_char)data;
	else
	if ( len == sizeof (short) )
		*(u_short *)addr = (u_short)data;
	else
	    *(u_long *)addr = data;

out:
	_commdat->c_nofault = saved_jb;
	return 0;
}

/*
** bcopy
**   copy 'from' to 'to' for 'sz' bytes.
** RECODE in assembly for speed (whistle while you work!)
*/
bcopy( from, to, sz )
register char	*from,
		*to;
register int	sz;
{
	while ( sz-- )
		*to++ = *from++;
}

/*
** setpte
**   program the page map entry 'indx' to the pte pointed to by 'ppte'
** RECODE in assembler
*/
setpte( indx, ppte )
register int		indx;
register struct pte	*ppte;
{
	register u_long	*ptr;

	ptr = (u_long *)PTMAP_BASE + indx;

	*ptr = *(u_long *)ppte;	/* pte is also a long	*/
}

#ifdef KURT
/*
** loops
**   performs the necessary actions for looping on read or write.
**   The code is organized in the following fashion because we
**   want the loop to poke at the processor as fast as possible.
**   During the loop on read command, we let the count expire then
**   report the information read during the last iteration.
*/
loops( type, len, addr, val, count )
short		type,		/* command			*/
		len;		/* length (byte, word, long)	*/
register char	*addr;		/* address to read/write	*/
register u_long	val;		/* value to store (if write)	*/
register long	count;		/* repetition count		*/
{
	long	savcnt = count;

	if ( type == LPW_CMD )
	{
		while ( 1 )
		{
			count = savcnt;
			switch ( len )
			{
			  case sizeof (char):
				while ( count-- )
					*(u_char *)addr = (u_char)val;
					break;

			    case sizeof (short):
				while ( count-- )
					*(u_short *)addr = (u_short)val;
					break;

			    case sizeof (long):
				while ( count-- )
					*(u_long *)addr = val;
				break;
			}
			if ( ! yes( Contfmt ) )
				break;
		}
	}
	else
	{
		while ( 1 )
		{
			count = savcnt;
			switch ( len )
			{
			    case sizeof (char):
				while ( count-- )
					val = *(u_char *)addr;
				printf( "%08x: %02x\n", 
					addr, val & 0xFF);
				break;

			    case sizeof (short):
				while ( count-- )
					val = *(u_short *)addr;
				printf( "%08x: %04x\n",
					addr, val & 0xFFFF);
				break;

			    case sizeof (long):
				while ( count-- )
					val = *(u_long *)addr;
				printf( "%08x: %08x\n",
					addr, val);
				break;
			}
			if ( ! yes( Contfmt ) )
				break;
		}
	}
}
#endif

/*
** this struct maps the value returned by slookup() to the correct
** routine to call for accessing the processor registers
*/
/*
** prdump
**   dump the processor registers
*/
prdump()
{
printf( "                     Processor Registers\n\n" );
printf( "      sr           vbr           cacr             caar\n" );
printf( "     %04x        %08x      %08x         %08x\n\n", getprsr() & 0xffff,
	 getprvbr(), getprcacr(), getprcaar() );

printf( "      sfc: %1x     dfc: %1x\n\n", getprsfc() & 0xf, getprdfc() & 0xf );
}

predit( regname )
char	*regname;
{
	extern	getprcacr(), getprcaar(), getprsr(), getprvbr(), getprsfc(),
		getprdfc();
	extern	setprcacr(), setprcaar(), setprsr(), setprvbr(), setprsfc(),
		setprdfc();
	long	val1;
	int	(*setpr)();

while ( 1 )
{
	if ( strcmp( regname, "cacr" ) == 0 )
	{
		printf( "cacr => %08x", getprcacr() );
		setpr = setprcacr;
	}
	else
	if ( strcmp( regname, "caar" ) == 0 )
	{
		printf( "caar => %08x", getprcaar() );
		setpr = setprcaar;
	}
	else
	if ( strcmp( regname, "sr" ) == 0 )
	{
		printf( "sr => %04x", getprsr() & 0xffff );
		setpr = setprsr;
	}
	else
	if ( strcmp( regname, "vbr" ) == 0 )
	{
		printf( "vbr => %08x", getprvbr() );
		setpr = setprvbr;
	}
	else
	if ( strcmp( regname, "sfc" ) == 0 )
	{
		printf( "sfc => %1x", getprsfc() & 0xf );
		setpr = setprsfc;
	}
	else
	if ( strcmp( regname, "dfc" ) == 0 )
	{
		printf( "dfc => %1x", getprdfc() & 0xf );
		setpr = setprsr;
	}
	else
	{
		printf( "%s: unknown processor register name\n", regname );
		return;
	}

	getargv( " ? " );

	if ( argc == 0 || (argv[ 0 ][ 0 ] == Ed_quit && argv[ 0 ][ 1 ] == Null ) )
		return;

	if ( ( argv[ 0 ][ 0 ] == Ed_hlp1 ||
	     argv[ 0 ][ 0 ] == Ed_hlp2 ) && argv[ 0 ][ 1 ] == Null )
	{
printf( "Legal Processor Register Editing commands are:\n\n" );
printf( "<cr>		do not modify the register.\n" );
printf( "q		do not modify the register.\n" );
printf( "VALUE		this hex value will be placed in the register.\n" );
		continue;
	}

	if ( ( val1 = aton( argv[ 0 ] ), 16 ) == -1 )
	{
		printf( "%s%s", argv[ 0 ], Einval );
		continue;
	}
	else
		break;
}
	
	(*setpr)( val1 );
}

/*
** mfill
**   fill memory with the info given
*/
mfill( sadr, len, sval, incr, cnt )
register long	sadr;
short		len;
long		sval,
		cnt,
		incr;
{
	int	i;
	jmp_buf	jb;
	int	*saved_jb;

	saved_jb = _commdat->c_nofault;
	_commdat->c_nofault = jb;

	if ( i = setjmp( jb ) )
	{
errout:
		_commdat->c_nofault = saved_jb;
		return i;
	}

	for ( ; cnt; sadr += len, sval += incr, cnt-- )
	{
	    if ( ( sadr & (long)CLK_DATA ) == (long)CLK_DATA )
	    {
		if ( todwrite( sadr, &sval, len ) < 0 )
		{
			i = T_BUSERR;
			goto errout;
		}
		continue;
	    }
	    if ( len == sizeof (char) )
		*(u_char *)sadr = (u_char)sval;
	    else
	    if ( len == sizeof (short) )
		*(u_short *)sadr = (u_short)sval;
	    else
		    *(u_long *)sadr = (u_long)sval;

	}
	_commdat->c_nofault = saved_jb;
	return 0;
}

/*
** aton
**  convert ascii to a number using supplied base
*/
long
aton( cptr, base )
register char	*cptr;
int		base;
{
	register long	v;
	register char	c;
	char		*savptr = cptr;
	extern struct strinfo	strinfo[];

	v = 0L;

	while ( c = *cptr++ )
	{
		if ( base == 16 && ( c >= 'A' && c <= 'F' ) )
			c = 10 + ( c - 'A' );
		else
		if ( base == 16 && ( c >= 'a' && c <= 'f' ) )
			c = 10 + ( c - 'a' );
		else
		if ( isdigit( c ) )
			c -= '0';
		else
		{
			if ( ( v = slookup( savptr, strinfo ) ) == -1L )
				return -1L;
			else
				return v;
		}

		if ( base == 16 )
			v = ( v << 4 ) + c;
		else
			v = ( v * 10L) + c;

	}
	return v;
}
