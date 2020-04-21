#include <stdio.h>
#include "globals.h"
FILE *errfile=stderr;

warning(tokenptr,string,a,b,c,d,e)
tokentype *tokenptr;
char *string;
{
#ifdef DEBUG
	fprintf(errfile,"\nas20:");
#else
	fprintf(errfile,"as20:");
#endif
	printfilename();
	if (tokenptr != (tokentype *)0)
		fprintf(errfile,"; statement %d, column %d:",tokenptr->line,tokenptr->col);
	else if (linenum)
		fprintf(errfile,"; statement %d:",linenum);
		
	fprintf(errfile,string,a,b,c,d,e);
	fprintf(errfile,"\n");

}

error(tokenptr,string,a,b,c,d,e)
tokentype *tokenptr;
char *string;
{
#ifdef DEBUG
	fprintf(errfile,"\nas20:");
#else
	fprintf(errfile,"as20:");
#endif
	printfilename();
	if (tokenptr != (tokentype *)0)
		fprintf(errfile,"; statement %d, column %d:",tokenptr->line,tokenptr->col);
	else if (linenum)
		fprintf(errfile,"; statement %d:",linenum);
	fprintf(errfile,string,a,b,c,d,e);
	fprintf(errfile,"\n");
#ifdef CATCH_BUG
	asm("\tbra\t0");
#endif
	Errors++;
}

fatal(tokenptr,string,a,b,c,d,e)
/* VARARGS */
tokentype *tokenptr;
char *string;
long a,b,c,d,e;
{

	/* a fatal error  has occurred.  abort */

	fprintf(errfile,"\nas20:");
	fprintf(errfile,"FATAL error ");
	printfilename();
	if (tokenptr != (tokentype *)0)
		fprintf(errfile,"; statement %d, column %d:",tokenptr->line,tokenptr->col);
	else if (linenum)
		fprintf(errfile,"; statement %d:",linenum);
	fprintf(errfile,string,a,b,c,d,e);
	fprintf(errfile,"\n");
	flush_errors();
	closeall();
	unlink(outfile);
	exit(-1);
}


flush_errors() 
{}

closeall() 
{
	if (input != (FILE *)0) fclose(input);
	if (output >= 0) close (output);
}

printfilename()
{
	/*  give an indication of the file we are processing.  This
	    is either a) the current input file name (if we are
	    opening the files directly), or b) the output file name
	    (if we are reading from standard input or a pipe
	*/

	if (isstdin)
	{
		fprintf(errfile,"<output = %s>",outfile);
	}
	else fprintf(errfile,"%s",inputfile[ipfileno]);
}
