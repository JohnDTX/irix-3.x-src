#include "pmII.h"
#include "pmIImacros.h"
#include "mem.h"
#include "duart.h"
#include "IrisConf.h"

typedef unsigned short ushort;


#define	PAT1	0xF0F0
#define	PAT2	0x9595
ushort	pat12[3];
#define	ERRLIM	5
#define ONEMEG	0x100
#define HALF	0x80
#define	MIN(a,b) ((a<b)?a:b)

int errs;
int gpasses;
int bpasses;
int summary;
int testlocal;
int testmulti;
int lowestmem;
int fullmem;
int verbose;

#define	NPATS	8
char	patf[NPATS];
char	*patnm[NPATS] = {
	"??",
	"F0F0",
	"9595",
	"decrementing",
	"incrementing",
	"long a->(a)",
	"marching 1's",
	"marching 0's"
};

jmp_buf cmd_buf;

extern unsigned short mbw_page;
unsigned long mbw;
short ppgoffset,mbwpgoffset;
int getedata(),getend();

main()
{
	register i, p, j, z = 0;
	register unsigned short *a;

	printf("\n\nMemory Test PM2.1 V0.0\n");

	/* get the configuration parameters */
	switches = _commdat->config = CONFIG_REG;

	mbw_page = find_multibus_window();
	memchk(1);
	verbose = 1;
	testlocal = 1;
	testmulti = 1;
	fullmem = 1;
/*	lowestmem = 2;*/
	lowestmem = 0;

	/* ppgoffset is the physical page offset necessary to avoid 
	   overwriting ourselves */
	ppgoffset = (i=getedata()>=ROM0)?
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
	pat12[1] = PAT1;
	pat12[2] = PAT2;

	printf("Type 'h' for help.\n");
  for(;;) {
    if(setjmp(cmd_buf) == 1) putchar('\n');
    cmd();
    for(p = 0; ;p++) {
	if(testlocal)
	    for(i = lowestmem; i < 31; i++)
		if(memfound[i]) {
		    if(setjmp(bejmp) == 0) {
			printf("Testing %06x-%06x ", ptoa(i*HALF),
					ptoa((i+1)*HALF) - 1);
			map(i*HALF, 0);
			check(i*HALF);
			printf("\n");
		    } else
			pberr();
		}
	if(testmulti)
	    for(i = lowestmem; i < 31; i++)
		if(memfound[i]) {
		    if(setjmp(bejmp) == 0) {
			printf("Multibus testing %06x-%06x ", ptoa(i*HALF),
					ptoa((i+1)*HALF) - 1);
			map(i*HALF, 1);
			check(i*HALF);
			printf("\n");
		    } else
			pberr();
		}
	if(chkkey() && fullmem) {
	    printf("Full mem fill ");
	    for(i = lowestmem; i < 31; i++)
		if(memfound[i]) {
		    if(setjmp(bejmp) == 0) {
			printf("X");
			map(i*HALF, 0);
			fillp(i*HALF);
		    } else
			pberr();
		}
	    errs = 0;
	    printf(", Test ");
	    for(i = lowestmem; i < 31; i++)
		if(memfound[i]) {
		    if(setjmp(bejmp) == 0) {
			map(i*HALF, 0);
			testp(i*HALF);
			printf("!");
		    } else
			pberr();
		}
	    perrs();
	    printf("\n");
	    if(errs) bpasses++; else gpasses++;
	}
	if(chkkey() && testmulti && fullmem) {
	    printf("Full mem fill via Multibus ");
	    for(i = lowestmem; i < 31; i++)
		if(memfound[i]) {
		    if(setjmp(bejmp) == 0) {
			printf("X");
			map(i*HALF, 0);
			fillp(i*HALF);
		    } else
			pberr();
		}
	    errs = 0;
	    printf(", Test ");
	    for(i = lowestmem; i < 31; i++)
		if(memfound[i]) {
		    if(setjmp(bejmp) == 0) {
			map(i*HALF, 0);
			testp(i*HALF);
			printf("!");
		    } else
			pberr();
		}
	    perrs();
	    printf("\n");
	    if(errs) bpasses++; else gpasses++;
	}
	if((p % 10) == 9 || summary)
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

check(h)
{
	/* check the half meg of memory starting at page h.
	*/
	register i, si;
	register ushort pat, pr;
	register ushort *a;
	register long *al, lpr;
	register ushort *sa;
	int errors;

	if(h == 0) {
		sa = (ushort *)(ptoa(mbw_page));
		si = HALF*4096/2 - ptoa(ppgoffset)/2;
	} else {
		sa = (ushort *)(ptoa(mbw_page));
		si = HALF*4096/2 - ptoa(mbwpgoffset)/2;
	}
	errs = 0;
#ifdef DEBUG
	printf("check: %x words starting at %x:",(unsigned long)si,
	  	(unsigned long)sa);
#endif
    if(patf[1]) {
	putchar('1');
	fillw(sa, pat12[1], si);
	cmpw(sa, pat12[1], si, h);
	perrs();
	errors = errs;
	errs = 0;
	if(chkkey() == 0) return;
    }
    if(patf[2]) {
	putchar('2');
	fillw(sa, pat12[2], si);
	cmpw(sa, pat12[2], si, h);
	perrs();
	errors += errs;
	errs = 0;
	if(chkkey() == 0) return;
    }
    if(patf[3]) {
	putchar('3');
	for(a = sa, i = si; i; i--, a++)
		*a = i;
	for(a = sa, i = si; i; i--, a++)
		if((pr = *a) != (ushort)i)
			perr((long)a, h, (ushort)i, pr);
	perrs();
	errors += errs;
	errs = 0;
	if(chkkey() == 0) return;
    }
    if(patf[4]) {
	putchar('4');
	for(a = sa, i = si; i; i--, a++)
		*a = ~i;
	for(a = sa, i = si; i; i--, a++)
		if((pr = *a) != (ushort)~i)
			perr((long)a, h, (ushort)~i, pr);
	perrs();
	errors += errs;
	errs = 0;
	if(chkkey() == 0) return;
    }
    if(patf[5]) {
	putchar('5');
	for(al = (long *)sa, i = si/2; i; i--, al++)
		*al = (long)al ^ ((long)al<<14);
	for(al = (long *)sa, i = si/2; i; i--, al++)
		if((lpr = *al) != ((long)al ^ ((long)al<<14)))
			perr((long)al, h, (long)al ^ ((long)al<<14), lpr);
	perrs();
	if(chkkey() == 0) return;
    }
    if(patf[6]) {
	putchar('6');
	for(pat = 1; pat; pat <<= 1) {
		errors += errs;
		errs = 0;
		fillw(sa, pat, si);
		cmpw(sa, pat, si, h);
		perrs();
		if(chkkey() == 0) return;
	}
    }
    if(patf[7]) {
	putchar('7');
	for(pat = 1; pat; pat <<= 1) {
		errors += errs;
		errs = 0;
		fillw(sa, ~pat, si);
		cmpw(sa, ~pat, si, h);
		perrs();
		if(chkkey() == 0) return;
	}
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

perr(a, h, pat, is)
long a;
{
	register e = errs, x = pat ^ is;

	if(e == 0) {
		xorp = 0;
		etrcp = 0;
	}
	x = ((x >> 16) | x) & 0xFFFF;
#ifdef foo
	if(errs++ < ERRLIM)
		printf("\n  Mem err @ %06x: should be %x, is %x, xor %x",
			a - mbw + ptoa(h), pat, is, x);
#endif
	if(etrcp == 0 || (etrc[etrcp-1].e_x & x) != x)
		printf("\n  Mem err @ %06x: should be %x, is %x, xor %x",
			a - mbw + ptoa(h), pat, is, x);
	if(etrcp == 0 || (etrc[etrcp-1].e_a & 0x60000) != (a & 0x60000)) {
		etrc[etrcp].e_a = a - mbw + ptoa(h);
		etrc[etrcp++].e_x = x;
	} else
		etrc[etrcp-1].e_x |= x;
	errs++;
	xorp |= x;
}

perrs()
{
	register i, b, e, bit = -1, h, bank;

	if(errs) {
	    printf("\n>>>0x%x Errors, %x - %x, XOR=%x\n>>> ", errs,
		etrc[0].e_a, etrc[etrcp-1].e_a, xorp);
	    h = etrc[0].e_a >> 19;
	    if(h == 30 || ((h & 1) == 0 && memfound[h+1] == 0))
		printf("PM ");
	    else
		printf("MX(%x) ", (1<<(7-(h>>1)/4))|(1<<(3-((h>>1)&3))));
	    for(e = 0; e < etrcp; e++) {
		for(i = 0, b = 1; i < 16; b <<= 1, i++)
		    if(etrc[e].e_x & b) {
			bank = (etrc[e].e_a >> 17) & 3;
			bit = i;
			if(bit > 7) {
				bit -= 8;
				bank += 4;
			}
			if(h == 30 || ((h & 1) == 0 && memfound[h+1] == 0))
				printf("%d%c ", bit + 15, "CDEFGHJK"[bank]);
			else
				printf("%d%c ", bit + ((h&1)?1:12),
						"ABCDEFGH"[bank]);
		    }
	    }
	    printf("<<<\n+++");
	} else
		printf(".");
}

putchar(c)
{
	putc(c, 1);
}

fillp(h)
{
	register long *al = (long *)mbw;
	register i = (HALF*4096/4);
	register pat = ptoa(h);

	if(h == 0) {
		i  -= (ptoa(ppgoffset)/4);
	}
	--i;
	do {
		*al++ = pat++;
	} while(--i != -1);
}

testp(h)
{
	register long *al = (long *)mbw;
	register i = (HALF*4096/4);
	register pat = ptoa(h);

	if(h == 0) {
		i -= (ptoa(ppgoffset)/4);
	}
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
	if((STATUS_REG & FAULT_PRESENT) == 0)
		printf("Not Present ");
	if((STATUS_REG & FAULT_MAP) == 0)
		printf("Map Fault ");
	if((STATUS_REG & FAULT_TIMEOUT) == 0)
		printf("Timeout ");
	if((STATUS_REG & FAULT_PARITY) == 0)
		printf("Parity Fault ");
	printf("\n");
}

chkkey()
{
	register c, r = -1;

    for(;;) {
	if((c = nwgetc(1))== -1)
		return r;
	switch(c) {
	case 'v':	verbose++;	break;
	case 'q':	verbose = 0;	break;
	case 's':	summary++;	break;
	case ' ':	r = 0;		break;
	case '\n':	longjmp(cmd_buf, 1);
	default:	putchar('\007'); break;
	}
    }
}

cmd()
{
	for(;;) {
		printf("MT> ");
		switch(getc(1)) {
		case 'f':	ff(); break;
		default:
		case 'h':	help(); break;
		case 'l':	lm(); break;
		case 'm':	fm(); break;
		case 'o':	om(); break;
		case 'p':	fp(); break;
		case 'q':	printf("Quiet\n"); verbose=0; break;
		case 'r':	printf("Run\n"); return;
		case 'v':	printf("Verbose\n"); verbose++; break;
		}
	}
}

help()
{
	printf("\
Help; commands are:\n\
  Fullmem test\n\
  Help\n\
  Low mem to test\n\
  Multibus tests\n\
  On-board tests\n\
  Pattern select\n\
  Quiet\n\
  Run\n\
  Verbose\n\
");
}

ff()
{
	printf("Fullmem (%s)? ", fullmem?"true":"false");
	switch(getc(1)) {
	case 't':	printf("True"); fullmem=1; break;
	case 'f':	printf("False"); fullmem=0; break;
	}
	printf("\n");
}

fm()
{
	printf("Multibus Mem tests (%s)? ", testmulti?"true":"false");
	switch(getc(1)) {
	case 't':	printf("True"); testmulti=1; break;
	case 'f':	printf("False"); testmulti=0; break;
	}
	printf("\n");
}

lm()
{
	register i;

again:
	printf("Lowest mem to test in 1/2 megs (0x%x): 0x", lowestmem);
	i = getn();
	if(i < 0 || i > 31) {
		printf(" Please answer 0-1f\n");
		goto again;
	}
	lowestmem = i;
	printf("\n");
}

om()
{
	printf("On-board Mem tests (%s)? ", testmulti?"true":"false");
	switch(getc(1)) {
	case 't':	printf("True"); testlocal=1; break;
	case 'f':	printf("False"); testlocal=0; break;
	}
	printf("\n");
}

fp()
{
	register i, c;

	printf("Patterns:\n");
	for(i = 1; i < NPATS; i++)
		if(i == 1 || i == 2)
			printf("%d: %04x %s\n", i, pat12[i],
				patf[i]?"True":"False");
		else
			printf("%d:%s %s\n", i, patnm[i],
				patf[i]?"True":"False");
	for(;;) {
		printf("# ");
		switch(c = getc(1)) {
			  case '1': case '2': case '3':
		case '4': case '5': case '6': case '7':
			c -= '0';
			printf("%d (t/f)? ", c);
			switch(getc(1)) {
			case 't': printf("True"); patf[c]=1;
				if(c == 1 || c == 2) {
					printf(" Pattern: 0x");
					pat12[c] = getn();
				}
				break;
			case 'f': printf("False"); patf[c]=0; break;
			}
			putchar('\n');
			break;
		case 'q':
			printf("Quit\n"); return;
		default:
			printf("Answer 1-7 or quit\n");
		}
	}
}

getn()
{
	register n = 0, c;

	while((c = getc(1)) != '\n') {
		putc(c, 1);
		n *= 16;
		if(c >= 'A' && c <= 'Z') c += 'a' - 'A';
		n += (c >= 'a')? c - 'a' + 10 : c - '0';
	}
	return n;
}

unsigned short find_mb_window()
{	
	/* for now, just assume the window is set at 0M */
	return(atop(0x0));
}
