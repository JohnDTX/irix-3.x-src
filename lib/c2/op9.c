#include "opt.h"

#define GHASH	64

struct gstruct *gtab[GHASH];

struct gstruct {
	char	*g_name;	/* pointer to global name */
	struct	gstruct *g_ptr;	/* pointer to next global name */
};

/*
 * generate jsr global list
 */
genjsr(fp)
register FILE *fp;
{
	register struct gstruct *gp;
	register char *cp;
	register c;
	char lbuf[256];

	for (;;) {
		cp = lbuf;
		while ((c = getc(fp)) != '\n') {
			if (c == EOF) {
				(void) fseek(fp, 0L, 0);
				return;
			}
			*cp++ = c;
		}
		*cp = 0;
		cp = lbuf;
		if (*(cp+1)!='.' || strncmp("\t.globl\t_", cp, 9) != 0)
			continue;
		c = ghash(cp+8);
		gp = (struct gstruct *) calloc(1, sizeof *gp);
		if (gp == 0) {
		    fprintf(stderr, "c2:out of global name storage space\n");
		    exitt(-1);
		}
		gp->g_name = copy(cp+8);
		gp->g_ptr = gtab[c];
		gtab[c] = gp;
	}
}

/*
 * return 1 if the global name is in ththe hashed table
 */
findglob(name)
register char *name;
{
	register struct gstruct *gp;

	for (gp = gtab[ghash(name)]; gp; gp = gp->g_ptr)
		if (strcmp(gp->g_name, name) == 0)
			return(1);
	return(0);
}

/*
 * generate a global hash number
 */
ghash(cp)
register char *cp;
{
	register n;

	n = 0;
	while (*cp)
		n = n << 3 + *cp++;
	return(n & (GHASH-1));
}
