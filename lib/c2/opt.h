#include <stdio.h>
/*
 * Header for object code improver
 */

#define MAXSIZE	40	/* maximum size of an operand */

#define	JBR	1
#define	CBR	2
#define	JMP	3
#define	LABEL	4
#define	DLABEL	5
#define	EROU	6
#define	MOV	7
#define	CLR	8
#define	NOT	9
#define	ADDQ	10
#define	SUBQ	11
#define	NEG	12
#define	TST	13
#define	ASR	14
#define	ASL	15
#define LSR	16
#define LSL	17
#define	EXT	18
#define	CMP	20
#define	ADD	21
#define	SUB	22
#define	AND	23
#define	OR	24
#define	EOR	25
#define LEA	26
#define PEA	27
#define MOVEM	28
#define LINK	29
#define UNLK	30
#define	JSR	31
#define	TEXT	32
#define	DATA	33
#define	BSS	34
#define	EVEN	35
#define	END	36
#define JSW	37
#define MOVEQ	38
#define MULS	39
#define MULU	40
#define DIVS	41
#define DIVU	42
#define XFER	43
#define DB	44
#define BSR	45
#define BTST	46
#define	SWAP	47			/* kipp */
#define	ROR	48			/* kipp */
#define	CMPM	49			/* kipp */
#define	EOSUB	50

#define	JEQ	0
#define	JNE	1
#define	JLE	2
#define	JGE	3
#define	JLT	4
#define	JGT	5
#define	JLO	6
#define	JHI	7
#define	JLOS	8
#define	JHIS	9
#define	JRA	10

#define	BYTE	100
#define WORD	101
#define LONG	102

struct node {
	char	op;
	char	subop;
	struct	node	*forw;
	struct	node	*back;
	struct	node	*ref;
	int	labno;
	char	*code;
	int	refc;
	int	lineno;		/* start of "C" line marker */
	char	misc;		/* end of "C" line marker */
};

struct optab {
	char	*opstring;
	int	opcode;
} optab[];

char	line[1024];
struct	node	first;
struct	node	*freenodes;
int	Bflg;		/* make jsrs into bsrs */
int	Kflg;		/* don't do kernel optimizations */
int	Sflg;		/* do stack optimization */
int	Pflg;		/* remove stack probe instruction */
int	skflg;		/* do special kernel optimizations */
char	*curlp;
int	nbrbr;
int	nsaddr;
int	redunm;
int	iaftbr;
int	njp1;
int	nrlab;
int	nxjump;
int	ncmot;
int	nrevbr;
int	loopiv;
int	nredunj;
int	nskip;
int	ncomj;
int	nrtst;

int	nchange;
int	isn;
int	debug;
char	revbr[];
char	regs[16][MAXSIZE];
char	conloc[MAXSIZE];
char	conval[MAXSIZE];
char	ccloc[MAXSIZE];

#define	RT1	14
#define	RT2	15
#define	NREG	14
#define	LABHS	127
#define	OPHS	129
#define	OPCDHS	183

struct optab *ophash[OPHS];
struct optab *opcdhash[OPCDHS];

struct {
	char lbyte;
};

char *copy();
char *calloc();
char *strcpy();
char *strcat();
char *sprintf();
char *findcon();
struct node *nonlab();
struct node *getnode();
struct node *insertl();
struct node *codemove();
long numcvt();

#define jmpcond(x) (x->subop==JEQ||x->subop==JNE||x->subop==JGE||x->subop==JLT)
