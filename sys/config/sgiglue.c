/*
 * $Source: /d2/3.7/src/sys/config/RCS/sgiglue.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:27:36 $
 */
#include <stdio.h>
#include "config.h"
#include "y.tab.h"

/* default interrupt table */
static	char *vecs[8] = {
	"default",
	"default",
	"default",
	"default",
	"default",
	"default",
	"default",
	"default"
};

/*
 * Make the interrupt file mbglue.s
 */

sgi_glue()
{
#ifdef	notdef
	register FILE *fp;
	register struct device *dp, *mp;
	register short i;

	return;
	fp = fopen(path("mbglue.s"), "w");
	if (fp == 0) {
		perror(path("mbglue.s"));
		exit(1);
	}
	for (dp = dtab; dp != 0; dp = dp->d_next) {
		mp = dp->d_conn;
		if (mp != 0 && mp != (struct device *)-1 &&
		    eq(mp->d_name, "mb")) {
			struct idlst *id, *id2;

			for (id = dp->d_vec; id; id = id->id_next) {
				for (id2 = dp->d_vec; id2; id2 = id2->id_next) {
					if (id2 == id) {
						sgidump_vec(fp, id->id, dp->d_unit);
						if (dp->d_pri >= 0)
							vecs[dp->d_pri] = id->id;
						break;
					}
					if (!strcmp(id->id, id2->id))
						break;
				}
			}
		}
	}
	sgidump_vec(fp, "default", 0);

    /* generate indirect vectors table for configure() */
	fprintf(fp, "\n\t.globl _intvectors\n");
	fprintf(fp, "\t.data\n_intvectors:\n");
	for (i = 0; i < 8; i++)
		fprintf(fp, "\t.long\t%s\n", vecs[i]);
	(void) fclose(fp);
#endif
}

#ifdef	notdef
/*
 * print an interrupt vector
 */
sgidump_vec(fp, vector, number)
	register FILE *fp;
	char *vector;
	int number;
{
	char nbuf[80];
	register char *v = nbuf;

	if (number)
		(void) sprintf(v, "%s%d", vector, number);
	else
		v = vector;

	fprintf(fp, "\n\t.globl %s\n", v);
	fprintf(fp, "%s:\n", v);
	fprintf(fp, "\tmovb\t#0, CONTEXT\n");
	fprintf(fp, "\tclrw\t_idleflg\n");
	fprintf(fp, "\tmoveml\t#0xC0C0, sp@-\n");
	fprintf(fp, "\tjsr\t_%s\n", v);
	fprintf(fp, "\tmoveml\tsp@+, #0x0303\n");
	fprintf(fp, "\tmovb\t_ccontext, CONTEXT\n");
	fprintf(fp, "\trte\n");
}
#endif
