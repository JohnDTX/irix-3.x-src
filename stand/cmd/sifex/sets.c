/*
**	sets.c		- Copyright (C), JCS Computer Services 1983
**			- Author: chase
**			- Date: January 1985
**			- Any use, copying or alteration is prohibited
**			- and morally inexcusable,
**			- unless authorized in writing by JCS.
**	$Source: /d2/3.7/src/stand/cmd/sifex/RCS/sets.c,v $
**	$Revision: 1.1 $
**	$Date: 89/03/27 17:13:19 $
*/

#include "types.h"
#include <sys/dklabel.h>
#include <sys/dktype.h>
#include "streg.h"
#include "disk.h"
#include "fex.h"

extern u_long dmacount;
extern char fmtflag;

setv()
{
	for(;;) {
		printf(" Set ? ");
		switch(gch()) {
		case 'l':	setfs(); break;
		case 'q':	printf(" Quit\n"); return;
		case 't':	type(); break;
		case 'u':	unitd(); break;
		case 'Z': 	spassword(); break;

		case 'a':	if (password) defaults(); break;
		case 'b':	if (password) spbads(); break;
		case 'c':	if (password) scache(); break;
		case 'd':	if (password) sdmacount(); break;
		case 'e':	if (password) setspecial(); break;
		case 'r':	if (password) sretries(); break;
		case 'g':	if (password) sgaps(); break;
		case 'i':	if (password) silv(); break;
		case 'j':	if (password) serrs(); break;
		case 'f':	if (password) slabel(); break;
		case 's':	if (password) sskew(); break;
		case 'v':	if (password) verb(); break;
		case 'w':	if (password) wlock(); break;
		case 'z':	if (password) szerol(); break;
		case 'E':	if (password) setesdi(); break;
		case 'F':	if (password) sfmttime(); break;
		case 'Q':	if (password) fquiet(); break;
		case 'R':	if (password) rnset(); break;
		case 'S':	if (password) ssize(); break;
		case 'W':	if (password) swlevel(); break;
		case 'X':	if (password) sfmtflag(); break;
		case '#':	if (password) xshelp(); break;
		case '\n':
		case ' ':	printf("\n"); break;
		case 'h':
		case '?':
		default:	shelp(); break;
		}
	}
}

shelp()
{
printf("\n    *** Set Commands ***\n");
printf("    l - change file system sizes\n");
printf("    q - quit\n");
printf("    t - set type of disk\n");
printf("    u - set disk unit number\n");
if (!password) return;
printf("    a - display ALL settings\n");
printf("    b - bad blocks display\n");
printf("    f - set up the label\n");
printf("    v - set verbose\n");
printf("    w - write lock switch\n");
}

xshelp()
{
printf("\n    *** Set Commands ***\n");
printf(" a - display ALL settings           b - bad blocks display\n");
printf(" c - set cache and zero latency     d - set dmacount\n");
printf(" e - special uib parameters         l - change file system sizes\n");
printf(" g - gap settings                   h - help\n");
printf(" i - interleave                     j - set error halt\n");
printf(" f - set up the label               m - mode disk address\n");
printf(" r - reset random number            s - set cylinder skew\n");
printf(" t - set type of disk               u - select disk unit\n");
printf(" v - set verbose                    w - write lock switch\n");
printf(" E - set ESDI disk                  F - set delay time\n");
printf(" R - reset random number            S - Set Sectors Sizes\n"); 
printf(" X - set format flag                W - Write levels\n");
}

spbads()
{
	printf("Bad Block list:\n");
	pbad();
}

#ifdef NOTDEF
gioport()
{
	register i;

	printf("Address of I/O Port (%x): ", st_ioport);
	if((i = getnum(16, 0, 0xFFFF)) != -1)
		st_ioport = i;
	printf("\n");
}

gmode()
{
loop:	printf("Mode cyl/hd/sec or logical block(%d) (%s)? ",
		secsize[dunit], emodes[emode]);
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
#endif				/* NOTDEF */

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

szerol()
{
	register char *cp;
	register i;

again:
	printf("\nZero Latency (%s)? ", zerolatency?"on":"off");
	cp = getline();
	if(*cp) {
		if(uleq(cp, "on")) {
			zerolatency = 1;
			drivep->label.d_misc[13] |= 2;
		} else if(uleq(cp, "off")) {
			zerolatency = 0;
			drivep->label.d_misc[13] &= ~(2);
		} else {
			printf(" enter 'on' or 'off'");
			goto again;
		}
		isuptodate[dunit] = 0;
		printf("\n");
	}
	printf("\n");
}

scache()
{
	register char *cp;
	register i;

again:
	printf("\nCache Enable (%s)? ", cacheenable?"on":"off");
	cp = getline();
	if(*cp) {
		if(uleq(cp, "on")) {
			cacheenable = 1;
			drivep->label.d_misc[13] |= 1;
		} else if(uleq(cp, "off")) {
			cacheenable = 0;
			drivep->label.d_misc[13] &= ~(1);
		} else {
			printf(" enter 'on' or 'off'");
			goto again;
		}
		isuptodate[dunit] = 0;
		printf("\n");
		szerol();
		fstinit();
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
	pd();
	plab();
}

pd()
{
	printf("\n       *** Interphase Storager ***\n");
	printf("  Drive: %s Unit: %d  address: %x (%d+%d/%d/%d(%d))\n",
		  dtype->tname, dunit, st_ioport, CYL,
		  drivep->label.d_nalternates/drivep->spc,
		  HD, SEC, secsize[dunit]);
	printf("  Skew:%d gap1:%d gap2:%d gap3:%d cache(%s) Zero latency(%s)\n",
		  spiralskew, gap1, gap2, gap3, cacheenable?"on":"off",
		  zerolatency?"on":"off");
	printf("  Step Width %d us, Interval %d us, Track Seek Time %d ms\n",
		  ((drivep->label.d_misc[3]*6)+8),
		  ((drivep->label.d_misc[4]*8)+12),
		  drivep->label.d_misc[5]);
	printf("  Head Load and Settle Time %d ms\n", drivep->label.d_misc[8]);
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
	printf("  Label: Name: %s, Serial: %s\n",
		prdtype(l->d_type,dk_dtypes,NTYPES), l->d_serial);
	printf("    %d/%d/%d alt: %d/%d bad: %d ilv: %d skew: %d\n",
		l->d_cylinders, HD, SEC, l->d_altstart/C,
		l->d_nalternates/C, l->d_badspots,
		l->d_interleave, l->d_cylskew);
	printf("    gap1: %d gap2: %d gap3: %d\n",
		l->d_misc[0], l->d_misc[1], l->d_misc[2]);
	printf("    cache %s, zerolatency %s\n",
		l->d_misc[13]&1?"on":"off", l->d_misc[13]&2?"on":"off");
	if(floppy)
		return 0;
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
	isinited[dunit] = 0;
	printf("Label:");
	getstr("Name", l->d_name, sizeof l->d_name);
	getstr("Serial #", l->d_serial, sizeof l->d_serial);
	printf("\n");
#ifdef NOTDEF
	getdrv();
	getctlr();
#endif
	l->d_cylinders = getdec("cylinders", l->d_cylinders, 1, 2048);
	l->d_heads = getdec("heads", HD, 1, 64);
	l->d_sectors = getdec("sectors", SEC, 1, 256);
	C = HD * SEC;
	drivep->spc = C;
	if(floppy) {
		printf("\n");
	} else {
		printf("\n");
		l->d_altstart = getdec("alternate cylinder", l->d_altstart/C, 1,
			l->d_cylinders-1) * C;
		drivep->ncyl = l->d_altstart / C;
		l->d_nalternates = getdec("number of alternate cylinders",
			l->d_nalternates/C, 1, 1024) * C;
		printf("\n");
		l->d_interleave = getdec("interleave", l->d_interleave, 0, 256);
		ilv = l->d_interleave;
		l->d_cylskew = getdec("spiral skew", l->d_cylskew, 0, 1024);
		spiralskew = l->d_cylskew;
		printf("\n");
	}
	smisc();		/* Set misceallaneous Variables */
	scache();		/* Set cache on or off and zerolatency */
	if(floppy)
		return 0;
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

#ifdef NOTDEF
getdrv()
{
	register struct disk_label *l = &drivep->label;
	register i;

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
#endif			/* NOTDEF */

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

/*
** getdec(string, default, minimum, maximum)
*/
getdec(p, n, l, h)
char *p;
{
	register i;

	printf("  %s: (%d) ", p, n);
	i = getnum(10, l, h);
	return (i == -1)? n : i;
}
gethex(p, n, l, h)
char *p;
{
	register i;

	printf("  %s: (0x%x) ", p, n);
	i = getnum(16, l, h);
	return (i == -1)? n : i;
}

/* 
** getf(default, min, max, cyls, blocks -or- cyls flag)
*/
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

	printf("\n");
	printf("  %s Unit (%d)= ", (dunit>1)?"Floppy":"Disk", dunit);
	if((i = getnum(10, 0, NUNIT-1)) != -1) {
		if(i > 1) { 
			if (verbose)
				printf(" Floppy ");
			floppy = 1;
			cacheenable = 0;
		} else {
			if (verbose)
				printf(" Hard Disk ");
			cacheenable = 1;
			floppy = 0;
		}
		if(dunit != i)
			qupdate();
		dunit = i;
		drivep = &drives[dunit];
		if(!isinited[i])
			(void) initl();
	/*	drivep->label = *dtype->tlabel;		*/
		drivep->spc = HD * SEC;
		drivep->ncyl = drivep->label.d_altstart/drivep->spc;
		ilv = drivep->label.d_interleave;
	}
	printf("\n");
}

sgaps()
{
	register i, s = 0;

	printf("gap 1 (0x%x)= ", gap1);
	if((i = getnum(16, 1, 50)) != -1) {
		printf("\n");
		if(gap1 != i)
			s = 1;
		drivep->label.d_misc[0] = i;
		gap1 = i;
	} else
		printf("\n");
	printf("gap 2 (0x%x)= ", gap2);
	if((i = getnum(16, 1, 50)) != -1) {
		printf("\n");
		if(gap2 != i)
			s = 1;
		drivep->label.d_misc[1] = i;
		gap2 = i;
	} else
		printf("\n");
	printf("gap 3 (0x%x)= ", gap3);
	if((i = getnum(16, 1, 50)) != -1) {
		printf("\n");
		if(gap3 != i)
			s = 1;
		drivep->label.d_misc[2] = i;
		gap3 = i;
	} else
		printf("\n");
	isinited[dunit] = 0;
	if(s)
		qupdate();
}

silv()
{
	register i;

	printf("ilv (%d)= ", ilv);
	if((i = getnum(10, 1, 64)) != -1) {
		printf("\n");
		isinited[dunit] = 0;
		ilv = drivep->label.d_interleave = i;
		qupdate();
	} else
		printf("\n");
}

sskew()
{
	register i;

	printf("spiral skew (%d)= ", spiralskew);
	if((i = getnum(10, 0, 50)) != -1) {
		printf("\n");
		isinited[dunit] = 0;
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
			if(uleq(cp, "T101") || uleq(cp, "T35E") ||
			    uleq(cp, "MITS") ||
			    uleq(cp, "T35B") || uleq(cp, "Q592")) {
				floppy = 1;
				cacheenable = 0;
				dunit = 2;
				unitd();
				printf("\n");
			} else {
				floppy = 0;
				if (dunit  > 1)
					dunit = 0;
				cacheenable = 1;
				printf("\n");
			}
			isinited[dunit] = 0;
			drivep->label = *dtype->tlabel;
			drivep->spc = HD * SEC;
			drivep->ncyl = drivep->label.d_altstart/
				drivep->spc;
			ilv = drivep->label.d_interleave;
			esditype[dunit] = drivep->label.d_misc[11];
			isinited[dunit] = 0;
			stinit();
			if (drivep->label.d_misc[11] == 0x37 ||
				esditype[dunit] == 0x37) {
				if (verbose)
					printf("Siemens Spin UP\n");
				spinup();
			}
			printf("\n");
			return;
		}
	printf(" Unknown. ");
help:	printf("**** Disk types are:\n");
	for(i = 0; dtypes[i].tname; i++) {
		printf("    %8s", dtypes[i].tname);
		if (i && (i%4 == 0))
			printf("\n");
	}
	printf("\n");
	goto loop;
}

sfmttime()
{
	register i;

	printf("time waiting in Format (%d): ", Fmtwait);
	if((i = getnum(10, 0, 100)) != -1) {
		Fmtwait = i;
	}
	printf("\n");
}

sretries()
{
	register i;

	printf("Firmware Retries (%d)? ", stretries);
	if((i = getnum(10, 0, 10)) != -1) {
		stretries = i;
		isinited[dunit] = 0;
		qupdate();
	}
	printf("\n");
}

smisc()
{
	register struct disk_label *l = &drivep->label;
	register i, b, m, c;

	/* Gaps */
	l->d_misc[0] = gethex("gap 1 size", l->d_misc[0], 0, 50);
	gap1 = l->d_misc[0];
	l->d_misc[1] = gethex("gap 2 size", l->d_misc[1], 0, 50);
	gap2 = l->d_misc[1];
	l->d_misc[2] = gethex("gap 3 size", l->d_misc[2], 0, 50);
	gap3 = l->d_misc[2];
	/*
	**      0 1 2    3    4    5              6             7     8
	** Gaps 0,1,2, spw, spi, ttst, write precom, reduced writ, hlst,
	**   9    10      11     12
	** ecc, mohu,    ddb,   smc
	*/
	printf("\nSTEP PULSE INFORMATION: Width * 5us -- Interval * 10us\n");
	printf("Revision L Firmware SPW (Start of 14us) SPI (Start of 20us)\n");
	l->d_misc[3] = getdec("step pulse Width", l->d_misc[3], 1, 100);
	l->d_misc[4] = getdec("step pulse Interval", l->d_misc[4], 1, 1000);
	printf("\n");
	l->d_misc[5] = getdec("Track to Track * 1ms", l->d_misc[5], 1, 100);
	swlevel();
	l->d_misc[8] = getdec("Head Settling Time (ms)", l->d_misc[8], 0, 100);
	printf("\n");
	l->d_misc[9] = getdec("ECC", l->d_misc[9], 0, 5);
	l->d_misc[10] = getdec("Motor Off/Head Unload", l->d_misc[10], 0, 0xff);
	printf("\n");
	l->d_misc[11] = gethex("Drive Descriptor", l->d_misc[11], 0, 0xff);
	l->d_misc[12] = gethex("Step Motor Control", l->d_misc[12], 0, 256);
	printf("\n");
}

swlevel()
{
	register struct disk_label *l = &drivep->label;
	register char *cp;
	register i;

	printf("\n   Write Precompensation (%s)? ", l->d_misc[6]?"off":"on");
	cp = getline();
	if(*cp) {
		if(uleq(cp, "on")) {
			printf("\n");
			l->d_misc[6] = getdec("At cylinder",0,0,l->d_cylinders);
		} else if(uleq(cp, "off")) {
			l->d_misc[6] = 0xffff;
		} else
			l->d_misc[6] = 0xffff;
	}
	printf("\n");
	printf("   Reduced Write Current (%s)? ", l->d_misc[7]?"off":"on");
	cp = getline();
	if(*cp) {
		if(uleq(cp, "on")) {
			printf("\n");
			l->d_misc[7] = getdec("At cylinder",0,0,l->d_cylinders);
		} else if(uleq(cp, "off")) {
			l->d_misc[7] = 0xffff;
		} else
			l->d_misc[7] = 0xffff;
	}
	printf("\n");
}

setspecial()
{
	register char *cp;
	register i;

	printf("\nReseek Enable (%s)? ", streseek?"on":"off");
	cp = getline();
	if(*cp) {
		if(uleq(cp, "on")) {
			streseek = 1;
		} else if(uleq(cp, "off")) {
			streseek = 0;
			fstinit();
			printf("\n");
			return;
		}
		else
			printf(" enter 'on' or 'off'");
	}
	printf("\n");
	printf("ECCON Enable (%s)(%d)? ", steccon?"on":"off", steccon);
	cp = getline();
	if(*cp) {
		if(uleq(cp, "on")) {
			steccon = 1;
			if (floppy)
				steccon = 2;
		} else if(uleq(cp, "off")) {
			steccon = 0;
			fstinit();
			printf("\n");
			return;
		}
		else
			printf(" enter 'on' or 'off'");
	}
	drivep->label.d_misc[9] = steccon;
	printf("\n");
	printf("Move Bad Data Enable (%s)? ", stmvbad?"on":"off");
	cp = getline();
	if(*cp) {
		if(uleq(cp, "on")) {
			stmvbad = 1;
		} else if(uleq(cp, "off")) {
			stmvbad = 0;
			fstinit();
			printf("\n");
			return;
		}
		else
			printf(" enter 'on' or 'off'");
	}
	printf("\n");
	printf("Increment by Head Enable (%s)? ", stinchd?"on":"off");
	cp = getline();
	if(*cp) {
		if(uleq(cp, "on")) {
			stinchd = 1;
		} else if(uleq(cp, "off")) {
			stinchd = 0;
			fstinit();
			printf("\n");
			return;
		}
		else
			printf(" enter 'on' or 'off'");
	}
	printf("\n");
}

serrs()
{
	register char *cp;

	printf("\n Halt on any error (%s) (yes/no) ? ", errhalt?"yes":"no");
	cp = getline();
	if(*cp) {
		if(uleq(cp, "yes"))
			errhalt = 1;
		else if(uleq(cp, "no"))
			errhalt = 0;
		else
			printf(" enter 'yes' or 'no'");
	}
	printf("\n");
}

ssize()
{
	register j = 0;
	register i;

	printf("\n Drive %d Sector Size = (%d) ? ", dunit, secsize[dunit]);
	if((i = getnum(10, 0, 4096)) != -1) {
		secsize[dunit] = i;
		j++;
	}
	printf("\n Number of Sectors per Track (%d) ? ",
		drivep->label.d_sectors);
	if ((i = getnum(10, 1, 128)) != -1) {
		drivep->label.d_sectors = i;
		j++;
	}
	printf("\n");
	if (j) {
		isinited[dunit] = 0;
		stinit();
	}
	printf("\n");
}

setesdi()
{
	register i;

	if (esditype[dunit] == 0)
		printf("\n !!! WARNING NOT AN ESDI DISK !!!");
	printf("\n Set type of ESDI disk (%x) ? ", esditype[dunit]);
	if ((i = getnum(16, 0, 0x7f)) != -1 ) {
		esditype[dunit] = i;
		drivep->label.d_misc[11] = i;
		if (verbose)
			printf("\n Changed to %x", esditype[dunit]);
		isinited[dunit] = 0;
		stinit();
	}
	printf("\n");
}

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

sdmacount()
{
	register i;

	printf(" set dmacount (%d) ? ", dmacount);
	i = getnum(10, 0, 256);
	if (i != -1) {
		dmacount = i;
		if (verbose)
			printf("\n Changed to %d", dmacount);
	}
	printf("\n");
	isinited[dunit] = 0;
	return;
}

setfs()
{
	register i, b, m;
	register struct disk_label *l = &drivep->label;
	int C;

	C = HD * SEC;
	printf("\nFile Systems info: lba or cylinder entry? ");
	isuptodate[dunit] = 0;
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

sfmtflag()
{
	register i;

	printf(" FORMAT FLAG: tells formatter to not format cylinder 0\n");
	printf(" Format Flag (%d) = ? ", fmtflag);
	if ((i = getnum(10, 0, 1)) != -1)
		fmtflag = i;
	printf("\n");
}
