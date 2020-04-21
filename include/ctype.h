/*
 * $Source: /d2/3.7/src/include/RCS/ctype.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 16:11:15 $
 */

#define	_U	01
#define	_L	02
#define	_N	04
#define	_S	010
#define	_P	020
#define	_C	040
#define	_B	0100
#define	_X	0200

extern char	_ctype[];

#ifndef lint
#define	isalpha(c)	((_ctype+1)[c]&(_U|_L))
#define	isupper(c)	((_ctype+1)[c]&_U)
#define	islower(c)	((_ctype+1)[c]&_L)
#define	isdigit(c)	((_ctype+1)[c]&_N)
#define	isxdigit(c)	((_ctype+1)[c]&_X)
#define	isalnum(c)	((_ctype+1)[c]&(_U|_L|_N))
#define	isspace(c)	((_ctype+1)[c]&_S)
#define	ispunct(c)	((_ctype+1)[c]&_P)
#define	isprint(c)	((_ctype+1)[c]&(_P|_U|_L|_N|_B))
#define	isgraph(c)	((_ctype+1)[c]&(_P|_U|_L|_N))
#define	iscntrl(c)	((_ctype+1)[c]&_C)
#define	isascii(c)	(!((c) & ~0177))
#define	_toupper(c)	((c)-'a'+'A')
#define	_tolower(c)	((c)-'A'+'a')
#define	toascii(c)	((c)&0177)
#endif
