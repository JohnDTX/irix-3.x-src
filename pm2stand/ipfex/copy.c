/*
** $Source: /d2/3.7/src/pm2stand/ipfex/RCS/copy.c,v $
** $Date: 89/03/27 17:11:12 $
** $Revision: 1.1 $
*/

#include <sys/types.h>
#include "disk.h"
#include "ipreg.h"
#include "dsdreg.h"
#include <sys/dklabel.h>
#include "test.h"

struct	descrip {
	ULONG	d_lun,
		d_lba;
	struct dtypes *d_type;
};

getd(p)
register struct descrip *p;
{
	p->d_lun = dunit;
	p->d_type = dtype;
}

setd(p)
register struct descrip *p;
{
	dunit = p->d_lun;
	drivep = &drives[dunit];
	dtype = p->d_type;
}

cpyinfo(p)
struct descrip *p;
{
	register char *cp;

	unitd();
	p->d_lun = dunit;
	if(!isinited[dunit])
		initl();
	p->d_type = dtype;

	do {
		printf("Disk address (%s)(", emodes[emode]);
		pb1(0, 1, 0);
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
	register char *cp;
	struct descrip fromx, tox, saved;

	printf("Copy Data\n");
	getd(&saved);
	printf("From:\n");
	cpyinfo(&fromx);
	if(dunit) dunit = 0;
	else dunit = 1;
	drivep = &drives[dunit];
	printf("\nTo:\n");
	cpyinfo(&tox);
oncemore:
	ns = SPC;
	printf("\n # of sectors per transfer (chunk)(%d): ", ns);
	i = getnum(10, 1, BUFSIZE*2/secsize);
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
	} else if(ipinit()) return;
	setd(&tox);
	if(!isinited[dunit]) {
		if(initl()) return;
	} else if(ipinit()) return;
	xcnt = 0;
	while(xcnt++ < nx) {
	rd:	setd(&fromx);
		if(rdata(ns, fromx.d_lba, BUF0)) {
			printf("Read Err @"); pb1(fromx.d_lba, 1, 0);
			printf("\n");
			switch(rsq(0)) {
			case RETRY:	goto rd;
			case QUIT:	goto out;
			}
		}
		setd(&tox);
	wt:	if(wdata(ns, tox.d_lba, BUF0)) {
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

		xx:	if(rdata(ns, tox.d_lba, BUF1)) {
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
	rdata(1, (long)0, BUF0);
	setd(&tox);
	rdata(1, (long)0, BUF0);
	qupdate();			/* Target probably zapped */
	setd(&saved);
}
