#include <stdio.h>
static FILE *pcons;

conserr(s, a)
char *a, *s;
{
	if (pcons==NULL)
		pcons = fopen("/dev/console", "w");

	if (pcons==NULL)
		return;

	fprintf(pcons, s, a); fflush(pcons);
}
