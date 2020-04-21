#include "defs.h"
/****************************************************************************

 DEBUGGER - input routines

****************************************************************************/
int mkfault;
char linebuf[LINSIZ];
FILE *infile;
FILE *outfile;
char *lp;
char lastc = EOR;
int eof;

/* input routines */

eol(c)
char c;
{
	return(c == EOR || c == ';');
}

rdc()
{
	do 
		(readchar());
	while (lastc == SPACE || lastc == TB);
	return(lastc);
}

readchar()
{
	if (eof) lastc = NULL;
	else
	{
		if (lp == 0)
		{
			lp = linebuf;
			do 
				(eof = (fread(lp, 1, 1, infile) != 1));
			while (eof == 0 && *lp++ != EOR);
			*lp = 0;
			lp = linebuf;
		}
		if (lastc = *lp) lp++;
	}
	return(lastc);
}

nextchar()
{
	if (eol(rdc()))
	{
		lp--;
		return(0);
	}
	else return(lastc);
}

quotchar()
{
	if (readchar() == '\\') return(readchar());
	else if (lastc == '\'') return(0);
	else return(lastc);
}

getformat(deformat)
char *deformat;
{
	char *fptr = deformat;
	int quote = FALSE;

	while (quote ? readchar() != EOR : !eol(readchar()))
		if ((*fptr++ = lastc) == '"') quote = ~quote;
	lp--;
	if (fptr != deformat) *fptr++ = '\0';
}


