
#include "defs.h"
#include <stdio.h>
#include "symbols.h"
#include "readj.h"
#include "getbytes.h"


struct	linetable {
	LINEHEADER	linehdr;
	LINEADDR	*lineaddrs;
	struct	linetable	*next;
};

struct srcfile		srcf;
struct	linetable	*firstblk;
struct	linetable	*lastblk;

bpoint1blk(jfilep, size)
FILE	*jfilep;
int	size;
{


	char	*malloc();
	LINEADDR	*lp;
	struct	linetable	*newblk;
	short	numentrys;
	char	enttype;
	char	*linkname;
	long	entryaddr;
	long	exitaddr;
	short	numlines;
	short	lineoff;
	short	lineno;
	short	i;
	char	*filename;

	srcf.nfuncs++;
	numentrys = getshort(jfilep);
	enttype = getc(jfilep);
	if (enttype != 0) {
		lineoff = getshort(jfilep);
		exitaddr = getw(jfilep);
		i = getshort(jfilep);
	}
	for (i = 0; i < numentrys; i++) {
		linkname = getnm(jfilep);
		entryaddr = getw(jfilep);
		exitaddr = getw(jfilep);
		if (i != numentrys -1) {
			getshort(jfilep);
		}
	}

	newblk = (struct linetable *) malloc(sizeof(struct linetable));
	if (!firstblk) {
		firstblk = lastblk = newblk;
	} else {
		lastblk->next = newblk;
		lastblk = newblk;
	}
	newblk->linehdr.funcnum = curfunc->symnum;
	newblk->linehdr.nlines = numlines = getshort(jfilep);
	if (numlines) {
		newblk->lineaddrs = (LINEADDR *) 
					malloc(sizeof(LINEADDR) * numlines);


		for (lp = &(newblk->lineaddrs[0]); lp < &(newblk->lineaddrs[numlines]);
						lp++) {
			if (fread(lp, sizeof(LINEADDR), 1, jfilep) != 1 ) {
				fprintf(stderr, "cannot read line numbers\n");
				exit (300);
			}
			if ((filename = getnm(jfilep)) != (char *) 0) {
				srcf.filename = 
					(long) identname(filename, true);
			}
		}
	}
}


line_dump(ofilep)
FILE	*ofilep;
{
	struct	linetable	*linetabp;

	linetabp = firstblk;
	srcf.filename = stroff((Name) srcf.filename);
	if (fwrite(&srcf, sizeof(SRCFILE), 1, ofilep) != 1) {
			fprintf(stderr, "cannot write file header\n");
			exit (300);
	}
	while (linetabp != nil) {
		if (fwrite(&(linetabp->linehdr), sizeof(LINEHEADER), 1, ofilep)
			!= 1) {
			fprintf(stderr, "cannot write line header\n");
			exit (300);
		}

		if (fwrite(linetabp->lineaddrs, sizeof(LINEADDR), 
				linetabp->linehdr.nlines, ofilep) 
				!= linetabp->linehdr.nlines) {
			fprintf(stderr, "cannot write line numbers\n");
			exit (300);
		}
		linetabp = linetabp->next;
	}
}
