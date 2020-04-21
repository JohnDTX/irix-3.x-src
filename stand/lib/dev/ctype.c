/*
 * $Source: /d2/3.7/src/stand/lib/dev/RCS/ctype.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:14:23 $
 */

#include "iriskeybd.h"

extern char kbdtype;

#define	_U	01
#define	_L	02
#define	_N	04
#define	_S	010
#define	_P	020
#define	_C	040
#define	_B	0100
#define	_X	0200

extern char	_ctype[];

isalpha( c )
{
	if ( kbdtype == KBD_IRIS )
	{
		if ( c > 0x7f )
			return ( 0 );
	}
	return ( ((_ctype+1)[c & 0xff ]&(_U|_L)) );
}

isupper( c )
{
	if ( kbdtype == KBD_IRIS )
	{
		if ( c > 0x7f )
			return ( 0 );
	}

	return ( ((_ctype+1)[c & 0xff ]&_U) );
}

islower( c )
{
	return ( ((_ctype+1)[c & 0xff ]&_L) );
}

isdigit( c )
{
	return ( ((_ctype+1)[c & 0xff ]&_N) );
}

isxdigit( c )
{
	return ( ((_ctype+1)[c & 0xff ]&_X) );
}

isalnum( c )
{
	return ( ((_ctype+1)[c & 0xff ]&(_U|_L|_N)) );
}

isspace( c )
{
	return ( ((_ctype+1)[c & 0xff ]&_S) );
}

ispunct( c )
{
	return ( ((_ctype+1)[c & 0xff ]&_P) );
}

isprint( c )
{
	if ( kbdtype == KBD_IRIS )
	{
		if ( c > 0x7f )
			return ( 0 );
	}

	return( ( _ctype + 1 )[ c & 0xff  ] & ( _P | _U | _L | _N | _B ) );

}

isgraph(c)
{
	return ( ((_ctype+1)[c & 0xff ]&(_P|_U|_L|_N)) );
}

iscntrl( c )
{
	return ( ((_ctype+1)[c & 0xff ]&_C) );
}

isascii( c )
{
	if ( kbdtype == KBD_IRIS )
	{
		if ( c > 0x7f )
			return ( 0 );
	}
	return ( ((unsigned char)(c)<=0xff) );
}

_toupper( c )
unsigned int	c;
{
	if ( kbdtype == KBD_IRIS || c <= 0x7f )
	{
		return ( ((c)-'a'+'A') );
	}
	return ( ((c) - (unsigned int)0xe0 + (unsigned int)0xc0 ) );
}

_tolower( c )
unsigned int	c;
{
	if ( kbdtype == KBD_IRIS || c <= 0x7f )
	{
		return ( ((c)-'A'+'a') );
	}
	return ( ((c) - (unsigned int)0xc0 + (unsigned int)0xe0 ) );
}

toascii( c )
{
	if ( kbdtype == KBD_IRIS )
	{
		return ( (c) & 0x7f );
	}

	return ( c & 0xff );
}

char	_ctype[] = { 0,

/*	 0	 1	 2	 3	 4	 5	 6	 7  */

/* 0*/	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C,
/* 10*/	_C,	_S|_C,	_S|_C,	_S|_C,	_S|_C,	_S|_C,	_C,	_C,
/* 20*/	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C,
/* 30*/	_C,	_C,	_C,	_C,	_C,	_C,	_C,	_C,
/* 40*/	_S|_B,	_P,	_P,	_P,	_P,	_P,	_P,	_P,
/* 50*/	_P,	_P,	_P,	_P,	_P,	_P,	_P,	_P,
/* 60*/	_N|_X,	_N|_X,	_N|_X,	_N|_X,	_N|_X,	_N|_X,	_N|_X,	_N|_X,
/* 70*/	_N|_X,	_N|_X,	_P,	_P,	_P,	_P,	_P,	_P,
/*100*/	_P,	_U|_X,	_U|_X,	_U|_X,	_U|_X,	_U|_X,	_U|_X,	_U,
/*110*/	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U,
/*120*/	_U,	_U,	_U,	_U,	_U,	_U,	_U,	_U,
/*130*/	_U,	_U,	_U,	_P,	_P,	_P,	_P,	_P,
/*140*/	_P,	_L|_X,	_L|_X,	_L|_X,	_L|_X,	_L|_X,	_L|_X,	_L,
/*150*/	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L,
/*160*/	_L,	_L,	_L,	_L,	_L,	_L,	_L,	_L,
/*170*/	_L,	_L,	_L,	_P,	_P,	_P,	_P,	_C,
/*200*/	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
/*210*/	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
/*220*/	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
/*230*/	 0,	 0,	 0,	 0,	 0,	 0,	 0,	 0,
/*240*/	 0,	 _P|_L,	 _P,	 _P,	 _P,	 _P,	 _P,	 _P,
/*250*/	 _P,	 _P,	 _P,	 _P,	 _P,	 _P,	 _P,	 _P,
/*260*/	 _P,	 _P,	 _P,	 _P,	 _P,	 _P,	 _P,	 _P,
/*270*/	 _P,	 _P,	 _P,	 _P,	 _P,	 _P,	 _P,	 _P,
/*300*/	 _U,	 _U,	 _U,	 _U,	 _U,	 _U,	 _U,	 _U,
/*310*/	 _U,	 _U,	 _U,	 _U,	 _U,	 _U,	 _U,	 _U,
/*320*/	 _U,	 _U,	 _U,	 _U,	 _U,	 _U,	 _U,	 _P,
/*330*/	 _U,	 _U,	 _U,	 _U,	 _U,	 _U,	 _U,	 _P,
/*340*/	 _L,	 _L,	 _L,	 _L,	 _L,	 _L,	 _L,	 _P,
/*350*/	 _L,	 _L,	 _L,	 _L,	 _L,	 _L,	 _L,	 _L,
/*360*/	 _L,	 _L,	 _L,	 _L,	 _L,	 _L,	 _L,	 _L,
/*370*/	 _L,	 _L,	 _L,	 _L,	 _L,	 _L,	 _L,	 _P
};
