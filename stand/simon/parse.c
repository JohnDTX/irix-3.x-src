/*
* $Source: /d2/3.7/src/stand/simon/RCS/parse.c,v $
* $Revision: 1.1 $
* $Date: 89/03/27 17:20:56 $
*/

#include	"sys/types.h"
#include	"cpureg.h"
#include	"common.h"
#include	"parse.h"
#include	"strs.h"
#include	"ctype.h"

/*
** This structure holds the commands and there associated code.
** (See parse.h for an explaination of the code).
*/
struct cmdinfo
{
	char	*cmd_name;
	short	cmd_id;
} cmdinfo[] = {
	"h",	HELP_CMD,		/* help commands		     */
	"?",	HELP_CMD,
	"b",	BOOT_CMD,		/* for brain damaged PM2 people	     */
	"boot",	BOOT_CMD,		/*     ditto			     */
	"d",	BOOT_CMD,		/*     ditto			     */
	"l",	LOAD_CMD,		/* load but don't execute	     */
	"n",	XNS_CMD,
	"dpr",	DISP_PREG,		/* display processor registers	     */
	"epr",	EDIT_PREG,		/* edit processor registers	     */
	"fm",	FILL_MEM | SZ_WORD,	/* fill memory: default - words	     */
	"fmb",	FILL_MEM | SZ_BYTE,
	"fmw",	FILL_MEM | SZ_WORD,
	"fml",	FILL_MEM | SZ_LONG,
	"dm",	DISP_MEM | SZ_WORD,	/* display memory: default - words   */
	"dmb",	DISP_MEM | SZ_BYTE,
	"dmw",	DISP_MEM | SZ_WORD,
	"dml",	DISP_MEM | SZ_LONG,
	"em",	EDIT_MEM | SZ_WORD,	/* edit memory: default - words	     */
	"emb",	EDIT_MEM | SZ_BYTE,
	"emw",	EDIT_MEM | SZ_WORD,
	"eml",	EDIT_MEM | SZ_LONG,
	"g",	GO_CMD,			/* go: initialize sp, jmp to addr    */
					/*  assumed pt already setup	     */
	"set",	SET_CMD,
	"ls",	LS_CMD,
	"exit",	RET_CMD,
#ifdef LATER
	"mmb",	MMB_CMD,
#endif
#ifdef KURT
	"rb",	LPR_CMD | SZ_BYTE,
	"rw",	LPR_CMD | SZ_WORD,
	"rl",	LPR_CMD | SZ_LONG,
	"wb",	LPW_CMD | SZ_BYTE,
	"ww",	LPW_CMD | SZ_WORD,
	"wl",	LPW_CMD | SZ_LONG,
#endif
	"",	0
};

struct strinfo	strinfo[] = {
	"pbase",	0x30000000,	
	"mbut",		0x30800000,
	"mquad",	0x31000000,
	"swtch",	0x31800000,
	"d0base",	0x32000000,
	"d1base",	0x32800000,
	"pgfltc",	0x33800000,
	"clkc",		0x34000000,
	"clkd",		0x35000000,
	"sreg",		0x38000000,
	"pcr",		0x39000000,
	"mbpr",		0x3a000000,
	"ptbase",	0x3b000000,
	"tdbr",		0x3c000000,
	"tdlr",		0x3d000000,
	"stkbr",	0x3e000000,
	"stklr",	0x3f000000,
	""
};

/*
** this structure contains the various bootable media devices we understand
*/
struct strinfo mediastrs[] = {
	"hd",		BT_HD,
	"ct",		BT_TAPE,
	"fd",		BT_FD,
	"xns",		BT_XNS,
	"rom",		BT_ROM,
	"mon",		BT_MONITOR,
	"ip",		BT_IP,
	"st",		BT_ST,
	"sq",		BT_ST,
	"sf",		BT_SF,
	"sd",		BT_SD,
	"si",		BT_SD,
	"mt",		BT_MT,
	"mq",		BT_MT,
	"mf",		BT_MF,
	"md",		BT_MD,
	""
};


/*
** clookup
**   lookup the given symbol as a command in the table and return the token id
*/
short
clookup( cptr )
register char *cptr;
{
	register i = 0;

	for ( i = 0; *cmdinfo[ i ].cmd_name; i++ )
	    if ( !strcmp( cptr, cmdinfo[ i ].cmd_name ) )
		return ( cmdinfo[ i ].cmd_id );

	return ( ILL_CMD );
}

/*
** slookup
**   lookup the given string in the table and return the value
*/
long
slookup( cptr, tble )
register char		*cptr;
register struct strinfo	*tble;
{
	register i = 0;

	for ( i = 0; *tble[ i ].s_name; i++ )
	    if ( !strcmp( cptr, tble[ i ].s_name ) )
		return ( tble[ i ].s_val );

	return ( -1L );
}

/*
** nlookup
**   lookup the given number in the table and return the string	
*/
char *
nlookup( num, tble )
register int		num;
register struct strinfo	*tble;
{
	register i = 0;

	for ( i = 0; *tble[ i ].s_name; i++ )
	    if ( tble[ i ].s_val == num )
		return ( tble[ i ].s_name );

	return ( 0 );
}

# define SCAN 0
# define FILL 1

getargv( prompt )
char	*prompt;
{
	register unsigned	count = 0;
	register char		*ptr;
	register char		*quote;
	int			pass = SCAN;
	int			shift = 0;
	extern char		*strncpy();
	extern char		*strchr();
	int			slen;

	printf( prompt );

	for ( ptr = argbuf; ptr < &argbuf[ sizeof (argbuf) ]; ptr++ )
		*ptr = Null;
	ptr = argbuf;
	gets( argbuf );
	slen = strlen(argbuf);

	for (;;) {
		while ((ptr-argbuf) < slen ) {
			while (*ptr) {
				if (*ptr != ' ' && *ptr != '\t')
					break;
				++ptr;
			}
			if (*ptr == Null)
				break;
			if (pass == FILL)
				argv [count] = ptr;
			++count;
			while (*ptr) {
				if (*ptr == ' ' || *ptr == '\t')
					break;
				if (*ptr == '"' || *ptr == '\'') {
					quote = strchr (&ptr [1], *ptr);
					if (pass == FILL) {
						strncpy (ptr, &ptr [1], 
							quote - ptr - 1);
						shift += 2;
					}
					ptr = quote;
				} else
					ptr [-shift] = *ptr;
				++ptr;
			}
			if (pass == FILL) {
				ptr [-shift] = Null;
				shift = 0;
				++ptr;
			}
		}
		if (pass == FILL) {
			argc = count;
			return;
		}
		ptr = argbuf;
		count = 0;
		pass = FILL;
	}
}

/*
** yes
**   prints the supplied string, and then tests the users reply for yes
**   or no.
*/
yes( qs )
char	*qs;
{
	register char	*cp;
	char	buf[ 20 ];

	cp = buf;
	printf( qs );
	printf( "?: " );
	gets(cp);
	while ( ispunct( *cp ) )
		cp++;
	if ( *cp == 'y' || *cp == Null || *cp == 'Y' )
		return(1);
	return(0);
}
