/*
 * $Source: /d2/3.7/src/sys/config/RCS/mkubglue.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:27:35 $
 */
#ifndef lint
static char sccsid[] = "@(#)mkubglue.c	1.11 (Berkeley) 8/11/83";
#endif

/*
 * Make the uba interrupt file ubglue.s
 */
#include <stdio.h>
#include "config.h"
#include "y.tab.h"

ubglue()
{
	register FILE *fp;
	register struct device *dp, *mp;

	fp = fopen(path("ubglue.s"), "w");
	if (fp == 0) {
		perror(path("ubglue.s"));
		exit(1);
	}
	for (dp = dtab; dp != 0; dp = dp->d_next) {
		mp = dp->d_conn;
		if (mp != 0 && mp != (struct device *)-1 &&
		    !eq(mp->d_name, "mba")) {
			struct idlst *id, *id2;

			for (id = dp->d_vec; id; id = id->id_next) {
				for (id2 = dp->d_vec; id2; id2 = id2->id_next) {
					if (id2 == id) {
						dump_vec(fp, id->id, dp->d_unit);
						break;
					}
					if (!strcmp(id->id, id2->id))
						break;
				}
			}
		}
	}
	(void) fclose(fp);
}

/*
 * print an interrupt vector
 */
dump_vec(fp, vector, number)
	register FILE *fp;
	char *vector;
	int number;
{
	char nbuf[80];
	register char *v = nbuf;

	(void) sprintf(v, "%s%d", vector, number);
	fprintf(fp, "\t.globl\t_X%s\n\t.align\t2\n_X%s:\n\tpushr\t$0x3f\n",
	    v, v);
	if (strncmp(vector, "dzx", 3) == 0)
		fprintf(fp, "\tmovl\t$%d,r0\n\tjmp\tdzdma\n\n", number);
	else {
		if (strncmp(vector, "uur", 3) == 0) {
			fprintf(fp, "#ifdef UUDMA\n");
			fprintf(fp, "\tmovl\t$%d,r0\n\tjsb\tuudma\n", number);
			fprintf(fp, "#endif\n");
		}
		fprintf(fp, "\tpushl\t$%d\n", number);
		fprintf(fp, "\tcalls\t$1,_%s\n\tpopr\t$0x3f\n", vector);
		fprintf(fp, "#if defined(VAX750) || defined(VAX730)\n");
		fprintf(fp, "\tincl\t_cnt+V_INTR\n#endif\n\trei\n\n");
	}
}
