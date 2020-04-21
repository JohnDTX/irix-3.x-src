#include "pmII.h"
#include "pmIImacros.h"
#include "mem.h"

#define	VERS	"V1.3"
#define	DATE	"August 6, 1984"

typedef unsigned short ushort;
typedef unsigned char uchar;

#define	PATa	0xF0F0
#define	PATb	0x9595
#define	PATh	0x1234
#define	PATi	0x4321
#define	PATm	0x5555

#define	ERRLIM	5
#define	MIN(a,b) ((a<b)?a:b)
/* If half is not below or above testing range then process it */
#define	INRANGE(i) (!((i*HALF*4096+HALF*4096)<=lowad||i*HALF*4096>=highad))

int errno;

int errs;
int gpasses;
int bpasses;
long bad_be[32];		/* Summary of bus-errors */
int refreshrate = 15;
				/* Flags */
char summary;			/* Used during each loop */
char sum_each_pass = 1;		/* The rest are global */
char enable_parity = 0;
char crossf = 1;
char mmuf = 0;
char multimapf = 0;
char testlocal = 1;
char testmulti = 1;
char fullmem = 1;
char verbose = 1;

int lowestmem;
long lowad = 0, highad = 0x1000000;	/* Lowest/highest addresses to test */

/* NOTE: Add entries to fp() for new patterns */
char	*patnm[] = {
	"??",
	"Pattern1",		/* a */
	"Pattern2",		/* b */
	"decrementing",		/* c */
	"incrementing",		/* d */
	"long a->(a)",		/* e */
	"marching 1's",		/* f */
	"marching 0's",		/* g */
	"write/read",		/* h */
	"write",		/* i */
	"read",			/* j */
	"random",		/* k */
	"decrementing byte",	/* l */
	"write/read/read",	/* m */
};
#define	NPATS	(sizeof patnm/sizeof (char *))
char	patf[NPATS];
ushort	pats[NPATS];
ushort	*patad[NPATS];

char	*tf[] = { "False", "True" };

jmp_buf cmd_buf;
jmp_buf	bejmp;			/* Jmp buf for catching bus errors */

extern unsigned short mbw_page;
unsigned long mbw;
short ppgoffset,mbwpgoffset;
int getedata(),getend();

#define	ledon(x) (STATUS_REG &= ~x)
#define	ledoff(x) (STATUS_REG |= x)

main()
{
	register i, p, j, z = 0;
	register unsigned short *a;

	printf("\n\nMemory Test PM2.1 %s, %s\n", VERS, DATE);
	mbw_page = 0x80;
	memfound = (char *)0x200;	/* memory byte vector */
	printf("Memory: ");
	for(i = 0; i < 31; i++)
		printf("%c", memfound[i]? 'X' : '.');
	printf("\n");
/*	STATUS_REG |= PAR_EN;		/***/
	STATUS_REG |= 0xF;		/* Set to 0 */
	STATUS_REG &= ~PAR_EN;		/* Turn parity off */
	lowestmem = 0;

	/* ppgoffset is the physical page offset necessary to avoid 
	   overwriting ourselves */
	ppgoffset = (i=getedata()>=(int)ROM0)?
		(atop(i-1) + 1):(atop(getend()-1) + 1);

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
	patf[9] = patf[10] = 0;		/* Clr i,j */
	pats[1] = PATa;
	pats[2] = PATb;
	pats[8] = PATh;
	pats[9] = PATi;
	pats[13] = PATm;
	patad[9] = (ushort *)0x20000;
	patad[10] = (ushort *)0x20000;

	printf("Type 'h' for help.\n");
  for(;;) {
	if(setjmp(bejmp) != 0) {
		printf("BUSERR: Exception reg: %x\n", EXCEPTION);
		pberr();
		continue;		/* Loop back to redo setjmp */
	}
	if(setjmp(cmd_buf) == 1) {
		printf("\n");
		continue;
	}
	cmd();
    for(p = 0; ;p++) {
	if(testlocal)
	    for(i = lowestmem; i < 31; i++)
		if(memfound[i] && INRANGE(i)) {
		    if(setjmp(bejmp) == 0) {
			printf("Test %06x-%06x ", ptoa(i*HALF),
					ptoa((i+1)*HALF) - 1);
			map(i*HALF, 0);
			check(i*HALF);
			printf("\n");
		    } else {
			pberr();
			errs++;
			bad_be[i]++;
		    }
		}
	if(testmulti)
	    for(i = lowestmem; i < 31; i++)
		if(memfound[i] && INRANGE(i)) {
		    if(setjmp(bejmp) == 0) {
			printf("Multibus test %06x-%06x ", ptoa(i*HALF),
					ptoa((i+1)*HALF) - 1);
			map(i*HALF, 1);
			check(i*HALF);
			printf("\n");
		    } else {
			pberr();
			errs++;
			bad_be[i]++;
		    }
		}
	if(chkkey() && fullmem) {
	    printf("Full mem fill ");
	    for(i = lowestmem; i < 31; i++)
		if(memfound[i] && INRANGE(i)) {
		    if(setjmp(bejmp) == 0) {
			printf("X");
			map(i*HALF, 0);
			fillp(i*HALF, i*HALF);
		    } else {
			pberr();
			errs++;
			bad_be[i]++;
		    }
		}
	    errs = 0;
	    printf(", Test ");
	    for(i = lowestmem; i < 31; i++)
		if(memfound[i] && INRANGE(i)) {
		    if(setjmp(bejmp) == 0) {
			map(i*HALF, 0);
			testp(i*HALF, i*HALF);
			printf("!");
		    } else {
			pberr();
			errs++;
			bad_be[i]++;
		    }
		}
	    perrs();
	    printf("\n");
	    if(errs) bpasses++; else gpasses++;
	}
	if(chkkey() && testmulti && fullmem) {
	    printf("Full mem fill via Multibus ");
	    errs = 0;
	    for(i = lowestmem; i < 31; i++)
		if(memfound[i] && INRANGE(i)) {
		    if(setjmp(bejmp) == 0) {
			printf("X");
			map(i*HALF, 1);
			fillp(i*HALF, i*HALF);
		    } else {
			pberr();
			errs++;
			bad_be[i]++;
		    }
		}
	    printf(", Test ");
	    for(i = lowestmem; i < 31; i++)
		if(memfound[i] && INRANGE(i)) {
		    if(setjmp(bejmp) == 0) {
			map(i*HALF, 1);
			testp(i*HALF, i*HALF);
			printf("!");
		    } else {
			pberr();
			errs++;
			bad_be[i]++;
		    }
		}
	    perrs();
	    printf("\n");
	    if(errs) bpasses++; else gpasses++;
	}
	if(chkkey() & crossf) {
	    printf("Cross test direct/multibus ");
	    errs = 0;
	    for(i = lowestmem; i < 31; i++)
		if(memfound[i] && INRANGE(i)) {
		    if(setjmp(bejmp) == 0) {
			putchar('<');
			map(i*HALF, 0);
			fillp(i*HALF, ~(i*HALF));
			map(i*HALF, 1);
			testp(i*HALF, ~(i*HALF));
			chkkey();
			putchar('!');
		    } else {
			pberr();
			errs++;
			bad_be[i]++;
		    }
		}
	    for(i = lowestmem; i < 31; i++)
		if(memfound[i] && INRANGE(i)) {
		    if(setjmp(bejmp) == 0) {
			putchar('>');
			map(i*HALF, 1);
			fillp(i*HALF, ~(i*HALF));
			map(i*HALF, 0);
			testp(i*HALF, ~(i*HALF));
			chkkey();
			putchar('!');
		    } else {
			pberr();
			errs++;
			bad_be[i]++;
		    }
		}
	    perrs();
	    printf("\n");
	    if(errs) bpasses++; else gpasses++;
	}
	if((p % 10) == 9 || summary || sum_each_pass)
		printf("--%3d Good passes, %2d Bad passes\n", gpasses, bpasses);
	summary = 0;
    }
  }
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
	int errors = 0;
	long x,y;

	i = p * 4096;			/* Address of this half meg */
	lpr = (long)(ptoa(mbw_page));	/* Virtual equivalent */
	if(p == 0)			/* Set length */
		si = HALF*4096 - ptoa(ppgoffset);
	else
/*		si = HALF*4096 - ptoa(mbwpgoffset);		/***/
		si = HALF*4096;
	x = lpr; y = si;
	if(i < lowad) {			/* In range? */
		lpr += lowad - i;	/* Move up */
		si -= lowad - i;	/* Adjust count */
		i = lowad;		/* Adjust phys low */
	}
	if(si <= 0)
		return;			/* Not in this half at all? */
	if(i + si > highad) {		/* base+len too far? */
		si = highad - i;	/* Yes, trim back */
	}
	if(si <= 0) return;		/* Pathalogical case */
	if(x != lpr || y != si) printf("(%x/%x)", i, si);
	si /= 2;			/* Adjust for shorts */
	sa = (ushort *)lpr;
	errs = 0;
#ifdef DEBUG
	printf("check: %x words starting at %x:",(unsigned long)si,
	  	(unsigned long)sa);
#endif
    if(patf[1]) {
	putchar('a');
	ledoff(1);
	fillw(sa, pats[1], si);
	cmpw(sa, pats[1], si, p);
	if(chkkey() == 0) return;
	perrs();
	errors = errs;
	errs = 0;
    }
    if(patf[2]) {
	putchar('b');
	ledoff(1);
	fillw(sa, pats[2], si);
	cmpw(sa, pats[2], si, p);
	if(chkkey() == 0) return;
	perrs();
	errors += errs;
	errs = 0;
    }
    if(patf[3]) {
	putchar('c');
	ledoff(1);
	for(a = sa, i = si; i; i--, a++)
		*a = i;
	for(a = sa, i = si; i; i--, a++)
		if((pr = *a) != (ushort)i)
			perr((long)a, p, (ushort)i, pr);
	if(chkkey() == 0) return;
	perrs();
	errors += errs;
	errs = 0;
    }
    if(patf[4]) {
	putchar('d');
	ledoff(1);
	for(a = sa, i = si; i; i--, a++)
		*a = ~i;
	for(a = sa, i = si; i; i--, a++)
		if((pr = *a) != (ushort)~i)
			perr((long)a, p, (ushort)~i, pr);
	if(chkkey() == 0) return;
	perrs();
	errors += errs;
	errs = 0;
    }
    if(patf[5]) {
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
	putchar('f');
	ledoff(1);
	for(pat = 1; pat; pat <<= 1) {
/*		if(pat != 1) putchar('\b');	/***/
		errors += errs;
		errs = 0;
		fillw(sa, pat, si);
		cmpw(sa, pat, si, p);
		if(chkkey() == 0) return;
		perrs();
	}
    }
    if(patf[7]) {
	putchar('g');
	ledoff(1);
	for(pat = 1; pat; pat <<= 1) {
/*		if(pat != 1) putchar('\b');	/***/
		errors += errs;
		errs = 0;
		fillw(sa, ~pat, si);
		cmpw(sa, ~pat, si, p);
		if(chkkey() == 0) return;
		perrs();
	}
    }
    if(patf[8]) {
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
	errors += errs;
	errs = 0;
    }
    if(patf[9]) {
	putchar('i');
	ledoff(1);
	pat = pats[9];
	i = 60000;
	a = patad[9];
	do
		*a = pat;
	while(--i != -1);
	if(chkkey() == 0) return;
	perrs();
    }
    if(patf[10]) {
	putchar('j');
	ledoff(1);
	i = 60000;
	a = patad[10];
	do
		pat = *a;
	while(--i != -1);
	if(chkkey() == 0) return;
	perrs();
    }
#define	RANDMULT	5
#define RANDOFFSET	17623
#define RANDOMIZE(r)	r = (r * RANDMULT + RANDOFFSET) & 0xffff
#define SEED		433
    if(patf[11]) {		/* Random data */
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
	errors += errs;
	errs = 0;
    }
    if(patf[12]) {
	putchar('l');
	ledoff(1);
	for(ac = (uchar *)sa, i = si<<1; i; i--, ac++)
		*ac = i;
	for(ac = (uchar *)sa, i = si<<1; i; i--, ac++)
		if((pr = *ac) != (uchar)i)
			perr((long)ac, p, (uchar)~i, pr);
	if(chkkey() == 0) return;
	perrs();
	errors += errs;
	errs = 0;
    }
    if(patf[13]) {
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
	errors += errs;
	errs = 0;
    }
    if(errors) bpasses++;
    else gpasses++;
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
	register e = errs, x = pat ^ is;

	ledon(1);
	if(e == 0) {
		xorp = 0;
		etrcp = 0;
	}
	x = ((x >> 16) | x) & 0xFFFF;
#ifdef foo
	if(errs++ < ERRLIM)
		printf("\n  Mem err @ %06x: should be %x, is %x, xor %x",
			a - mbw + ptoa(p), pat, is, x);
#endif
	if(etrcp == 0 || (etrc[etrcp-1].e_x & x) != x)
		printf("\n  Mem err @ %06x: should be %08x, is %08x, xor %08x",
			a - mbw + ptoa(p), pat, is, x);
	if(etrcp == 0 || (etrc[etrcp-1].e_a & 0x60000) != (a & 0x60000)) {
		etrc[etrcp].e_a = a - mbw + ptoa(p);
		etrc[etrcp++].e_x = x;
	} else
		etrc[etrcp-1].e_x |= x;
	errs++;
	xorp |= x;
}

long	bad_pm[8][8];
long	bad_mx[16][18][8];

perrs()
{
	register i, b, e, bit;
	int h, bank, onboard;

	if(errs) {
	    printf("\n>>>0x%x Errors, %x - %x, XOR=%x\n>>> ", errs,
		etrc[0].e_a, etrc[etrcp-1].e_a, xorp);
	    h = etrc[0].e_a >> 19;
	    if(h == 30 || ((h & 1) == 0 && memfound[h+1] == 0)) {
		onboard = 1;
		printf("PM ");
	    } else {
		onboard = 0;
		h >>= 1;
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
				printf("U%d%c ", bit + 16, "ABCDEFGH"[bank]);
				bad_pm[bit][bank]++;
			} else {
				bank = ((((etrc[e].e_a >> 10) & 3) << 1) |
					((etrc[e].e_a >> 19) & 1));
				if((bit = i+1) > 8) bit++;
				printf("U%d%c ", bit, "ABCDEFGH"[bank]);
				bad_mx[h][bit-1][bank]++;
			}
		    }
	    }
	    printf("<<<\n+++");
	} else
		printf(".");
}

elist()
{
	register h, i, j, x;
	long sum;

	printf("Error list:\n");
	x = 0; sum = 0;
	for(i = 0; i < 31; i++)
	    if(bad_be[i]) {
		if(!x++)
		    printf("  Bus errors: 1/2 meg(#): ");
		printf("%x(%d) ", i, bad_be[i]);
	    }
	x = 0; sum = 0;
	for(i = 0; i < 8; i++)
	    for(j = 0; j < 8; j++)
		if(bad_pm[i][j]) {
		    if(!x++)
			printf("  PM ");
		    printf("U%d%c ", i+16, "ABCDEFGH"[j]);
		    sum += bad_pm[i][j];
		}
	if(x)
	    printf("(%d errors)\n", sum);
	for(h = 0; h < 16; h++) {
	    x = 0; sum = 0;
	    for(i = 0; i < 18; i++)
		for(j = 0; j < 8; j++)
		    if(bad_mx[h][i][j]) {
			if(!x++)
			    printf("  MX(%d) ", h);
			printf("U%d%c ", i+1, "ABCDEFGH"[j]);
			sum += bad_mx[h][i][j];
		    }
	    if(x)
		printf("(%d errors)\n", sum);
	}
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
	case 'v':	verbose++;	break;
	case 'q':	verbose = 0;	break;
	case 's':	summary++;	break;
	case ' ':	r = 0;		break;
	case '\177':
	case '\n':	longjmp(cmd_buf, 1);
	default:	putchar('\007'); break;
	}
	return r;
}

cmd()
{
	for(;;) {
		printf("MT %s> ", VERS);
		switch(negetchar()) {
		case 'C':	clr(); break;
		case 'c':	crosstest(); break;
		case 'd':	dm(); break;
		case 'E':	elist(); break;
		case 'e':	ep(); break;
		case 'f':	ff(); break;
		default:
		case 'h':	help(); break;
		case 'l':	lm(); break;
		case 'M':	memsel(); break;
		case 'm':	fm(); break;
		case 'p':	fp(); break;
		case 'q':	quit(); break;
		case 'R':	dorefresh(); break;
		case 'r':	printf("Run\n"); return;
		case 's': 	sp(); break;
		case 'v':	vb(); break;
/*		case 'x':	mmutest(); break;	/***/
/*		case 'X':	multimaptest(); break;	/***/
		}
	}
}

help()
{
	printf("Help; commands are:\n");
	printf("  Clear all patterns/tests\n");
	printf("  cross test direct/multibus (%s)\n", tf[crossf]);
	printf("  direct memory tests (%s)\n", tf[testlocal]);
	printf("  Error list\n");
	printf("  enable Parity (%s)\n", tf[enable_parity]);
	printf("  fullmem test (%s)\n", tf[fullmem]);
	printf("  help\n");
	printf("  low mem to test\n");
	printf("  Memory select\n");
	printf("  multibus tests (%s)\n", tf[testmulti]);
	printf("  pattern select\n");
	printf("  quit\n");
	printf("  Refresh rate\n");
	printf("  run\n");
	printf("  summary each pass (%s)\n", tf[sum_each_pass]);
	printf("  verbose (%s)\n", tf[verbose]);
/*	printf("  x-mmu test (%s)\n", tf[mmuf]);		/***/
/*	printf("  X-multibus map test (%s)\n", tf[multimapf]);	/***/
}

quit()
{
	printf("Quit--confirm with 'y':");
	if(negetchar() == 'y') {
		printf("Bye\n");
		exit(1);
	}
	printf("\n");
}

clr()
{
	register i;

	printf("Clear all patterns\n");
	for(i = 1; i < NPATS; i++)
		patf[i] = 0;
	sum_each_pass = 0;
	enable_parity = 0;
	crossf = 0;
	mmuf = 0;
	multimapf = 0;
	testlocal = 0;
	testmulti = 0;
	fullmem = 0;
	verbose = 0;
}

ask(s, v)
char *s, *v;
{
	printf("%s (%s)? ", s, tf[*v]);
	switch(negetchar()) {
	case 't':	printf("True"); *v=1; break;
	case 'f':	printf("False"); *v=0; break;
	}
	printf("\n");
}

ep()
{
	ask("Enable parity", &enable_parity);
	if(enable_parity)
		STATUS_REG |= PAR_EN;
}

sp()
{
	ask("Summary each pass", &sum_each_pass);
}

crosstest()
{
	ask("Cross test direct/multibus", &crossf);
}

#ifdef SOON
mmutest()
{
	ask("Test mmu", &mmuf);
}

multimaptest()
{
	ask("Test multibus map", &multimapf);
}
#endif SOON

vb()
{
	ask("Verbose", &verbose);
}

ff()
{
	ask("Fullmem", &fullmem);
}

fm()
{
	ask("Multibus Mem tests", &testmulti);
}

dorefresh()
{
	register i;

again:
	printf("Refresh rate (us) (0x%x): ", refreshrate);
	i = getn();
	if(i == -1) {
		printf("\n");
		return;
	}
	if(i < 1) {
		printf(" Greater than 0 please...\n");
		goto again;
	}
	refreshrate = i;
	i = (i * 27 + 8) / 15;		/* Adjust for crystal rate */
	printf(" CTR=%d\n", i);
	UART0[12] = i >> 8;
	UART0[14] = i;
}

lm()
{
	register i;

again:
	printf("Lowest mem to test in 1/2 megs (0x%x): ", lowestmem);
	i = getn();
	if(i == -1) {
		printf("\n");
		return;
	}
	if(i < 0 || i > 31) {
		printf(" Please answer 0-1f\n");
		goto again;
	}
	lowestmem = i;
	printf("\n");
}

memsel()
{
	register i;

	printf("Lowest address to test (0x%x): ", lowad);
	i = getn();
	if(i == -1)
		printf("%x", lowad);
	else 
		lowad = i;
	printf(" highest: (0x%x) ", highad);
	i = getn();
	if(i == -1)
		printf("%x", highad);
	else
		highad = i;
	printf("\n");
}

dm()
{
	ask("direct memory tests", &testlocal);
}

fp()
{
	register i, c;

	printf("Patterns:\n");
	for(i = 1; i < NPATS; i++)
		if(i == 1 || i == 2)
			printf("%c:(%s) %04x\n", i+'a'-1,
				patf[i]?"True ":"False", pats[i]);
		else
			printf("%c:(%s) %s\n", i+'a'-1,
				patf[i]?"True ":"False", patnm[i]);
	for(;;) {
		printf("? ");
		switch(c = negetchar()) {
/* Add new patterns here */
		case 'a': case 'b': case 'c': case 'd': case 'e':
		case 'f': case 'g': case 'h': case 'i': case 'j':
		case 'k': case 'l': case 'm':
			printf("%c: %s (true/false)? ", c, patnm[c-'a'+1]);
			switch(negetchar()) {
			case 't': printf("True"); patf[c-'a'+1]=1;
				if(c == 'a' || c == 'b' || c == 'h' ||
				   c == 'i' || c == 'm') {
					printf(" Pattern: ");
					i = getn();
					if(i == -1)
						printf("%x", pats[c-'a'+1]);
					else
						pats[c-'a'+1] = i;
				}
				if(c == 'i' || c == 'j') {
					printf(" Address: ");
					i = getn();
					if(i == -1)
						printf("%x", patad[c-'a'+1]);
					else {
						patad[c-'a'+1] = (ushort *)i;
						patad[c-'a'+1] += mbw;
					}
				}
				break;
			case 'f': printf("False"); patf[c-'a'+1]=0; break;
			}
			putchar('\n');
			break;
		case '\n': case '\r':
			printf("Quit\n"); return;
		default:
			printf("Answer <letter> or <return> to quit\n");
		}
	}
}

getn()
{
	register n = 0, c;
	register digits = 0;

	printf("0x");
	while((c = negetchar()) != '\n') {
		putchar(c);
		n *= 16;
		if(c >= 'A' && c <= 'Z') c += 'a' - 'A';
		n += (c >= 'a')? c - 'a' + 10 : c - '0';
		digits++;
	}
	return digits ? n : -1;
}

unsigned short find_mb_window()
{	
	/* for now, just assume the window is set at 0M */
	return(atop(0x0));
}
