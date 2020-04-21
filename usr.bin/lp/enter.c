/* enter(name, array, size, max) -- enters name into an argv-type array.

	name -- item to be entered into array
	array -- array to be added to
	size -- pointer to the # of items in array
	max -- max # of items allowed in array

	return codes: 0 for success
		     -1 for failure
*/

#include	"lp.h"

SCCSID("@(#)enter.c	3.1")

int
enter(name, array, size, max)
char *name;
char *array[];
int *size;
int max;
{
	char *p, *malloc(), *strcpy();

	if(*size >= max)	/* no room in array */
		return(-1);
	if(name == NULL)
		array[(*size)++] = NULL;
	else {
		if((p = malloc((unsigned)(strlen(name)+1))) == NULL)
			return(-1);
		array[(*size)++] = p;
		strcpy(p, name);
	}
	return(0);
}
