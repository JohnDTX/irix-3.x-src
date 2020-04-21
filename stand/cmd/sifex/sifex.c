/*
**	sifex.c		- Copyright (C), JCS Computer Services 1983
**			- Author: chase
**			- Date: January 1985
**			- Any use, copying or alteration is prohibited
**			- and morally inexcusable,
**			- unless authorized in writing by JCS.
**	$Source: /d2/3.7/src/stand/cmd/sifex/RCS/sifex.c,v $
**	$Revision: 1.1 $
**	$Date: 89/03/27 17:13:20 $
*/

extern char *version;
extern char *compdate;

#include "types.h"
#include <setjmp.h>
jmp_buf rgoto;
#include <sys/dklabel.h>
#include "streg.h"
#include "disk.h"
#include "fex.h"

long	st_ioport = 0x7200; 	/* I/O Port of controller */
u_long	graphicsscreen = 0;
#define BASEADDR 0x20000	/* PM-68K Multibus Base Address */
#ifdef pmII
#define IOBASE 0xF70000		/* I/O Base Address */
#endif
#ifdef juniper
#define IOBASE 0x50000000		/* I/O Base Address */
#endif
short	password = 0;
short 	passloop = 0;
#define	RANDMULT	5
#define RANDOFFSET	17623
#define RANDOMIZE(r)	r = (r * RANDMULT + RANDOFFSET) & 0xffff
#define SEED		433

char	errhalt = 0;
u_long  esditype[NUNIT];
char	zero = 0;
u_long st_ioaddr;
u_long	ilv = 0;		/* Default interleave */
char	informat = 0;		/* Is the drive in format routines */
char	fmtflag = 0;
u_long	Fmtwait = 100;		/* Format delay in formatting */
u_long	floppy	= 0;

u_short	secsize[NUNIT];
char	dunit = 0;		/* Which disk unit on that controller */
char	verbose = 0;		/* On->lots of verbose output */
u_long	emode = 0;
char 	firmretry = 1;		/* Firmware retries are enabled */
char	writelock = 0;		/* Write lock starts on */
char	cacheenable = 1;	/* Enable cache */
char	zerolatency = 1;	/* Zero latency or Normal sec sequence */
long	spiralskew;
char	stretries = 3;		/* Retries in the storager firmware */
char	steccon = 1;
char	stmvbad = 0;
char	streseek = 1;
char	stinchd = 1;
/*
** Gap 1 -  gap before the header field.
** Gap 2 -  gap between the header and the data field.
** Gap 3 -  gap between sectors.
*/
long 	gap1;			/* Set to 17 per Interphase */
long 	gap2;			/* Set to 17 per Interphase */
long 	gap3;			/* Set to 32-50 per Interphase */
char *emodes[] = {
	"cyl/hd",
	"logical block",
};

#define ST506DISK 0x01				/*** Vertex ***/
#define ESDIDISK  0x12				/*** Wren II ***/
#define HITACHI	  21
#define MAXTOR	  18
struct dtypes *dtype = &dtypes[HITACHI];	/*** Hitachi 512-17 ***/
struct drive *drivep = &drives[0];
long dontupdate = 0;

main()
{
	extern char end[2];
	register i;

	spl7();
#ifdef pmII
	MULTIBASE = (char *)BASEADDR;

	/* args mbpage, npages, physpage */
	setmbmap(0x10,0x70,0x10);		/* BUS MAP: 64K - 512K */
	kbprobe();
#endif
#ifdef juniper
	MULTIBASE = (char *)mbmalloc(0x40000);
#endif

	st_ioaddr = IOBASE+st_ioport;

restart:
	for (i=0; i<NUNIT; i++) {
		secsize[i] = 512;
		esditype[i] = 0;
	}
	passloop = SEED;
	RANDOMIZE(passloop);
	passloop &= 0x07;
	isinited[dunit] = 0;
	scrsetup();
	streset();
	initl();
	if (setjmp(rgoto))
		printf("\n");
	for(;;) {
		printf("sifex %s> ", version);
		switch(gch()) {
		case '\n':
		case ' ':	printf("\n"); continue;
		case '?':
		case 'h':	help(); break;

		case 'n':	netcopy(); break;
		case 'q':	quit(); break;
		case 's':	setv(); break;
		case 't':	tapex(); break;

		case 'a':	if (password) recalibrate(); break;
		case 'b':	if (password) bads(); break;
		case 'c':	if (password) copy(); break;
		case 'd':	if (password) displayiopb(); break;
		case 'e':	if (password) exercise(); break;
		case 'f':	if (password) format(); break;
		case 'g':	if (password) printuib(); break;
		case 'i':	if (password) stinit(); break;
		case 'm':	if (password) mapbad(); break;
		case 'r':	if (password) printf("\n"); goto restart;
		case 'u':	if (password) fupdate(); break;
		case 'v':	if (password) verify(); break;
		case 'x':	if (password) streset(); break;
		case 'A':	if (password) readhdrs(); break;
		case 'F':	if (password) selformat(); break;
		case 'B':	if (password) dumpblk(); break;
		case 'C':	if (password) scrsetup(); break;
		case 'D':	if (password) spindown(); break;
		case 'I':	if (password) initl(); break;
		case 'R':	if (password) report(); break;
		case 'S':	if (password) spinup(); break;
		case 'T':	if (password) taperewind(); break;
		case 'U':	if (password) restor(); break;
		case 'V':	if (password) tapespace(); break;
		case 'X':	if (password) rdtapestatus(); break;
		case 'Y':	if (password) ftapeinit(); break;
		case '#':	if (password) xhelp(); break;
		case 'Z': 	spassword(); break;
		default:	help(); break;
		}
	}
}

fstinit()
{
	if (verbose)
		printf("Forced Re-Init\n");
	streset();
	isinited[dunit] = 0;
	stinit();
}

initl()
{
	register struct disk_label *l = (struct disk_label *)BUF0;
	register short found = 0;
	register char *cp;
	register i;

	if(verbose)
		printf("Initialize drive %d\n", dunit);
	else
		printf("\n");
	if(isinited[dunit] && !isuptodate[dunit]) {
		printf("***Warning: in-core label newer than disk copy.\n");
		printf("***Clobber in-core label? ");
		if(confirm() == 0)
			return 1;
	}
	dtype = &dtypes[HITACHI];	/*** Hitachi 512-17 ***/
	bcopy((char *)dtype->tlabel, (char *)&drivep->label,
		sizeof (struct disk_label));
	drivep->spc = HD * SEC;
	drivep->ncyl = drivep->label.d_altstart / SPC;
	if (stinit()) {
		printf("initl: failed call to stinit\n");
		return 1;
	}
	l->d_magic = 0;
#ifdef NEWSTUFF
	printf("Do you want to read the label on the disk?\n");
	printf("Type 'go<return>' to start...");
	cp = getline();
	if(!uleq(cp, "go")) {
		printf(" Aborted.\n");
		goto cont;
	}
#endif NEWSTUFF
	if (floppy)
		goto cont;
	if (verbose) {
		printf("\n..Reading........\n");
		printf("Trying to Read Hard Sectored ESDI label...%s\n",
			dtype->tname);
	}
	rdata(1+NBBMBLK, 0, BUF0);	/* Get disk label & BBM*/
	if (l->d_magic == D_MAGIC) {		/* Good label! */
		printf("Name: %s, Serial: %s\n", l->d_name, l->d_serial);
		drivep->label = *l;		/* Copy it */
		lcopy((long *)(BUF0+secsize[dunit]), (long *)drivep->bbm, NBBMBLK*128);
		/* added to update drive type to correct type */
 		for(i=0;dtypes[i].tname;i++){ /* darrah */
 			if ((short)dtypes[i].ttype == drivep->label.d_type){
 				dtype = &dtypes[i];
 				dtype->tname = dtypes[i].tname;
 			}
 		} /* darrah */
		isinited[dunit] = 0;
		isuptodate[dunit] = 1;
		found = 1;
	}
	if (!found) {
		isinited[dunit] = 0;
		dtype = &dtypes[MAXTOR];
		drivep->label = *dtype->tlabel;
		drivep->spc = HD * SEC;
		drivep->ncyl = drivep->label.d_altstart / SPC;
		stinit();
		if (verbose)
			printf("Try to Read Soft Sectored ESDI label...%s\n",
				dtype->tname);
		rdata(1+NBBMBLK, 0, BUF0);
		if(l->d_magic == D_MAGIC) {		/* Good label! */
			if(!quiet)
		    		printf("  Name: %s, Serial: %s\n",
				l->d_name, l->d_serial);
			drivep->label = *l;		/* Copy it */
			lcopy((long *)(BUF0+secsize[dunit]),
				(long *)drivep->bbm, NBBMBLK*128);
		/* added to update drive type to correct type */
 			for(i=0;dtypes[i].tname;i++){ /* darrah */
 				if ((short)dtypes[i].ttype == drivep->label.d_type){
 					dtype = &dtypes[i];
 					dtype->tname = dtypes[i].tname;
 				}
 			} /* darrah */
			isinited[dunit] = 0;
			isuptodate[dunit] = 1;
			found = 1;
		}
	}

	if (!found) {
		isinited[dunit] = 0;
		dtype = &dtypes[ST506DISK];
		drivep->label = *dtype->tlabel;
		drivep->spc = HD * SEC;
		drivep->ncyl = drivep->label.d_altstart / SPC;
		stinit();
		if (verbose)
			printf("Try to ST506 label...%s\n", dtype->tname);
		rdata(1+NBBMBLK, 0, BUF0);
		if(l->d_magic == D_MAGIC) {		/* Good label! */
			if(!quiet)
		    		printf("  Name: %s, Serial: %s\n",
				l->d_name, l->d_serial);
			drivep->label = *l;		/* Copy it */
			lcopy((long *)(BUF0+secsize[dunit]),
				(long *)drivep->bbm, NBBMBLK*128);
		/* added to update drive type to correct type */
 			for(i=0;dtypes[i].tname;i++){ /* darrah */
 				if ((short)dtypes[i].ttype == drivep->label.d_type){
 					dtype = &dtypes[i];
 					dtype->tname = dtypes[i].tname;
 				}
 			} /* darrah */
			isinited[dunit] = 0;
			isuptodate[dunit] = 1;
			found = 1;
		}
	}
cont:
	if (!found || floppy) {
		dtype = &dtypes[HITACHI];
		drivep->label = *dtype->tlabel;
		drivep->spc = HD * SEC;
		drivep->ncyl = drivep->label.d_altstart / SPC;
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
	gap1 = drivep->label.d_misc[0];
	gap2 = drivep->label.d_misc[1];
	gap3 = drivep->label.d_misc[2];
	spiralskew = drivep->label.d_cylskew;
	isinited[dunit] = 0;
	if (stinit()) {
		printf("Failure in reinitialization\n");
		return 1;
	}
	return 0;
}

updatelabel()
{
	register struct disk_label *l = (struct disk_label *)BUF0;
	register struct disk_bbm *b = drivep->bbm;
	register i;

	if(fmtflag || dontupdate || floppy) {
		printf("\n");
		return 0;
	}
	filll((long *)BUF0, (long)0, 128*(1+NBBMBLK));	/* Fill BUF0 with 0 */
	*l = drivep->label;			/* Stuff in label */
						/* Stuff in bbm */
	lcopy((long *)b, (long *)(BUF0+secsize[dunit]), NBBMBLK*128);
	if(wdata(1+NBBMBLK, 0, BUF0)) {		/* Write the label */
		printf("Can't write label & bad block map!\n");
		return;
	}
	isinited[dunit] = 1;		/* Really initted */
	isuptodate[dunit] = 1;
}

fupdate()
{
	if(floppy) {
		printf("\n");
		return 0;
	}
	printf("Updating label and bad block info on drive %d\n", dunit);
	updatelabel();
}

qupdate()
{
	int sunit = dunit;

	if(floppy) {
		printf("\n");
		return 0;
	}
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
    isinited[dunit] = 0;
}

#ifdef SGI
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
#else
nwgch()
{
	return nwgetchar();
}
gch()
{
	register i;

	if((i = negetchar()) == 0177)
		longjmp(rgoto, 1);
	return i;
}
#endif SGI

help()
{
printf("\n    *** SIFEX -- Commands ***\n\n");
printf("    n - remote file copy routine\n");
printf("    q - quit to the monitor\n");
printf("    s - set variables\n");
printf("    t - tape copy routine\n");
printf("\n");
if (!password) return;
printf("    b - bad block\n");
printf("    c - copy data\n");
printf("    e - exercise\n");
printf("    f - format\n");
printf("    u - update the disk label\n");
}

xhelp()
{
printf("\n    *** Main Menu -- Commands ***\n");
printf("    a     - recalibrate disk\n");
printf("    b     - bad block                c     - copy data\n");
printf("    d     - display iopb             e     - exercise\n");
printf("    f     - format                   h     - print this message\n");
printf("    g     - byte dump of the uib and the iopb\n");
printf("    i     - initialize drive         m     - map out a bad track\n");
printf("    q     - quit                     r     - restart fer sure\n");
printf("    s     - set variables            u     - update the disk label\n");
printf("    v     - verify                   x     - reset the controller\n");
printf("    A     - read disk headers        B     - dump blocks of disk\n");
printf("    C     - Clear Screen             D     - Spin Down the Siemens\n");
printf("    F     - Selective Format\n");
printf("    I     - Initialize drive         R     - Report firmware level\n");
printf("    S     - Spin up Siemens Drive    z     - Board Reset\n");
printf("    T     - Tape rewind              V     - Tape space file mark test\n");
printf("    X     - Read Tape drive status   Y     - Force Tape drive init\n");
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
	if(stinit())
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
	if(floppy) {
		printf("\nFormatting complete.\n");
		return;
	}
	if(badi) printf("\nFormatting bad tracks...\n");
	for(i = 0; i < badi; i++)
		if(b[i].d_good)
			fmtb(b[i].d_bad, b[i].d_good);
	if (!fmtflag) {
		printf("\nWriting label...\n");
		updatelabel();
	}
	printf("\nFormatting complete.\n");
}

mapbad()
{
	register struct disk_label *l = &drivep->label;
	register struct disk_bbm *b = drivep->bbm;
	register badi;
	register bad, i, good, bb;
	int tries, c, h;
	register char *cp;

	if(floppy) {
		printf("\n");
		return 0;
	}
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
ok:	stinit();
	printf(" Would you like to read the track (yes) (yes/no)? ");
	cp = getline();
	if(uleq(cp, "no") || uleq(cp, "n")) {
		printf(" not reading");
		printf("\n");
		goto exit;
	}
	printf("\n");
	filll(BUF0, 0, 128 * SEC);	/* Zap the track */
	if(!quiet) printf("  Reading old data into memory...\n");
						/* Go read current data */
	for(i = 0; i < SEC; i++) {
		tries = 0;
		for(;;) {
			if(rdata(1, bad+i, BUF0+(i*secsize[dunit])) == 0)
				break;		/* Read OK */
#ifdef NOTDEF
			if((++tries % 2) == 0) {
				isinited[dunit] = 0;
				stinit();
			}
#endif NOTDEF
			if(tries == 2) {
				printf("Skipping sector\n");
				break;
			}
			if((++tries % 5) == 0) {
				switch(rsq("Old data read error")) {
					case RETRY: continue;
					case SKIP:  goto exit;
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
exit:
	if(!quiet) printf("  Re-formatting bad track and its alternate...\n");
	fmtb(bad, good);			/* Do the reformatting */
	if(!quiet) printf("  Re-writing data to new track...\n");
	tries = 0;
	for(;;) {				/* Write the data back */
		if(wdata(SEC, bad, BUF0) == 0)
			break;
#ifdef NOTDEF
		if((++tries % 3) == 0) {
			isinited[dunit] = 0;
			stinit();
		}
#endif NOTDEF
		if((++tries % 2) == 0)
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
	register u_long a = 0;

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

	if(floppy) {
		printf("\n   Floppy with Bad Tracks ???\n");
		return 0;
	}
	printf("Bad Block edit, type h for help\n");
    for(;;) {
	printf("   bb> ");
	switch(gch()) {
	case 'h':
	case '?':
	default:
		printf("     *** Bad Block Commands ***\n");
		printf("     a     - add bad blocks\n");
		printf("     c     - clear bad block list\n");
		printf("     e     - edit list\n");
		printf("     p     - print list\n");
		printf("     q     - quit\n");
		printf("     s     - setup alternates\n");
		printf("     z     - zap alternate assignments\n");
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
		setupbads();
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
			 addabad(bad);
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
	err:		printf(" bad format: use cyl/hd%s",
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

	printf("Quit\n");
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

displayiopb()
{
	screenclear();
	printf("\n");
	printf("*** hex representation ***\n");
	ippp("IOPB", (u_short *)iop, sizeof *iop);
	ippp("UIPB", (u_short *)uib, sizeof *uib);
}

Reformat()
{
	register char *cp;
	register struct disk_bbm *b = drivep->bbm;
	register i;
	short badi = drivep->label.d_badspots;

	printf("Reformatting disk.\n");
	printf("***WARNING -- ALL DATA ON UNIT %d WILL BE LOST!!!\n", dunit);
	pd();
	if(stinit()) return;
	printf("Type 'go<return>' to start...");
	cp = getline();
	if(!uleq(cp, "go")) {
		printf(" Aborted.\n");
		return;
	}
	printf("\nStarting format...\n");
	if(reformat()) return;
	printf("\nWriting label...\n");
	updatelabel();
	printf("\nFormatting complete.\n");
}

#ifdef pmII
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
#endif pmII

/*
** Setup the screen for the test
*/
scrsetup()
{
	screenclear();
	printf(" SIFEX for ESDI/ST412/506 Disk Drives and QIC-02 Tape\n");
	printf(" Interphase Storager Disk/Tape Controller Model 3030\n");
	printf(" ** Version: %s\n", version);
}

selformat()
{
	register char *cp;
	register u_long lba;

	printf("\n  selective format of unit %d\n", dunit);
	if(stinit())
		return;
	do {
		printf("  ** select the cyl/hd ? ");
		cp = getline();
		if(*cp == 0) {
			printf(" Quit\n");
			return;
		}
		else
			lba = bparse(cp, 0);
	} while(lba == -1);
	printf("\n");
	explode(lba);
	printf("  ** type 'go<return>' to format %d/%d ? ", dcyl, dhd);
	cp = getline();
	if(!uleq(cp, "go")) {
		printf("  ** aborted.\n");
		return;
	}
	printf("\n");
	if (fmtsel(dcyl, dhd)) {
		printf("  ** format of %d/%d failed\n", dcyl, dhd);
		return;
	}
	if (dcyl == 0 && dhd == 0)
		updatelabel();
	printf("\n  ** format of %d/%d complete.\n", dcyl, dhd);
}

readhdrs()
{
	register char *cp;
	register u_long lba;
	register char *address;
	register i;

	printf("\n  read disk headers\n");
	if(stinit())
		return;
	do {
		printf("  ** select the cyl/hd ? ");
		cp = getline();
		if(*cp == 0) {
			printf(" Quit\n");
			return;
		}
		else
			lba = bparse(cp, 0);
	} while(lba == -1);
	printf("\n");
	explode(lba);
	printf("  ** type 'go<return>' to read header at %d/%d ? ", dcyl, dhd);
	cp = getline();
	if(!uleq(cp, "go")) {
		printf("  ** aborted.\n");
		return;
	}
	printf("\n");
	if (rdheaders(dcyl, dhd)) {
		printf("  ** read header of %d/%d failed\n", dcyl, dhd);
		return;
	}
	printf("\n  ** read header of %d/%d complete.\n", dcyl, dhd);
	printf(" SEC  DATA\n");
	address = (char *)BUF0;
	for (i=0; i<17; i++) {
		printf(" %2d: ", i);
		dumpmemory(address, 16);
		address += 16;
	}
	printf("\n");
}

printuib()
{
	register char *address;

	screenclear();
	address = (char *)iop;
	printf("\n\n ** the IOPB @ 0x%x **\n\n", iop);
	dumpmemory(address, 16);
	address += 16;
	dumpmemory(address, 16);
	printf("\n\n");
	address = (char *)uib;
	printf("\n\n ** the UIB @ 0x%x **\n\n", uib);
	dumpmemory(address, 16);
	address += 16;
	dumpmemory(address, 16);
	printf("\n\n");
}

screenclear()
{
#ifdef pmII
	if (graphicsscreen) {
		clearscreen();
		return;
	}
#endif
/*	printf("v");		/* Visual 50 */
	printf("");		/* Adds Viewpoint */
}

rdtapestatus()
{
	if (verbose)
		printf("\n  Read tape status\n");
	tapestatus();
	if (!verbose)
		printstatus();
}

extern long tape_flags;
ftapeinit()
{
	printf("\n  Tape init....");
	tape_flags = 0;
	tapeinit();
	printf("\n");
}

taperewind()
{
	printf("\n  rewinding....");
	rewind();
	printf("\n");
}

tapespace()
{
	register i = 1;

	printf("\n  Tape space file mark test\n");
	printf("  How many file marks to space (%d) ? ", i);
	i = (getnum(10, 1, 5));
	printf("\n");
	if (i == 0 || i > 5) {
		printf(" Can't space forward %d file marks\n", i);
		return;
	}
	spacef(i);
	printf("\n");
}

char *passwords[] = {
	"Carter", "Ludwig", "Chase", "Darrah",
	"Donl", "Bradley", "Ellis", "Luttner", 0
};
spassword()
{
	register i;
	char *cp;
	char *xp;

	screenclear();
	i = passloop;
	printf(" *** SECURITY PASSWORD ***\n");
	printf(" enter ");
again:
	if (i == 0)
		printf("Password: ");
	else if (i == 1)
		printf("pAssword: ");
	else if (i == 2)
		printf("paSsword: ");
	else if (i == 3)
		printf("pasSword: ");
	else if (i == 4)
		printf("passWord: ");
	else if (i == 5)
		printf("passwOrd: ");
	else if (i == 6)
		printf("passwoRd: ");
	else if (i == 7)
		printf("passworD: ");
	else {
		i = 0;
		goto again;
	}
	xp = passwords[i];
	cp = getline();
	i++;
	passloop = i & 0x7;
	if(!uleq(cp, xp)) {
		printf("\n sorry\n");
		return;
	}
	password = 1;
	printf("\n accepted\n");
}
