/*
** fex.c	- Copyright (C) Silicon Graphics - Mountain View, CA 94043
**		- Author: chase
**		- Date: April 1985
**		- Any use, copy or alteration is strictly prohibited
**		- and gosh darn awfully -- morally inexcusable
**		- unless authorized by SGI.
**
** $Author: root $
** $State: Exp $
** $Source: /d2/3.7/src/pm2stand/mdfex/RCS/mdfex.c,v $
** $Revision: 1.1 $
** $Date: 89/03/27 17:11:45 $
*/

/* Version:
** 4.1  Jun 04, 1984 	From BSB.
** 4.2	Aug 10, 1984	Added the tape fex stuff. Added fux.
** 4.3  Aug 30, 1984	Added the Fujitsu 2243.
** 4.4  Sep 24, 1984	Clean up for Release and Easier use for non-engineers.
**			Capabilities are still there.
** 4.5  Oct 17, 1984	Added the 85MB Maxtor and removed some older drives.
** 4.6  Dec 10, 1984	Added special test for Sammy.
** 5.0  Jan  9, 1985	Added V185 disk to the data.c and Increased buf size.
** 5.1  Feb  5, 1985	Added the CDC Wren II 86MB disk.
** 5.2  Feb 19, 1985	Added a memory test for PM2 boards.
**			Started the addition of Screen Attributes.
** 5.3  Apr 18, 1985	No screen attributes, yet. Support GL1 and GL2.
** 5.4  June 12, 1985	Redid the Standalone Libraries, Added SMD stuff,
**			cleaned up dklabel.h data.c. Added Tandon 362.
** 5.5  July 24, 1985	Added a lot of 20 MByte Disks.
** 5.6  Aug   8, 1985	Cleaned up some Herb Stuff, made 362 == 262, Added 5214
*/

extern char *version;
extern char *compdate;

#include <sys/types.h>
#include <setjmp.h>
jmp_buf rgoto;
#include <sys/dklabel.h>
#include "dsdreg.h"
#include "disk.h"
#include "fex.h"

ULONG	ioport = 0x7f00; 	/* I/O Port of controller */
#define BASEADDR 0x20000	/* PMII 128KB Base */

USHORT	secsize = 512;		/* Size of a sector */
char	dunit = 0;		/* Which disk unit on that controller */
char	tunit = 0;		/* Which Tape unit on that controller */
char	verbose = 0;		/* On->lots of verbose output */
char	emode = 0;
char 	firmretry = 1;		/* Firmware retries are disabled */
char	Retry = 0;		/* Error Retries */
char	incompex = 0;		/* Error printout */
char	writelock = 1;		/* Write lock starts on */
ULONG	lbas_left = 0;		/* Added for 4.6 for tktest */
char	secnotfound = 0;	/* Added for 4.6 for tktest */
USHORT switches;
char autosetup;
char graphicsscreen	= 0;
char *emodes[] = {
	"cyl/hd(/sec)",
	"logical block",
};

struct dtypes *dtype = &dtypes[DEFDISK];
struct drive *drivep = &drives[0];

#undef DEBUG
main()
{
	(void) spl7();
	MULTIBASE = (char *) BASEADDR;

	/* args mbpage, npages, physpage */
	setmbmap(0x10,0x70,0x10);		/* PMII mapping */
	/* Probe for graphics and set the bit if necessary */
	kbprobe();

restart:
	screenclear();
	printf("  SGI Formatter/Exerciser  Version: %s  %s\n",
			version, compdate);
	(void) initl();
	if(setjmp(rgoto)) {
		printf("\n");
	}
	for(;;) {
		printf("mdfex %s> ", version);
		switch(gch()) {
		case ' ':
		case '\n':	printf("\n"); continue;
		case 'b':	bads(); break;
		case 'c':	copy(); break;
		case 'e':	exercise(); break;
		case 'f':	format(); break;
		default:	help(); break;
		case 'i':	finit(); break;
		case 'm':	mapbad(); break;
		case 'q':	quit(); break;
		case 'r': 	sd_flags = 0; goto restart;
		case 's':	setv(); break;
		case 't':	tapex(); break;
		case 'u':	fupdate(); break;
		case 'x':	tapeops(); break;
		case 'C':	screenclear(); break;
		case 'K':	kill(); break;
		case 'M':	memtest(); break;
		case 'E':	derrs(); break;
		}
	}
}

finit()
{
	if (verbose)
		printf("Forced reinit of the Controller\n");
	else
		printf("\n");
	sd_flags = 0;
	config();
}

initl()
{
	register struct disk_label *l = (struct disk_label *)BUF0;

	if (verbose)
		printf("Initialize drive %d\n", dunit);
	if (isinited[dunit] && !isuptodate[dunit]) {
		printf("***Warning: in-core label newer than disk copy.\n");
		printf("***Clobber in-core label? ");
		if(confirm() == 0)
			return 1;
	}
	bcopy((char *)dtype->tlabel, (char *)&drivep->label,
		sizeof (struct disk_label));
	drivep->spc = HD * SEC;
	drivep->ncyl = drivep->label.d_altstart / SPC;
	drivep->tdev = D_WIN;			/* Must Be */
	if (config())
		return 1;
	l->d_magic = 0;
	if(verbose) { 
		printf("Reading label...%x\n", BUF0);
	}
	(void) rdata(1+NBBMBLK, 0, (char *)BUF0); /* Get disk label & BBM*/
	if(l->d_magic == D_MAGIC) {		/* Good label! */
		if(!quiet)
		    printf("    *** Drive: %d  Name: %s, Serial: %s\n",
			dunit, l->d_name, l->d_serial);
		bcopy((char *)l, (char *)&drivep->label,
			sizeof (struct disk_label));
		lcopy((long *)(BUF0+secsize), (long *)drivep->bbm, NBBMBLK*128);
		for(dtype = dtypes; dtype->tname; dtype++)
			if(dtype->ttype == l->d_type)
				goto gotcha;
		printf("Warning: no drive type matches label type\n");
	gotcha:
		isinited[dunit] = 1;
		isuptodate[dunit] = 1;
	} else {
		printf("   *** WARNING: No label-- ***");
		type();				/* Ask for drive type */
		printf("  Using default label\n");
		bcopy((char *)dtype->tlabel, (char *)&drivep->label,
			sizeof (struct disk_label));
		filll((long *)drivep->bbm, (long)0, NBBMBLK*128);
		isinited[dunit] = -1;
		isuptodate[dunit] = 0;
	}
	drivep->spc = HD * SEC;
	drivep->ncyl = drivep->label.d_altstart / SPC;
	if((ilv = drivep->label.d_interleave) == 0)
		ilv = 1;
	finit();
	return 0;
}

updatelabel()
{
	register struct disk_label *l = (struct disk_label *)BUF0;
	register struct disk_bbm *b = drivep->bbm;

	filll((long *)BUF0, (long)0, 128*(1+NBBMBLK));	/* Fill BUF0 with 0 */
	bcopy((char *)&drivep->label, (char *)l,
		sizeof (struct disk_label));
/*	*l = drivep->label;			/* Stuff in label */
						/* Stuff in bbm */
	lcopy((long *)b, (long *)(BUF0+secsize), NBBMBLK*128);
	if(wdata(1+NBBMBLK, 0, (char *)BUF0)) {	/* Write the label */
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
printf("     *      mdfex Commands      *\n");
printf("     b    - enter bad block edit mode\n");
printf("     c    - copy data\n");
printf("     e    - run drive read/write/seek tests\n");
printf("     f    - format the selected drive\n");
printf("     h    - print this message\n");
printf("     m    - map out a bad track\n");
printf("     q    - quit return to IRIS monitor\n");
printf("     r    - restart the test & read the label.\n");
printf("     s    - set miscellaneous variables\n");
printf("     t    - tape copy to disk utility\n");
printf("     u    - update disk label\n");
printf("     C    - Clear the Screen\n");

#ifdef RELEASED
printf("     date       - display the date\n");
printf("     initialize - initialize controller\n");
printf("     xtra       - tapeops\n");
printf("     Errors     - Error Display/Reset\n");
printf("     Memtest    - Special test for Multibus Testing\n");
#endif RELEASED
}


format()
{
	register char *cp;
	register struct disk_bbm *b = drivep->bbm;
	register i;
	short badi = drivep->label.d_badspots;

	printf("Format disk.\n");
	if(!quiet && drivep->tdev==D_WIN)
	  printf("***WARNING -- ALL DATA ON UNIT %d WILL BE LOST!!!\n", dunit);
	pd();
	if(config())
		return;
	if(drivep->tdev == D_WIN && setupbads())	/* Map bad to good */
		return;
	if(drivep->tdev == D_WIN && badi == 0 && !quiet)
		printf("***WARNING -- No bad blocks!!!\n");
	printf("Type 'go<return>' to start...");
	cp = getline();
	if(!uleq(cp, "go")) {
		printf(" Aborted.\n");
		return;
	}
	if(!quiet)
		printf("\nStarting format...\n");
	if(fmt()) return;
	if(drivep->tdev == D_WIN && badi && !quiet)
		printf("Formatting bad tracks...\n");
	for(i = 0; i < badi; i++)
		if(b[i].d_good)
			fmtb((int)b[i].d_bad, (int)b[i].d_good);
	if(drivep->tdev == D_WIN) {
		if(!quiet)
			printf("Writing label...\n");
		updatelabel();
	}
	printf("Formatting complete.\n");
}

mapbad()
{
	register struct disk_label *l = &drivep->label;
	register struct disk_bbm *b = drivep->bbm;
	register badi;
	register bad, i, good;
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
	for(i = 0; i < badi; i++)
		if(b[i].d_bad == bad) {
			if((good = b[i].d_good) == 0)
				break;
			goto ok;
		}
	printf("  Internal error: Bad track not in list or not mapped!!\n");
	return;
ok:	(void) config();
	filll((long *)BUF0, 0, 128 * SEC);	/* Zap the track */
	if(!quiet) printf("  Reading old data into memory...\n");
						/* Go read current data */
	for(i = 0; i < SEC; i++) {
		tries = 0;
		for(;;) {
			if(rdata(1, bad+i, (char *)BUF0+(i*secsize)) == 0)
				break;		/* Read OK */
			if((++tries % 3) == 0)
				(void) config();
			if((tries % 10) == 0)
				switch(rsq("Old data read error")) {
					case RETRY: continue;
					case SKIP:  goto exit;
					case QUIT:  return;
				}
		}
	}
exit:
	if(!quiet) printf("  Re-formatting bad track and its alternate...\n");
	fmtb(bad, good);			/* Do the reformatting */
	if(!quiet) printf("  Re-writing data to new track...\n");
	tries = 0;
	for(;;) {				/* Write the data back */
		if(wdata(SEC, bad, (char *)BUF0) == 0)
			break;
		if((++tries % 3) == 0)
			(void) config();
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
			prevhd = atoi(sp, 10);	/* Save previous hd */
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
/*		if(verbose) {		*/
			printf(":"); pb1(b[i].d_good, 0, 1);
/*		}			*/
		++n;
		if(i+1 == badi) {
			printf("\n");
			return;
		}
		if((n%4) == 0)
			printf("\n");
	}
}

pb1(b, secf, dofmt)
long b;
{
	register c, h, s;

	switch(emode) {
	case 0:
		c = b / SPC;
		h = b % SPC;
		s = h % SEC;
		h /= SEC;
		if(dofmt) {
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

screenclear()
{
	if (graphicsscreen) {
		clearscreen();
		return;
	}
	printf("v");		/* Visual 50 */
}
