
typedef struct {
		char *tokenstr;
		int  tokenid; } token_t;

/* name is the default token for unrecognized strings of characters */
#define MINOR_VAL(t) (t & 0x3f)
#define MAJOR_VAL(t) (t & ~0x3f)

/* classes - type modifiers */
/* any type modifier that doesnt matter (short, unsigned) is just CLASS */
#define STATIC  3
#define LONG	2	 /* this is assigned to yylval */
#define UNSIGNED 1


/* types */
#define INT	 0
#define SHORT	1
#define CHAR	2
#define FLOAT	3
#define LFLOAT  4
			/* fix for scr1943 */
#define VOID	5
			/* end of fix for scr1943 */
/* arbitrary pointer - only restriction is NOT to CHAR */
#define PTR	0x10
#define ARRAY	0x20

#define STRING	(PTR|CHAR)
#define CHARARRAY (ARRAY|CHAR)

#ifndef NAME
#define NAME 0x100
#define CLASS	(NAME | 0x80)
#define TYPE	(NAME | 0x40)
#define SU	0x200
#define ERROR	0x400
#define INDIRECT  2
#define LP	 3
#define RP	4
#define LBRACKET	5
#define RBRACKET	6
#define LBRACE	7
#define RBRACE	(8)
#define CM (0xa)
#define SM (0xb)
#define NUMBER (0xc)
#endif
