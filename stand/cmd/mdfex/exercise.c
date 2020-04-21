/*
** exercise.c	- Copyright (C) Silicon Graphics - Mountain View, CA 94043
**		- Author: chase
**		- Date: April 1985
**		- Any use, copy or alteration is strictly prohibited
**		- and gosh darn awfully -- morally inexcusable
**		- unless authorized by SGI.
**
** $Author: root $
** $State: Exp $
** $Source: /d2/3.7/src/stand/cmd/mdfex/RCS/exercise.c,v $
** $Revision: 1.1 $
** $Date: 89/03/27 17:12:48 $
*/

#include <sys/types.h>
#include "disk.h"
#include "dsdreg.h"
#include <sys/dklabel.h>
#include "fex.h"

/* Error logging stuff */
#define	NELOG	200	/* Max logs to keep */
#define	LSOFT	1	/* Will retry this error */
#define	LHARD	2	/* This one was HARD */
#define	LDATA	4	/* Data miscompare */

struct	elog {
	USHORT	etype;	/* Types of errors for this block */
	USHORT	ecnt;	/* Number of errors for this block */
	USHORT	eunit;	/* Keep track of these */
	int	elba;	/* lba of block (track) */
} elogs[NELOG];
ULONG	elogi;		/* How many current logs */
char	*ertypes[] = {
	"None??", "Soft", "Hard", "Soft+Hard",
	"Data", "Soft+Data", "Hard+Data", "Soft+Hard+Data"
};

ULONG	cpats[] = {
	0xB1B6DB6D,	0xB1F6DB6D,	0xB1B6D96D,	0xD1D9B6BD,
	0xB16BD6BD,	0xB16FD6BD,	0xB1BD669D,	0xD19DBB6D,
	0x1B6BD6BD,	0x1B6FD6BD,	0xB16B9DD6,	0xDD19B6DB,
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
		(void) initl();		/* Init the disk */
	if(writelock && drivep->tdev == D_WIN)
		printf("***Warning: set write lock off to scribble on disk\n");
	if(config()) return;
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
	case 't':
		tktest(0); break;
	case 'T':
		tktest(1); break;
	case 'x':
		xdiskrw(); break;
	default:
		printf("Exercise Help -- Choose from:\n");
		printf("  complete write/read multi pass/multi pattern\n");
		printf("  Complete write/read one pass\n");
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
		printf("  track tests multi pass/multi pattern\n");
		printf("  Track test one pass\n");
		printf("  xtra disk read write multiple tracks\n");
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
		(void) rdata(1, lba, (char *)BUF0);
		if(nwgch() != -1)
			break;
		if((m % 100) == 0)
			QP0(".");
	}
	grand = rand;
}

elog(lba, type)
{
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
	register loop, lba;
	register ns;
	ULONG cmd, c;
	register short seq;
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
				if(writelock && drivep->tdev == D_WIN) {
		printf("\nWrite lock on--reset to scribble on disk\n");
					return;
				}
				break;
		default:  printf("Answer r/w/q please\n"); goto d1;
	}
	do {
		printf("\n # of sectors (%d bytes each): ", secsize);
		ns = getnum(10, 1, (int)(BUFSIZE*2/secsize));
	} while(ns == -1);
	do {
		printf("\nWhere for %s (block# or sequential)? ",
			(cmd == 0)? "Read":"Write");
		cp = getline();
		seq = 0;
		if(*cp == 's') {
			printf("equential");
			lba = 0;
			printf("\nPick the step size(%d): ", ns);
			seq = getnum(10, 1, BIGNUM);
			printf(" ");
			if(seq == -1) {
				seq = ns;
				printf("%d", ns);
			}
		} else if(*cp == 0)
			return;
		else
			lba = bparse(cp, 1);
	} while(lba == -1);
	printf("\n");
	while(loop--) {
		if(cmd == 0) {
			if(rdata(ns, lba, (char *)BUF0)) {
				sd_flags = 0;
				return;
			}
		} else
			if(wdata(ns, lba, (char *)BUF0)) {
				sd_flags = 0;
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
			lba += seq;
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
	register loop, ns, bad, nbad;
	register USHORT *shp;
	int lba, i, c;
	register char *cp;

	printf("Multiple sector write/read\n");
	if(writelock && drivep->tdev == D_WIN) {
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
		ns = getnum(10, 1, (int)(BUFSIZE/secsize));
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
	for(i = 0, shp = (USHORT *)BUF0; i < ns*(secsize/2); i++)
		*shp++ = i;		/* 0 with seq pattern */
    	while(loop--) {
		if(wdata(ns, lba, (char *)BUF0)) {/* Write out BUF0 */
			sd_flags = 0;
			continue;
		}
		filll((long *)BUF1, 0x10001, (int)(ns*(secsize/4)));
		if(rdata(ns, lba, (char *)BUF1)) {
			sd_flags = 0;
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
	register loop, altunit;
	register nbad, ns, lba, c;
	int maxlba, first;
	int retries, cmd, loopcnt;

	printf("Complete Exercise -- track writes and reads\n");
	if(writelock && drivep->tdev == D_WIN) {
		printf("Write lock on--reset to scribble on disk\n");
		return;
	}
	altunit = 0;
	if(!lpcnt) {
		printf("Repeat how many times? ");
		loop = getnum(10, 1, BIGNUM);
		if(loop == -1) {
			loop = BIGNUM;
			printf("Forever");
		}
		if(drivep->tdev == D_WIN) {
		    printf("\nAlternate units? ");
		    switch(gch()) {
			case 'y':
				altunit = 1;
				printf("Yes");
				dunit = 0;	
				Tlbad[dunit] = 0;
				Tlret[dunit] = 0;
				if(config()) {
					printf("Can't config disk drive: %d\n",							dunit);
					return;
				}
				dunit = 1;
				Tlbad[dunit] = 0;
				Tlret[dunit] = 0;
				if(config()) {
					printf("Can't config disk drive: %d\n",							dunit);
					return;
				}
				break;
			default:	printf("No");
				break;
		    }
		}
		printf("\n");
	} else
		loop = 1;
	loopcnt = retries = cpati = nbad = 0;
	first = 1;
	incompex = 1;
	while(loop) {
		cmd = cpats[cpati&0x7];
		if(altunit) {		/* New pattern after both drives */
			if(first) {
				first = 0;
				dunit = 0;
			} else if(dunit) {
				dunit = 0;
				loop--;
				loopcnt++;
			} else dunit++;
			drivep = &drives[dunit];
			if(loopcnt & 1) cpati++;
		} else {
			cpati++;
			loopcnt++;
		}
		if(!quiet)
			printf("START LOOP %d   Unit %d: Pattern 0x%x\n",
					loopcnt, dunit, cmd);
		ns = SEC;
		maxlba = CYL*HD*ns;
	    	for(lba = 0; lba < maxlba; lba += ns) {
			filll((long *)BUF0, cmd^lba, (int)(ns*(secsize/4)));
			Retry = 0;
			/* Write out BUF0 */
			while(wdata(ns, lba, (char *)BUF0)) {
				elog(lba, LSOFT);
				if(Retry++ < 5) {
					retries++;
					continue;
				}
				nbad++;
				elog(lba, LHARD);
#ifdef NOTDEF
				switch(rsq(0)) {
					case RETRY: continue;
					case SKIP:  break;
					case QUIT:  return;
				}
#endif
			sd_flags = 0;
			goto CLcont;
			}
			Retry = 0;
			/* Read back data to Buf1 */
			while(rdata(ns, lba, (char *)BUF1)) {
				elog(lba, LSOFT);
				if(Retry++ < 5) {
					retries++;
					continue;
				}
				nbad++;
				elog(lba, LHARD);
#ifdef NOTDEF
				switch(rsq(0)) {
					case RETRY: continue;
					case SKIP:  break;
					case QUIT:  return;
				}
#endif
				sd_flags = 0;
				goto CLcont;
			}
			if(cmpl(BUF1, cmd^lba, (int)(ns*(secsize/4)))) {
				printf("Data mismatch: %d+%d %x->%x\n",
					lba+(clea-BUF1)/secsize, clea%secsize,
					cmd^lba, cleis);
				elog(lba, LDATA);
				nbad++;
			}
CLcont:			if((c=nwgch()) != -1) {
				if(c == ' ')
					printf("%d\n", lba);
				else {
					incompex = 0;
					return;
				}
			}
			if(lba && (lba % (ns*HD*10)) == 0)
				QP1("%3d ",lba/(ns*HD));
	    	}
		Tlbad[dunit] += nbad;
		Tlret[dunit] += retries;
		printf("LOOP: %d.   Unit: %d    retries: %d     BAD: %d\n",
			loopcnt, dunit, nbad, retries);
		if(!altunit)
			printf("ACCUMLATED   RETRIES: %d     BAD: %d\n",
				Tlret[dunit], Tlbad[dunit]);
		else {
			for(first=0;first < 2; first++) {
			  printf("UNIT %d ACCUMLATED   RETRIES: %d   BAD: %d\n",
				first, Tlret[first], Tlbad[first]);
			}
			first = 0;
		}
		nbad = retries = 0;
	}
	incompex = 0;
}

seektest()
{
	register char *cp;
	register lba, i;
	register loopcnt;

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
		if(rdata(1, lba, (char *)BUF0)) {
			printf("error: lba=%d\n",lba);
			return;
		}
		if(rdata(1, 0, (char *)BUF0)) {
			printf("error: lba=0\n");
			return;
		}
		if((++i % 100) == 0) printf(".");
	}
	printf("\n");
}

seek()
{
	register i;

	printf("Seek test on:\n"); pd();
	if(config()) return;
	printf("Butterfly\n");
	for(i = 0; i < CYL; i++) {
		if(dseek(i) || dwait()) if(err()) return;
		if(dseek(CYL-i-1) || dwait()) if(err()) return;
		if(nwgch() != -1) return;
	}
	printf("Sequential up/down. Tic per 100\n");
	if(xwait()) return;
	for(i = 0; i < CYL*2; i++) {
		if(dseek(i<CYL?i:CYL*2-i-1) || dwait())
			if(err()) return;
		if(i && (i%100) == 0) printf(".");
		if(nwgch() != -1) return;
	}
	printf("Worst case. Tic per 100\n");
	if(xwait()) return;
	for(i = 0; i < 301; i++) {
		if(dseek(0) || dwait()) if(err()) return;
		if(dseek(CYL-1) || dwait()) if(err()) return;
		if(i && (i%100) == 0) printf(".");
		if(nwgch() != -1) return;
	}
	printf("Seek test complete\n");
}

xwait()
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

xdiskrw()
{
	register loop, lba, lba2;
	register ns;
	ULONG cmd, c;
	register char *cp;

	printf("Multiple Pointer Disk read or write test\n");
	printf("Repeat how many times? ");
	loop = getnum(10, 1, BIGNUM);
	printf(" ");
	if(loop == -1) {
		loop = BIGNUM;
		printf("forever ");
	}
d1:	printf("\nDisk read or write? ");
	switch(gch()) {
		case 'q': printf("Quit\n"); return;
		case 'r': printf("Read"); cmd = 0; break;
		case 'w': printf("Write"); cmd = 1;
				if(writelock && drivep->tdev == D_WIN) {
		printf("\nWrite lock on--reset to scribble on disk\n");
					return;
				}
				break;
		default:  printf("Answer r/w/q please\n"); goto d1;
	}
	do {
		printf("\n NUMBER of sectors (%d bytes each): ", secsize);
		ns = getnum(10, 1, (int)(BUFSIZE*2/secsize));
	} while(ns == -1);
	do {
		printf("\nLOCATION for 1st %s, block(cyl/hd/sec)? ",
			(cmd == 0)? "Read":"Write");
		cp = getline();
		if(*cp == 0) return;
		else
			lba = bparse(cp, 1);
	} while(lba == -1);
	printf("\n");
	do {
		printf("\nLOCATION for 2nd %s, block(cyl/hd/sec)? ",
			(cmd == 0)? "Read":"Write");
		cp = getline();
		if(*cp == 0) return;
		else
			lba2 = bparse(cp, 1);
	} while(lba2 == -1);
	printf("\n");
	explode(lba);
	printf("1. %d/%d/%d ", dcyl, dhd, dsec);
	explode(lba2);
	printf("%d/%d/%d (%d)\n", dcyl, dhd, dsec, ns);
	while(loop--) {
		if(cmd == 0) {
			if(rdata(ns, lba, (char *)BUF0)) {
				sd_flags = 0;
				return;
			}
			if(rdata(ns, lba2, (char *)BUF0)) {
				sd_flags = 0;
				return;
			}
		} else
			if(wdata(ns, lba, (char *)BUF0)) {
				sd_flags = 0;
				return;
			}
			if(wdata(ns, lba2, (char *)BUF0)) {
				sd_flags = 0;
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
	}
}

tktest(lpcnt)
	char lpcnt;
{
	register loop, altunit;
	register nbad, ns, lba, c;
	int maxlba;
	int retries, cmd, loopcnt, first;

	printf("Special Sammy Test/Exercise -- track writes and reads\n");
	if(writelock && drivep->tdev == D_WIN) {
		printf("Write lock on--reset to scribble on disk\n");
		return;
	}
	altunit = 0;
	if(!lpcnt) {
		printf("Repeat how many times? ");
		loop = getnum(10, 1, BIGNUM);
		if(loop == -1) {
			loop = BIGNUM;
			printf("Forever");
		}
		if(drivep->tdev == D_WIN) {
		    printf("\nAlternate units? ");
		    switch(gch()) {
			case 'y':
				altunit = 1;
				printf("Yes");
				dunit = 0;	
				Tlbad[dunit] = 0;
				Tlret[dunit] = 0;
				if(config()) {
					printf("Can't config disk drive: %d\n",							dunit);
					return;
				}
				dunit = 1;
				Tlbad[dunit] = 0;
				Tlret[dunit] = 0;
				if(config()) {
					printf("Can't config disk drive: %d\n",							dunit);
					return;
				}
				break;
			default:	printf("No");
				break;
		    }
		}
		printf("\n");
	} else
		loop = 1;
	nbad = loopcnt = retries = cpati = 0;
	first = 1;
	incompex = 1;
	while(loop--) {
		cmd = cpats[cpati&0x7];
		if(altunit) {		/* New pattern after both drives */
			if(first) {
				first = 0;
				dunit = 0;
			} else if(dunit) {
				dunit = 0;
				loop--;
				loopcnt++;
			} else dunit++;
			drivep = &drives[dunit];
			if(loopcnt & 1) cpati++;
		} else {
			cpati++;
		}
		ns = SEC;
		maxlba = CYL*HD*ns;

		printf("Unit %d New Test (%d/%d/%d)\n", dunit, CYL, HD, ns);
		explode(maxlba);
		printf("*** Multiple Track Write/Read Test (0/0 to %d/%d)\n",
			dcyl, dhd);
		maxlba -= ns*2;		/* OFFSET Necessary */
		/*
		** This test will skew every 17 sectors (or one track),
		** Therefore this test will take some time
		** and will and should catch all possible errors
		*/
	    	for(lba = 0; lba < maxlba; lba += ns) {
			filll((long *)BUF0, cmd^lba, (int)(ns*2*(secsize/4)));
			Retry = 0;
			/* 2 Track Write out from BUF0 */
			while(wdata(ns*2, lba, (char *)(BUF0))) {
				elog(lba, LSOFT);
				if(Retry++ < 5) {
					retries++;
					continue;
				}
				if(!secnotfound) {
					nbad++;
					elog(lba, LHARD);
					goto TRKcont;
				}
				lba += lbas_left;
				printdefect(lba, 0);
				return;
			}
			Retry = 0;
			/* 2 Track Read back data to Buf1 */
			while(rdata(ns*2, lba, (char *)(BUF1))) {
				elog(lba, LSOFT);
				if(Retry++ < 5) {
					retries++;
					continue;
				}
				if(!secnotfound) {
					elog(lba, LHARD);
					nbad++;
					goto TRKcont;
				}
				lba += lbas_left;
				printdefect(lba, 1);
				return;
			}
			if(cmpl(BUF1, cmd^lba, (int)(ns*2*(secsize/4)))) {
				printf("Data mismatch: %d+%d %x->%x\n",
					lba+(clea-BUF1)/secsize, clea%secsize,
					cmd^lba, cleis);
				printdefect(lba, 2);
				return;
			}
TRKcont:		if((c=nwgch()) != -1) {
				if(c == ' ')
					printf("%d\n", lba);
				else
					return;
			}
			if(lba && (lba % (ns*HD*10)) == 0)
				printf("%3d ",lba/(ns*HD));
	    	}
		Tlbad[dunit] += nbad;
		Tlret[dunit] += retries;
		printf("LOOP: %d.   Unit: %d    retries: %d     BAD: %d\n",
			loopcnt, dunit, nbad, retries);
		if(!altunit)
			printf("ACCUMLATED   RETRIES: %d     BAD: %d\n",
				Tlret[dunit], Tlbad[dunit]);
		else {
			for(first=0;first < 2; first++) {
			  printf("UNIT %d ACCUMLATED   RETRIES: %d   BAD: %d\n",
				first, Tlret[first], Tlbad[first]);
			}
			first = 0;
		}
		nbad = retries = 0;
	}
	incompex = 0;
}

printdefect(lba, what)
	int lba;
	UCHAR what;
{
	explode(lba);
	printf(" WARNING SEC NOT FOUND    MULTIPLE TRACK (%s) UNIT %d \n",
		what == 0? "Write":(what == 1? "Read": "Data"), dunit);
	printf("   *** ERROR at lba = %d, cyl/hd/sec= %d/%d/%d ***\n",
		lba, dcyl, dhd, dsec);
	return 0;
}
