/*
** $Source: /d2/3.7/src/pm2stand/ipfex/RCS/exercise.c,v $
** $Date: 89/03/27 17:11:17 $
** $Revision: 1.1 $
*/

/*
** exercise.c
*/
#include <sys/types.h>
#include "disk.h"
#include "ipreg.h"
#include "dsdreg.h"
#include <sys/dklabel.h>
#include "test.h"

/* Error logging stuff */
#define	NELOG	200	/* Max logs to keep */
#define	LSOFT	1	/* Will retry this error */
#define	LHARD	2	/* This one was HARD */
#define	LDATA	4	/* Data miscompare */

struct	elog {
	USHORT	etype;	/* Types of errors for this block */
	USHORT	ecnt;	/* Number of errors for this block */
	USHORT	eunit;	/* Keep track of these */
	ULONG	elba;	/* lba of block (track) */
} elogs[NELOG];
ULONG	elogi;		/* How many current logs */
char	*ertypes[] = {
	"None??", "Soft", "Hard", "Soft+Hard",
	"Data", "Soft+Data", "Hard+Data", "Soft+Hard+Data"
};

ULONG	cpats[] = {
	0xB1B6DB6D,	0xB1F6DB6D,	0xB1B6D96D,	0xD1D9B6BD,
/*	0x55555555,	0xAAAAAAAA,	0x00000000,	0xFFFFFFFF,	*/
	0x12345678,	0x87654321,	0xF0F0F0F0,	0x0F0F0F0F
};
ULONG	cpati;

#define	RANDMULT	5
#define RANDOFFSET	17623
#define RANDOMIZE(r)	r = (r * RANDMULT + RANDOFFSET) & 0xffff
#define SEED		433
USHORT	grand;

exercise()
{
	printf("Exercise\n"); pd();
	if(!isinited[dunit])
		initl();		/* Init the disk */
	if(writelock)
		printf("***Warning: set write lock off to scribble on disk\n");
	if(ipinit()) return;
    for(;;) {
	printf("Which exercise? ");
	switch(gch()) {
	case 'c':
		compex(0); break;
	case 'C':
		compex(1); break;
	case 'd':
		diskrw(0); break;
	case 'D':
		diskrw(1); break;
	case 'e':
		derrs(); break;
	case 'm':
		secwr(0); break;
	case 'M':
		secwr(1); break;
	case 'q':
		printf("Quit\n"); return;
	case 'r':
		ranread(); break;
	case 'R':
		rnset(); break;
	case 's':
		seek(); break;
	case 'S':
		seektest(); break;
	default:
		printf("Help--Choose from:\n");
		printf("  complete write/read multi pass/multi pattern\n");
		printf("  Complete write/read -- One pass\n");
		printf("  disk read or write multiple\n");
		printf("  Disk read or write single chunk\n");
		printf("  error display/reset\n");
		printf("  multiple sector write/read repeated\n");
		printf("  Multiple sector write/read once\n");
		printf("  quit\n");
		printf("  random reads\n");
		printf("  Random number reset\n");
		printf("  seek tests canned\n");
		printf("  Seek test selective\n");
		printf("\n");
		break;
	}
    }
}

rnset()
{
	grand = SEED;
	printf("Random number reset\n");
}

ranread()
{
	register USHORT rand;
	register USHORT ns;
	register m, lba;

	printf("Random read test\n");
	printf("Number of random reads? ");
	if((m = getnum(10, 1, BIGNUM)) == -1) {
		m = BIGNUM;
		printf("forever");
	}
	printf("\nTic per 100\n");
	ns = HD * SEC;
	rand = grand;
	while(m--) {
		RANDOMIZE(rand);
		lba = (rand % CYL) * ns;
		RANDOMIZE(rand);
		lba += (rand % ns);
		rdata(1, lba, BUF0);
		if(nwgch() != -1)
			break;
		if((m % 100) == 0)
			QP0(".");
	}
	grand = rand;
}

elog(lba, type)
ULONG lba;
{
	register i;
	register struct elog *ep, *ep1;
	static overflow;

	for(ep = elogs; ep < &elogs[elogi]; ep++) {
		if(lba < ep->elba)
			break;
		if(lba == ep->elba && dunit == ep->eunit) {
			ep->etype |= type;
			ep->ecnt++;
			return;
		}
	}
	if(elogi == NELOG) {
		if(overflow++ == 0)
			printf("Error log overflow\n");
		return;
	}
	for(ep1 = &elogs[elogi++]; ep1 > ep; ep1--)
		*ep1 = ep1[-1];
	ep->elba = lba;
	ep->etype = type;
	ep->eunit = dunit;
	ep->ecnt = 1;
}

/*
 * Disk read or write exercise()
 */
diskrw(lpcnt)
char lpcnt;
{
	register ULONG loop, ns, lba;
	ULONG seq, cmd, c;
	register char *cp;

	printf("Disk read or write test\n");
	if(!lpcnt) {
		printf("Repeat how many times? ");
		loop = getnum(10, 1, BIGNUM);
		printf(" ");
		if(loop == -1) {
			loop = BIGNUM;
			printf("forever ");
		}
	} else {
		loop = 1;
	}
d1:	printf("Disk read or write? ");
	switch(gch()) {
		case 'q': printf("Quit\n"); return;
		case 'r': printf("Read"); cmd = 0; break;
		case 'w': printf("Write"); cmd = 1;
				if(writelock) {
		printf("\nWrite lock on--reset to scribble on disk\n");
					return;
				}
				break;
		default:  printf("Answer r/w/q please\n"); goto d1;
	}
	do {
		printf("\n # of sectors (%d bytes each): ", secsize);
		ns = getnum(10, 1, BUFSIZE*2/secsize);
	} while(ns == -1);
	do {
		printf("\n Where (block# or sequential)? ");
		cp = getline();
		seq = 0;
		if(*cp == 's') {
			printf("equential");
			lba = 0; seq = 1;
		} else if(*cp == 0)
			return;
		else
			lba = bparse(cp, 1);
	} while(lba == -1);
	printf("\n");
	while(loop--) {
		if(cmd == 0) {
			if(rdata(ns, lba, BUF0)) {
				ip_flags = 0;
				return;
			}
		} else
			if(wdata(ns, lba, BUF0)) {
				ip_flags = 0;
				return;
			}
		if(!quiet) {
			printf(".");
			if((c = nwgch()) != -1)
				if(c == ' ')
					printf("At %d\n", lba);
				else
					return;
		}
		if(seq) {
			lba += ns*seq;
			if(lba <  0 ||
			   lba >= CYL*HD*SEC) {
				printf("Pass\n");
				seq = -seq;
				lba += ns*seq;
			}
		}
	}
}
/*
 * Multiple Sector write and read
 */
secwr(lpcnt)
char lpcnt;
{
	register ULONG loop, ns, bad, nbad;
	register USHORT *shp;
	ULONG lba, i, c;
	register char *cp;

	printf("Multiple sector write/read\n");
	if(writelock) {
		printf("Write lock on--reset to scribble on disk\n");
		return;
	}
	if(!lpcnt) {
		printf("Repeat how many times? ");
		loop = getnum(10, 1, BIGNUM);
		if(loop == -1) {
			loop = BIGNUM;
			printf("forever");
		}
	} else {
		loop = 1;
	}
	do {
		printf("\n # of sectors (%d bytes each): ", secsize);
		ns = getnum(10, 1, BUFSIZE/secsize);
	} while(ns == -1);
	do {
		printf("\n Where (%s)? ", emodes[emode]);
		cp = getline();
		if(*cp == 0) return;
		lba = bparse(cp, 1);
	} while(lba == -1);
	if(verbose)printf("    LBA = %x", lba);
	printf("\n");
	bad = loop; nbad = 0;
mult:	for(i = 0, shp = (USHORT *)BUF0; i < ns*(secsize/2); i++)
		*shp++ = i;		/* 0 with seq pattern */
    	while(loop--) {
		if(wdata(ns, lba, BUF0)) {/* Write out BUF0 */
			ip_flags = 0;
			continue;
		}
		filll(BUF1, 0x10001, ns*(secsize/4));
		if(rdata(ns, lba, BUF1)) {/* Read back the data to Buf1 */
			ip_flags = 0;
			continue;
		}
		for(i = 0, shp = (USHORT *)BUF1; i < ns*(secsize/2); i++) {
			if(*shp++ != i) {
				--shp;
				printf("B1: @%d %x->%x\n", ((int)shp)-BUF1,
					i, *shp);
				nbad++;
				bad=loop;
				return;
			}
		}
		if((c=nwgch()) != -1) {
			if(c == ' ')
				printf("%d %d %d\n",500000-loop, bad-loop,
						nbad);
			else
				return;
		}
		printf(".");
    	}
}

/*
 * Complete exercise for the disk unit 
 */
static ULONG Tlbad[NUNIT];
static ULONG Tlret[NUNIT];

compex(lpcnt)
char lpcnt;
{
	register ULONG loop, altunit;
	ULONG nbad, ns, lba, c, i;
	ULONG maxlba;
	ULONG retries, cmd, loopcnt, Retry;
	ULONG first;

	printf("Complete Excercise -- track writes and reads\n");
	if(writelock) {
		printf("Write lock on--reset to scribble on disk\n");
		return;
	}
	if(!lpcnt) {
		printf("Repeat how many times? ");
		loop = getnum(10, 1, BIGNUM);
		if(loop == -1) {
			loop = BIGNUM;
			printf("forever");
		}
		printf("\nAlternate units? ");
		altunit = 0;
		switch(gch()) {
			case 'y':
				printf("Yes");
				getunits();
				for(i=0;i<numunits;i++) {
					dunit = i;
					Tlbad[dunit] = 0;
					Tlret[dunit] = 0;
					if(ipinit()) {
					 printf("Can't ipinit disk drive: %d\n",							dunit);
					 return;
					}
				}
				dunit = (numunits-1);
				altunit = 1;
				break;
			default:	printf("No");
				break;
		}
		printf("\n");
	} else {
		altunit = 0;
		loop = 1;
	}
	nbad = retries = loopcnt = cpati = 0;
	first = 1;
	while(loop) {
		cmd = cpats[cpati&0x7];
		if(altunit) {		/* New pattern after both drives */
			if(first) {
				dunit = first = 0;
			} else if(++dunit > (numunits-1)) {
				dunit = 0;
				loop--;
				loopcnt++;
			}
			drivep = &drives[dunit];
			if(loopcnt & 1) cpati++;
		} else {
			if(first) first = 0;
			else loop--;
			loopcnt++;
			cpati++;
		}
		if(!quiet) printf("Unit %d: Pattern 0x%x\n", dunit, cmd);

		ns = SEC;
		maxlba = CYL*HD*ns;
	    	for(lba = 0; lba < maxlba; lba += ns) {
			filll(BUF0, cmd^lba, ns*(secsize/4));
			Retry = 0;
			/* Write out BUF0 */
			while(wdata(ns, lba, BUF0)) {
				elog(lba, LSOFT);
				if(Retry++ < 5) {
					retries++;
					continue;
				}
				nbad++;
				elog(lba, LHARD);
#ifdef WHATTHEFUCK
				switch(rsq(0)) {
					case RETRY: continue;
					case SKIP:  break;
					case QUIT:  return;
				}
#endif
			ip_flags = 0;
			goto CLcont;
			}
			Retry = 0;
			/* Read back data to Buf1 */
			while(rdata(ns, lba, BUF1)) {
				elog(lba, LSOFT);
				if(Retry++ < 5) {
					retries++;
					continue;
				}
				nbad++;
				elog(lba, LHARD);
#ifdef WHATTHEFUCK
				switch(rsq(0)) {
					case RETRY: continue;
					case SKIP:  break;
					case QUIT:  return;
				}
#endif
				ip_flags = 0;
				goto CLcont;
			}
			if(cmpl(BUF1, cmd^lba, ns*(secsize/4))) {
				printf("Data mismatch: %d+%d %x->%x\n",
					lba+(clea-BUF1)/secsize, clea%secsize,
					cmd^lba, cleis);
				elog(lba, LDATA);
				nbad++;
			}
CLcont:			if((c=nwgch()) != -1) {
				if(c == ' ')
					printf("%d\n", lba);
				else
					return;
			}
			if(lba && (lba % (ns*HD*10)) == 0)
				QP1("%3d ",lba/(ns*HD));
	    	}
		Tlbad[dunit] += nbad;
		Tlret[dunit] += retries;
		printf("LOOP %d:     Unit %d, bad %d, retries %d\n",
			loopcnt, dunit, nbad, retries);
		retries = nbad = 0;
		if(!altunit) {
			printf("TOTAL UNIT %d COUNT RETRIES: %d, BAD: %d\n",
				dunit, Tlret[dunit], Tlbad[dunit]);
		} else {
			printf("*** Total COUNT - BAD AREAS FOUND ***\n");
			for(i=0;i<numunits;i++) {
				printf("    UNIT %d: RETRIES: %d, BAD: %d\n",
					i, Tlret[altunit], Tlbad[dunit]);
			}
		}
	}
}

seektest()
{
	register ULONG ns, cylin, head, sect;
	register char *cp;
	ULONG lba, i;
	ULONG loopcnt;

	printf("Seek test");
	do {
		printf("\n  Seek address (%s)? ", emodes[emode]);
		if(*(cp = getline()) == 0) return;
		lba = bparse(cp, 1);
	} while(lba == -1);
	explode(lba);
	if(verbose) printf("lba=%d, cyl=%d, hd=%d, sec=%d\n",
		lba, dcyl, dhd, dsec);
	printf("  How many times? ");
	if((loopcnt = getnum(10,1,BIGNUM)) == -1) {
		loopcnt = BIGNUM;
		printf(" forever");
	}
	printf("\nStart\n");
	for(i=0;i<loopcnt;) {
		if(rdata(1,lba,BUF0)) {
			printf("error: lba=%d\n",lba);
			return;
		}
		if(rdata(1,0,BUF0)) {
			printf("error: lba=0\n");
			return;
		}
		if((++i % 100) == 0) printf(".");
	}
	printf("\n");
}

seek()
{
	register char *cp;
	register i;

	printf("Seek test on:\n"); pd();
	if(ipinit()) return;
	printf("Butterfly\n");
	for(i = 0; i < CYL; i++) {
		if(dseek(i) || dwait()) if(err()) return;
		if(dseek(CYL-i-1) || dwait()) if(err()) return;
		if(nwgch() != -1) return;
	}
	printf("Sequential up/down. Tic per 100\n");
	if(wait()) return;
	for(i = 0; i < CYL*2; i++) {
		if(dseek(i<CYL?i:CYL*2-i-1) || dwait())
			if(err()) return;
		if(i && (i%100) == 0) printf(".");
		if(nwgch() != -1) return;
	}
	printf("Worst case. Tic per 100\n");
	if(wait()) return;
	for(i = 0; i < 301; i++) {
		if(dseek(0) || dwait()) if(err()) return;
		if(dseek(CYL-1) || dwait()) if(err()) return;
		if(i && (i%100) == 0) printf(".");
		if(nwgch() != -1) return;
	}
	printf("Seek test complete\n");
}

wait()
{
	if(verbose) {
		printf("~");
		if(gch() != '\n') {
			printf("Quit\n");
			return 1;
		}
	}
	return 0;
}

err()
{
	return 0;
}

derrs()
{
	register struct elog *ep;
	register char *cp;
	register i;

again:
	printf("Error Display or Reset? ");
	switch(gch()) {
	case 'd':
		printf("Display. %d Errors logged\n", elogi);
		for(i = 0, ep = elogs; ep < &elogs[elogi]; ep++) {
			explode(ep->elba);
			printf("Unit %d Cnt %d %d/%d/%d %s\n",ep->eunit,
				ep->ecnt, dcyl, dhd, dsec, ertypes[ep->etype]);
			if((++i)%20 == 0) {
				printf("...<space> to continue...");
				switch(gch()) {
				case ' ': printf("\n"); break;
				default:  printf("\n"); return;
				}
			}
		}
		break;
	case 'r':
		printf("Reset errors.\n");
		elogi = 0;
		break;
	default:
		printf("d/r please\n");
		goto again;
	}
}

dwait()
{
	register i;

	for(i=0; i<10000; i++);
	return 0;
}
