/*
* $Source: /d2/3.7/src/stand/mon/RCS/parse.h,v $
* $Revision: 1.1 $
* $Date: 89/03/27 17:15:43 $
*/

/* token not found identifier */
#define NOTFOUND 0x8000

/*
** a command identifer consists of two 8-bit fields:
**	
**	XXXXXXLLCCCCCCCC
**
** where:
**	CCCCCCCC	is the command
**	LLL		is the length
**	
*/

#define CMDMSK		0xff		/* mask to isolate the command	*/
#define	LNMSK		0x7		/* mask to isolate the length	*/
#define	LNSHIFT		0x8
#define CMDSHIFT	0x8		/* number of bits		*/

/*
** command codes. The codes match with the indexes for help messages.
*/
#define	ILL_CMD		0
#define	BOOT_CMD	1
#define	TCP_CMD		2
#define	XNS_CMD		3
#define	LS_CMD		4
#define	LOAD_CMD	5
#define	GO_CMD		6
#define	HELP_CMD	7
#define	SET_CMD		8
#define	RET_CMD		9
#define	FILL_MEM	10
#define	DISP_MEM	11
#define	EDIT_MEM	12
#define DISP_PREG 	13
#define	EDIT_PREG	14

/*
** these codes are special cases for bringing up the IP2 processor
** are are recognized by all ones in the high nibble
*/
#define	SPEC_CMD	0xf0
#define	LPR_CMD		(SPEC_CMD+0xE)
#define	LPW_CMD		(SPEC_CMD+0xF)

/*
** lengths
*/
#define SZ_BYTE 0x100
#define SZ_WORD 0x200
#define SZ_LONG 0x400

struct strinfo 
{
	char	*s_name;
	long	s_val;
};

/*
** defines for the editing functions
*/
extern char	Ed_prev;	/* previous location	*/
extern char	Ed_hlp1;	/* asking for help	*/
extern char	Ed_hlp2;	/* asking for help	*/
extern char	Ed_quit;	/* done editing		*/

short	clookup();	/* lookup routine for commands	*/
long	slookup();	/* lookup routine for strings	*/
char	*nlookup();	/* lookup for boot types	*/
