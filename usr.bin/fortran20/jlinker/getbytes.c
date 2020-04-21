#include <stdio.h>
#include "getbytes.h"


get3byts(jfilep, retval)
FILE	*jfilep;
long	*retval;
{
	char	sizebuf[3];

	if (fread(sizebuf, sizeof(sizebuf), 1, jfilep) != 1) {
		fprintf(stderr, "cannot read 3 byte value\n");
		exit(200);
	}
	*retval = ((unsigned char) sizebuf[0]*256 + 
			(unsigned char) sizebuf[1])*256 + 
			(unsigned char) sizebuf[2];
}
	
#define	NMPOOLSZ	4096
char	*namepool;
char	*curnmpool;
int	nameind;
char	*endnmpool;

	



char	*
getnm(jfilep)
FILE	*jfilep;
{
	int	nameln;
	char	*name;
	char	*malloc();

	if (nameln = getc(jfilep)) {
		name = malloc(nameln + 1);
		if (fread((char *) name, nameln, 1, jfilep) != 1) {
			fprintf(stderr, "cannot read procedure information\n");
			exit(600);
		}
		name[nameln] = (char) 0;
		return(name);
	} else {
		return((char *) 0);
	}
}

short
getshort(jfilep)
FILE	*jfilep;
{
	short	shortbuf;

	if (fread((char *) &shortbuf, sizeof(shortbuf), 1, jfilep) != 1) {
		fprintf(stderr, "cannot read short\n");
		exit(600);
	}
	return (shortbuf);
}


getdata(dest, source, nbytes)
char	*dest;
char	**source;
short	nbytes;
{

	int	i;

	for (i = 0; i < nbytes; i++) {
		*dest++ = *source[i];
		*source++;
	}
}
