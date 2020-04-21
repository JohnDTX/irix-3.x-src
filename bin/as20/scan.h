/*
 *   scan.h -	header for the scanner of the 68020 assembler
 *		(as20).  
 *
 *
 *
 */
#define LINEBUFMAX 0x2000
unsigned char linebuf[LINEBUFMAX + 1];
unsigned char *linebufend;
unsigned char *lineptr,*lineend;
int ispipe,isterm;
int compound_line;


typedef enum {
	L_ERROR,L_TOOMANYTOKENS,L_COMMENT,L_PSEUDO,
	L_STATEMENT,L_EQUATES,L_EOF
	     } linetype_t;

#define L_IGNORE (int)L_COMMENT

linetype_t parse_line();

/* special char to token map */
char char_to_token[];
/* character map */
char cmap[] ;
