/****************************************************************************
 
 NUNIX DEBUGGER - common definitions
 
*****************************************************************************
 debugger revisions for NUNIX by - J. Test 1/81
 modified for 4.2 a.out format by G.Boyd (SGI) 6/20/84
****************************************************************************/

/* Layout of a.out file (fsym):
 *
 * header of 8 longs magic number 405, 407, 410, 411
 * text size )
 * data size ) 
 * bss size )
 * symbol table size ) in bytes
 * text relocation size )
 * data relocation size )
 * entry point )
 *
 * header: 0
 * text: 32
 * data: 32+textsize
 * text relocation : data + datasize
 * data relocation : text relocation + text relocation size
 * symbols: data relocation + data relocation size
 * string table: symbols + symbol table size
 *
 */


#include <a.out.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <machine/reg.h>
#include <signal.h>
#include <sys/user.h>
#include <termio.h>
#include <stdio.h>

/*#define SYMTABSIZE 3500 /* symbol table size  - doubled 9/24/86 for fortran */
#define SYMTABSIZE 50000 /* symbol table size */

#define VARB 11
#define VARD 13
#define VARE 14
#define VARM 22
#define VARS 28
#define VART 29

#define COREMAGIC 0140000

#define RD 0
#define WT 1
#define NSP 0
#define ISP 1
#define DSP 2
#define STAR 4
#define STARCOM 0200

#define NOT_SYM 0
#define DSYM N_DATA
#define ISYM N_TEXT
#define ASYM N_ABS
#define NSYM NOT_SYM

#define ESYM (-1)
#define BKPTSET 1
#define BKPTEXEC 2
#define SYMSIZ 100
#define MAXSIG 20

#define BPT 0x4E42 /* Trap 2, breakpoint instruction */

#define FD 0200
#define SETTRC 0
#define RDUSER 2
#define RIUSER 1
#define WDUSER 5
#define WIUSER 4
#define RUREGS 10
#define WUREGS 11
#define CONTIN 7
#define EXIT 8
#define P_SINGLE 9
#define FROFF (&(0->fpsr))
#define FRLEN 25
#define FRMAX 6

#define ps 17 /* register offset definitions */
#define sp 15
#define pc 16
#define a6 14

#define MAXOFF 255
#define MAXPOS 80
#define MAXLIN 128
#define EOR '\n'
#define TB '\t'
#define QUOTE 0200
#define STRIP 0177
#define LOBYTE 0377
#define EVEN -2

union
{
	int I[2];
	long L;
} itolws;

#define leng(a) ((long)((unsigned)(a)))
#define shorten(a) ((int)(a))
#define itol(a,b) (itolws.I[0]=(a), itolws.I[1]=(b), itolws.L)
#define itol68(a,b) ((a << 16) | (b & 0xFFFF))

struct termio adbtty, subtty;


#define TRUE (-1)
#define FALSE 0
#define LOBYTE 0377
#define HIBYTE 0177400
#define STRIP 0177
#define HEXMSK 017

#define SPACE ' '
#define TB '\t'
#define NL '\n'

#define DBNAME "adb\n"
#define LPRMODE "%Q"
#define OFFMODE "+%o"
#define TXTRNDSIZ 8192L

typedef struct symb SYMTAB;
typedef struct symb *SYMPTR;
typedef struct exec BHDR;

struct symb
{
	char symf; /* symbol type */
	long vals; /* symbol value */
	int smtp; /* SYMTYPE */
	char *symc; /* pointer to symbol name */
};


/**GB**/
#define SYMCHK 0xf
#define SYMTYPE(st) (((st & 0x11)==N_EXT)||(st == N_TEXT))?(st & 0xe):\
 ((st == N_DATA)||(st == N_BSS))?N_DATA:NOT_SYM

/**GB**/
#define MAXCOM 64
#define MAXARG 32
#define LINSIZ 256

typedef char MSG[];

long int inkdot();
SYMPTR lookupsym();
SYMPTR symget();
unsigned get();
unsigned chkget();
char * exform();
long int round();
struct bkpt * scanbkpt();
int fault();

/* file address maps */
struct map {
	long int b1;
	long int e1;
	long int f1;
	long int b2;
	long int e2;
	long int f2;
	int ufd;
};

struct bkpt {
	int loc;
	int ins;
	int count;
	int initcnt;
	int flag;
	char comm[MAXCOM];
	struct bkpt *nxtbkpt;
};

typedef struct reglist REGLIST;

typedef REGLIST *REGPTR;
struct reglist {
	char * rname;
	int roffs;
	long int rval;
};

struct {
	int junk[2];
	int fpsr;
	float Sfr[6];
};

struct {
	int junk[2];
	int fpsr;
	long float Lfr[6];
};

