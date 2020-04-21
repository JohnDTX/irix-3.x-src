/*
**	exercise.c	- Copyright (C), JCS Computer Services 1983
**			- Author: chase
**			- Date: January 1985
**			- Any use, copying or alteration is prohibited
**			- and morally inexcusable,
**			- unless authorized in writing by JCS.
**	$Source: /d2/3.7/src/stand/cmd/sifex/RCS/exercise.c,v $
**	$Revision: 1.1 $
**	$Date: 89/03/27 17:13:13 $
*/
#include "types.h"
#include "disk.h"
#include "streg.h"
#include <sys/dklabel.h>
#include "fex.h"

/* Error logging stuff */
#define	NELOG	200	/* Max logs to keep */
#define	LSOFT	1	/* Will retry this error */
#define	LHARD	2	/* This one was HARD */
#define	LDATA	4	/* Data miscompare */

struct	elog {
	u_short	etype;	/* Types of errors for this block */
	u_short	ecnt;	/* Number of errors for this block */
	u_short	eunit;	/* Keep track of these */
	u_long	elba;	/* lba of block (track) */
} elogs[NELOG];
u_long	elogi;		/* How many current logs */
char	*ertypes[] = {
	"None??", "Soft", "Hard", "Soft+Hard",
	"Data", "Soft+Data", "Hard+Data", "Soft+Hard+Data"
};

u_long	cpats[] = {
	0xB1B6DB6D,	0xB1F6DB6D,	0xB1B6D96D,	0xD1D9B6BD,
	0xB1D9fB6D,	0x91F6DE6D,	0xD196D96D,	0xDBD9BEBD
/*	0x55555555,	0xAAAAAAAA,	0x00000000,	0xFFFFFFFF,	*/
/*	0x12345678,	0x87654321,	0xF0F0F0F0,	0x0F0F0F0F	*/
};
u_long	cpati;

#define	RANDMULT	5
#define RANDOFFSET	17623
#define RANDOMIZE(r)	r = (r * RANDMULT + RANDOFFSET) & 0xffff
#define SEED		433
u_short	grand;

exercise()
{
	screenclear();
	printf("  **** EXERCISE ROUTINES ****\n");
	if(!isinited[dunit])
		stinit();		/* Init the disk */
	if(writelock)
		printf("***Warning: set write lock off to scribble on disk\n");
	isuptodate[dunit] = 0;
	for(;;) {
		printf("exercise> ");
		switch(gch()) {
		case 'a': newtest(); break;
		case 'b': dispbuf1(); break;
		case 'c': compex(0); break;
		case 'C': compex(1); break;
		case 'd': diskrw(0); break;
		case 'D': diskrw(1); break;
		case 'e': derrs(); break;
		case 'f': screenclear(); break;
		case 'm': secwr(0); break;
		case 'M': secwr(1); break;
		case 'q': printf("Quit\n"); return;
		case 'r': ranread(); break;
		case 'R': rnset(); break;
		case 's': seek(); break;
		case 'S': seektest(); break;
		case 'v': xverify(); break;
		case 'w': dispbuf0(); break;
		case '\n':
		case ' ':	printf("\n"); break;
		case '?':	xehelp(); break;
		case 'h':
		default:	ehelp(); break;
		}
	}
}

ehelp()
{
	printf("  *** Tests ***\n");
	printf("\n");
	printf("  c - complete write/read multi pass/multi pattern\n");
	printf("  d - disk read or write multiple\n");
	printf("  e - error display/reset\n");
	printf("  m - multiple sector write/read repeated\n");
	printf("  q - quit\n");
	printf("  r - random reads\n");
	printf("  R - Random number reset\n");
	printf("\n");
}
xehelp()
{
	printf("  *** Tests ***\n");
	printf("\n");
/*	printf("  a - newtest to appease Craig's over zealous imagination\n");*/
	printf("  b - display the memory buffer's for READ\n");
	printf("  c - complete write/read multi pass/multi pattern\n");
	printf("  d - disk read or write multiple\n");
	printf("  e - error display/reset\n");
/*	printf("  f - fuck the display - like, nuke it, dude\n");*/
	printf("  m - multiple sector write/read repeated\n");
	printf("  q - quit\n");
	printf("  r - random reads\n");
	printf("  s - seek tests canned\n");
	printf("  v - verify the disk selective\n");
	printf("  w - display the memory buffer's for WRITE\n");
	printf("\n");
	printf("  D - Disk read or write single chunk\n");
	printf("  R - Random number reset\n");
	printf("  S - Seek test selective\n");
	printf("\n");
}

rnset()
{
	grand = SEED;
	printf("Random number reset\n");
}

ranread()
{
	register u_short rand;
	register u_short ns;
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
u_long lba;
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
	register u_long loop, ns, lba;
	u_long seq, cmd, c;
	register char *cp;

	printf("\n*** Disk read or write test ***\n");
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
	printf("\n");
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
		printf("\n # of sectors (%d bytes each): ", secsize[dunit]);
		ns = getnum(10, 1, BUFSIZE*2/secsize[dunit]);
	} while(ns == -1);
	printf("\n");
	do {
		printf(" Where do ya all want to %s (block# or sequential)? ",
			(cmd == 0)? "Read":"Write");
		cp = getline();
		seq = 0;
		if(*cp == 's') {
			printf("equential");
			printf("\n Pick the step size (%d): ", ns);
			seq = getnum(10, 1, BIGNUM);
			printf(" ");
			if(seq == -1) {
				seq = ns;
				printf("%d", ns);
			}
			printf("\n Starting from cyl/hd/sec ? ");
			cp = getline();
			lba = bparse(cp, 1);
		} else if(*cp == 0)
			return;
		else
			lba = bparse(cp, 1);
	} while(lba == -1);
	printf("\n");
	while(loop--) {
		if(cmd == 0) {
			if(rdata(ns, lba, (char *)BUF0)) {
				if (errhalt) {
					isinited[dunit] = 0;
					return;
				}
			}
		} else
			if(wdata(ns, lba, (char *)BUF0)) {
				if (errhalt) {
					isinited[dunit] = 0;
					return;
				}
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
	register u_long loop, ns, bad, nbad;
	register u_short *shp;
	u_long lba, i, c;
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
		printf("\n # of sectors (%d bytes each): ", secsize[dunit]);
		ns = getnum(10, 1, BUFSIZE/secsize[dunit]);
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
mult:	for(i = 0, shp = (u_short *)BUF0; i < ns*(secsize[dunit]/2); i++)
		*shp++ = i;		/* 0 with seq pattern */
    	while(loop--) {
		if(wdata(ns, lba, BUF0)) {/* Write out BUF0 */
			isinited[dunit] = 0;
			continue;
		}
		filll(BUF1, 0x10001, ns*(secsize[dunit]/4));
		if(rdata(ns, lba, BUF1)) {/* Read back the data to Buf1 */
			isinited[dunit] = 0;
			continue;
		}
		for(i=0, shp=(u_short *)BUF1; i < ns*(secsize[dunit]/2); i++) {
			if(*shp++ != i) {
				--shp;
				printf("\nB1: @%d %x->%x\n", ((int)shp)-BUF1,
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
compex(lpcnt)
	char lpcnt;
{
	register u_long loop, altunit = 0;
	u_long nbad, ns, lba, c, i;
	u_long maxlba;
	u_long cmd, loopcnt, Tlbad0, Tlbad1;

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
				altunit = 1;
				printf("Yes");
				dunit = 0;	
				isuptodate[dunit] = 0;
				drivep = &drives[dunit];
				if(stinit()) {
					printf("Can't init disk ctrl: %d\n",							dunit);
					return;
				}
				dunit = 1;
				isuptodate[dunit] = 0;
				drivep = &drives[dunit];
				if(!isinited[i])
					(void) initl();
				drivep->spc = HD * SEC;
				drivep->ncyl = drivep->label.d_altstart/drivep->spc;
				ilv = drivep->label.d_interleave;
				break;
			default:	printf("No");
				break;
		}
		printf("\n");
	} else {
		altunit = 0;
		loop = 1;
	}
	ns = SEC;
	loopcnt = Tlbad0 = Tlbad1 = cpati = 0;
	while(loop--) {
		cmd = cpats[cpati&0x7];
		if(altunit) {		/* New pattern after both drives */
			if(dunit)
				dunit = 0;
			else
				dunit++;
			if(loopcnt & 1)
				cpati++;
		} else {
			cpati++;
		}
		drivep = &drives[dunit];
		printf("Unit %d: Pattern 0x%x\n", dunit, cmd);
		nbad = 0;
/*
 * NOTE: This test aught to write/read cylinders, and then home in on the
 *	bad track(s) if an error is encountered.
 */
		ns = SEC;
		maxlba = CYL*HD*SEC;
	    	for(lba = ns; lba < maxlba; lba += ns) {
			filll(BUF0, cmd^lba, ns*(secsize[dunit]/4));
rewrite:
			/* Write out BUF0 */
			if (wdata(ns, lba, BUF0)) {
				nbad++;
				elog(lba, LHARD);
				if (errhalt) {
					switch(rdwrsq(0)) {
						case 0: goto reread;
						case 1: goto rewrite;
						case 2:  break;
						case 3:  return;
					}
				}
				isinited[dunit] = 0;
				goto CLcont;
			}
reread:
			/* Read back data to Buf1 */
			if (rdata(ns, lba, BUF1)) {
				nbad++;
				elog(lba, LHARD);
				if (errhalt) {
					switch(rdwrsq(0)) {
						case 0: goto reread;
						case 1: goto rewrite;
						case 2:  break;
						case 3:  return;
					}
				}
				isinited[dunit] = 0;
				goto CLcont;
			}
			if (cmpl(BUF1, cmd^lba, ns*(secsize[dunit]/4))) {
				c = (clea - BUF1);
				c /= secsize[dunit];
				c += lba;
				explode(c);
				printf("\n*** Data Error at %d/%d/%d (lba=%d) ",
					dcyl, dhd, dsec, c);
				printf("Mem Add: 0x%x, ", clea);
				printf("DATA S/B: 0x%x, IS: 0x%x\n",
					cmd^lba, cleis);
				elog(lba, LDATA);
				nbad++;
				if (errhalt) {
					switch(rdwrsq(0)) {
						case 0: goto reread;
						case 1: goto rewrite;
						case 2:  break;
						case 3:  return;
					}
				}
			}
CLcont:
			if((c=nwgch()) != -1) {
				if(c == ' ')
					printf("%3d ",lba/(ns*HD));
				else
					return;
			}
			if(lba && (lba % (ns*HD*10)) == 0)
				QP1("%3d ",lba/(ns*HD));
	    	}
		if(dunit == 0) Tlbad0 += nbad;
		else Tlbad1 += nbad;
		loopcnt++;
		printf("\nPass %d Unit %d bad %d ",
			loopcnt, dunit, nbad);
		if(!altunit)
			printf(" total bad %d\n",
				(dunit == 0)? Tlbad0 : Tlbad1);
		else {
			printf("\nTotal Bad for unit 0: %d\n", Tlbad0);
			printf("Total Bad for unit 1: %d\n", Tlbad1);
		}
	}
}

seektest()
{
	register u_long ns, cylin, head, sect;
	register char *cp;
	u_long lba, i;
	u_long loopcnt;

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
			printf("\nerror: lba=%d\n",lba);
			return;
		}
		if(rdata(1,0,BUF0)) {
			printf("\nerror: lba=0\n");
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
	if(stinit()) return;
	printf("\nButterfly\n");
	for(i = 0; i < CYL; i++) {
		if(dseek(i) || dwait()) if(err()) return;
		if(dseek(CYL-i-1) || dwait()) if(err()) return;
		if(nwgch() != -1) return;
	}
	printf("\nSequential up/down. Tic per 100\n");
	if(xwait()) return;
	for(i = 0; i < CYL*2; i++) {
		if(dseek(i<CYL?i:CYL*2-i-1) || dwait())
			if(err()) return;
		if(i && (i%100) == 0) printf(".");
		if(nwgch() != -1) return;
	}
	printf("\nWorst case. Tic per 100\n");
	if(xwait()) return;
	for(i = 0; i < 301; i++) {
		if(dseek(0) || dwait()) if(err()) return;
		if(dseek(CYL-1) || dwait()) if(err()) return;
		if(i && (i%100) == 0) printf(".");
		if(nwgch() != -1) return;
	}
	printf("\nSeek test complete\n");
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

#ifdef NOTDEF
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
				isinited[dunit] = 0;
				return;
			}
		} else
			if(wdata(ns, lba, BUF0)) {
				isinited[dunit] = 0;
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
#endif

xverify()
{
	register i, j, lba;
	register count, loop;
	register char *cp;

	printf("\n Selective Verify of the Disk\n");
	printf("  ** Enter cyl/hd/sec to verify ? ");
	do {
		cp = getline();
		if(*cp == 0)
			return;
		else
			lba = bparse(cp, 1);
	} while(lba == -1);
	printf("\n  ** Enter loops to do ? ");
	if((loop = getnum(10,1,BIGNUM)) == -1) {
		loop = BIGNUM;
		printf(" forever");
	}
	printf("\n  ** Number of sectors to verify ? ");
	if((count = getnum(10,1,256)) == -1) {
		count = drivep->label.d_sectors;
	}
	printf("\n");
	printf("Type 'go<return>' to start...");
	cp = getline();
	if(!uleq(cp, "go")) {
		printf(" Aborted.\n");
		return;
	}
	printf("\nStarting verify...\n");
	j = 0;
	while (loop--) {
		if (selverify(lba,count)) {
			if (errhalt)
				return;
		}
		j++;
		if (j && ((j%10) == 0))
			printf("%3d ", j);
	}
	printf("\n");
}

newtest()
{
	register u_long loop;
	register char *cp;
	register u_long ns;
	register u_long lba;
	register u_long c;
	register u_long pattern;
	long j;

	printf("Special Excercise -- track writes and reads\n");
	if(writelock) {
		printf("Write lock on--reset to scribble on disk\n");
		return;
	}
	printf("Repeat how many times? ");
	loop = getnum(10, 1, BIGNUM);
	if(loop == -1) {
		loop = BIGNUM;
		printf(" FOREVER");
	}
	printf("\n");
	cpati = 0;
	pattern = cpats[cpati&0x7];
	printf("set your pattern (0x%x) ? ", pattern);
	pattern = getnum(16, 0, 0xffffffff);
	if (pattern == -1)
		pattern = cpats[cpati&0x7];
	printf("\n");
	printf("How many sectors to test (%d) ? ", SEC);
	ns = getnum(10, 1, BIGNUM);
	if(ns == -1) {
		ns = SEC;
	}
	printf("\n");
	/* Set up the LBA to test */
	printf("  ** Enter cyl/hd/sec to test ? ");
	do {
		cp = getline();
		if(*cp == 0)
			return;
		else
			lba = bparse(cp, 1);
	} while(lba == -1);
	printf("\n");
	filll(BUF0, pattern, ns*(secsize[dunit]/4));
	printf("Unit %d: Pattern 0x%x\n", dunit, pattern);
	j = 0;
	while(loop--) {
rewrite:
		if (wdata(ns, lba, BUF0)) {
			elog(lba, LHARD);
			if (errhalt) {
				switch(rdwrsq(0)) {
					case 0: goto reread;
					case 1: goto rewrite;
					case 2:  break;
					case 3:  return;
				}
			}
			isinited[dunit] = 0;
			goto CLcont;
		}
		/* Read back data to Buf1 */
reread:
		if (rdata(ns, lba, BUF1)) {
			elog(lba, LHARD);
			if (errhalt) {
				switch(rdwrsq(0)) {
					case 0: goto reread;
					case 1: goto rewrite;
					case 2:  break;
					case 3:  return;
				}
			}
			isinited[dunit] = 0;
			goto CLcont;
		}
		if (cmpl(BUF1, pattern, ns*(secsize[dunit]/4))) {
			c = (clea - BUF1);
			c /= secsize[dunit];
			c += lba;
			explode(c);
			printf("\n*** Data Error at %d/%d/%d (lba=%d) ",
				dcyl, dhd, dsec, c);
			printf("Mem Add: 0x%x, ", clea);
			printf("DATA S/B: 0x%x, IS: 0x%x\n",
				pattern, cleis);

			elog(lba, LDATA);
			if (errhalt) {
				switch(rdwrsq(0)) {
					case 0: goto reread;
					case 1: goto rewrite;
					case 2:  break;
					case 3:  return;
				}
			}
		}
CLcont:		if ((c = nwgch()) != -1) {
			if (c == 'q')
				return;
		}
		if (j && (j%100 == 0)) {
			printf(" %3d", j);
		}
		j++;
	}
}

dispbuf0()
{
	register char *address;
	register u_short count;
	register u_short sector = 0;

	address = (char *)BUF0;
	count = 256;
	screenclear();
	printf(" *** Display WRITE buffer ****\n");
	printf("Enter starting sector ? ");
	sector = getnum(10, 0, SEC-2);
	if (sector == -1) {
		sector = 0;
	}
	address += (sector*512);
	printf("\n");
	for (sector; sector < SEC; sector++) {
		printf("   ******** Sector %d (1st half) ********\n", sector);
		dumpmemory(address, count);
		address += 256;
		printf("press <space> to continue.......");
		switch(gch()) {
			case 'q': printf("quit\n"); return;
			case ' ':
			default: printf("\n"); break;
		}

		printf("   ******** Sector %d (2nd half) ********\n", sector);
		dumpmemory(address, count);
		address += 256;
		printf("press <space> to continue.......");
		switch(gch()) {
			case 'q': printf("quit\n"); return;
			case ' ':
			default: printf("\n"); break;
		}
	}
}

dispbuf1()
{
	register char *address;
	register u_short count;
	register u_short sector = 0;

	address = (char *)BUF1;
	count = 256;
	screenclear();
	printf(" **** Display READ buffer ****\n");
	printf("Enter starting sector ? ");
	sector = getnum(10, 0, SEC-2);
	if (sector == -1) {
		sector = 0;
	}
	address += (sector*512);
	printf("\n");
	for (sector; sector < SEC; sector++) {
		printf("   ******** Sector %d (1st half) ********\n", sector);
		dumpmemory(address, count);
		address += 256;
		printf("press <space> to continue.......");
		switch(gch()) {
			case 'q': printf("quit\n"); return;
			case ' ':
			default: printf("\n"); break;
		}

		printf("   ******** Sector %d (2nd half) ********\n", sector);
		dumpmemory(address, count);
		address += 256;
		printf("press <space> to continue.......");
		switch(gch()) {
			case 'q': printf("quit\n"); return;
			case ' ':
			default: printf("\n"); break;
		}
	}
}
