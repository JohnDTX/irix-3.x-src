/*
** copy.c	- Copyright (C) Silicon Graphics - Mountain View, CA 94043
**		- Author: chase
**		- Date: April 1985
**		- Any use, copy or alteration is strictly prohibited
**		- and gosh darn awfully -- morally inexcusable
**		- unless authorized by SGI.
**
** $Author: root $
** $State: Exp $
** $Source: /d2/3.7/src/stand/cmd/mdfex/RCS/copy.c,v $
** $Revision: 1.1 $
** $Date: 89/03/27 17:12:42 $
*/

#include <sys/types.h>
#include "disk.h"
#include "dsdreg.h"
#include <sys/dklabel.h>
#include "fex.h"

struct	descrip {
	long	d_lun,
		d_lba;
	struct dtypes *d_type;
	struct drive  *d_drvp;
};

getd(p)
register struct descrip *p;
{
	p->d_lun = dunit;
	p->d_type = dtype;
	p->d_drvp = drivep;
}

setd(p)
register struct descrip *p;
{
	dunit = p->d_lun;
	drivep = p->d_drvp;
	dtype = p->d_type;
}

cpyinfo(p)
struct descrip *p;
{
	register char *cp;

	unitd();
	p->d_lun = dunit;
	if(!isinited[dunit])
		(void) initl();
	p->d_type = dtype;
	p->d_drvp = drivep;

	do {
		printf("Disk address (%s)(", emodes[emode]);
		pb1((long)0, 1, 0);
		printf(")? ");
		if(*(cp = getline()) == 0)
			p->d_lba = 0;
		else {
			p->d_lba = bparse(cp, 1);
			if(p->d_lba == -1) printf("\n");
		}
	} while(p->d_lba == -1);
}

copy()
{
	register nx, ns, vfy, xcnt;
	register i;
	struct descrip fromx, tox, saved;

	printf("Copy Data\n");
	getd(&saved);
	printf("From:\n");
	cpyinfo(&fromx);
	if(drivep->tdev == D_WIN) {
		if(dunit) dunit = 0;
		else dunit = 1;
		drivep = &drives[dunit];
	}
	printf("\nTo:\n");
	cpyinfo(&tox);
oncemore:
	ns = SPC;
	printf("\n # of sectors per transfer (chunk)(%d): ", ns);
	i = getnum(10, 1, (int)(BUFSIZE*2/secsize));
	if(i == -1)
		i = ns;
	ns = i;
	nx = ((SPC*CYL) - fromx.d_lba)/ns;
	printf("\n # of chunks to transfer (%d): ", nx);
	i = getnum(10, 1, nx);
	if(i == -1)
		i = nx;
	nx = i;
	for(;;) {
		printf("\nVerify? ");
		i = gch();
		if(i == 'y') {
			vfy = 1; printf("Yes"); break;
		} else if(i == 'n') {
			vfy = 0; printf("No"); break;
		} else printf("Answer y/n please");
	}
	printf("\n");
	if(vfy && ns*secsize > BUFSIZE) {
	    printf("Sorry, not enough memory, use smaller chunk or no verify");
	    goto oncemore;
	}
	setd(&fromx);
	if(!isinited[dunit]) {
		if(initl()) return;
	} else if(config()) return;
	setd(&tox);
	if(!isinited[dunit]) {
		if(initl()) return;
	} else if(config()) return;
	xcnt = 0;
	while(xcnt++ < nx) {
	rd:	setd(&fromx);
		if(rdata(ns, (int)fromx.d_lba, (char *)BUF0)) {
			printf("Read Err @"); pb1(fromx.d_lba, 1, 0);
			printf("\n");
			switch(rsq(0)) {
			case RETRY:	goto rd;
			case QUIT:	goto out;
			}
		}
		setd(&tox);
	wt:	if(wdata(ns, (int)tox.d_lba, (char *)BUF0)) {
			printf("Write Err @"); pb1(tox.d_lba, 1, 0);
			printf("\n");
			switch(rsq(0)) {
			case RETRY:	goto wt;
			case QUIT:	goto out;
			}
		}
		if(vfy) {
			register ULONG *old = (ULONG *)BUF0;
			register ULONG *new = (ULONG *)BUF1;

		xx:	if(rdata(ns, (int)tox.d_lba, (char *)BUF1)) {
				printf("ReRead Err @");
				pb1(tox.d_lba, 1, 0); printf("\n");
				switch(rsq(0)) {
				case RETRY:	goto xx;
				case SKIP:	goto skp;
				}
				goto out;
			}
			for(i = ns*64; i--; )
				if(*old++ != *new++) {
					--old; --new;
					printf("VFY ERR ->");
					pb1(tox.d_lba, 1, 0);
					printf(" offset %x", ((int)old)-BUF0);
					printf(" %x=>%x\n", *old, *new);
					switch(rsq(0)) {
					case RETRY: goto rd;
					case SKIP:  goto skp;
					}
					goto out;
				}
		skp: ;
		}
		fromx.d_lba += ns; tox.d_lba += ns;
		if((i = nwgch()) != -1)
			if(i == ' ') {
				printf("\n");
				pb1(fromx.d_lba, 1, 0);
				printf("->");
				pb1(tox.d_lba, 1, 0);
				printf("\n");
			} else
				goto out;
		if((xcnt % 10) == 0) QP0(".");
	}
	printf("Copy complete!\n");
out:
	setd(&fromx);			/* Be nice... home both drives */
	(void) rdata(1, (int)0, (char *)BUF0);
	setd(&tox);
	(void) rdata(1, (int)0, (char *)BUF0);
	qupdate();			/* Target probably zapped */
	setd(&saved);
}
