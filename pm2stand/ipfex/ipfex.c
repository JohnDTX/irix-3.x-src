/*
** ipfex.c	- Copyright (C) Silicon Graphics - Mountain View, CA 94043
**		- Author: chase
**		- Date: jan 1984
**		- Any use, copy or alteration is strictly prohibited
**		- and gosh darn awfully -- morally inexcusable
**		- unless authorized by SGI.
**
**	$Source: /d2/3.7/src/pm2stand/ipfex/RCS/ipfex.c,v $
**	$Revision: 1.1 $
**	$Date: 89/03/27 17:11:19 $
*/
/* Version:
** 1.1	jan 1984	Initial Version
** 1.2  Aug 1984	Added a bunch of stuff
** 1.3  Aug 30, 1984	Added the tape stuff for standalone disk creatation.
** 2.0  Sep 24, 1984	Cleaned up for release.
** 2.1  Dec 12, 1984	Added support for 4 disks for SAMMY and Test.
** 2.2  Jan  9, 1985	Made buf larger to support whole cylinder reads/writes.
** 2.3	Mar  9, 1985
** 2.4	May  6, 1985	Updated to support GL2 and GL1.
** 2.5  May 13, 1985	Corrected bug with fmtb. and %n.
** 2.6  June 10, 1985	Testing the clock/date functions with the AIM Drive.
*/

char *Vers = "2.6";
char *DATE = "June 7, 1985";
extern char *version;
extern char *compdate;

#include <sys/types.h>
#include <setjmp.h>
jmp_buf rgoto;
#include <sys/dklabel.h>
#include "ipreg.h"
#include "dsdreg.h"
#include "disk.h"
#include "test.h"

long	ioport = 0x7010; 	/* I/O Port of controller */
long	mdioport = 0x7f00;	/* I/O port of the dsd board */
#define IOBASE 0xf70000
long  ip_ioaddr;
long  md_ioaddr;


USHORT	secsize = 512;		/* Size of a sector */
char	dunit = 0;		/* Which disk unit on that controller */
char	numunits = 1;
char	verbose = 0;		/* On->lots of verbose output */
ULONG	emode = 0;
char 	firmretry = 1;		/* Firmware retries are enabled */
char	writelock = 1;		/* Write lock starts on */
char	directmode = 0;		/* Direct is not on */
char	groupsize = 11;
#ifdef NOTDEF
char	groupsize = 22;
#endif
char	cacheenable = 1;
ULONG	graphicsscreen = 0;
long	spiralskew;
long 	gap1;
long 	gap2;
char *emodes[] = {
	"cyl/hd",
	"logical block",
};

#define DEFDISK 0				/*** EAGLE ***/
struct dtypes *dtype = &dtypes[DEFDISK];
struct drive *drivep = &drives[0];

main()
{
	extern char end[2];

	asm("movw #0x2700,sr");
	MULTIBASE = (char *)((((int)end) + 511) & ~511);
	if((int)MULTIBASE < 0x10000)
		MULTIBASE = (char *)0x10000;	/* Can't DMA below 64K !! */

	/* args mbpage, npages, physpage */
	setmbmap(0x10,0x70,0x10);		/* BUS MAP: 64K - 512K */
	kbprobe();

restart:
	ip_ioaddr = IOBASE+ioport;
	md_ioaddr = IOBASE+mdioport;
	screenclear();
	printf("    SGI IPFEX     Version: %s\n", version);
	printf("                  Compiled: %s\n", compdate);
	(void) initl();
	if(setjmp(rgoto))
		printf("\n");
	for(;;) {
		printf("ipfex %s> ", version);
		switch(gch()) {
		case ' ':
		case '\n':	printf("\n"); continue;
		case 'b':	bads(); break;
		case 'c':	copy(); break;
		case 'e':	exercise(); break;
		case 'f':	format(); break;
		case 'h':
		case '?':
		default:	help(); break;
		case 'i':	(void) initl(); break;
		case 'm':	mapbad(); break;
		case 'q':	quit(); break;
		case 'r':	goto restart;
		case 's':	setv(); break;
		case 'u':	fupdate(); break;
		case 't':	tapex(); break;
		case 'v':	verify(); break;
		case 'C':	screenclear(); break;
		case 'E':	derrs(); break;
		case 'I':	fipinit(); break;
		case 'R':	report(); break;
		case 'T':	tapeops(); break;
		}
	}
}

fipinit()
{
	printf("Init\n");
	ip_flags = 0;
	ipinit();
}

initl()
{
	register struct disk_label *l = (struct disk_label *)BUF0;

	if(!quiet) printf("Initialize drive %d\n", dunit);
	if(isinited[dunit] && !isuptodate[dunit]) {
		printf("***Warning: in-core label newer than disk copy.\n");
		printf("***Clobber in-core label? ");
		if(confirm() == 0)
			return 1;
	}
	drivep->label = *dtype->tlabel;
	drivep->spc = HD * SEC;
	drivep->ncyl = drivep->label.d_altstart / SPC;
	if(ipinit()) {
		printf("initl: failed call to ipinit\n");
		return 1;
	}
	l->d_magic = 0;
	if(verbose)				/*** DEBUG ***/
		printf("Reading label...%x\n", BUF0);
	rdata(1+NBBMBLK, 0, BUF0);		/* Get disk label & BBM*/
	if(l->d_magic == D_MAGIC) {		/* Good label! */
		if(!quiet)
		    printf("  Name: %s, Serial: %s\n", l->d_name, l->d_serial);
		drivep->label = *l;		/* Copy it */
		lcopy((long *)(BUF0+secsize), (long *)drivep->bbm, NBBMBLK*128);
		isinited[dunit] = 1;
		isuptodate[dunit] = 1;
	} else {
		printf("  No label--");
		type();				/* Ask for drive type */
		printf("  Using default label\n");
		drivep->label = *dtype->tlabel;
		filll((long *)drivep->bbm, (long)0, NBBMBLK*128);
		isinited[dunit] = -1;
		isuptodate[dunit] = 0;
	}
	drivep->spc = HD * SEC;
	drivep->ncyl = drivep->label.d_altstart / SPC;
	if((ilv = drivep->label.d_interleave) == 0)
		ilv = 1;
	groupsize = drivep->label.d_misc[2];
	gap1 = drivep->label.d_misc[0];
	gap2 = drivep->label.d_misc[1];
	spiralskew = drivep->label.d_cylskew;
	ip_flags = 0;
	return 0;
}

updatelabel()
{
	register struct disk_label *l = (struct disk_label *)BUF0;
	register struct disk_bbm *b = drivep->bbm;
	register i;

	filll((long *)BUF0, (long)0, 128*(1+NBBMBLK));	/* Fill BUF0 with 0 */
	*l = drivep->label;			/* Stuff in label */
						/* Stuff in bbm */
	lcopy((long *)b, (long *)(BUF0+secsize), NBBMBLK*128);
	if(wdata(1+NBBMBLK, 0, BUF0)) {		/* Write the label */
		printf("Can't write label & bad block map!\n");
		return;
	}
	isinited[dunit] = 1;		/* Really initted */
	isuptodate[dunit] = 1;
}

fupdate()
{
	printf("Updating label and bad block info on drive %d\n", dunit);
	updatelabel();
}

qupdate()
{
	int sunit = dunit;

    for(dunit = 0; dunit < 2; dunit++) {
	if(!isinited[dunit] || isuptodate[dunit]) continue;
again:
	drivep = &drives[dunit];
	printf("Label on drive %d needs updating... do it? ", dunit);
	switch(gch()) {
	case 'y':	printf("Yes\n"); updatelabel(); break;
	case 'n':	printf("No\n"); break;
	default:	printf("Answer y/n please\n"); goto again;
	}
    }
    dunit = sunit;
    drivep = &drives[dunit];
    ip_flags = 0;
}

nwgch()
{
	register char i;

	i = nwgetchar();
	if (i == '\r')
		i = '\n';
	return i;
}

gch()
{
	register i;

	if((i = negetchar()) == 0177)
		longjmp(rgoto, 1);
	if (i == '\r')
		i = '\n';
	return i;
}

help()
{
printf("\n     *** Main Menu Commands ***\n");
printf("    b     - bad block edit mode\n");
printf("    c     - copy data\n");
printf("    e     - exerciser\n");
printf("    f     - format the selected drive\n");
printf("    h     - help\n");
printf("    i     - initialize drive & read label\n");
printf("    m     - map out a bad track\n");
printf("    q     - quit return to monitor\n");
printf("    r     - restart\n");
printf("    s     - set variables\n");
printf("    t     - tape copy to disk\n");
printf("    C     - Clear the Screen\n");
}


format()
{
	register char *cp;
	register struct disk_bbm *b = drivep->bbm;
	register i;
	short badi = drivep->label.d_badspots;

	printf("Format disk.\n");
	printf("***WARNING -- ALL DATA ON UNIT %d WILL BE LOST!!!\n", dunit);
	pd();
	if(ipinit())
		return;
	if(setupbads())			/* Map bad to good */
		return;
	if(badi == 0)
		printf("***WARNING -- No bad blocks!!!\n");
	printf("Type 'go<return>' to start...");
	cp = getline();
	if(!uleq(cp, "go")) {
		printf(" Aborted.\n");
		return;
	}
	printf("\nStarting format...\n");
	if(fmt()) return;
	if(badi) printf("Formatting bad tracks...\n");
	for(i = 0; i < badi; i++)
		if(b[i].d_good)
			fmtb((int)b[i].d_bad, (int)b[i].d_good);
	printf("Writing label...\n");
	updatelabel();
	printf("Formatting complete.\n");
}

mapbad()
{
	register struct disk_label *l = &drivep->label;
	register struct disk_bbm *b = drivep->bbm;
	register badi;
	register bad, i, good, bb;
	int tries, c, h;

	printf("Map a bad track\n");
	if(isinited[dunit] < 1) {
		if(initl())
			return;
		if(isinited[dunit] < 1) {
			printf("Drive has no label!! Use \"Format\".\n");
			return;
		}
	} else
		printf("  Name: %s, Serial: %s", l->d_name, l->d_serial);
	do {
		printf("\n  Bad track: Cylinder: ");
		c = getnum(10, 0, CYL);
	} while(c == -1);
	do {
		printf("  Head: ");
		h = getnum(10, 0, HD);
		printf("\n");
	} while(h == -1);
	bad = c * (SPC) + (h * SEC);
	if(addabad(bad)) {
		printf("\n");
		return;
	}
	if(setupbads())
		return;
	badi = l->d_badspots;
	for(bb = 0; bb < badi; bb++)
		if(b[bb].d_bad == bad) {
			if((good = b[bb].d_good) == 0)
				break;
			goto ok;
		}
	printf("  Internal error: Bad track not in list or not mapped!!\n");
	return;
ok:	ipinit();
	filll(BUF0, 0, 128 * SEC);	/* Zap the track */
	if(!quiet) printf("  Reading old data into memory...\n");
						/* Go read current data */
	for(i = 0; i < SEC; i++) {
		tries = 0;
		for(;;) {
			if(rdata(1, bad+i, BUF0+(i*secsize)) == 0)
				break;		/* Read OK */
			if((++tries % 2) == 0) {
				ip_flags = 0;
				ipinit();
			}
			if(tries == 10) {
				printf("Skipping sector\n");
				break;
			}
			if((tries % 5) == 0) {
				switch(rsq("Old data read error")) {
					case RETRY: continue;
					case SKIP:  break;
					case QUIT:
						b[bb].d_bad = 0;
						b[bb].d_good = 0;
						l->d_badspots--;
						return;
				}
				break;
			}
		}
	}
	if(!quiet) printf("  Re-formatting bad track and its alternate...\n");
	fmtb(bad, good);			/* Do the reformatting */
	if(!quiet) printf("  Re-writing data to new track...\n");
	tries = 0;
	for(;;) {				/* Write the data back */
		if(wdata(SEC, bad, BUF0) == 0)
			break;
		if((++tries % 3) == 0) {
			ip_flags = 0;
			ipinit();
		}
		if((tries % 10) == 0)
			switch(rsq("Data rewrite error")) {
				case RETRY: continue;
				case SKIP:  
				case QUIT: 
					printf("Old data LOST!!\n");
					return;
			}
	}
	if(!quiet) printf("  Rewriting Label\n");
	updatelabel();				/* Write back the label */
	if(!quiet) printf("Track remap complete.\n");
}

setupbads()
{
	register struct disk_label *l = &drivep->label;
	register struct disk_bbm *b = drivep->bbm;
	register i, j, badi = l->d_badspots;
	int c, alt;
	register char *cp;
	register ULONG a = 0;

	if(!badi)
		return 0;
	for(i = 0; i < badi; i++)
		if(b[i].d_good > a)
			a = b[i].d_good;
	if(a == 0)
		a = l->d_altstart;
	else
		a += SEC;
	for(i = 0; i < badi; i++) {
		if(a >= l->d_altstart + l->d_nalternates) {
			printf(" Attempt to alternate out of range\n");
			printf(" Alt=%d, Legal: %d-%d\n", a, l->d_altstart,
				 l->d_altstart+l->d_nalternates);
			return 1;
		}
		if(b[i].d_good)
			continue;
		if(b[i].d_bad >= l->d_altstart &&
		   b[i].d_bad < l->d_altstart + l->d_nalternates)
			continue;
	again:
		for(j = 0; j < badi; j++)
			if(a == b[j].d_bad) {
				a += SEC;
				goto again;
			}
		b[i].d_good = a;
		a += SEC;
	}
	return 0;
}

bads()
{
	register i;

	printf("Bad Block edit, type h for help\n");
    for(;;) {
	printf(" bb> ");
	switch(gch()) {
	case 'h':
	default:
		printf("Help--choose one of\n");
		printf("  add bad blocks\n");
		printf("  clear bad block list\n");
		printf("  edit list\n");
		printf("  print list\n");
		printf("  quit\n");
		printf("  setup alternates\n");
		printf("  zap alternate assignments\n");
		break;
	case 'a':
		abad();
		break;
	case 'c':
		cbad();
		break;
	case 'e':
		ebad();
		break;
	case 'p':
		printf("Print bad blocks:\n");
		pbad();
		break;
	case 'q':
		printf("Quit\n");
		return;
	case 's':
		printf("Setup Alternates\n");
		(void) setupbads();
		break;
	case 'z':
		zbads();
		break;
	}
    }
}

zbads()
{
	register struct disk_bbm *b = drivep->bbm;
	register i, badi = drivep->label.d_badspots;

	printf("Zap assigned alternates");
	if(!confirm())
		return;
	for(i = 0; i < badi; i++)
		b[i].d_good = 0;
}

cbad()
{
	register struct disk_bbm *b = drivep->bbm;
	register i, badi = drivep->label.d_badspots;

	printf("Clear bad block table");
	if(!confirm())
		return;
	for(i=0; i<badi; i++)
		b[i].d_bad = b[i].d_good = 0;
	drivep->label.d_badspots = 0;
}

abad()
{
	register struct disk_bbm *b = drivep->bbm;
	register badi = drivep->label.d_badspots;
	register char *cp, *sp;
	register bad;
	int prevhd = 0;
	char buf[8], *index();

	printf("Add new entries.  Mode %s, end with a blank line:\n",
		emodes[emode]);
	if(verbose) printf("bbm=%x, badi=%d, &drive[%d]=%x\n", b, badi,
		dunit, &drives[dunit]);
	for(;;) {
		printf(" bb add: ");
		cp = getline();
		if(*cp == 0) {
			printf("\n");
			return;
		}
		if((sp = index(cp, '/')) || (sp = index(cp, '.'))) {
			*sp++ = '/';	/* Change '.' to '/' for bparse */
			prevhd = atoi(sp);	/* Save previous hd */
		} else {
			strcpy(buf, "/");
			strcat(buf, itoa(prevhd));
			strcat(cp, buf);
			printf(buf);
		}
		if((bad = bparse(cp, 0)) != -1)
			(void) addabad(bad);
		printf("\n");
	}
}

addabad(bad)
{
	register struct disk_bbm *b = drivep->bbm;
	register badi = drivep->label.d_badspots;
	register i, j;

	if(badi == MAXBBM) {
		printf("Bad Block list FULL (%d)!!\n", MAXBBM);
		return 1;
	}
	for(i = 0; i < badi; i++) {
		if(bad == b[i].d_bad) {
			printf(" already in bad track list");
			return 1;
		}
		if(bad < b[i].d_bad) {
			for(j = ++badi; j > i; j--)
				b[j]=b[j-1];
			b[i].d_good = 0;
			b[i].d_bad = bad;
			goto next;
		}
	}
	b[badi].d_good = 0;
	b[badi].d_bad = bad;
	badi++;
next:
	drivep->label.d_badspots = badi;
	isuptodate[dunit] = 0;
	return 0;
}

bparse(str, secf)
register char *str;
{
	register b;
	register char *cp, *sp = str;
	register unsigned c, h, s;

	switch(emode) {
	case 0:
		for(cp = sp; *cp && *cp != '/' && *cp != ' '; cp++);
		if(*cp != '/' && *cp != ' ') {
	err:		printf(" bad format; use cyl/hd%s",
				secf?"/sec":"");
			return -1;
		}
		*cp++ = 0;
		c = atoi(sp, 10);
		if(c >= drivep->label.d_cylinders) {
			printf(" bad cyl %d: 0 <= cyl < %d", c,
				drivep->label.d_cylinders);
			return -1;
		}
		for(sp = cp; *cp && *cp != '/' && *cp != ' '; cp++);
		if(*cp && !secf)
			goto err;
		if(*cp) *cp++ = 0;
		h = atoi(sp, 10);
		if(h >= HD) {
			printf(" bad hd %d: 0 <= hd < %d", h, HD);
			return -1;
		}
		if(secf) {
			if(!*cp) goto err;
			s = atoi(cp, 10);
			if(s >= SEC) {
				printf(" bad sec %d: 0 <= sec <= %d", s, 
					 SEC);
				return -1;
			}
		} else
			s = 0;
		if(verbose) printf(" %d/%d", c, h);
		if(secf) if(verbose) printf("/%d", s);
		b = c * SPC + h * SEC + s;
		break;
	case 1:	b = atoi(str, 10);
		if(!secf && (b % SEC)) {
			printf(" only lba of track allowed, sector set to 0");
			b -= b % SEC;	/* Round to track */
		}
		break;
	}
	if(b > drivep->label.d_cylinders * SPC) {
		printf(" off end of disk");
		return -1;
	}
	if(verbose) printf("->%d", b);
	return b;
}

ebad()
{
	register struct disk_bbm *b = drivep->bbm;
	register badi = drivep->label.d_badspots;
	register i, j;

	printf("Edit bad blocks:\n\
For each bad block, press 'space' to keep, 'd' to delete, 'q' to quit...\n");
	for(i = 0; i < badi; i++) {
		printf("  bb edit ");
		pb1(b[i].d_bad, 0, 0); printf("? ");
		switch(gch()) {
		case 'k':
		case ' ':
			printf("Kept");
			break;
		case 'd':
			printf("Deleted");
			for(j = i; j < badi; j++)
				b[j] = b[j+1];
			i--; badi--;
			break;
		case 'q':
			printf("Quit\n");
			goto out;
		default:
			printf("Press <space>, d or q please");
			i--;
			break;
		}
		printf("\n");
	}
out:
	drivep->label.d_badspots = badi;
}

pbad()
{
	register struct disk_bbm *b = drivep->bbm;
	register badi = drivep->label.d_badspots;
	register i;
	register n = 0;

	for(i = 0; i < badi; i++) {
		printf("  ");
		pb1(b[i].d_bad, 0, 1);
		if(verbose) {
			printf(":"); pb1(b[i].d_good, 0, 1);
		}
		++n;
		if(i+1 == badi) {
			printf("\n");
			return;
		}
		if((n%4) == 0)
			printf("\n");
	}
}

pb1(b, secf, fmt)
{
	register c, h, s;

	switch(emode) {
	case 0:
		c = b / SPC;
		h = b % SPC;
		s = h % SEC;
		h /= SEC;
		if(fmt) {
			printf("%4d/%d", c, h);
			if(secf) printf("/%2d", s);
		} else {
			printf("%d/%d", c, h);
			if(secf) printf("/%d", s);
		}
		break;
	case 1:	printf("%6d", b);
		break;
	}
}

lcopy(from, to, n)
register long *from, *to, n;
{
	do
		*to++ = *from++;
	while(--n);
}

char *
itoa(n)
register n;
{
	static char buf[16];
	register char *cp = &buf[16];

	*--cp = 0;
	do {
		*--cp = (n%10) + '0';
		n /= 10;
	} while(n);
	return cp;
}

quit()
{
	register c;

	printf("Quit ");
	qupdate();
	printf("--confirm quit with 'y': ");
	if(gch() != 'y') {
		printf("No\n");
		return;
	}
	printf("Yes\n");
	exit(0);
}

confirm()
{
	for(;;) {
		printf("--Confirm: ");
		switch(gch()) {
		case 'y':	printf("Yes\n"); return 1;
		case 'n':	printf("No\n"); return 0;
		}
		printf("Answer y/n please\n");
	}
}

extern long __ismicsw;
kbprobe()
{

	if (__ismicsw) {
		printf("\n *** Graphics Screen ***\n");
		graphicsscreen = 1;
	} else {
		printf("\n *** No Graphics Screen ***\n");
	    	graphicsscreen = 0;
	}
}

/*
** Setup the screen for the test
*/

screenclear()
{
	if (graphicsscreen) {
		clearscreen();
		return;
	}
	printf("v");		/* Visual 50 */
}
