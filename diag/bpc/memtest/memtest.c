#include "pmII.h"
#include "pmIImacros.h"
#include "mem.h"

#define	VERS	"V.Prom.1.1"
#define	DATE	"August 8, 1984"

typedef unsigned short ushort;
typedef unsigned char uchar;

#define	PATa	0xF0F0
#define	PATb	0x9595
#define	PATh	0x1234
#define	PATm	0x5555

char memfound[32];		/* One byte per half meg */
int memterrs;
long bad_be[32];		/* Summary of bus-errors */
				/* Flags */
char verbose;

#define	NPATS	14
char	patf[NPATS];
ushort	pats[NPATS];
ushort	*patad[NPATS];

jmp_buf cmd_buf;
jmp_buf	bejmp;			/* Jmp buf for catching bus errors */

extern unsigned short mbw_page;
unsigned long mbw;
short ppgoffset,mbwpgoffset;
int getedata(),getend();

#define	ledon(x) (STATUS_REG &= ~x)
#define	ledoff(x) (STATUS_REG |= x)
#ifdef COMMENT
char	*patnm[] = {
	"??",
	"Pattern1",		/* 1: a */
	"Pattern2",		/* 2: b */
	"decrementing",		/* 3: c */
	"incrementing",		/* 4: d */
	"long a->(a)",		/* 5: e */
	"marching 1's",		/* 6: f */
	"marching 0's",		/* 7: g */
	"wrt/rd",		/* 8: h */
	"xxx",
	"xxx",
	"random",		/* 11: k */
	"decrementing byte",	/* 12: l */
	"write/read/read",	/* 13: m */
};
#endif

memtest(v)
char v;
{
	/* KBA - added argument v to pass verbose info */
	int toterrs;			/* Total errors to return upstairs */
	register i, p, j, z = 0;
	register unsigned short *a;
	extern _edata, _end;

	verbose = v;
	if(verbose) printf("Memory Test PM2.1 %s, %s\n", VERS, DATE);
	mbw_page = 0x80;
	if(verbose)
		printf("Memory: ");
	for(i = 0; i < 31; i++)
		memfound[i] = findmem(i);
	if(verbose)
		for(i = 0; i < 31; i++)
			printf("%c", memfound[i]? 'X' : '.');
	if(verbose)
		printf("\n");
/*	STATUS_REG |= PAR_EN;		/***/
	STATUS_REG |= 0xF;		/* Set to 0 */
	STATUS_REG &= ~PAR_EN;		/* Turn parity off */

	/* ppgoffset is the physical page offset necessary to avoid 
	   overwriting ourselves */
	ppgoffset = (i=((long)&_edata)>=(int)ROM0)?
		(atop(i-1) + 1):(atop(((long)&_end)-1) + 1);

	/* if the multibus window is at zero, dont generate addresses
	   that will overwrite us !! */
	if (!mbw_page) mbwpgoffset = ppgoffset; else mbwpgoffset=0;
	mbw_page += mbwpgoffset;
	mbw = ptoa(mbw_page);
#ifdef DEBUG
	printf("multibus accessible memory starts at page %x\n",mbw_page);
#endif
	for(i = 1; i < NPATS; i++)
		patf[i] = 1;
	pats[1] = PATa;
	pats[2] = PATb;
	pats[8] = PATh;
	pats[13] = PATm;

	toterrs = 0;
	if(setjmp(bejmp)) {
		printf("BUSERR: Exception reg: %x\n", EXCEPTION);
		pberr();
		toterrs++;
		return toterrs;
	}
	if(setjmp(cmd_buf) == 1) {	/* Someone typed a <return> */
		printf("\n");
		return toterrs;
	}
	for(i = 0; i < 31; i++)
		if(memfound[i]) {
		    if(setjmp(bejmp) == 0) {
			if (verbose)
			    printf("Test %06x-%06x ", ptoa(i*HALF),
					ptoa((i+1)*HALF) - 1);
			map(i*HALF, 0);
			check(i*HALF);
			if (verbose)
			    printf("\n");
		    } else {
			pberr();
			memterrs++;
			bad_be[i]++;
		    }
		}
	toterrs += memterrs;
	for(i = 0; i < 31; i++)
		if(memfound[i]) {
		    if(setjmp(bejmp) == 0) {
			if (verbose)
			    printf("Multibus test %06x-%06x ", ptoa(i*HALF),
					ptoa((i+1)*HALF) - 1);
			map(i*HALF, 1);
			check(i*HALF);
			if (verbose)
			    printf("\n");
		    } else {
			pberr();
			memterrs++;
			bad_be[i]++;
		    }
		}
	toterrs += memterrs;
	if(chkkey()) {
	    if (verbose)
		printf("Full mem fill ");
	    memterrs = 0;
	    for(i = 0; i < 31; i++)
		if(memfound[i]) {
		    if(setjmp(bejmp) == 0) {
			if (verbose)
			    printf("X");
			map(i*HALF, 0);
			fillp(i*HALF, i*HALF);
		    } else {
			pberr();
			memterrs++;
			bad_be[i]++;
		    }
		}
	    if (verbose)
		printf(", Test ");
	    for(i = 0; i < 31; i++)
		if(memfound[i]) {
		    if(setjmp(bejmp) == 0) {
			map(i*HALF, 0);
			testp(i*HALF, i*HALF);
			if (verbose)
			    printf("!");
		    } else {
			pberr();
			memterrs++;
			bad_be[i]++;
		    }
		}
	    perrs();
	    if (verbose)
		printf("\n");
	}
	toterrs += memterrs;
	if(chkkey()) {
	    if (verbose)
		printf("Full mem fill via Multibus ");
	    memterrs = 0;
	    for(i = 0; i < 31; i++)
		if(memfound[i]) {
		    if(setjmp(bejmp) == 0) {
			if (verbose)
			    printf("X");
			map(i*HALF, 1);
			fillp(i*HALF, i*HALF);
		    } else {
			pberr();
			memterrs++;
			bad_be[i]++;
		    }
		}
	    if (verbose)
		printf(", Test ");
	    for(i = 0; i < 31; i++)
		if(memfound[i]) {
		    if(setjmp(bejmp) == 0) {
			map(i*HALF, 1);
			testp(i*HALF, i*HALF);
			if (verbose)
			    printf("!");
		    } else {
			pberr();
			memterrs++;
			bad_be[i]++;
		    }
		}
	    perrs();
	    if (verbose)
		printf("\n");
	}
	toterrs += memterrs;
	if(chkkey()) {
	    if (verbose)
		printf("Cross test direct/multibus ");
	    memterrs = 0;
	    for(i = 0; i < 31; i++)
		if(memfound[i]) {
		    if(setjmp(bejmp) == 0) {
			if (verbose)
			    putchar('<');
			map(i*HALF, 0);
			fillp(i*HALF, ~(i*HALF));
			map(i*HALF, 1);
			testp(i*HALF, ~(i*HALF));
			chkkey();
			if (verbose)
			    putchar('!');
		    } else {
			pberr();
			memterrs++;
			bad_be[i]++;
		    }
		}
	    for(i = 0; i < 31; i++)
		if(memfound[i]) {
		    if(setjmp(bejmp) == 0) {
			if (verbose)
			    putchar('>');
			map(i*HALF, 1);
			fillp(i*HALF, ~(i*HALF));
			map(i*HALF, 0);
			testp(i*HALF, ~(i*HALF));
			chkkey();
			if (verbose)
			    putchar('!');
		    } else {
			pberr();
			memterrs++;
			bad_be[i]++;
		    }
		}
	    perrs();
	    if (verbose)
		printf("\n");
	}
	toterrs += memterrs;
	return toterrs;
}

map(firstpg, multibus_mem)
{
	/* map virtual addresses from page mbw_page to page mbw_page+HALF
	   to physical addresses firstpg to firstpg + HALF. If 
	   multibus_mem!=0, the memory is to be multibus memory.
	*/

	register curpg;
	register short 	*mapptr = &PAGEMAP[mbw_page], 
			*protptr = &PROTMAP[mbw_page];
	register prt;
	short endpg,startpg;

	/* if firstpg indicates that we are mapping into the physical memory
	   at zero (used by the current program), dont remap our program
	   space !! */
	startpg=(firstpg==0)?ppgoffset:firstpg;
	endpg = firstpg+HALF;
	if(multibus_mem) {
		/* protection is multibus mem with rwx prot*/
/*		prt = PROT_MBMEM|PROT_RWX___;
		startpg_thismeg = firstpg & 0xFF;
		*protptr = prt;
		*mapptr = HALF + HALF;
		*(short *)VAD = firstpg >> 8; 
*/
		/* enable multibus map write access */
		STATUS_REG &= ~(EN0|EN1);	
		
		/* set up the protection */
		prt = PROT_MBMEM|PROT_RWX___;

		/* now set the multibus memory to be visible from 
		   off-board */

		/* if there is only one half meg of memory (i.e., the
		   on-board memory), dont remap the first few pages (stack
		   and our program) */
		for (curpg = 0; curpg<HALF; curpg++) {
		    *mapptr++ = (short) (curpg + mbw_page);
		    *protptr++ = (short) prt;
		}
		/* and do the same thing for the pagemap */
		mapptr = &PAGEMAP[mbw_page+ONEMEG];
		protptr = &PROTMAP[mbw_page+ONEMEG];
		for (curpg=ONEMEG; curpg <(ONEMEG + HALF); curpg++) {
		    *mapptr++ = (short) (mbw_page+curpg);
		    *protptr++ = (short) prt;
		} 

		/* now set the multibus pagemap */
		mapptr = (short *) ptoa(mbw_page + ONEMEG);
		for (curpg=0; curpg < HALF; curpg++) {
		    *mapptr = startpg++;
		    mapptr += ptoa(1)/2;
		}

		/* reset the ability to write the multibus pagemap */
/*		STATUS_REG |= EN0|EN1;*/


	} else {
		prt = PROT_LOCMEM|PROT_RWX___;
		for(curpg = startpg; curpg < endpg; curpg++) {
		    *mapptr++ = startpg++ ;
		    *protptr++ = prt;
		}
	}
}

check(p)
{
	/* check the half meg of memory starting at page p.
	*/
	register i, si;
	register ushort pat, pr;
	register ushort *a;
	register long *al, lpr;
	register ushort *sa;
	register uchar *ac;
	long x,y;

	i = p * 4096;			/* Address of this half meg */
	lpr = (long)(ptoa(mbw_page));	/* Virtual equivalent */
	if(p == 0)			/* Set length */
		si = HALF*4096 - ptoa(ppgoffset);
	else
/*		si = HALF*4096 - ptoa(mbwpgoffset);		/***/
		si = HALF*4096;
	si /= 2;			/* Adjust for shorts */
	sa = (ushort *)lpr;
	memterrs = 0;
#ifdef DEBUG
	printf("check: %x words starting at %x:",(unsigned long)si,
	  	(unsigned long)sa);
#endif
    if(patf[1]) {
	if (verbose)
	    putchar('a');
	ledoff(1);
	fillw(sa, pats[1], si);
	cmpw(sa, pats[1], si, p);
	if(chkkey() == 0) return;
	perrs();
	memterrs = 0;
    }
    if(patf[2]) {
	if (verbose)
	    putchar('b');
	ledoff(1);
	fillw(sa, pats[2], si);
	cmpw(sa, pats[2], si, p);
	if(chkkey() == 0) return;
	perrs();
	memterrs = 0;
    }
    if(patf[3]) {
	if (verbose)
	    putchar('c');
	ledoff(1);
	for(a = sa, i = si; i; i--, a++)
		*a = i;
	for(a = sa, i = si; i; i--, a++)
		if((pr = *a) != (ushort)i)
			perr((long)a, p, (ushort)i, pr);
	if(chkkey() == 0) return;
	perrs();
	memterrs = 0;
    }
    if(patf[4]) {
	if (verbose)
	    putchar('d');
	ledoff(1);
	for(a = sa, i = si; i; i--, a++)
		*a = ~i;
	for(a = sa, i = si; i; i--, a++)
		if((pr = *a) != (ushort)~i)
			perr((long)a, p, (ushort)~i, pr);
	if(chkkey() == 0) return;
	perrs();
	memterrs = 0;
    }
    if(patf[5]) {
	if (verbose)
	    putchar('e');
	ledoff(1);
	for(al = (long *)sa, i = si/2; i; i--, al++)
		*al = (long)al ^ ((long)al<<14);
	for(al = (long *)sa, i = si/2; i; i--, al++)
		if((lpr = *al) != ((long)al ^ ((long)al<<14)))
			perr((long)al, p, (long)al ^ ((long)al<<14), lpr);
	if(chkkey() == 0) return;
	perrs();
    }
    if(patf[6]) {
	if (verbose)
	    putchar('f');
	ledoff(1);
	for(pat = 1; pat; pat <<= 1) {
/*		if(pat != 1) putchar('\b');	/***/
		memterrs = 0;
		fillw(sa, pat, si);
		cmpw(sa, pat, si, p);
		if(chkkey() == 0) return;
		perrs();
	}
    }
    if(patf[7]) {
	if (verbose)
	    putchar('g');
	ledoff(1);
	for(pat = 1; pat; pat <<= 1) {
/*		if(pat != 1) putchar('\b');	/***/
		memterrs = 0;
		fillw(sa, ~pat, si);
		cmpw(sa, ~pat, si, p);
		if(chkkey() == 0) return;
		perrs();
	}
    }
    if(patf[8]) {
	if (verbose)
	    putchar('h');
	ledoff(1);
	pat = pats[8];
	for(a = sa, i = si; i; i--, a++) {
		*a = pat;
		if(*a != pat) {
			ledon(1);
			perr((long)a, p, pat, *a);
		}
	}
	if(chkkey() == 0) return;
	perrs();
	memterrs = 0;
    }
#define	RANDMULT	5
#define RANDOFFSET	17623
#define RANDOMIZE(r)	r = (r * RANDMULT + RANDOFFSET) & 0xffff
#define SEED		433
    if(patf[11]) {		/* Random data */
	if (verbose)
	    putchar('k');
	ledoff(1);
	pat = SEED;
	for(a = sa, i = si; i; i--, a++) {
		RANDOMIZE(pat);
		*a = pat;
	}
	pat = SEED;
	for(a = sa, i = si; i; i--, a++) {
		RANDOMIZE(pat);
		if(*a != pat)
			perr((long)a, p, pat, *a);
	}
	if(chkkey() == 0) return;
	perrs();
	memterrs = 0;
    }
    if(patf[12]) {
	if (verbose)
	    putchar('l');
	ledoff(1);
	for(ac = (uchar *)sa, i = si<<1; i; i--, ac++)
		*ac = i;
	for(ac = (uchar *)sa, i = si<<1; i; i--, ac++)
		if((pr = *ac) != (uchar)i)
			perr((long)ac, p, (uchar)~i, pr);
	if(chkkey() == 0) return;
	perrs();
	memterrs = 0;
    }
    if(patf[13]) {
	if (verbose)
	    putchar('m');
	ledoff(1);
	pat = pats[8];
	for(a = sa, i = si; i; i--, a++) {
		*a = pat;
		if(*a != pat || *a != pat) {
			ledon(1);
			perr((long)a, p, pat, *a);
		}
	}
	if(chkkey() == 0) return;
	perrs();
	memterrs = 0;
    }
}

long xorp;
struct	{
	long	e_a;
	long	e_x;
} etrc[16*8];
int etrcp;

perr(a, p, pat, is)
long a;
{
	register e = memterrs, x = pat ^ is;

	ledon(1);
	if(e == 0) {
		xorp = 0;
		etrcp = 0;
	}
	x = ((x >> 16) | x) & 0xFFFF;
	if(etrcp == 0 || (etrc[etrcp-1].e_x & x) != x)
		if (verbose)
		    printf("\n  Mem err @ %06x: should be %08x, is %08x, xor %08x",
			a - mbw + ptoa(p), pat, is, x);
	if(etrcp == 0 || (etrc[etrcp-1].e_a & 0x60000) != (a & 0x60000)) {
		etrc[etrcp].e_a = a - mbw + ptoa(p);
		etrc[etrcp++].e_x = x;
	} else
		etrc[etrcp-1].e_x |= x;
	memterrs++;
	xorp |= x;
}

long	bad_pm[8][8];
long	bad_mx[16][18][8];

perrs()
{
	register i, b, e, bit;
	int h, bank, onboard;

	if(memterrs) {
	    if (verbose)
		printf("\n>>>0x%x Errors, %x - %x, XOR=%x\n>>> ", memterrs,
		etrc[0].e_a, etrc[etrcp-1].e_a, xorp);
	    h = etrc[0].e_a >> 19;
	    if(h == 30 || ((h & 1) == 0 && memfound[h+1] == 0)) {
		onboard = 1;
		if (verbose)
		    printf("PM ");
	    } else {
		onboard = 0;
		h >>= 1;
		if (verbose)
		    printf("MX(%x) ", h);
	    }
	    for(e = 0; e < etrcp; e++) {
		for(i = 0, b = 1; i < 16; b <<= 1, i++)
		    if(etrc[e].e_x & b) {
			if(onboard) {
				bank = ((etrc[e].e_a >> 17) & 3) << 1;
				bit = i;
				if(bit > 7) {
					bit -= 8;
					bank++;
				}
				if (verbose)
				 printf("U%d%c ", bit + 16, "ABCDEFGH"[bank]);
				bad_pm[bit][bank]++;
			} else {
				bank = ((((etrc[e].e_a >> 10) & 3) << 1) |
					((etrc[e].e_a >> 19) & 1));
				if((bit = i+1) > 8) bit++;
				if (verbose)
				    printf("U%d%c ", bit, "ABCDEFGH"[bank]);
				bad_mx[h][bit-1][bank]++;
			}
		    }
	    }
	    if (verbose)
		printf("<<<\n+++");
	} else
		if (verbose)
		    printf(".");
}

fillp(h, pat)
register pat;
{
	register long *al = (long *)mbw;
	register short i = (HALF*4096/4);

	if(h == 0)
		i  -= (ptoa(ppgoffset)/4);
	--i;
	do {
		*al++ = pat++;
	} while(--i != -1);
}

testp(h, pat)
register pat;
{
	register long *al = (long *)mbw;
	register short i = (HALF*4096/4);

	if(h == 0)
		i -= (ptoa(ppgoffset)/4);
	--i;
	do {
		if(*al != pat)
			perr((long)al, h, pat, *al);
		al++; pat++;
	} while(--i != -1);
}

pberr()
{
	printf(" Bus Error: ");
	if((EXCEPTION_REG & FAULT_PRESENT) == 0)
		printf("Not Present ");
	if((EXCEPTION_REG & FAULT_MAP) == 0)
		printf("Map Fault ");
	if((EXCEPTION_REG & FAULT_TIMEOUT) == 0)
		printf("Timeout ");
	if((EXCEPTION_REG & FAULT_PARITY) == 0)
		printf("Parity Fault ");
	printf("\n");
}

chkkey()
{
	register c, r = -1;

	if((c = nwgetchar())== -1)
		return r;
	switch(c) {
	case 'v':	verbose = 1;	break;
	case 'q':	verbose = 0;	break;
	case ' ':	r = 0;		break;
	case '\177':
	case '\n':	longjmp(cmd_buf, 1);
	default:	putchar('\007'); break;
	}
	return r;
}

unsigned short find_mb_window()
{	
	/* for now, just assume the window is set at 0M */
	return(atop(0x0));
}

findmem(h)
{
	register ushort *sp;

	if(setjmp(bejmp)) {
/*		printf("Bus error findmem: %d 1/2 meg!\n", h);	/***/
		return 0;			/* Huh? */
	}
	map(h*HALF, 0);				/* Map the chunk directly */
	sp = (ushort *)(ptoa(mbw_page));	/* Virtual equivalent */
	*sp++ = 0xf0f0;
	*sp++ = 0x0f0f;
	*sp++ = 0xa5a5;
	*sp++ = 0x5a5a;
	sp = (ushort *)(ptoa(mbw_page));	/* Virtual equivalent */
	if(*sp++ != 0xf0f0) return 0;
	if(*sp++ != 0x0f0f) return 0;
	if(*sp++ != 0xa5a5) return 0;
	if(*sp++ != 0x5a5a) return 0;
	return 1;
}
