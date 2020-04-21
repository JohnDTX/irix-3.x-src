/* fatal - prints error message preceded by the command name found in
	   the global string "f_name".

	   If code is non-zero, then the routine found in f_clean is called
	   and fatal exits with the code.
	   Otherwise, fatal returns to the caller.
*/

#include	"lp.h"

SCCSID("@(#)fatal.c	3.1")

char *f_name = NULL;
int (*f_clean)() = NULL;

fatal(msg, code)
char *msg;
int code;
{
	if(f_name != NULL)
		fprintf(stderr, "%s: ", f_name);
	fprintf(stderr, "%s\n", msg);
	fflush(stderr);

	if(code != 0) {
		if(f_clean != NULL)
			(*f_clean)();
		exit(code);
	}
}
