/*
 *
 *     scan.c - scanner for as20.
 *		basic operation is as follows:
 *
 *		* a 4K byte input buffer is maintained.  No
 *		  unnecessary copying of the input is done. When 
 *		  the end of buffer is hit, that input which is 
 *		  current (in the current statement) is copied
 *		  to the front of the buffer, and the rest of
 *		  the buffer is filled.  As tokenize()
 *		  is a destructive operation, the line cannot simply
 *		  be re-tokenized.  (Tokenize null-terminates its tokens,
 *		  and single-character tokens are turned into nulls.)  
 *		  The buffer may be less than 4K due to blocking on 
 *		  the input pipe.
 *
 *		* The current input line (in the input buffer)
 *		  is tokenized (tokenize()).  An array of tokens is filled
 *		  and a pointer to each token placed in an array
 *		  of pointers to tokens (tokenlist).  The final
 *		  real token pointer is followed by a NULL token
 *		  pointer.
 *
 *		* parse_line() is responsible for parsing the current
 *		  line.  It is called by do_statement() in statement.c.
 *		  The first few tokens determine the linetype.
 *		  This linetype is returned to do_statement(), to
 *		  indicate the basic disposition of the line (comment,
 *		  statement, pseudo-op, equates, or an error condition).
 *
*/

#include <stdio.h>
#include "globals.h"
#include "scan.h"
#include "tokens.h"

/* save a few static pointers: the end of the active buffer,
   the end of the current line, the pointer into the current line,
   and the beginning of the current line.
*/
static unsigned char *linebufend = linebuf;
static unsigned char *lineend = linebuf;
static unsigned char *lineptr = linebuf;
static unsigned char *linebegin = linebuf;


init_scan()
{
	/* 0xff is the 'end of active buffer' indicator.
	   Mark the real end of the buffer forever, and 
	   the first element to get started with.
	*/
	*linebuf = 0xff;
	*(linebuf + LINEBUFMAX) = 0xff;
	linebufend = linebuf;
	lineend = linebuf;
	lineptr = linebuf;
	linebegin = linebuf;
	ntokens = 0;
}

int nbcopied[0x1000],bufsz[0x1000],nbufs = 0;

linetype_t 
parse_line()
{
	/*  parse the current line into tokens. Return
	    the type of the line -- either statement, equates,
	    pseudo-op, comment, or error.
	*/

	int psindx;
	int nbread,nb,class;
	int i;
	int restart;

	tokenlist[0] = (tokentype *)0;

	linebegin = lineptr;
	restart = 0;
	/* tokenize returns zero on error, 1 on success and -1
	   if end-of-buffer is hit.  If tokenize has to be re-called
	   (due to end-of-buffer hit), it must be called with a non-zero
	   value.  If it is to be called for a 'virgin' line, it must
	   be called with zero.
	*/
	while ((class = tokenize(restart))<=0)
	{
		if (class == (-1))
		{
			/* the end of the buffer has been reached.
			   refill it.  Have to refresh the input buffer.  
			   Copy the remainder of the buffer to the front.
			*/
			token_t *tokenptr = tokenlist;
			nb = linebufend - linebegin;
			fastcopy(linebuf,linebegin,(nb));
#ifdef CATCH_BUG
			nbcopied[nbufs] = nb;
#endif
			lineptr -= (linebegin - linebuf);

			/* The tokens which have already been parsed
			   and are of one of the indeterminate types
			   (string, alpha or number) have pointers into
			   the input buffer in them.  As the buffer has 
			   just been shifted, we need to 
			   update the pointers to the token strings
			   with the new buffer address. 
			*/
			while (*tokenptr != (token_t)0)
			{
				(*tokenptr)->u.cptr -= (linebegin - linebuf);
				tokenptr++;
			}

			/* a statement cannot be larger than the buffer */
			if ((LINEBUFMAX - nb) <= 0) return(L_ERROR);

			/* read enough bytes to fill the remainder of
			   the buffer.
			*/
			nbread = read(fileno(input),
				linebuf + nb,LINEBUFMAX - nb);

#ifdef CATCH_BUG
			bufsz[nbufs++] = nbread;
#endif
			/* if we cant get anything, this is really the
			   end of the file.
			*/
			if (!nbread) return(L_EOF);

			/* mark the end of the buffer */
			linebufend = linebuf + nb + nbread;
			*linebufend = 0xff;

			/* reset the start of the line to the beginning of 
			   the buffer */
			linebegin = linebuf;

			/* and set the flag to pass to tokenize for
			   restarting the line.
			*/
			restart = 1;
		}
		else 
			/* error, tokenize returned zero.  It found
			   an illegal character on the input line.
			*/
			return(L_ERROR);
	}
	/* the line was of some legal format, and needs standard 
	   handling.
	*/


	/* check for too many tokens */
	if (ntokens >= MAX_TOKEN) return(L_TOOMANYTOKENS);

	/* ignore preprocessor-type lines.  These begin with '#',
	   which is the single-character token T_IMM
	*/
	if ((tokenlist[0] != (tokentype *)0) &&
		(tokenlist[0]->tokennum == T_IMM))
		return(L_COMMENT);

	/* ignore blank lines */
	if (tokenlist[0] == (tokentype *)0)
		return(L_COMMENT);

	/* look for direct assignment statements (equates).  These
	   are of the form <symbol> '=' ... 
	*/
	if ((tokenlist[1] != (tokentype *)0) &&
		(tokenlist[1]->tokennum == T_EQ))
		return(L_EQUATES);

	/* look for pseudo-ops.  They can have a label, so
	   check for it.
	*/
	if ((tokenlist[1] != (tokentype *)0) &&
		(tokenlist[1]->tokennum == T_COLON))
		/* label on line.  look for pseudo-op at token #2 */
		psindx = 2;
	else 
		/* no label.  look for pseudo-op at token #0 */
		psindx = 0;

	/* the line is a pseudo-op if the token at psindx exists and
	   is T_IDENT and its first char is a period.
	*/

	if ((tokenlist[psindx] != (tokentype *)0) &&
	    (tokenlist[psindx]->tokennum == 0) &&
	    (*(tokenlist[psindx]->u.cptr) == '.'))
		return(L_PSEUDO);

	/* default type is statement */
	return(L_STATEMENT);


		
}

#define get_token() (&token[ntokens])

/* some state information used when a line spans an input
   block.  We need to save the pointer to the current token
   and the pointer to the current position in the array of pointers
   to tokens.  There is also a state variable indicating whether
   a comment was being processed when the end-of-block was hit.
*/
static tokentype *tokenptrsv ;
static tokentype **tokenlistptrsv ;
static int commentpending = 0;

tokenize(restart)
{
	/*  
		tokenize divides the input line into a list of 
		null terminated tokens.  The problem of null terminating
		a token without copying it is handled by replacing 
		all single-character tokens with a single null and placing
		their tokenid in the token structure.  tokenize returns
		the following values upon the indicated termination
		conditions:

		   *	a newline has been seen.  return(1).

		   *  	the input buffer is exhausted.  The return
			value of (-1) indicates that the current
			line must be restarted.

		   *	an error has been encountered.  In this case,
			tokenize gobbles characters until a newline is
			found and then returns zero.
		
		If tokenize is called with restart non-zero, a line
		has been partially tokenized.  One of the following conditions
		is true:

		   *	an error or comment has been encountered, and 
		   	tokenize was attempting to read to the end-of-line
			when the end-of-buffer was hit.  Simply gobble up
			characters until a newline.

		   *	we are in the middle of a token.  The pointer to 
			the current token is restored, and the token is
			restarted.

	*/

	register char curclass = C_IGN;

	/* a pointer to the start of the current token, and its first
	   and last character.
	*/
	unsigned char *tokenst,beginc;
	unsigned char lastc;
	/* a pointer to the current character on the current line */
	register unsigned char *lp = lineptr;
	register nextclass;
	/* a pointer to the current token structure. */
	register tokentype *tokenptr ;
	/* a pointer into the current slot in the array of pointers to 
	   tokens.
	*/
	register tokentype **tokenlistptr;

	if ((restart)&&(ntokens))
	{
		/* there is a partially-tokenized line pending.
		   Restore the current token pointer and the
		   pointer to the current slot in the array of 
		   pointers to tokens.
		*/
		tokenptr = tokenptrsv;
		tokenlistptr = tokenlistptrsv;

		/* if there is an unfinished comment, gobble it up. */
		if (commentpending) {
			commentpending=0;
			tokenst = lp;
			curclass = C_COMMENT;
			goto trytogobbleline;
		}
	}
	else
	{
		/* we are at the start of a line. Point to the 
		   first token. 
		*/
		tokenptr = token;
		tokenlistptr = tokenlist;
		ntokens = 0;

		/* There may have been a comment pending even if no
		   tokens have been found.  This would be true, for
		   example, if the line was ONLY a comment, and
		   an end-of-block was hit while parsing it.   Gobble 
		   the rest of the line.
		*/
		if (commentpending) {
			commentpending=0;
			tokenst = lp;
			curclass = C_COMMENT;
			goto trytogobbleline;
		}
		commentpending = 0;
		do
		{
			/* find the start of the first token */
			curclass = *(cmap + *lp);

			/* if we hit the end of the block, return
			   the restart flag.
			*/
			if (curclass == C_ENDOFBLOCK) return(-1);

			/* break when a character which is NOT to be
			   ignored is found.
			*/
			if (curclass != C_IGN) break;
			lp++;
		} while (1);
	}
	do
	{
		/* save the first character, and a pointer to the start
		   of the token.
		*/
		beginc = *lp;
		tokenst = lp;

		/* get the current class */
		curclass = (*(cmap + beginc));

		/* zero the current token pointer */
		*(tokenlistptr) = (tokentype *)0;

		if (curclass < C_SPECIAL)
		{
			/* ALPHA or IGN */
			/* if this is a whitespace token, use its first
			   character to null-terminate the last token.
			*/
			if (curclass == C_IGN) *lp++ = 0;

			/* while the next class is the current one, advance */
			while ((nextclass = *(cmap + (*lp))) == curclass) lp++;

			/* if we hit the end-of-block while we were parsing
			   an alphabetic token ...
			*/
			if ((nextclass == C_ENDOFBLOCK)&&(curclass == C_ALPHA))
			{
				/* this token may not be finished.  Back
				   out to the start of the current token.
				*/
				lineptr = tokenst;
				tokenptrsv = tokenptr;
				tokenlistptrsv = tokenlistptr;
				return(-1);
			}
			/* else point to the first character which broke the
			   class
			*/
			lp--;
		}

		/* now take appropriate action acctd to the class of token */
		switch (curclass)
		{
		case C_IGN:	
			/* just passed some whitespace. Advance
			   to the first non-ignoring character.
			*/
			lp++; 
			break;

		case C_COMMENT:
			/* use the comment character to null terminate
			   the last token.
			*/
			*lp++ = 0;
			break;

		case C_ERR:
			break;

		case C_ENDOFBLOCK:
			/* save the current position in the token
			   arrays
			*/
			lineptr = lp;
			tokenptrsv = tokenptr;
			tokenlistptrsv = tokenlistptr;
			return(-1);
		case C_NL:
			/* end of line.  Null-terminate the last
			   token and return a successful tokenization.
			*/
			*lp++ = 0;
			lineptr = lp;
			return(1);
			
		case C_VHASH:
			if (ntokens == 0)
			{
				/* this is a cpp control line.  Treat
			   	   it as a comment.
				*/
				curclass = C_COMMENT;
				*lp++ = 0;
				break;
			}
		case C_DQ:
		case C_SQ:
		case C_SPECIAL:
			/* the class will be used to get the tokenid.
			   This is all the data we need.  Use this
			   single-character token to null-terminate
			   the previous one.
			*/
			*lp = 0;

		case C_ALPHA:
			/* the token is alphabetic, numeric or special.  Place
			   a pointer to it in the token array, the line
			   and col it occurred on, its length, and get its
			   tokenid.
			*/
			lastc = *lp++;
			tokenptr->u.cptr = (char *)tokenst;
			tokenptr->line = linenum;
			tokenptr->col = tokenst - lineptr;
			tokenptr->length = lp - tokenst;
			if ((beginc == '.') && (tokenptr->length == 1))
				tokenptr->tokennum = T_DOT;
			else
				tokenptr->tokennum = 
					*(char_to_token + beginc);

			/* 10$ is really alphabetic (a temporary label) */
			if ((tokenptr->tokennum == T_NUMBER)&&(lastc == '$'))
				tokenptr->tokennum = T_ALPHA;

			/* quotes are not counted as characters - only
			   the strings they enclose.
			*/
			if ((curclass != C_DQ) && (curclass != C_SQ))
			{
				if (ntokens >= MAX_TOKEN) {
					curclass = C_ALPHA;
					goto trytogobbleline;
				}
				ntokens++;
				*tokenlistptr++ = tokenptr++;
			}
		}

		tokenst = lp;
		if ((curclass == C_DQ)||(curclass == C_SQ))
		{
			/* delimit the string and enter it as a token */
			beginc = *lp;
			tokenptr->line = linenum;
			tokenptr->col = tokenst - lineptr;
			tokenptr->u.cptr = (char *)tokenst;
			do
			{
				/* when we find the matching quote, break */
				if ((nextclass = (*(cmap + *lp++))) == curclass)
					break;

				/* if the terminator is escaped, gobble it */
				if ((*(lp-1) == '\\')&&
				    ((*lp == '\\')||
				    ((*(cmap + *lp)) == curclass)))
					lp++;

				/* ';' is also a newline character.  Ignore
				   it in strings.  Look only for '\n'.
				*/
				if (nextclass == C_NL)
				{
					if (*(lp-1) == '\n')
					{
						error(tokenptr,
					     "newline in string constant");
						break;
					}
				}

				/* if we hit the end of the block, restore
				   the quote character before the string 
				   and signal restart.
				*/
				if (nextclass == C_ENDOFBLOCK)
				{
					lineptr = tokenst - 1;
					*lineptr = (curclass == C_SQ)?'\'':'\"';
					tokenptrsv = tokenptr;
					tokenlistptrsv = tokenlistptr;
					return(-1);
				}
			} while(1);
			lp--;

			/* get the length */
			tokenptr->length = lp - tokenst;

			/* make sure it's null-terminated. */
			*lp++ = 0;

			/* dont run off the end of the token array */
			if (ntokens >= MAX_TOKEN) {
				curclass = C_ALPHA;
				goto trytogobbleline;
			}

			/* and enter the token number as appropriate */
			tokenptr->tokennum = ((curclass == C_SQ) && 
				(tokenptr->length == 1))? T_CHAR:T_STRING;
			ntokens++;


			/* enter the pointer to the token into the array 
			   of pointers to tokens
			*/
			*tokenlistptr++ = tokenptr++;
			*(tokenlistptr) = (tokentype *)0;

			if (nextclass == C_NL) 
			{
				lineptr = lp;
				return(1);
			}
		}
				

	} while (curclass < C_COMMENT);

trytogobbleline:

	/* some sort of error has been seen or the line is a 
	   comment.  Try to gobble up the rest of the line.
	*/
	do
	{
		nextclass = *(cmap + *lp);
		/* the end of the line is delimited by a C_NL character
		   (';' or '\n') UNLESS we are processing a comment, in
		   which case ';' doesn't count.
		*/
		if ((nextclass == C_NL)&&((curclass != C_COMMENT)||(*lp == '\n')))
		/* if (*lp == '\n') */
		{
			lineptr = ++lp;
			return((curclass == C_ERR)?0:1);
		}
		if (nextclass == C_ENDOFBLOCK)
		{
			tokenptrsv = tokenptr;
			lineptr = tokenst;
			tokenlistptrsv = tokenlistptr;
			commentpending = 1;
			return(-1);
		}
		lp++;
	} while(1);
}
				

unsigned long 
buildhex ( cptr) 
register char *cptr;
{
	/* build a hexadecimal number.  Return its value */
	register unsigned long retval=0;
	register char c;
	while (*cptr == '0') cptr++;
	while (c = (*cptr++))
	{
		if ((c >= 'a')&&(c <= 'z')) c -= 'a' - 'A';
		if (((c < 'A')&&(c > 'F'))&&((c < '0')||(c > '9')))
		{
			error(0,"illegal hexadecimal number");
			errors_in_statement++;
			return(0);
		}
		retval = retval*16 + ((c >= 'A')? (c - 'A' + 10): (c - '0'));
	}
	return(retval);
}  /*  buildhex  */


unsigned long 
builddec ( cptr) 
register char *cptr;
{
	/* build a decimal number.  Return its value. */
	register unsigned long retval=0;
	while (*cptr == '0') cptr++;
	while (*cptr)
	{
		if ((*cptr < '0')||(*cptr > '9'))
		{
			error(0,"illegal decimal number");
			errors_in_statement++;
			return(0);
		}
		else retval = retval*10 + *cptr++ - '0';
	}
	return(retval);

}  /*  builddec  */

unsigned long 
buildoct ( cptr) 
register char *cptr;
{
	/* build an octal number.  Return its value */
	register unsigned long retval=0;
	while (*cptr == '0') cptr++;
	while (*cptr)
	{
		if ((*cptr < '0')||(*cptr > '7'))
		{
			error(0,"illegal octal number");
			errors_in_statement++;
			return(0);
		}
		retval = retval*8 + *cptr++ - '0';
	}
	return(retval);
}  /*  buildoct  */

long 
buildnum (cptr)
register char *cptr;
{
	/* build a number.  Decides whether the number is octal (has
	   a leading zero), hexadecimal (has a leading 0x), or, by
	   default, decimal.  The number may have a leading minus sign
	   for negative.  
	*/
	int isneg=0;
	register long retval;
	if (*cptr == '-')
	{
		isneg++;
		cptr++;
	}
	if (*cptr == '0')
	{
		cptr++;
		if (*cptr == 'x') retval = (buildhex(++cptr));
		else retval = (buildoct(cptr));
	}
	else retval = (builddec(cptr));

	if (isneg) return(-retval); else return(retval);
}

char *
doesc(lbptr,end) 
register char *lbptr;
char *end;
{
	/* an escape character has been seen, and is at the
	   current position in the buffer.  This
	   must be an escaped character in a string.
	   The escape value is returned in the global integer
	   escval, and a pointer to the next character in the
	   input buffer is returned.
	   If no character is encountered, escval is set to (-1).

	*/

	int ishex=0,i;
	register retval;
	register char curc;
	/* past the escape.. */
	escval = (-1);
	lbptr++;	
	if (lbptr >= end) return(end);

	/* ok, there is really an escaped character */
	curc = *lbptr;

	if (curc == 'x') {
	    /* hexadecimal number to follow */
	    ishex++;
	    lbptr++;
	    curc = 0;
	    goto donum;
	}
	if ((curc < '0')||(curc > '9'))
	{
		/*  not a digit.  Must be a special escape character
		    encoding (tab, form-feed, etc.)
		*/
		lbptr++;
		switch (curc) {

			default:
				warning((tokentype *)0,
				   "unrecognizable escape sequence in string");
			case '\'':
			case '\\':
			case '\"':	escval = curc; break;
			case 'b':	escval = ('\b'); break;
			case 'a':	escval = 0x7   ; break;
			case 'f':	escval = ('\f'); break;
			case 'n':	escval = ('\n'); break;
			case 'r':	escval = ('\r'); break;
			case 't':	escval = ('\t'); break;
			case 'v':	escval = ('\v'); break;
		}
		return(lbptr);
	}
	
donum:
	/* assemble the number */
	retval = 0;
	for (i=0;i<((ishex)?2:3);i++)
	{
		register char c;
		register hexdig;

		c = *lbptr++;

		if (lbptr > end) break;
		if ((c >= 'a')&&(c <= 'z')) c -= 'a' - 'A';
		if ((c >= '0')&&(c <= '7')) hexdig = 0;
		else if ((ishex) && 
			  ((c=='8')||(c=='9')||((c >= 'A')&&(c <= 'F')))) 
			hexdig = 1;
		else {
			error((tokentype *)0,
				"illegal non-digit in escape sequence");
			break;
		}
		if (hexdig) {
			retval *= 16;
			if (c >= 'A') 
				retval += c - 'A' + 10;
			else retval += c - '0';
		} else
		{
			retval *= 8;
			retval += c - '0';
		}
	} 
	escval = retval;
	return(lbptr);
}


#ifdef DEBUG
dump_tokens(linetype)
linetype_t linetype;
{
	/* dump the current token list and linetype */
	int indx,i;
	tokentype *tokenptr;
	char *cptr;

	fprintf(stderr,"\ndump_tokens: statement type = ");
	switch (linetype)
	{
	case L_ERROR:
				fprintf(stderr,"L_ERROR:");
				return(0);
	case L_PSEUDO:
				fprintf(stderr,"L_PSEUDO:");
				break;
				
	case L_STATEMENT:
				fprintf(stderr,"L_STATEMENT:");
				break;
	case L_COMMENT:
				fprintf(stderr,"L_COMMENT:");
				break;
	case L_EQUATES:
				fprintf(stderr,"L_EQUATES:");
				break;
		default:	fprintf(stderr,"UNKNOWN:");
	} 

	/* dump the token array */

	for (indx=0;indx<ntokens;indx++)
	{
		tokenptr = tokenlist[indx];
		fprintf(stderr,"\ntoken[%d] = ",indx);
		cptr = tokenptr->u.cptr;
		fprintf(stderr,"%s",cptr);
		fprintf(stderr,": num = %d, statement = %d, col = %d, len = %d",
			tokenptr->tokennum,tokenptr->line,
			tokenptr->col,tokenptr->length);
	}
	putc('\n',stderr);

}

#endif
