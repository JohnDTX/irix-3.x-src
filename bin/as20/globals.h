

/* 
	globals.h -  globals needed by as20

*/

/* the structure of a token.  */
typedef struct tokentype_s 
{
	/* the token number. */
	int tokennum;
	/* the line and column number that the token appeared on */
	int line,col;
	/* a pointer to the token string, null-terminated.  This
	   is only meaningful if the tokennum is one of T_STRING,
	   T_ALPHA, T_NUMBER.  In all other cases, the token is
	   a single character, and is represented completely in 
	   the tokennum.
	*/
	union 	
	{
		char *cptr;
	} u;
	/* the length of the token, again if tokennum is T_STRING, T_ALPHA,
	   or T_STRING. 
	*/
	int length;
} tokentype ;
	
/* maximum number of tokens */
#define MAX_TOKEN 50

/* array of tokens */
tokentype token[MAX_TOKEN+1];

/* array of pointers to tokens */
typedef tokentype *token_t;
token_t tokenlist[MAX_TOKEN+1];

/* number of tokens parsed in the current statement */
int ntokens;

/* file number for output file */
int output;

/* set if there have been errors */
int Errors;

/* number of input files */
int nipfiles;

/* number of current input file */
int ipfileno;

/* set if we are to do a listing */
int dolisting;

/* current output file name */
char *outfile;

/* array of file names */
char *inputfile[];

/* file descriptors for input file and listing file */
FILE *input,*listing;

/* routine for processing an escape within a string, and its
   global return value
*/
int escval;
char *doesc();

/* binary code buffer for the current statement. */
/* we need enough room in the buffer for
   MAX_TOKEN/2 longs, or 2*MAX_TOKEN bytes.
*/
#define BUFMAX (MAX_TOKEN<<1)
union gbinary
{
	unsigned char chars[BUFMAX];
	unsigned long longs[BUFMAX>>2];
	unsigned short shorts[BUFMAX>>1];
} binary;

/* number of bytes in the binary code buffer in the current statement */
int nbinary;

/* fast copy macro */
#define fastcopy(dest,src,nb) { bcopy(src,dest,nb);*(dest+nb) = 0;}

/* fast compare macro */
#define fastcomp(a,b) (!strcmp(a,b))

/* set if there have been errors in the current statement.  If set,
   code for the statement is never generated.
*/
int errors_in_statement;

/* number of the last statement parsed */
int last_statementno;

/* byte array of condensation info.  If condensation_info[56] = 2,
   then two words were condensed from the code sequence initially generated
   for statement 56.  The address of statement 56 is given by summing all
   of the condensation array elements less than 56 whose statements are
   in the same control section.  
*/
unsigned char *condensation_info;

#define update_dot(b) nbinary += b; \
		    dot[cur_csect] += b

#define TRUE 1
#define FALSE 0

int Inherantsize;
int debug;
int linenum;
int stabsinsource;
#define LONG 2
#define WORD 1
#define BYTE 0

tokentype *allocate_templab();
long curstat_labaddr;
int isstdin;
int is68020;
#define STR_EXTENT 0x1000 
