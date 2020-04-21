/*
** sets.c	- Copyright (C) Silicon Graphics - Mountain View, CA 94043
**		- Author: chase
**		- Date: April 1985
**		- Any use, copy or alteration is strictly prohibited
**		- and gosh darn awfully -- morally inexcusable
**		- unless authorized by SGI.
**
** $Author: root $
** $State: Exp $
** $Source: /d2/3.7/src/stand/cmd/mdfex/RCS/sets.c,v $
** $Revision: 1.1 $
** $Date: 89/03/27 17:12:57 $
*/

#include <sys/types.h>
#include <sys/dklabel.h>
#include <sys/dktype.h>
#include "dsdreg.h"
#include "disk.h"
#include "fex.h"

setv()
{
	screenclear();
	printf("\n");
	for(;;) {
		printf(" Set ? ");
		switch(gch()) {
		case '?':
		case 'h':	shelp(); break;
		case 'l':	sfs(); break;
		case 'q':	printf("\n  Quit\n"); return;
		case 't':	type(); break;
		case 'u':	unitd(); break;

		case 'a':	if (password) gioport(); break;
		case 'b':	if (password) spbads(); break;
		case 'c':	if (password) slabel(); break;
		case 'd':	if (password) defaults(); break;
		case 'f':	if (password) fmware(); break;
		case 'i':	if (password) silv(); break;
		case 'm':	if (password) gmode(); break;
		case 'r':	if (password) rnset(); break;
		case 'v':	if (password) verb(); break;
		case 'w':	if (password) wlock(); break;
		case 'Q':	if (password) fquiet(); break;
		case '\n':
		case ' ':
		default:	printf("\n"); continue;
		}
	}
}

shelp()
{
printf("     *** Set commands ***\n\n");
printf("     l - change drive file system information\n");
printf("     q - quit and return to main routine\n");
printf("     t - select type of drive (? for list)\n");
printf("     u - select unit for testing (0/1/f)\n");
if (!password) return;
printf("\n\n");
printf("     a - i/o port (and WUB) address\n");
printf("     b - display current drive bad block list\n");
printf("     c - change drive label information\n");
printf("     d - display settings\n");
printf("     f - disable firmware retries\n");
printf("     i - set interleave\n");
printf("     h - this message\n");
printf("     m - mode of disk address entry\n");
printf("     r - reset random number sequence\n");
printf("     v - verbose (on/off)\n");
printf("     w - write lock switch (on/off)\n");
printf("     Q - quiet all extraneous printout (on/off)\n");
}

spbads()
{
	printf("Bad Block list:\n");
	if(drivep->tdev == D_WIN)
		pbad();
}

gioport()
{
	register i;

	printf("Address of I/O Port (%x): ", ioport);
	if((i = getnum(16, 0, 0xFFFF)) != -1) {
		ioport = i;
		printf(" (WUB set to %x)", i << 4);
	}
	printf("\n");
}

gmode()
{
loop:	printf("Mode cyl/hd/sec or logical block(%d) (%s)? ",
		secsize, emodes[emode]);
	switch(gch()) {
	case 'c':	printf("cyl/hd/sec\n");
			emode = 0;
			break;
	case 'l':	printf("logical block\n");
			emode = 1;
			break;
	case '\n':	printf("\n");
			break;
	default:	printf("please enter c or l\n");
			goto loop;
	}
}

wlock()
{
	register char *cp;

	printf("Write lock (%s)? ", writelock?"On":"Off");
	cp = getline();
	if(*cp)
		if(uleq(cp, "on"))
			writelock = 1;
		else if(uleq(cp, "off"))
			writelock = 0;
		else printf("Enter on/off please");
	printf("\n");
}

fmware()
{
	register char *cp;

	printf("Firmware retrys (%s)? ", firmretry?"On":"Off");
	cp = getline();
	if(*cp)
		if(uleq(cp, "on"))
			firmretry = 1;
		else if(uleq(cp, "off"))
			firmretry = 0;
		else printf("Enter on/off please");
	printf("\n");
}

verb()
{
	register char *cp;

	printf("Verbose (%s)? ", verbose?"on":"off");
	cp = getline();
	if(*cp) {
		if(uleq(cp, "on"))
			verbose = 1;
		else if(uleq(cp, "off"))
			verbose = 0;
		else
			printf(" enter 'on' or 'off'");
	}
	printf("\n");
}

fquiet()
{
	register char *cp;

	printf("Quiet (%s)? ", quiet?"on":"off");
	cp = getline();
	if(*cp) {
		if(uleq(cp, "on"))
			quiet = 1;
		else if(uleq(cp, "off"))
			quiet = 0;
		else
			printf(" enter 'on' or 'off'");
	}
	printf("\n");
}

defaults()
{
	printf("Defaults: ");
	if(verbose) printf("Verbose ");
	if(quiet) printf("Quiet ");
	pad();
	pd();
	plab();
}

pad()
{
	printf("I/O Addr=%x, WUB=%X\n", ioport,
			(ULONG)(ioport<<4));
}

#ifdef NOTDEF
char *DTypes[] = {
	"Atasi 3046","Vertex V170", "Toshiba MK56FB", "(not used)",
	"Maxtor 1085", "CDC Wren II", "Vertex V185", "Hitachi DK-511K",
	"Maxtor 1140", "Micropolis 1325", "Vertex V130", "Fujitsu 2243",
	"NEC 1055", "Tandon 101", "Tandon TM-252", "Qume 592",
	"AST 96202", "AST 96203", "Cynthia D570", "Miniscribe 3212",
	"AIM SMD Dart 130", "Tandon TM-362/TM-262", "(not used)", "(not used)",
	"(not used)", "(not used)", "(not used)", "CMI 3426",
	"NEC D5126", "(not used)", "(not used)", "(not used)",
	"(not used)", "(not used)", "(not used)", "(not used)",
	0
};

char *DCtlr[] = {
	"Qualogy 5217/5214", "(not used)", "(not used)", "(not used)",
	"Storager", "Xylogics 421", "(not used)", "(not used)",
	0
};
#endif NOTDEF

pd()
{
	if(drivep->tdev == D_WIN)
	    printf("  Drive: %s ",
		prdtype(drivep->label.d_type,dk_dtypes,NTYPES));
	else if(drivep->tdev == D_FLP)
	    printf("  Floppy: ");
	else if(drivep->tdev == D_217) {
	    printf("  Tape\n");
	    return;
	}
	printf("Unit=%d, (%d+%d/%d/%d(%d)) ILV=%d\n",
		  dunit, CYL,
		  drivep->label.d_nalternates/drivep->spc,
		  HD, SEC, secsize, ilv);
}

plab()
{
	register struct disk_label *l = &drivep->label;
	register u_char rootfs = (l->d_rootnotboot)? l->d_rootfs: l->d_bootfs;
	register C;
	register i;

	if(drivep->tdev != D_WIN)
		return;
	if(l->d_magic != D_MAGIC) {
		printf("  Drive not initialized\n");
		return;
	}
	C = HD * SEC;
	printf("  Label: Name: %s, Serial: %s\n", l->d_name, l->d_serial);
	printf("         drive: %s, ctrlr: %s\n",
		prdtype(l->d_type,dk_dtypes,NTYPES),
		prdtype(l->d_controller,dk_dcont,NCONTS));
	printf("         %d/%d/%d alt=%d/%d bad=%d, ilv=%d, trkskw=%d, cylskw=%d\n",
		l->d_cylinders, HD, SEC, l->d_altstart/C,
		l->d_nalternates/C, l->d_badspots,
		l->d_interleave, l->d_trackskew, l->d_cylskew);
	printf("\t\tfs     base          size\n");
	for(i = 0; i < NFS; i++)
		if(l->d_map[i].d_size)
		    printf("\t\t%c: %6d(%4d%s), %6d(%4d%s)%s\n", i+'a',
			l->d_map[i].d_base, l->d_map[i].d_base / C,
			(l->d_map[i].d_base % C) ? "+":"",
			l->d_map[i].d_size, l->d_map[i].d_size / C,
			(l->d_map[i].d_size % C) ? "+":"",
			(i == rootfs)? " Root":
			(i == l->d_swapfs)? " Swap":
			(i == l->d_bootfs)? " Boot": "");
}

slabel()
{
	register struct disk_label *l = &drivep->label;
	register i, b, m;
	int C;

	if(drivep->tdev != D_WIN) {
		printf("No label on floppy or tape\n");
		return;
	}
	isuptodate[dunit] = 0;			/* Assume some change */
	printf("Label:");
	getstr("Name", l->d_name, sizeof l->d_name);
	getstr("Serial #", l->d_serial, sizeof l->d_serial);
	printf("\n");
	getdrv();
	getctlr();
	l->d_cylinders = getdec("cylinders", (int)l->d_cylinders, 1, 2048);
	l->d_heads = getdec("heads", HD, 1, 64);
	l->d_sectors = getdec("sectors", (int)SEC, 1, 256);
	C = HD * SEC;
	drivep->spc = C;
	printf("\n");
	l->d_altstart = getdec("alternate cylinder", (int)l->d_altstart/C, 1,
		l->d_cylinders-1) * C;
	drivep->ncyl = l->d_altstart / C;
	l->d_nalternates = getdec("number of alternate cylinders",
		(int)l->d_nalternates/C, 1, 1024) * C;
	printf("\n");
	l->d_interleave = getdec("interleave", (int)l->d_interleave, 0, 256);
	ilv = l->d_interleave;
	l->d_trackskew = getdec("trackskew", (int)l->d_trackskew, 0, 1024);
	l->d_cylskew = getdec("cylinder skew", (int)l->d_cylskew, 0, 1024);
	printf("\nFile Systems info: lba or cylinder entry? ");
	switch(gch()) {
		case 'l':	printf("lba"); m=0; break;
		case 'c':	printf("cylinder"); m=1; break;
		default:	printf("using cylinder"); m=1; break;
	}
	printf("\n");
	for(i = 0; i < NFS; i++) {
		printf("\t%c: base: ", i+'a');
		b = l->d_map[i].d_base =
			getf((int)l->d_map[i].d_base, 0, (int)C*CYL, C, m);
		printf(" size: ");
		l->d_map[i].d_size =
			getf((int)l->d_map[i].d_size, 0, (int)C*CYL-b, C, m);
		printf("\n");
	}
	l->d_rootfs = grootswap("Root", (l->d_rootnotboot)?
					l->d_rootfs: l->d_bootfs);
	l->d_rootnotboot = 1;
	l->d_swapfs = grootswap("Swap", l->d_swapfs);
	l->d_bootfs = grootswap("Boot", l->d_bootfs);
	printf("\n");
}

grootswap(p, n)
char *p;
{
	register c;

	printf("\t%s: ", p);
	if(n >= 0)
		printf("(%c) ", n + 'a');
	else
		printf("(None) ");
	c = gch();
	if(c >= 'a' && c <= 'h') {
		printf("%c", c);
		return c - 'a';
	} else if(c == '0' || c == 'n') {
		printf("None");
		return -1;
	} else if(c != '\r' && c != '\n')
		printf("\007");
	return n;
}

getdrv()
{
	register struct disk_label *l = &drivep->label;
	register i = 0;
	register j = NTYPES;

	printf("  Drive: (%s)\n", prdtype(l->d_type,dk_dtypes,NTYPES));
	while (--j) {
		printf("    %d: %s", i, prdtype(i, dk_dtypes, NTYPES));
		if((++i % 4) == 0)
			printf("\n");
	}
	printf("  #? (%d) ", l->d_type);
	i = getnum(10, 0, --i);
	if(i != -1)
		l->d_type = i;
	printf("\n");
}

getctlr()
{
	register struct disk_label *l = &drivep->label;
	register i = 0;
	register j = NCONTS;

	printf("  Controller: (%s)\n",
		prdtype(l->d_controller,dk_dcont,NCONTS));
	while (--j) {
		printf("    %d: %s", i, prdtype(i,dk_dcont,NCONTS));
		if((++i % 3))
			printf("\n");
	}
	printf("  #? (%d) ", l->d_controller);
	i = getnum(10, 0, --i);
	if(i != -1)
		l->d_controller = i;
	printf("\n");
}

getstr(p, b, l)
char *p, *b;
{
	register char *cp;

again:
	printf("  %s: (%s) ", p, b);
	cp = getline();
	if(strlen(cp)+1 > l) {
		printf("Limit %d characters\n", l);
		goto again;
	}
	if(*cp)
		strcpy(b, cp);
}

getdec(p, n, l, h)
char *p;
{
	register i;

	printf("  %s: (%d) ", p, n);
	i = getnum(10, l, h);
	return (i == -1)? n : i;
}

getf(n, l, h, c, m)
{
	register i;

	if(m) {
		printf("(%4d) ", n/c);
		i = getnum(10, l/c, h/c);
		return (i == -1)? n : i*c;
	} else {
		printf("(%6d) ", n);
		i = getnum(10, l, h);
		return (i == -1)? n : i;
	}
}

unitd()
{
	register i;
	register char *cp;

	printf("  Unit (%c)= ", drivep->tdev==D_FLP?'f':
		drivep->tdev==D_217?'t':'0'+dunit);
	cp = getline();
	if(*cp) {
		if(*cp == '0' || *cp == '1') {
			i = *cp - '0';
			printf("\n");
			if(dunit != i)
				qupdate();
			dunit = i;
			drivep = &drives[i];
			sd_flags = 0;
			if(!isinited[i])
				(void) initl();
		} else if(*cp == 'f') {
			if(drivep->tdev != D_FLP)
				qupdate();
			dunit = 0;
			drivep = &floppydrive;
			dtype = &dtypes[FLOPPY];
			bcopy((char *)dtype->tlabel, (char *)&drivep->label,
				sizeof (struct disk_label));
			drivep->spc = HD * SEC;
			drivep->ncyl = drivep->label.d_altstart / SPC;
			drivep->tdev = dtype->tdev;
			printf("\n");
		}
	} else
		printf("\n");
}

silv()
{
	register i;

	printf("Interleave (%d)= ", ilv);
	if((i = getnum(10, 0, 4)) != -1)
		ilv = i;
	printf("\n");
}

unitt()
{
	register i;

	printf("Tape Unit (%d)= ", tunit);
	if((i = getnum(10, 0, 1)) != -1)
		tunit = i;
	printf("\n");
}

type()
{
	register char *cp;
	register i;

loop:
	printf("Type of drive (%s)? ", dtype->tname);
	cp = getline();
	if(*cp == 0) {
		printf("\n");
		return;
	}
	if(*cp == '?')
		goto help;
	for(i = 0; dtypes[i].tname; i++)
		if(dtypes[i].tdev == D_WIN && uleq(cp, dtypes[i].tname)) {
			dtype = &dtypes[i];
#ifdef NEEDSWORK
			if(uleq(cp, "special"))
				getspcl();
#endif
			if(isinited[dunit] < 0) {
				drivep->label = *dtype->tlabel;
				drivep->spc = HD * SEC;
				drivep->ncyl = drivep->label.d_altstart/
					drivep->spc;
				ilv = drivep->label.d_interleave;
				drivep->tdev = dtype->tdev;
			}
			printf("\n");
			return;
		}
	printf(" Unknown. ");
help:	printf("Types are:\n");
	for(i = 0; dtypes[i].tname; i++)
		if(dtypes[i].tdev == D_WIN)
			printf("    %s\n", dtypes[i].tname);
	dtype = &dtypes[0];
	goto loop;
}

#ifdef NEEDSWORK
getspcl()
{
	printf("\nEnter each value:");
	do {
		printf("\n  Type(0=Win,1=Flp,2=Tap): ");
		dtype->tdev = getnum(10, 0, 2);
	} while(dtype->tdev == -1);
	if(dtype->tdev == 2)
		dtype->tdev = 0x10;
	do {
		printf("\n  Cylinders:  ");
		dtype->tcyl = getnum(10, 1, 2000);
	} while(dtype->tcyl == -1);
	if(dtype->tdev == D_WIN)
		do {
			printf("\n  Alternate Cylinders:  ");
			dtype->tacyl = getnum(10, 1, 255);
		} while(dtype->tacyl == -1);
	if(dtype->tdev == D_FLP)
		do {
			printf("\n  Single/Double (0/1): ");
			dtype->tacyl = getnum(10, 0, 1);
		} while(dtype->tacyl == -1);
	do {
		printf("\n  Heads:      ");
		dtype->thd  = getnum(10, 1, 64);
	} while(dtype->thd == -1);
}
#endif

char *
prdtype(type, dkt, ntypes)
	register u_long type, ntypes;
	register struct dk_type *dkt;
{
	static char buf[40];

	while (ntypes--) {
		if (dkt->d_type == type)
			return (dkt->d_name);
		dkt++;
	}
	sprintf(buf, "unknown disk type (%d)", type);
	return (buf);
}

sfs()
{
	register struct disk_label *l = &drivep->label;
	register i, b, m;
	int C;

	if(drivep->tdev != D_WIN) {
		printf("No label on floppy or tape\n");
		return;
	}
	isuptodate[dunit] = 0;			/* Assume some change */
	C = HD * SEC;
	drivep->spc = C;
	printf("\nFile Systems info: lba or cylinder entry? ");
	switch(gch()) {
		case 'l':	printf("lba"); m=0; break;
		case 'c':	printf("cylinder"); m=1; break;
		default:	printf("using cylinder"); m=1; break;
	}
	printf("\n");
	for(i = 0; i < NFS; i++) {
		printf("\t%c: base: ", i+'a');
		b = l->d_map[i].d_base =
			getf((int)l->d_map[i].d_base, 0, (int)C*CYL, C, m);
		printf(" size: ");
		l->d_map[i].d_size =
			getf((int)l->d_map[i].d_size, 0, (int)C*CYL-b, C, m);
		printf("\n");
	}
	l->d_rootfs = grootswap("Root", (l->d_rootnotboot)?
					l->d_rootfs: l->d_bootfs);
	l->d_rootnotboot = 1;
	l->d_swapfs = grootswap("Swap", l->d_swapfs);
	l->d_bootfs = grootswap("Boot", l->d_bootfs);
	printf("\n");
}
