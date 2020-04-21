/* routines for writing and reading request files under request directory */

#include	"lp.h"

SCCSID("@(#)request.c	3.1")

/* putrent -- add a request to the specified request file.
	       a request consists of a single character request code
	       followed by a space, followed by text	*/

putrent(request, text, file)
char request;
char *text;
FILE *file;
{
	fprintf(file, "%c %s\n", request, text);
}

/* rmreq -- remove a request file and associated data files */

rmreq(dest, seqno)
char *dest;
int seqno;
{
	char cmd[sizeof(REQUEST)+DESTMAX+SEQLEN+10];

	sprintf(cmd, "rm -f %s/%s/*-%d", REQUEST, dest, seqno);
	system(cmd);
}

/* getrent(request, text, file) -- gets the next request file entry from file.
	'request' is the type of request entry
	'text' is the associated text

	returns: EOF on end of file
		 0 otherwise
*/

int
getrent(request, text, file)
char *request;
char *text;
FILE *file;
{
	char c, *t;
	*request = fgetc(file);
	if(feof(file))
		return(EOF);
	fgetc(file);		/* skip past blank */
	t = text;
	while((c = fgetc(file)) != '\n')
		*(t++) = c;
	*t = '\0';
	return(0);
}
