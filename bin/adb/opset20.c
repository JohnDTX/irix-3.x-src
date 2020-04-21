#include "defs.h"
/****************************************************************************

 DEBUGGER - 68000 instruction printout

****************************************************************************/

int dotinc;
unsigned space;

char *badop = "\t???"; /* what to print for a bad opcode */
char *IMDF; /* immediate data format */
char *SIMDF; /* signed immediate data format */

char *bname[16] = {
	"ra", "sr", "hi", "ls", "cc", "cs", "ne",
	"eq", "vc", "vs", "pl", "mi", "ge", "lt", "gt", "le" };

char *sccname[16] = {
	"t", "f", "hi", "ls", "cc", "cs", "ne",
	"eq", "vc", "vs", "pl", "mi", "ge", "lt", "gt", "le" };

char *dbname[16] = {
	"t ", "ra", "hi", "ls", "cc", "cs", "ne",
	"eq", "vc", "vs", "pl", "mi", "ge", "lt", "gt", "le" };

char *shro[4] = {
	"as", "ls", "rox", "ro" };

char *bit[4] = {
	"btst", "bchg", "bclr", "bset" };

char *creg[8] = {
	"sfc", "dfc", "cacr", "usp", "vbr", "caar", "msp", "isp" };

char scalechar[4] = {
	'1', '2', '4', '8' };
#define BYTE 0x1
#define WORD 0x2
#define LONG 0x4

int omove(), obranch(), oimmed(), oprint(), oneop(), soneop(), oreg();
int ochk(), olink(), omovem(), oquick(), omoveq(), otrap(), odbcc();
int oscc(), opmode(), shroi(), extend(), biti(), omovec(), omoves();
int ortd(), ostop(), omovesr(), oorsr();

int callm(), cmp2_chk2(), cas(), long_mul(), long_div(), otrapcc(), bitfield();
int cp_inst(), opack();

struct opdesc {
	int mask, match;
	int (*opfun)();
	char *farg;
} opdecode[] = {
	/* order of table is important */
	0xF000, 0x1000, omove, "b", /* move instructions */
	0xF000, 0x2000, omove, "l",
	0xF000, 0x3000, omove, "w",

/* FIX branches must be modified for 32-bit offsets! */
	0xF000, 0x6000, obranch, 0, /* branches */
	0xFFFF, 0x003C, oorsr, 0,
	0xFFFF, 0x007C, oorsr, 0,
	0xFFFF, 0x023C, oorsr, 0,
	0xFFFF, 0x027C, oorsr, 0,

	/* GB cmp2/chk2 byte must go here (before 'or'), with 0xFFF0, 0x00c0 */
	0xFFC0, 0x00c0, cmp2_chk2, (char *)BYTE, /* cmp2b and chk2b instrs */
	0xFF00, 0x0000, oimmed, "or", /* op class 0 */
	/* GB cmp2/chk2 word must go here (before 'or'), with 0xFFF0, 0x02c0 */
	0xFFC0, 0x02c0, cmp2_chk2, (char *)WORD, /* cmp2w and chk2w instrs */
	0xFF00, 0x0200, oimmed, "and",
	/* GB cmp2/chk2 long must go here (before 'or'), with 0xFFF0, 0x04c0 */
	0xFFC0, 0x04c0, cmp2_chk2, (char *)LONG, /* cmp2l and chk2l instrs */
	0xFF00, 0x0400, oimmed, "sub",
	/* GB rtm here 0xfff0, 0x05c0 */
	0xFFF0, 0x05c0, otrap, "rtm",
	/* GB callm here 0xfff0, 0x06c0 */
	0xFFC0, 0x06c0, callm, 0, /* call module #args, ea */
	0xFF00, 0x0600, oimmed, "add",
	/* GB casb here (0xfff0, 0x0ac0) */
	0xFFC0, 0x0ac0, cas, (char *)BYTE, /* casb */
	0xFF00, 0x0A00, oimmed, "eor",
	/* GB casw here (0xfff0, 0x0cc0) */
	0xFFC0, 0x0cc0, cas, (char *)WORD, /* casw, cas2w */
	0xFF00, 0x0C00, oimmed, "cmp",
	0xFFC0, 0x0ec0, cas, (char *)LONG, /* casl,cas2l */
	0xF8C0, 0xe8c0, bitfield, 0, /* all bfxxx instructions */
	0xFF00, 0x0E00, omoves, 0,
	0xF100, 0x0100, biti, 0,
	0xF800, 0x0800, biti, 0,

	0xFFC0, 0x40C0, omovesr, 0, /* op class 4 */
	0xFF00, 0x4000, soneop, "negx",
	0xFFC0, 0x42C0, omovesr, 0,
	0xFF00, 0x4200, soneop, "clr",
	0xFFC0, 0x44C0, omovesr, 0,
	0xFF00, 0x4400, soneop, "neg",
	0xFFC0, 0x46C0, omovesr, 0,
	0xFF00, 0x4600, soneop, "not",
	/* add a linkl version to the linkw version (0x4808) goes before nbcd */
	0xFFF8, 0x4808, olink, (char *)LONG,

	0xFFC0, 0x4800, oneop, "nbcd",
	0xFFF8, 0x4840, oreg, "\tswap\td%D",
	/* add the bkpt here before pea (encoded like trap) 0x4848 */
	0xFFF8, 0x4848, otrap, "bkpt",
	0xFFC0, 0x4840, oneop, "pea",
	0xFFF8, 0x4880, oreg, "\textw\td%D",
	0xFFF8, 0x48C0, oreg, "\textl\td%D",
	/* extbl here (0xfff8, 0x49c0) */
	0xFFF8, 0x49c0, oreg, "\textbl\td%D",
	/* here, moveml is 0x48c0 and movemw is 0x4880 on the 68020 */
	0xFBc0, 0x4880, omovem, (char *)WORD,
	0xFBc0, 0x48C0, omovem, (char *)LONG,
	0xFFC0, 0x4AC0, oneop, "tas",
	0xFF00, 0x4A00, soneop, "tst",
	0xFFF0, 0x4E40, otrap, "trap",
	0xFFF8, 0x4E50, olink, (char *)WORD,
/* (redundant) 
	0xFFF8, 0x4880, oreg, "\textw\td%D",
	0xFFF8, 0x48C0, oreg, "\textl\td%D",
*/

/**** PRELIMINARY TESTS TO HERE (GB) ****/

/* the long multiply instrs with mask 0xffc0 and key 0x4c00 */
	0xffc0, 0x4c00, long_mul, 0,
/* the long divide instrs with mask 0xffc0 and key 0x4c40 */
	0xffc0, 0x4c40, long_div ,0,

	0xFFF8, 0x4E58, oreg, "\tunlk\ta%D",
	0xFFF8, 0x4E60, oreg, "\tmove\ta%D,usp",
	0xFFF8, 0x4E68, oreg, "\tmove\tusp,a%D",
	0xFFFF, 0x4E70, oprint, "reset",
	0xFFFF, 0x4E71, oprint, "nop",
	0xFFFF, 0x4E72, ostop, "stop",
	0xFFFF, 0x4E73, oprint, "rte",
	0xFFFF, 0x4E74, ortd, 0,
	0xFFFF, 0x4E75, oprint, "rts",
	0xFFFF, 0x4E76, oprint, "trapv",
	0xFFFF, 0x4E77, oprint, "rtr",
	0xFFFE, 0x4E7A, omovec, 0,
	0xFFC0, 0x4E80, oneop, "jsr",
	0xFFC0, 0x4EC0, oneop, "jmp",
/* GB the chk here is the word version.  The long version is 0x4100 */
	0xF1C0, 0x4180, ochk, "chkw", 
	0xF140, 0x4100, ochk, "chkl",
	0xF1C0, 0x41C0, ochk, "lea",
/* GB the trapcc instrs */
	0xF0F8, 0x50F8, otrapcc, 0,
	0xF0F8, 0x50C8, odbcc, 0,
	0xF0C0, 0x50C0, oscc, 0,
	0xF100, 0x5000, oquick, "addq",
	0xF100, 0x5100, oquick, "subq",
	0xF000, 0x7000, omoveq, 0,
	0xF1C0, 0x80C0, ochk, "divu",
	0xF1C0, 0x81C0, ochk, "divs",
	0xF1f0, 0x8140, opack, "pack",
	0xF1f0, 0x8180, opack, "unpk",
	0xF1F0, 0x8100, extend, "sbcd",
	0xF000, 0x8000, opmode, "or",
	0xF1C0, 0x91C0, opmode, "sub",
	0xF130, 0x9100, extend, "subx",
	0xF000, 0x9000, opmode, "sub",
	0xF1C0, 0xB1C0, opmode, "cmp",
	0xF138, 0xB108, extend, "cmpm",
	0xF100, 0xB000, opmode, "cmp",
	0xF100, 0xB100, opmode, "eor",
	0xF1C0, 0xC0C0, ochk, "muluw",
	0xF1C0, 0xC1C0, ochk, "mulsw",
	0xF1F8, 0xC188, extend, "exg",
	0xF1F8, 0xC148, extend, "exg",
	0xF1F8, 0xC140, extend, "exg",
	0xF1F0, 0xC100, extend, "abcd",
	0xF000, 0xC000, opmode, "and",
	0xF1C0, 0xD1C0, opmode, "add",
	0xF130, 0xD100, extend, "addx",
	0xF000, 0xD000, opmode, "add",
	0xF100, 0xE000, shroi, "r",
	0xF100, 0xE100, shroi, "l",
	0xf000, 0xf000, cp_inst, 0,
	0, 0, 0, 0
};

printins(f, idsp, inst)
register int inst;
{
	register struct opdesc *p;

	space = idsp;
	dotinc = 2;
	IMDF = "#%X";
	SIMDF = "#%D.";
	for (p = opdecode; p->mask; p++)
		if ((inst & p->mask) == p->match)
			break;
	if (p->mask != 0)
		(*p->opfun)(inst, p->farg);
	else
		printf(badop);
}

long
instfetch(size, doextend)
int size;
{
	unsigned long l1, l2;

	if (size==4)
	{
		l1 = leng(chkget(inkdot(dotinc), space));
		l1 <<= 16;
		l2 = leng(chkget(inkdot(dotinc += 2), space));
		l1 = (l1 | l2);
	}
	else
	{
		l1 = (unsigned long)(chkget(inkdot(dotinc), space)) & 0xFFFF;
		if (doextend)
			if (l1 & 0x8000) l1 |= 0xFFFF0000;
	}
	dotinc += 2;
	return(l1);
}

printea(mode,reg,size)
long mode, reg;
int size;
{
	unsigned long index;
	long basedisp;
	long outerdisp;
	long disp;
	unsigned short format_word;
	char odsize, isindexed, a_index, indexreg, 
	     scale, l_index, ismemind, ispc;


	switch ((int)(mode)) {
	case 0:
		printf("d%D",reg);
		break;

	case 1:
		printf("a%D",reg);
		break;

	case 2:
		printf("a%D@",reg);
		break;

	case 3:
		printf("a%D@+",reg);
		break;

	case 4:
		printf("a%D@-",reg);
		break;

	case 5:
		/* an@(d) d = 16-bit */
		printf("a%D@(%D.)",reg,instfetch(2, 1));
		break;

	case 6:
		ispc = 0;
doindex:
		/* GB modes 6 and 7 get much more complex on the 68020 */
		/* mode 6 is aN indexed.  If bit 8 of the format word
		   is zero, it is the old simple 8-bit index */
		format_word = instfetch(2, 0);
		if (!(format_word & 0x100)) {

			/* old eight-bit index.  Disp is still low 8 bits.
			*/
			disp = (char)(format_word&0377);

			/* With 68020, index can be scaled by 1,2,4 or 8.
			*/
			printf("%c%c@(%d,%c%D:%c*%c)",(ispc?'p':'a'),
				(ispc?'c':(reg + '0')),disp,
		    		(format_word&0100000)?'a':'d',(format_word>>12)&07,
		    		(format_word&04000)?'l':'w',
				(scalechar[(format_word&0x600)>>9]));
			break;
		}
		/* more complex index mode */
		/* if the base is NOT supressed, get it. */
		basedisp = 0;
		if (!(format_word & 0x80)) {
			int bdsize;

			/* bdsize is bits 4,5 */
			bdsize = (format_word & 0x30) >> 4;

			/* bd size == 2 for word, 3 for long */
			if (bdsize >= 2)
				basedisp = instfetch((bdsize == 2)?2:4, TRUE);
		}

		/* if the index is NOT suppressed, decode it */
		if (!(format_word & 0x40)) {
			isindexed = 1;
			a_index = ((format_word & 0x8000) != 0);
			indexreg = (format_word & 0x7000) >> 12;
			scale = (format_word & 0x600) >> 9;
			l_index = ((format_word & 0x800) != 0);
		}
		else {
			isindexed = 0;
		}

		/* is there memory indirection ? */
		ismemind = ((format_word & 7) != 0);

		odsize = (format_word & 3);
		outerdisp = 0;
		/* last, is there an outer displacement? */
		if ((ismemind) && (odsize != 1)) {
			/* get the outer displacement */
			outerdisp = instfetch((odsize == 2)?2:4, TRUE);
		}

		/* ok, we print the formats as follows:

			aN@(disp,[Xn])  	- indexed, non-indirect
			an@(disp,[Xn])@(d)	- pre-indexed, indirect
			an@(disp)@(disp,[Xn])   - post-indexed, indirect
		*/
		printf("a%d@(%d",reg,basedisp);
		/* the index reg comes next if it is indexed and 
		   it is NOT post-indexed
		*/
		if ((isindexed) && (!(format_word & 4)))
		{
			printf(",[%c%D:%c*%c]",(a_index?'a':'d'),indexreg,
				(l_index?'l':'w'),(scalechar[scale]));
			isindexed=0;
		}

		printf(")");
		if (ismemind) {
			printf("@(%d",outerdisp);
			if (isindexed)
				printf(",[%c%D:%c*%c]",(a_index?'a':'d'),
					indexreg,(l_index?'l':'w'),
					(scalechar[scale]));
			printf(")");
		}
		break;


	case 7:
		switch ((int)(reg))
		{
		case 0:
			index = instfetch(2, 0);
			printf("%x:w",index);
			break;

		case 1:
			index = instfetch(4, 1);
			psymoff(index, ISYM, "");
			break;

		case 2:
			printf("pc@(%D.)",instfetch(2, 1));
			break;

		case 3:
			ispc = 1;
			goto doindex;

			/*
			index = instfetch(2, 0);
			disp = (char)(index&0377);
			printf("a%D@(%D,%c%D:%c)",reg,disp,
			    (index&0100000)?'a':'d',(index>>12)&07,
			    (index&04000)?'l':'w');
			break;
			*/

		case 4:
			index = instfetch(size==4?4:2, 0);
			printf(IMDF, index);
			break;

		default:
			printf("???");
			break;
		}
		break;

	default:
		printf("???");
	}
}

printEA(ea, size)
long ea;
int size;
{
	printea((ea>>3)&07,ea&07,size);
}

mapsize(inst)
register long inst;
{
	inst >>= 6;
	inst &= 03;
	return((inst==0) ? 1 : (inst==1) ? 2 : (inst==2) ? 4 : -1);
}

char suffix(size)
register int size;
{
	return((size==1) ? 'b' : (size==2) ? 'w' : (size==4) ? 'l' : '?');
}

int suffix_len(str)
char *str;
{
	/* returns length of BYTE, WORD, or LONG depending on the
	   last character of str (GB)
	*/
	int len;
	char suf;
	len = strlen(str);
	suf = str[len-1];
	if (suf == 'l') return(LONG);
	if (suf == 'w') return(WORD);
	if (suf == 'b') return(BYTE);
	return(0);
}


omove(inst, s)
register long inst;
char *s;
{
	register int size;

#ifdef CALLS_ONLY
	printf("omove (0x%x, %s)\n",inst,s);
	return;
#endif
	printf("\tmov%c\t",*s);
	size = ((*s == 'b') ? 1 : (*s == 'w') ? 2 : 4);
	printea((inst>>3)&07,inst&07,size);
	printc(',');
	printea((inst>>6)&07,(inst>>9)&07,size);
}

obranch(inst,dummy)
long inst;
{
	long disp = inst & 0377;
	char *s;
	/* GB - has to handle 32-bit offsets for Juniper */

#ifdef CALLS_ONLY
	printf("obranch (0x%x)\n",inst);
	return;
#endif
	s = "s ";
	if (disp == 255) {
		s = " ";
		disp = instfetch(4,1);
	}
	else if (disp > 127)
		disp |= ~0377;
	else
		if (disp == 0) {
			s = " ";
			disp = instfetch(2, 1);
		}
	printf("\tb%s%s\t", bname[(int)((inst>>8)&017)], s);
	psymoff(disp+inkdot(2), ISYM, "");
}

odbcc(inst,dummy)
long inst;
{
	long disp;
#ifdef CALLS_ONLY
	printf("odbcc (0x%x)\n",inst);
	return;
#endif
	printf("\tdb%s\t",dbname[(int)((inst>>8)&017)]);
	printea(0,inst&07,1);
	printc(',');
	disp = instfetch(2, 1);
	psymoff(disp+inkdot(2),ISYM,"");
}

oscc(inst,dummy)
register long inst;
{
#ifdef CALLS_ONLY
	printf("oscc (0x%x)\n",inst);
	return;
#endif
	printf("\ts%s\t",sccname[(int)((inst>>8)&017)]);
	printea((inst>>3)&07,inst&07,1);
}

biti(inst, dummy)
register long inst;
{
#ifdef CALLS_ONLY
	printf("biti (0x%x)\n",inst);
	return;
#endif
	printf("\t%s\t", bit[(int)((inst>>6)&03)]);
	if (inst&0x0100)
		printf("d%D,", inst>>9);
	else {
		printf(IMDF, instfetch(2, 0));
		printc(',');
	}
	printEA(inst,WORD);
}

opmode(inst,opcode)
register long inst;
{
	register int opmode = (int)((inst>>6) & 07);
	register int reg = (int)((inst>>9) & 07);
	register int size;

	size = (opmode==0 || opmode==4) ?
	    1 : (opmode==1 || opmode==3 || opmode==5) ? 2 : 4;
	printf("\t%s%c\t", opcode, suffix(size));
	if (opmode>=4 && opmode<=6) {
		printf("d%d,",reg);
		printea((inst>>3)&07,inst&07, size);
	} else {
		printea((inst>>3)&07,inst&07, size);
		printf(",%c%d",(opmode<=2)?'d':'a',reg);
	}
}

shroi(inst,ds)
register long inst;
char *ds;
{
	int rx, ry;
	register char *opcode;
#ifdef CALLS_ONLY
	printf("shroi (0x%x, %s)\n",inst,ds);
	return;
#endif
	if ((inst & 0xC0) == 0xC0) {
		opcode = shro[(int)((inst>>9)&03)];
		printf("\t%s%s\t", opcode, ds);
		printEA(inst,WORD);
	} else {
		opcode = shro[(int)((inst>>3)&03)];
		printf("\t%s%s%c\t", opcode, ds, suffix(mapsize(inst)));
		rx = (int)((inst>>9)&07);
		ry = (int)(inst&07);
		if ((inst>>5)&01)
			printf("d%d,d%d", rx, ry);
		else {
			printf(IMDF, (rx ? rx : 8));
			printf(",d%d", ry);
		}
	}
}

oimmed(inst,opcode)
long inst;
register char *opcode;
{
	register int size = mapsize(inst);
	unsigned long const;

#ifdef CALLS_ONLY
	printf("oimmed (0x%x, %s)\n",inst,opcode);
	return;
#endif
	if (size > 0) {
		const = instfetch(size==4?4:2, 0);
		printf("\t%s%c\t", opcode, suffix(size));
		printf(IMDF, const);
		printc(',');
		printEA(inst,size);
	} else
		printf(badop);
}

oreg(inst,opcode)
long inst;
char *opcode;
{
#ifdef CALLS_ONLY
	printf("oreg (0x%x, %s)\n",inst,opcode);
	return;
#endif
	printf(opcode, (inst & 07));
}

extend(inst, opcode)
register long inst;
char *opcode;
{
	register int size = mapsize(inst);
	int ry = (inst&07), rx = ((inst>>9)&07);
	char c;

#ifdef CALLS_ONLY
	printf("extend (0x%x, %s)\n",inst,opcode);
	return;
#endif
	c = ((inst & 0x1000) ? suffix(size) : ' ');
	printf("\t%s%c\t", opcode, c);
	if (*opcode == 'e') {
		if (inst & 0x0080)
			printf("d%D,a%D", rx, ry);
		else
			if (inst & 0x0008)
				printf("a%D,a%D", rx, ry);
			else
				printf("d%D,d%D", rx, ry);
	} else
		if ((inst & 0xF000) == 0xB000)
			printf("a%D@+,a%D@+", ry, rx);
		else
			if (inst & 0x8)
				printf("a%D@-,a%D@-", ry, rx);
			else
				printf("d%D,d%D", ry, rx);
}

olink(inst,size)
long inst;
int size;
{
	/* (GB) size is now LONG or WORD */
#ifdef CALLS_ONLY
	printf("olink (0x%x, %d)\n",inst, size);
	return;
#endif
	printf("\tlink%c\ta%D,", (size == LONG)?'l':'w',inst&07);
	printf(SIMDF, instfetch((size == LONG)?4:2, 1));
}

otrap(inst,opcode)
long inst;
char *opcode;
{
	int type = ((inst & 0x0c00) >> 10);
	char *fmt = "#%d\n";
	int mask = 7;

	/* GB - decode bkpt, trap and rtm instructions */
#ifdef CALLS_ONLY
	printf("otrap (0x%x, %s)",inst, opcode);
	return;
#endif
	printf("\t%s\t",opcode);
	switch (type)
	{
		case 1: /* rtm */
			if (inst & 8) fmt = "a%d";
			else fmt = "d%d";
			break;
		case 2: /* bkpt */
			break;
		case 3: /* trap */
			mask = 0xf;
			break;

	}
	printf(fmt, inst&mask);
}

oneop(inst,opcode)
long inst;
register char *opcode;
{
#ifdef CALLS_ONLY
	printf("oneop (0x%x, %s)\n",inst,opcode);
	return;
#endif
	printf("\t%s\t",opcode);
	printEA(inst,suffix_len(opcode));
}

pregmask(mask)
register int mask;
{
	register int i;
	register int flag = 0;

	printf("#<");
	for (i=0; i<16; i++)
	{
		if (mask&1)
		{
			if (flag) printc(',');
			else flag++;
			printf("%c%d",(i<8)?'d':'a',i&07);
		}
		mask >>= 1;
	}
	printf(">");
}

omovem(inst,size)
long inst;
int size;
{
	/* GB - size now passed as WORD or LONG */
	register int i, list = 0, mask = 0100000;
	register int reglist = (int)(instfetch(2, 0));

#ifdef CALLS_ONLY
	printf("omovem (0x%x)\n",inst);
	return;
#endif
	if ((inst & 070) == 040) { /* predecrement */
		for(i = 15; i > 0; i -= 2) {
			list |= ((mask & reglist) >> i);
			mask >>= 1;
		}
		for(i = 1; i < 16; i += 2) {
			list |= ((mask & reglist) << i);
			mask >>= 1;
		}
		reglist = list;
	}
	printf("\tmovem%c\t",(size == LONG)?'l':'w');
	if (inst&02000) {
		printEA(inst,size);
		printc(',');
		pregmask(reglist);
	} else {
		pregmask(reglist);
		printc(',');
		printEA(inst,size);
	}
}

ochk(inst,opcode)
long inst;
register char *opcode;
{
	/* GB - used for chkw, chkl, lea, {div,mul}uw and {div,mul}sw. 
	*/
	int size;
#ifdef CALLS_ONLY
	printf("ochk (0x%x, %s)\n",inst,opcode);
	return;
#endif
	/* size is WORD unless lea or chkl */
	size = (int)WORD;
	if ((*opcode == 'l')||((*opcode == 'c')&&(*(opcode+3)=='l')))
		size = (int)LONG;

	printf("\t%s\t",opcode);
	printEA(inst,size);
	printf(",%c%D",(*opcode=='l')?'a':'d',(inst>>9)&07);
}

soneop(inst,opcode)
long inst;
register char *opcode;
{
	register int size = mapsize(inst);
#ifdef CALLS_ONLY
	printf("soneop (0x%x, %s)\n",inst,opcode);
	return;
#endif

	if (size > 0) {
		printf("\t%s%c\t",opcode,suffix(size));
		printEA(inst,suffix_len(opcode));
	} else
		printf(badop);
}

oquick(inst,opcode)
long inst;
register char *opcode;
{
	register int size = mapsize(inst);
	register int data = (int)((inst>>9) & 07);
#ifdef CALLS_ONLY
	printf("oquick (0x%x, %s)\n",inst,opcode);
	return;
#endif

	if (data == 0) data = 8;
	if (size > 0) {
		printf("\t%s%c\t", opcode, suffix(size));
		printf(SIMDF, data);
		printc(',');
		printEA(inst,0);
	}
	else printf(badop);
}

omoveq(inst,dummy)
long inst;
{
	register int data = (int)(inst & 0377);
#ifdef CALLS_ONLY
	printf("omoveq (0x%x)\n",inst);
	return;
#endif

	if (data > 127)
		data |= ~0377;
	printf("\tmoveq\t");
	printf(SIMDF, data);
	printf(",d%D", (inst>>9)&07);
}

oprint(inst,opcode)
long inst;
register char *opcode;
{
#ifdef CALLS_ONLY
	printf("oprint (0x%x, %s)\n",inst,opcode);
	return;
#endif
	printf("\t%s",opcode);
}

omovec(inst,dummy)
long inst;
{
	register long ext, cntrl;

	/* GB - control regs for m68020 are:

		000	sfc	(m68010 also)
		001	dfc	(m68010 also)
		002	cacr
		800	usp	(m68010 also)
		801	vbr	(m68010 also)
		802	caar
		803	msp
		804	isp
	*/

#ifdef CALLS_ONLY
	printf("omovec (0x%x)\n",inst);
	return;
#endif
	ext = instfetch(2, 0);

	/* 
	 * 000...002 -> 000...002
	 * 800...804 -> 003...007
	 *
	*/
	cntrl = (ext & 7);
	if ((ext & 0x800)) cntrl += 3;
	else if (cntrl > 2) cntrl = 0x20;
	if (cntrl > 7) printf(badop);
	else {
		printf("\tmovec\t");
		if (inst & 1) {
			printf("%c%D,%s", (ext & 0x8000)? 'a': 'd',
			    	(ext >> 12) & 07, creg[cntrl]);
		} else {
			printf("%s,%c%D", creg[cntrl],
			    	(ext & 0x8000)? 'a': 'd', 
				(ext >> 12) & 07);
		}
	}
}

omoves(inst, dummy)
long inst;
{
	register long ext;
	register int size = mapsize(inst);

#ifdef CALLS_ONLY
	printf("omoves (0x%x)\n",inst);
	return;
#endif
	ext = instfetch(2, 0);
	if (ext & 0x7FF)
		printf(badop);
	else {
		printf("\tmoves%c\t", suffix(size));
		if (ext & 0x800) {
			printf("%c%D,", (ext & 0x8000)? 'a': 'd',
			    (ext >> 12) & 07);
			printea((inst>>3)&07,inst&07, size);
		} else {
			printea((inst>>3)&07,inst&07, size);
			printf(",%c%D", (ext & 0x8000)? 'a': 'd',
			    (ext >> 12) & 07);
		}
	}
}

ortd(inst,dummy)
long inst;
{
#ifdef CALLS_ONLY
	printf("ortd (0x%x)\n",inst);
	return;
#endif
	printf("\trtd\t");
	printf(SIMDF, instfetch(2, 1));
}

ostop(inst,dummy)
long inst;
{
#ifdef CALLS_ONLY
	printf("ostop (0x%x)\n",inst);
	return;
#endif
	printf("\tstop\t");
	printf(SIMDF, instfetch(2, 1));
}

omovesr(inst, dummy)
long inst;
{
	int mode, reg;

#ifdef CALLS_ONLY
	printf("omovesr (0x%x)\n",inst);
	return;
#endif
	printf("\tmovw\t");
	mode = (inst >> 3) & 7;
	reg = inst & 7;
	inst &= ~077; /* strip out mode,reg bits */
	switch (inst) {
	case 0x40c0: /* move from sr */
		printf("sr,");
		printea(mode, reg, 2);
		break;
	case 0x42c0: /* move from ccr */
		printf("ccr,");
		printea(mode, reg, 2);
		break;
	case 0x44c0: /* move to ccr */
		printea(mode, reg, 2);
		printf(",ccr");
		break;
	case 0x46c0: /* move to sr */
		printea(mode, reg, 2);
		printf(",sr");
		break;
	}
}

oorsr(inst, dummy)
long inst;
{
#ifdef CALLS_ONLY
	printf("oorsr (0x%x)\n",inst);
	return;
#endif
	printf("\t%s%c\t#%x,%s",
	    inst & 0x0200 ? "and" : "or",
	    inst & 0x0040 ? 'w' : 'b',
	    instfetch(2, 0),
	    inst & 0x0040 ? "sr" : "ccr");
}


callm(inst,dummy)
long inst;
int dummy;
{
	int argc;
#ifdef CALLS_ONLY
	printf("callm(0x%x)\n",inst);
	return;
#endif
	argc = instfetch(WORD,FALSE);
	printf("\tcallm\t#%d,",argc);
	printEA(inst,0);
}


cmp2_chk2(inst,size)
long inst;
int size;
{
	int extension;
	char *str;
#ifdef CALLS_ONLY
	printf("cmp2_chk2(0x%x, %d)\n",inst,size);
	return;
#endif
	extension = instfetch(WORD,FALSE);
	if (extension & 0x4000) str = "\tchk2%c\t";
	else str = "\tcmp2%c\t";

	printf(str,suffix(size));
	printEA(inst,size);

	printf(",%c%d",(extension & 0x8000)?'a':'d',
		((extension & 7000)>>12));

}


cas(inst,size)
long inst;
int size;
{
	/* decode the cas and cas2 instructions. */
	int dual;
	int extension0;
	int extension1;
#ifdef CALLS_ONLY
	printf("cas(0x%x, %d)\n",inst,size);
	return;
#endif
	/* dual is set if cas2 */
	dual =  ((inst & 0x3f) == 0x3c) ;

	extension0 = instfetch(WORD,FALSE);
	if (dual) {
		extension1 = instfetch(WORD,FALSE);
		printf("\tcas2%c\td%d:d%d,d%d:d%d,",
			suffix(size), (extension0 & 7),
			(extension1 & 7), 
			((extension0 & 0x1c0)>>6),
			((extension1 & 0x1c0)>>6));
		printf("%c%d@,%c%d@", 
			(extension0 & 0x8000)?'a':'d',
			((extension0 & 0x7000)>>12),
			(extension1 & 0x8000)?'a':'d',
			((extension1 & 0x7000)>>12));
	}
	else {
		printf("\tcas%c\td%d,d%d,",suffix(size),
			(extension0&7), 
			((extension0 & 0x1c0)>>6));
		printEA(inst,size);
	}

}



long_div(inst,dummy)
long inst;
{
	int is64bitdividend = 0;
	int doublereg=0;
	int extension;
	char dq,dr;

#ifdef CALLS_ONLY
	printf("long_div(0x%x)\n",inst);
	return;
#endif
	extension = instfetch(WORD,FALSE);
	is64bitdividend = (extension & 0x400);
	dq = ((extension & 0x7000) >> 12);
	dr = (extension & 7);
	doublereg = (dr != dq);

	printf("\tdiv%cl%c\t",(extension & 0x800)?'s':'u',
		((doublereg)&&(!is64bitdividend))?'l':' ');

	printEA(inst,LONG);

	if (doublereg) {
		/* two reg form */
		printf(",d%d:d%d",dr,dq);
	}
	else {
		printf(",d%d",dq);
	}

}

long_mul(inst,dummy)
long inst;
{
	int is64bitproduct = 0;
	int extension;
	char dh,dl;

#ifdef CALLS_ONLY
	printf("long_mul(0x%x)\n",inst);
	return;
#endif
	extension = instfetch(WORD,FALSE);
	is64bitproduct = (extension & 0x400);
	dl = ((extension & 0x7000) >> 12);
	dh = (extension & 7);

	printf("\tmul%cl%c\t",(extension & 0x800)?'s':'u');

	printEA(inst,LONG);

	if (is64bitproduct) {
		/* two reg form */
		printf(",d%d:d%d",dh,dl);
		if (dh == dl) printf(" <illegal>");
	}
	else {
		printf(",d%d",dl);
	}
}

otrapcc(inst,dummy)
long inst;
{
	long data;
	int opmode = inst & 07;
	char sz = ' ';
	int nodata = 0;
	/* GB - has to handle 32-bit offsets for Juniper */

#ifdef CALLS_ONLY
	printf("trapcc(0x%x)\n",inst);
	return;
#endif
	if (opmode == 3) {
		sz = 'l';
		data = instfetch(LONG,TRUE);
	}
	else if (opmode == 2) {
		sz = 'w';
		data = instfetch(WORD,TRUE);
	}
	else nodata++;

	printf("\ttrap%s%c\t", sccname[(int)((inst>>8)&017)], sz);
	if (!nodata) printf("#0x%x",data);
}

typedef enum { BFTST, BFEXTU, BFCHG, BFEXTS, BFCLR, BFFFO, BFSET, BFINS}
	    bftype_t;

char *bfstr[8] = { "bftst", "bfextu", "bfchg", "bfexts", "bfclr",
		   "bfffo", "bfset", "bfins" } ;

bitfield(inst,dummy)
long inst;
{
	/* bfXXX.  the 3 least significant bits of the
	   most significant byte indicate which type of bitfield it is
	*/

	bftype_t bftype;

	unsigned extension;
	unsigned short offset, width;
	unsigned char offset_is_reg, width_is_reg;
	unsigned char reg;

	bftype = (bftype_t)((inst & 0x0700)>>8);

	extension = instfetch(WORD,FALSE);

	/* hack apart the extension word */
	width = (extension  & 0x1f);
	width_is_reg = ((extension & 0x20) != 0);
	offset = ((extension & 0x07c0)>>6);
	offset_is_reg = ((extension & 0x800) != 0);
	reg = ((extension & 0x7000) >> 12); 

	printf("\t%s\t",bfstr[(int)bftype]);
	if (bftype == BFINS)
		printf("d%d,",reg);

	printEA(inst,0);

	/* now print the offset and width */
	printf(" {%s%d:%s%d}", (offset_is_reg)?"d":"",offset,
		(width_is_reg)?"d":"",width);

	if ((bftype == BFFFO)||(bftype == BFEXTU)||(bftype == BFEXTS))
		printf(",d%d",reg);

}

opack(inst,opcode)
unsigned inst;
char *opcode;
{

	/* do the pack and unpk instructions.  These
	   will not be tested, as they are not in
	   as20 yet.
	*/

	short adjustment;
	char yreg, xreg, memory_op;

	printf("\t%s\t",opcode);

	adjustment = instfetch(WORD,TRUE);

	/* get the registers, and whether the source/dest is memory
	   or registers
	*/
	yreg = ((inst & 0xe00)>>8);
	xreg = (inst & 7);
	memory_op = ((inst & 8) != 0);

	if (memory_op)
		printf("a%@d-,a%@d-,",xreg,yreg);
	else
		printf("d%d,d%d,",xreg,yreg);

	printf("#%d",adjustment);

}

cp_inst(inst,opcode)
unsigned inst;
char *opcode;
{
	/* do the co-processor instructions.
	   These are funny.  We adopt the following convention:

		We assume NO optional co-processor defined 
		extension words.

		The coprocessor id is encoded into the instruction
		as a number, as cpN (cp4).

		If there is a coprocessor condition in the 
		instruction, it is disassembled in hex 
		into the cc field.
		Thus, cpbxx 0x3044 for coprocessor 5, where 
		condition xx is 0x15 would
		be disassembled as 

			cp5B15 0x3044

		the cpGEN instruction is assumed to have an effective address.
	*/

	/* if bit 7 is on in the instruction, it is
	   cpBcc
	*/

	char cpid;
	char islong;
	char code;
	short condition;
	short command;
	long disp;
	long operand;

	cpid = ((inst & 0xe00)>>9);
	printf("\tcp%d",cpid);

	if (inst & 0x80) {
		/* cpBcc */
		disp = instfetch((inst & 0x40)?LONG:WORD,TRUE);
		condition = (inst & 0x3f);
		printf("B%x\t,",condition);
		psymoff(disp+inkdot(2),ISYM,"");
	}
	else if ((inst & 0x1f8) == 0x48) {
		/* cpDBcc */
		condition = instfetch(WORD,FALSE);
		condition &= 0x3f;
		disp = instfetch(WORD,TRUE);
		printf("DB%x\t,",condition);
		psymoff(disp+inkdot(2),ISYM,"");
	}
	else if (((inst & 0x1f8) == 0x78) && (inst & 6)) {
		/* cpTRAPcc */
		condition = instfetch(WORD,FALSE);
		condition &= 0x3f;
		printf("TRAP%x\t",condition);
		if (inst & 2) {
			/* get operand word(s) */
			operand = instfetch((inst & 1)?LONG:WORD,TRUE);
			printf("#%d",operand);
		}
	}
	else {
		code = ((inst & 0x1c0)>>6);

		if (code == 0) {
			/* cpGEN */
			command = instfetch(WORD,FALSE);
			printf("GEN\t#0x%x,",command);
			printEA(inst,0);
		}
		else if (code == 5) {
			/* cpRESTORE */
			printf("RESTORE\t");
			printEA(inst,0);
		}
		else if (code == 4) {
			/* cpSAVE */
			printf("SAVE\t");
			printEA(inst,0);
		}
		else if (code == 1) {
			/* cpScc */
			condition = instfetch(WORD,FALSE);
			condition &= 0x3f;
			printf("S%x\t",condition);
			printEA(inst,1);
		}
	}

}
