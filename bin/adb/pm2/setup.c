#include "defs.h"
/*
 * setup routines for a.out and core files
 */
#include <pmII/vmparam.h>
#include <pmII/cpureg.h>

#ifndef UDOT_VBASE
#define UDOT_VBASE UDOTBASE
#endif

extern struct var v;
MSG BADNAM;
MSG BADMAG;
struct map txtmap;
struct map datmap;
int wtflag;
int fcor;
char *corhdr;
int regoff;
int fsym;
long maxfile;
long txtsiz;
long datsiz;
long datbas;
long stksiz;
char *errflg;
long magic;
long symbas;
long entrypt;
int signum = 0;
char *symfil = "a.out";
char *corfil = "core";
BHDR bhdr;

#undef ctob
ctob(c)
{
	return(c<<PGSHIFT);
}

setbout() /* setup a.out file */
{
	short sw = 0;

	corhdr = (char *)malloc(ctob(UPAGES));
	fsym = open(symfil, wtflag);
	if ((fsym < 0) && wtflag)
		if ((fsym = creat(symfil, 0644)) >= 0)
			sw++;
	printf("a.out file = %s", symfil);
	if (sw)
		printf(" (created)");
	if (fsym < 0)
		printf(" - cannot open or create");

	txtmap.ufd = fsym;
	if (read(fsym, &bhdr, sizeof(bhdr)) == sizeof(bhdr)) {
		magic = bhdr.a_magic;
		if (N_BADMAG(bhdr)) {
			magic = 0;
			printf(" - warning: not in a.out format");
		} else {
			txtsiz = bhdr.a_text;
			datsiz = bhdr.a_data;
			entrypt = bhdr.a_entry;
			symbas = txtsiz + datsiz;
			txtmap.b1 = entrypt;
			txtmap.e1 = txtmap.b1+(magic==OMAGIC ? symbas : txtsiz);
			txtmap.f1 = N_TXTOFF(bhdr);
			if (magic != OMAGIC)
				datbas = (txtmap.e1 + NBPG - 1) & (-NBPG);
			else
				datbas = txtmap.b1;
			txtmap.b2 = datbas;
			txtmap.e2 = datbas + (magic==OMAGIC ? symbas : datsiz);
			txtmap.f2 = N_TXTOFF(bhdr) +
			    (magic==OMAGIC ? 0 : txtsiz);
#ifdef	sgi
			{
				extern int kflag;

				/*
				 * If kflag is set, then fix up maps based on
				 * processor type kernel is for.
				 */
				if (kflag) {
					if (entrypt == 0x400) {
						/*
						 * pmII kernel
						 */
						txtmap.b1 += 0xC00000;
						txtmap.e1 += 0xC00000;
						txtmap.b2 = txtmap.b1;
						txtmap.e2 = txtmap.e1;
						txtmap.f2 = txtmap.f1;
					} else
					if (entrypt == 0x20000000) {
						/*
						 * ipII kernel
						 */
						txtmap.b1 -= 0x400;
						txtmap.e1 -= 0x400;
						txtmap.b2 = txtmap.b1;
						txtmap.e2 = txtmap.e1;
						txtmap.f2 = txtmap.f1;
					} else
						printf(" - not a kernel a.out");
				}
			}
#endif
			symbas = N_SYMOFF(bhdr);

			/* read in symbol table from a.out file */
			setupsym(bhdr.a_syms);
			symset();
		}
	}
	if (magic == 0)
		txtmap.e1 = maxfile;
	printf("\n");
}

/* open core file */
setcor()
{
	register struct user *usp;

	if (*corfil == '-') {
		fcor = -1;
		return;
	} else
		if ((fcor = open(corfil, wtflag)) == -1)
			printf("cannot open %s", corfil);
		else
			printf("core file = %s", corfil);
	datmap.ufd = fcor;
	signum = 0;
	if (read(fcor, corhdr, ctob(UPAGES)) == ctob(UPAGES)) {
		usp = (struct user *)corhdr;
		txtsiz = ctob(usp->u_tsize);
		datsiz = ctob(usp->u_dsize);
		stksiz = ctob(usp->u_ssize);
		regoff = ((int)usp->u_ar0 - UDOT_VBASE);
		datbas = USRTEXT + txtsiz;
		if (magic==0410)
			datbas = (datbas + NBPG -1) & (-NBPG);
		datmap.b1 = datbas;
		datmap.e1 = datmap.b1 + datsiz;
		datmap.f1 = ctob(UPAGES) + (magic==0410 ? 0 : txtsiz);
		datmap.b2 = USRSTACK - stksiz;
		datmap.e2 = USRSTACK;
		datmap.f2 = ctob(UPAGES) + datsiz + (magic==0410 ? 0 : txtsiz);
		/* GB - added structure member */
		if ((magic == 0) || (magic != usp->u_exdata.ux_mag)) {
			datmap.b1 = 0;
			datmap.e1 = maxfile;
			datmap.f1 = 0;
			printf(" - warning: bad magic number");
		} else
			readregs(fcor);
	} else
		datmap.e1 = maxfile;
	printf("\n");
}

create(f)
char *f;
{
	int fd;

	if ((fd = creat(f, 0644)) >= 0) {
		close(fd);
		return(open(f, wtflag));
	} else
		return(-1);
}
