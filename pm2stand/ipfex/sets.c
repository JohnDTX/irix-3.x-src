/*
** $Source: /d2/3.7/src/pm2stand/ipfex/RCS/sets.c,v $
** $Date: 89/03/27 17:11:25 $
** $Revision: 1.1 $
*/

#include <sys/types.h>
#include <sys/dklabel.h>
#include <sys/dktype.h>
#include "ipreg.h"
#include "dsdreg.h"
#include "disk.h"
#include "test.h"

setv()
{
	for (;;) {
	printf("   Set ? ");
		switch(gch()) {
		case ' ':
		case '\n':	continue;
		case 'a':	defaults(); break;
		case 'b':	spbads(); break;
		case 'c':	scache(); break;
		case 'g':	sgaps(); break;
		case 'i':	silv(); break;
		case 'l':	slabel(); break;
		case 'm':	gmode(); break;
/*		case 'q':	fquiet(); break;	*/
		case 'r':	rnset(); break;
		case 's':	sskew(); break;
		case 't':	type(); break;	/***/
		case 'u':	unitd(); break;
		case 'v':	verb(); break;
		case 'w':	wlock(); break;
		case 'D':	sdirect(); break;
		case 'G':	sgroup(); break;
		case 'q':
		case 'Q':	printf("Quit\n"); return;
		case 'h':
		case '?':
		default:	shelp(); break;
		}
	}
}

shelp()
{
printf("\n");
printf("    *** Set Commands ***\n");
printf("    a     - display settings\n");
printf("    b     - bad block list\n");
printf("    l     - label set up\n");
printf("    q     - quit\n");
printf("    u     - select disk unit for testing (0/1/2/3)\n");
printf("    v     - verbose (on/off)\n");
printf("    w     - write lock switch (on/off)\n");

#ifdef NOTDEF
/* printf("  cache      - set cache on or off and also groupsize\n");	*/
/* printf("  addr       - i/o port address\n");	*/
/* printf("  gaps       - gap settings\n");	*/
/* printf("  interleave - interleave settings\n");	*/
/* printf("  mode       - mode of disk address entry\n");	*/
/* printf("  quiet      - quiet all extraneous printout (on/off)\n");	*/
/* printf("  randreset  - reset random number sequence\n");	*/
/* printf("  skew       - set spiral skew factor\n");	*/
/* printf("  Direct     - Turn on/off direct mode\n");	*/
/* printf("  Groupsize  - Set the groupsize\n");	*/
#endif	NOTDEF
}

spbads()
{
	printf("Bad Block list:\n");
	pbad();
}

gioport()
{
	register i;

	printf("Address of I/O Port (%x): ", ioport);
	if((i = getnum(16, 0, 0xFFFF)) != -1) {
		ioport = i;
		ip_ioaddr = i + 0xf70000;
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

sdirect()
{
	register char *cp;

	printf("Direct Mode (%s)? ", directmode?"On":"Off");
	cp = getline();
	if(*cp)
		if(uleq(cp, "on"))
			directmode = 1;
		else if(uleq(cp, "off"))
			directmode = 0;
		else printf("Enter on/off please");
	printf("\n");
	if(directmode && cacheenable) {
		printf("Can't have cache enabled and directmode on\n");
		printf("I am turning off CACHE\n");
		cacheenable = 0;
	}
	ip_flags = 0;
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

scache()
{
	register char *cp;
	register i;

	printf("\nCache Enable (%s)? ", cacheenable?"on":"off");
	cp = getline();
	if(*cp) {
		if(uleq(cp, "on")) {
			cacheenable = 1;
			directmode = 0;
		} else if(uleq(cp, "off")) {
			cacheenable = 0;
			fipinit();
			printf("\n");
			return;
		}
		else
			printf(" enter 'on' or 'off'");
	}
	printf("\n");
	ip_flags = 0;
	printf("Groupsize (%d)? ", groupsize);
	if((i = getnum(10, 1, 23)) != -1) {
		groupsize = drivep->label.d_misc[2] = i;
		qupdate();
		printf("\n");
	} else
		printf("\n");
}

sgroup()
{
	register i;

	printf("Groupsize (%d)? ", groupsize);
	if((i = getnum(10, 1, 23)) != -1) {
		groupsize = drivep->label.d_misc[2] = i;
		ip_flags = 0;
		qupdate();
		printf("\n");
	} else
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
	pd();
	plab();
}

pd()
{
	printf("  \nDrive: %s Unit: %d  address: %x (%d+%d/%d/%d(%d)) ilv=%d\n",
		  dtype->tname, dunit, ioport, CYL,
		  drivep->label.d_nalternates/drivep->spc,
		  HD, SEC, secsize, ilv);
	printf("  Skew:%d groupsize:%d gap1:%d gap2:%d cache(%s) direct(%s)\n",
		  spiralskew, groupsize, gap1, gap2,
		  cacheenable?"on":"off",
		  directmode?"on":"off");
}


plab()
{
	register struct disk_label *l = &drivep->label;
	register u_char rootfs = (l->d_rootnotboot)? l->d_rootfs: l->d_bootfs;
	register C;
	register i;

	if(l->d_magic != D_MAGIC) {
		printf("  Drive not initialized\n");
		return;
	}
	C = HD * SEC;
	printf("  Label: Name: %s, Serial: %s\n", l->d_name, l->d_serial);
	printf("         drive: %s, ctrlr: %s\n",
		prdtype(l->d_type,dk_dtypes,NTYPES),
		prdtype(l->d_controller,dk_dcont,NCONTS));
	printf("         %d/%d/%d alt: %d/%d bad: %d ilv: %d cylskw: %d\n",
		l->d_cylinders, HD, SEC, l->d_altstart/C,
		l->d_nalternates/C, l->d_badspots,
		l->d_interleave, l->d_cylskew);
	printf("         spiral skew: %d gap1: %d gap2: %d groupsize: %d\n",
		l->d_cylskew, l->d_misc[0], l->d_misc[1], l->d_misc[2]);
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
	register i, b, m, c;
	int C;

	isuptodate[dunit] = 0;			/* Assume some change */
	ip_flags = 0;
	printf("Label:");
	gets("Name", l->d_name, sizeof l->d_name);
	gets("Serial #", l->d_serial, sizeof l->d_serial);
	printf("\n");
	getdrv();
	getctlr();
	l->d_cylinders = getdec("cylinders", l->d_cylinders, 1, 2048);
	l->d_heads = getdec("heads", HD, 1, 64);
	l->d_sectors = getdec("sectors", SEC, 1, 256);
	C = HD * SEC;
	drivep->spc = C;
	printf("\n");
	l->d_altstart = getdec("alternate cylinder", l->d_altstart/C, 1,
		l->d_cylinders-1) * C;
	drivep->ncyl = l->d_altstart / C;
	l->d_nalternates = getdec("number of alternate cylinders",
		l->d_nalternates/C, 1, 1024) * C;
	printf("\n");
	l->d_interleave = getdec("interleave", l->d_interleave, 0, 256);
	ilv = l->d_interleave;
	l->d_trackskew = getdec("trackskew", l->d_trackskew, 0, 1024);
	l->d_cylskew = getdec("spiral skew", l->d_cylskew, 0, 1024);
	spiralskew = l->d_cylskew;
	printf("\n");
	l->d_misc[0] = getdec("gap 1 size", l->d_misc[0], 0, 50);
	l->d_misc[1] = getdec("gap 2 size", l->d_misc[1], 0, 50);
	gap1 = l->d_misc[0];
	gap2 = l->d_misc[1];
	l->d_misc[2] = getdec("groupsize", l->d_misc[2], 0, 23);
	groupsize = l->d_misc[2];
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
			getf(l->d_map[i].d_base, 0, C*CYL, C, m);
		printf(" size: ");
		l->d_map[i].d_size =
			getf(l->d_map[i].d_size, 0, C*CYL-b, C, m);
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
	register i;
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

gets(p, b, l)
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

getunits()
{
	register i;

	printf("  How many Disk Units (%d)= ", numunits);
	if((i = getnum(10, 0, NUNIT)) != -1) {
		printf("\n");
		if(numunits != i)
		numunits = i;
	} else
		printf("\n");
}

unitd()
{
	register i;

	printf("  Disk Unit (%d)= ", dunit);
	if((i = getnum(10, 0, NUNIT)) != -1) {
		printf("\n");
		if(dunit != i)
			qupdate();
		dunit = i;
		drivep = &drives[i];
		if(!isinited[i])
			initl();
	} else
		printf("\n");
}

sgaps()
{
	register i, s = 0;

	printf("gap 1 (%d)= ", gap1);
	if((i = getnum(10, 1, 50)) != -1) {
		printf("\n");
		if(gap1 != i)
			s = 1;
		drivep->label.d_misc[0] = i;
		gap1 = i;
	} else
		printf("\n");
	printf("gap 2 (%d)= ", gap2);
	if((i = getnum(10, 1, 50)) != -1) {
		printf("\n");
		if(gap2 != i)
			s = 1;
		drivep->label.d_misc[1] = i;
		gap2 = i;
	} else
		printf("\n");
	ip_flags = 0;
	if(s)
		qupdate();
}

silv()
{
	register i;

	printf("ilv (%d)= ", ilv);
	if((i = getnum(10, 1, 64)) != -1) {
		printf("\n");
		ip_flags = 0;
		ilv = drivep->label.d_interleave = i;
		qupdate();
	} else
		printf("\n");
}

sskew()
{
	register i;

	printf("spiral skew (%d)= ", spiralskew);
	if((i = getnum(10, 1, 50)) != -1) {
		printf("\n");
		ip_flags = 0;
		spiralskew = drivep->label.d_cylskew = i;
		qupdate();
	} else
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
		if(uleq(cp, dtypes[i].tname)) {
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
				ip_flags = 0;
			}
			printf("\n");
			return;
		}
	printf(" Unknown. ");
help:	printf("Types are:\n");
	for(i = 0; dtypes[i].tname; i++)
		printf("    %s\n", dtypes[i].tname);
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
