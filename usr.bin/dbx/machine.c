/* Copyright (c) 1982 Regents of the University of California */

static char sccsid[] = "@(#)machine.c 1.9 8/5/83";

static char rcsid[] = "$Header: /d2/3.7/src/usr.bin/dbx/RCS/machine.c,v 1.1 89/03/27 17:44:35 root Exp $";

/*
 * Target machine dependent stuff.
 */

#include "defs.h"
#include "machine.h"
#include "process.h"
#include "runtime.h"
#include "events.h"
#include "main.h"
#include "symbols.h"
#include "source.h"
#include "mappings.h"
#include "object.h"
#include "tree.h"
#include "eval.h"
#include "keywords.h"
#include "ops.h"
#include <signal.h>

#ifndef public
typedef unsigned int Address;
typedef unsigned char Byte;
typedef unsigned int Word;
private	Boolean used_usignal;

#ifdef vax

#   define NREG 16

#   define ARGP 12
#   define FRP 13
#   define STKP 14
#   define PROGCTR 15

#   define CODESTART 0
#   define FUNCOFFSET 2

#   define MAXSTKADDR (0x80000000 - ctob(UPAGES))

#   define nargspassed(frame) argn(0, frame)

#endif
#if (defined(pmII)||defined(juniper))

/*
 * On the 68000, the pc isn't in a register, but we make believe
 * so there's one more register.
 *
 * Note that there's also no argument pointer, this means code
 * involving "ARGP" should always be #ifdef'd.
 *
 * The address corresponding to the beginning of a function is recorded
 * as the address + FUNCOFFSET (skip the link instruction so that
 * local information is available).
 */

#   define NREG 17

#   define MFRP 13
#   define FRP 14
#   define STKP 15
#   define PROGCTR 16

#   define CALL_RETADDR	0x800c		/* Return address for 'call' command */
#   define FUNCOFFSET 4

#   ifdef sun
#       define CODESTART 0x8000
#       define MAXSTKADDR 0x1000000
#   else /* sgi */
#include <machine/vmparam.h>
#	define CODESTART USRTEXT
#       define MAXSTKADDR USRSTACK
#   endif
#endif

#define BYTE 1
#define WORD 2
#define LONG 4

/* Unfortunately, machine/vmparam.h tends to define BITSPERBYTE.  Thus
   we have renamed it BITS_PER_BYTE.  
    (GB)
*/
#define BITS_PER_BYTE 8
#define BITSPERWORD (BITS_PER_BYTE * sizeof(Word))

Address pc;
Address prtaddr;

#endif

private Address printop();

/*
 * Decode and print the instructions within the given address range.
 */

public printinst(lowaddr, highaddr)
Address lowaddr;
Address highaddr;
{
    register Address addr;

    for (addr = lowaddr; addr <= highaddr; ) {
	addr = printop(addr);
    }
    prtaddr = addr;
}
public printinst1(lowaddr, highaddr)
Address lowaddr;
Address highaddr;
{
    register Address addr;

    for (addr = lowaddr; addr < highaddr; ) {
	addr = printop(addr);
    }
    prtaddr = addr;
}

/*
 * Another approach:  print n instructions starting at the given address.
 */

public printninst(count, addr)
int count;
Address addr;
{
    register Integer i;
    register Address newaddr;

    if (count <= 0) {
	error("non-positive repetition count");
    } else {
	newaddr = addr;
	for (i = 0; i < count; i++) {
	    newaddr = printop(newaddr);
	}
	prtaddr = newaddr;
    }
}

/*
 * Print the contents of the addresses within the given range
 * according to the given format.
 */

typedef struct {
    String name;
    String printfstring;
    int length;
} Format;

private Format fmt[] = {
    { "d", " %d", sizeof(short) },
    { "D", " %ld", sizeof(long) },
    { "o", " %o", sizeof(short) },
    { "O", " %lo", sizeof(long) },
    { "x", " %04x", sizeof(short) },
    { "X", " %08x", sizeof(long) },
    { "b", " \\%o", sizeof(char) },
    { "c", " '%c'", sizeof(char) },
    { "s", "%c", sizeof(char) },
    { "f", " %f", sizeof(float) },
#ifdef sgi /* GB - -Ddouble='long float' wont change the print format! */
    { "g", " %lg", sizeof(double) },
#else
    { "g", " %g", sizeof(double) },
#endif
    { nil, nil, 0 }
};

private Format *findformat(s)
String s;
{
    register Format *f;

    f = &fmt[0];
    while (f->name != nil and not streq(f->name, s)) {
	++f;
    }
    if (f->name == nil) {
	error("bad print format \"%s\"", s);
    }
    return f;
}

/*
 * Retrieve and print out the appropriate data in the given format.
 * Floats have to be handled specially to allow the compiler to
 * convert them to doubles when passing to printf.
 */

private printformat (f, addr)
Format *f;
Address addr;
{
    union {
	char charv;
	short shortv;
	int intv;
	float floatv;
	double doublev;
    } value;

    dread(&value, addr, f->length);
    if (streq(f->name, "f")) {
	printf(f->printfstring, value.floatv);
    } else {
#ifdef GB_FIXES
        if (streq(f->name, "x")) {
	    printf(f->printfstring, (unsigned short)value.shortv);
        } else {
	    switch (f->length)
	    {
	    case 1:	printf(f->printfstring,value.charv);
		    	break;
	    case 2:	printf(f->printfstring,value.shortv);
		    	break;
	    case 4:	printf(f->printfstring,value.intv);
		    	break;
	    case 8:	printf(f->printfstring,value.doublev);
		    	break;
	    }
	}
#else
	printf(f->printfstring, value);
#endif
    }
}

public Address printdata(lowaddr, highaddr, format)
Address lowaddr;
Address highaddr;
String format;
{
    int n;
    register Address addr;
    Format *f;

    if (lowaddr > highaddr) {
	error("first address larger than second");
    }
    f = findformat(format);
    n = 0;
    for (addr = lowaddr; addr <= highaddr; addr += f->length) {
	if (n == 0) {
	    printf("%08x: ", addr);
	}
	printformat(f, addr);
	++n;
	if (n >= (16 div f->length)) {
	    printf("\n");
	    n = 0;
	}
    }
    if (n != 0) {
	printf("\n");
    }
    prtaddr = addr;
    return addr;
}

/*
 * The other approach is to print n items starting with a given address.
 */

public printndata(count, startaddr, format)
int count;
Address startaddr;
String format;
{
    int i, n;
    Address addr;
    Format *f;
    Boolean isstring;
    char c;

    if (count <= 0) {
	error("non-positive repetition count");
    }
    f = findformat(format);
    isstring = (Boolean) streq(f->name, "s");
    n = 0;
    addr = startaddr;
    for (i = 0; i < count; i++) {
	if (n == 0) {
	    printf("%08x: ", addr);
	}
	if (isstring) {
	    printf("\"");
	    dread(&c, addr, sizeof(char));
	    while (c != '\0') {
		printchar(c);
		++addr;
		dread(&c, addr, sizeof(char));
	    }
	    printf("\"\n");
	    n = 0;
	    addr += sizeof(String);
	} else {
	    printformat(f, addr);
	    ++n;
	    if (n >= (16 div f->length)) {
		printf("\n");
		n = 0;
	    }
	    addr += f->length;
	}
    }
    if (n != 0) {
	printf("\n");
    }
    prtaddr = addr;
}

/*
 * Print out a value according to the given format.
 */

public printvalue(v, format)
long v;
String format;
{
    Format *f;
    char *p, *q;

    f = findformat(format);
    if (streq(f->name, "s")) {
	putchar('"');
	p = (char *) &v;
	q = p + sizeof(v);
	while (p < q) {
	    printchar(*p);
	    ++p;
	}
	putchar('"');
    } else {
	printf(f->printfstring, v);
    }
    putchar('\n');
}

/*
 * Print out an execution time error.
 * Assumes the source position of the error has been calculated.
 *
 * Have to check if the -r option was specified; if so then
 * the object file information hasn't been read in yet.
 */

public printerror()
{
    extern Integer sys_nsig;
    extern String sys_siglist[];
    integer err;

    if (isfinished(process)) {
	err = exitcode(process);
	if (err == 0) {
	    printf("\"%s\" terminated normally\n", objname);
	} else {
	    printf("\"%s\" terminated abnormally (exit code %d)\n",
		objname, err
	    );
	}
	erecover();
    }
    if (runfirst) {
	fprintf(stderr, "Entering debugger ...\n");
	init();
    }
    err = errnum(process);
    putchar('\n');
    printsig(err);
    putchar(' ');
    printloc();
    putchar('\n');
    if (curline > 0) {
	printlines(curline, curline);
    } else {
	printinst(pc, pc);
    }
    erecover();
}

/*
 * Print out a signal.
 */

private String illinames[] = {
    "reserved addressing fault",
    "priviliged instruction fault",
   "reserved operand fault"
};

private String fpenames[] = {
    nil,
    "integer overflow trap",
    "integer divide by zero trap",
    "floating overflow trap",
    "floating/decimal divide by zero trap",
    "floating underflow trap",
    "decimal overflow trap",
    "subscript out of range trap",
    "floating overflow fault",
    "floating divide by zero fault",
    "floating undeflow fault"
};

public printsig (signo)
integer signo;
{
    integer code;

    if (signo < 0 or signo > sys_nsig) {
	printf("[signal %d]", signo);
    } else if (signo == 0) {	/* sgi kernel peculiarity */
	printf("memory fault");
    } else {
	printf("%s", sys_siglist[signo]);
    }
    code = errcode(process);
    if (signo == SIGILL) {
	if (code >= 0 and code < sizeof(illinames) / sizeof(illinames[0])) {
	    printf(" (%s)", illinames[code]);
	}
    } else if (signo == SIGFPE) {
	if (code > 0 and code < sizeof(fpenames) / sizeof(fpenames[0])) {
	    printf(" (%s)", fpenames[code]);
	}
    }
}

/*
 * Note the termination of the program.  We do this so as to avoid
 * having the process exit, which would make the values of variables
 * inaccessible.  We do want to flush all output buffers here,
 * otherwise it'll never get done.
 */

public endprogram()
{
    Integer exitcode;

    stepto(nextaddr(pc, true));
    printnews();
    exitcode = argn(1, nil);
    if (exitcode != 0) {
	printf("\nexecution completed (exit code %d)\n", exitcode);
    } else {
	printf("\nexecution completed\n");
    }
    getsrcpos();
    erecover();
}

/*
 * Single step the machine a source line (or instruction if "inst_tracing"
 * is true).  If "isnext" is true, skip over procedure calls.
 */

private Address getcall();

public dostep(isnext)
Boolean isnext;
{
    register Address addr;
    register Lineno line;
    String filename;
    Address startaddr;

    used_usignal = false;

    startaddr = pc;
    addr = nextaddr(pc, isnext);
    if (not inst_tracing and nlhdr.nlines != 0) {
	line = linelookup(addr);
	while (line == 0) {
	    addr = nextaddr(addr, isnext);
	    line = linelookup(addr);
	}
	curline = line;
    } else {
	curline = 0;
    }
    stepto(addr);
    filename = srcfilename(addr);
    setsource(filename);
}

#ifdef vax

typedef char Bpinst;

#define BP_OP       O_BPT       /* breakpoint trap */

#endif
#if (defined(juniper)||defined(pmII))

typedef short Bpinst;

extern Bpinst BP_OP;
#   ifdef sun
	asm("_BP_OP: trap #15");
#   else /* sgi */
	asm("_BP_OP: trap #1");
#   endif

#endif

#define BP_ERRNO    SIGTRAP     /* signal received at a breakpoint */

/*
 * Setting a breakpoint at a location consists of saving
 * the word at the location and poking a BP_OP there.
 *
 * We save the locations and words on a list for use in unsetting.
 */

typedef struct Savelist *Savelist;

struct Savelist {
    Address location;
    Bpinst save;
    short refcount;
    Savelist link;
};

private Savelist savelist;

/*
 * Set a breakpoint at the given address.  Only save the word there
 * if it's not already a breakpoint.
 */

public setbp(addr)
Address addr;
{
    Bpinst w, save;
    register Savelist newsave, s;

    for (s = savelist; s != nil; s = s->link) {
	if (s->location == addr) {
	    s->refcount++;
	    return;
	}
    }
    iread(&save, addr, sizeof(save));
    newsave = new(Savelist);
    newsave->location = addr;
    newsave->save = save;
    newsave->refcount = 1;
    newsave->link = savelist;
    savelist = newsave;
    w = BP_OP;
    iwrite(&w, addr, sizeof(w));
}

/*
 * Unset a breakpoint; unfortunately we have to search the SAVELIST
 * to find the saved value.  The assumption is that the SAVELIST will
 * usually be quite small.
 */

public unsetbp(addr)
Address addr;
{
    register Savelist s, prev;

    prev = nil;
    for (s = savelist; s != nil; s = s->link) {
	if (s->location == addr) {
	    iwrite(&s->save, addr, sizeof(s->save));
	    s->refcount--;
	    if (s->refcount == 0) {
		if (prev == nil) {
		    savelist = s->link;
		} else {
		    prev->link = s->link;
		}
		dispose(s);
	    }
	    return;
	}
	prev = s;
    }
    panic("unsetbp: couldn't find address %d", addr);
}

/*
 * Every line from now on is conditionally compiled in for the target machine,
 * first the 68000 and then the VAX.
 */

#if (defined(juniper)||defined(pmII))
/*
 * Instruction decoding routines for 68000, derived from adb.
 *
 * The shared boolean variable "printing" is true if the decoded
 * instruction is to be printed, false if not.  In either case,
 * the address of the next instruction after the given one is returned.
 */

private Boolean printing;
private Boolean following;
private Boolean followcalls;
private Address instaddr;

#define instread(var) \
{ \
    iread(&var, instaddr, sizeof(var)); \
    instaddr += sizeof(var); \
}

private Optab *decode(inst, addr)
Word inst;
Address addr;
{
    register Optab *o;

    o = &optab[0];
    while (o->mask != 0 and (inst&o->mask) != o->match) {
	++o;
    }
    return o;
}

private Address printop(addr)
Address addr;
{
    Optab *o;
    short inst;

    printf("%08x  ", addr);
    iread(&inst, addr, sizeof(inst));
    o = decode(inst, addr);
    if (o->mask == 0) {
	printf("\tbadop");
	instaddr = addr + sizeof(inst);
    } else {
	printing = true;
	following = false;
	instaddr = addr + sizeof(inst);
	(*o->opfun)(inst, o->farg);
	printing = false;
    }
    printf("\n");
    return instaddr;
}

/*
 * Quickly find the return address of the current procedure or function
 * while single stepping.  Just get the word pointed at by sp.
 */

private Address currtnaddr ()
{
    Address retaddr;

    dread(&retaddr, reg(STKP), sizeof(retaddr));
    return retaddr;
}

/*
 * Print out the effective address for the given parameters.
 */

#ifdef pmII
private printea(mode, reg, size)
long mode, reg;
int size;
{
    long index, disp;
    static char *aregs[] = { "a0", "a1", "a2", "a3", "a4", "a5", "a6", "sp" };
    Byte b;
    short w;
    long l;

    switch ((int)(mode)) {
	case 0:
	    if (printing) {
		printf("d%D", reg);
	    }
	    break;

	case 1:
	    if (printing) {
		printf("%s", aregs[reg]);
	    }
	    break;

	case 2:
	    if (printing) {
		printf("%s@", aregs[reg]);
	    }
	    break;

	case 3:
	    if (printing) {
		printf("%s@+", aregs[reg]);
	    }
	    break;

	case 4:
	    if (printing) {
		printf("%s@-", aregs[reg]);
	    }
	    break;

	case 5:
	    instread(w);
	    if (printing) {
		printf("%s@(%D)", aregs[reg], w);
	    }
	    break;

	case 6:
	    instread(w);
	    if (printing) {
		index = w;
		disp = (char)(index&0377);
		printf("%s@(%d,%c%D:%c)", aregs[reg], disp,
		    (index&0100000)?'a':'d',(index>>12)&07,
		    (index&04000)?'l':'w');
	    }
	    break;

	case 7:
	    switch ((int)(reg)) {
		case 0:
		    instread(w);
		    if (printing) {
			index = w;
			psymoff(index);
		    }
		    break;

		case 1:
		    instread(l);
		    if (printing) {
			index = l;
			psymoff(index);
		    }
		    break;

		case 2:	
		    instread(w);
		    if (printing) {
			disp = w;
			psymoff(disp + instaddr);
		    }
		    break;

		case 3:
		    instread(w);
		    if (printing) {
			index = w;
			disp = (char)(index&0377);
			printf("pc@(%D,%c%D:%c)", disp,
			    (index&0100000)?'a':'d',(index>>12)&07,
			    (index&04000)?'l':'w');
		    }
		    break;

		case 4:
		    switch (size) {
			case sizeof(b):
			    instread(w);
			    index = (w&0xff);
			    break;

			case sizeof(w):
			    instread(w);
			    index = w;
			    break;

			case sizeof(l):
			    instread(l);
			    index = l;
			    break;

			default:
			    if (printing) {
			    	printf("unexpected size %d in printea\n", size);
			    }
			    instread(l);
			    index = l;
			    break;
		    }
		    if (printing) {
			printf(IMDF, index);
		    }
		    break;

		default:
		    if (printing) {
			printf("???");
		    }
		    break;
	    }
	    break;

	default:
	    if (printing) {
		printf("???");
	    }
	    break;
    }
}
#endif

#ifdef juniper
private printea(mode,reg,size)
long mode, reg;
int size;
{
	unsigned long index;
	long basedisp;
	long outerdisp;
	long disp;
	unsigned short us;
	short s;
	unsigned short format_word;
	char odsize, isindexed, a_index, indexreg, 
	     scale, l_index, ismemind, ispc;


	switch ((int)(mode)) {
	case 0:
		if (printing) printf("d%D",reg);
		break;

	case 1:
		if (printing) printf("a%D",reg);
		break;

	case 2:
		if (printing) printf("a%D@",reg);
		break;

	case 3:
		if (printing) printf("a%D@+",reg);
		break;

	case 4:
		if (printing) printf("a%D@-",reg);
		break;

	case 5:
		/* an@(d) d = 16-bit */
		instread(s);
		if (printing) printf("a%D@(%D.)",reg,s);
		break;

	case 6:
		ispc = 0;
doindex:
		/* GB modes 6 and 7 get much more complex on the 68020 */
		/* mode 6 is aN indexed.  If bit 8 of the format word
		   is zero, it is the old simple 8-bit index */
		instread(format_word);
		if (!(format_word & 0x100)) {

			/* old eight-bit index.  Disp is still low 8 bits.
			*/
			disp = (char)(format_word&0377);

			/* With 68020, index can be scaled by 1,2,4 or 8.
			*/
			if (printing) printf("%c%c@(%d,%c%D:%c*%c)",(ispc?'p':'a'),
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
				if (bdsize == 2) {
					instread(s);
					basedisp = s;
				}
				else instread(basedisp);
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
			if (odsize == 2) {
				instread(s);
				outerdisp = s;
			}
			else instread(outerdisp);
		}

		/* ok, we print the formats as follows:

			aN@(disp,[Xn])  	- indexed, non-indirect
			an@(disp,[Xn])@(d)	- pre-indexed, indirect
			an@(disp)@(disp,[Xn])   - post-indexed, indirect
		*/
		if (printing) printf("a%d@(%d",reg,basedisp);
		/* the index reg comes next if it is indexed and 
		   it is NOT post-indexed
		*/
		if ((isindexed) && (!(format_word & 4)))
		{
			if (printing) printf(",[%c%D:%c*%c]",(a_index?'a':'d'),indexreg,
				(l_index?'l':'w'),(scalechar[scale]));
			isindexed=0;
		}

		if (printing) printf(")");
		if (ismemind) {
			if (printing) printf("@(%d",outerdisp);
			if (isindexed)
				if (printing) printf(",[%c%D:%c*%c]",(a_index?'a':'d'),
					indexreg,(l_index?'l':'w'),
					(scalechar[scale]));
			if (printing) printf(")");
		}
		break;


	case 7:
		switch ((int)(reg))
		{
		case 0:
			instread(us);
			index = us;
			if (printing) printf("%x:w",index);
			break;

		case 1:
			instread(index);
			if (printing) psymoff(index);
			break;

		case 2:
			instread(s);
			if (printing) printf("pc@(%D.)",s);
			break;

		case 3:
			ispc = 1;
			goto doindex;


		case 4:
			if (size <= 2) {
				instread(us);
				index = us;
			} 
			else instread(index);
			if (printing) printf(IMDF, index);
			break;

		default:
			if (printing) printf("???");
			break;
		}
		break;

	default:
		if (printing) printf("???");
	}
}
#endif

private printEA(ea, size)
long ea;
int size;
{

    printea((ea>>3)&07, ea&07, size);
}

private mapsize(inst)
register long inst;
{
    int m;

    inst >>= 6;
    inst &= 03;
    switch (inst) {
	case 0:
	    m = 1;
	    break;

	case 1:
	    m = 2;
	    break;

	case 2:
	    m = 4;
	    break;

	default:
	    m = -1;
	    break;
    }
    return m;
}


private int suffix_len(str)
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

private char suffix(size)
int size;
{
    char c;

    switch (size) {
	case 1:
	    c = 'b';
	    break;

	case 2:
	    c = 'w';
	    break;

	case 4:
	    c = 'l';
	    break;

	default:
	    panic("bad size %d in suffix", size);
    }
    return c;
}

/*
 * Print an address offset.  Eventually this should attempt to be symbolic,
 * but for now its just printed in hex.
 */

private psymoff (off)
Word off;
{
    Symbol f;

    f = whatblock((Address) (off + FUNCOFFSET));
    if (codeloc(f) == off + FUNCOFFSET) {
	printf("%s", symname(f));
    } else {
	printf("0x%x", off);
    }
}

/*
 * Instruction class specific routines.
 */

public omove(inst, s)
long inst;
String s;
{
    register int c;
    int size;

    c = s[0];
    if (printing) {
	printf("\tmov%c\t", c);
    }
    size = ((c == 'b') ? 1 : (c == 'w') ? 2 : 4);
    printea((inst>>3)&07, inst&07, size);
    if (printing) {
	printf(",");
    }
    printea((inst>>6)&07, (inst>>9)&07, size);
}

/* 
 * Two types: bsr (4 bytes) and bsrs (2 bytes)
 */

public obranch(inst, dummy)
long inst;
{
    long disp;
    String s; 
    short w;
    Address startingaddr;	/* address of branch instruction */
    int branchtype;		/* type of branch (0 = unconditional) */
    Address dest;
    Address retaddr;		/* for bsr instruction */

    startingaddr = instaddr - 2;
    disp = inst&0377;
    s = "s ";
#ifdef juniper
    if (disp == 0xff)
	retaddr = startingaddr + 6;
    else 
#endif
    if (disp == 0) {
	retaddr = startingaddr + 4;
    } else {
	retaddr = startingaddr + 2;
    }
#ifdef juniper
    if (disp == 255) {
	s = "l ";
	instread(disp);
    }
    else 
#endif
    if (disp > 127) {
	disp |= ~0377;
    } else if (disp == 0){
	s = " ";
	instread(w);
	disp = w;
    }
    branchtype = (int)((inst>>8)&017);
    dest = startingaddr + 2 + disp;
    if (printing) {
	printf("\tb%s%s\t", bname[branchtype], s);
	psymoff(dest);
    }
    if (following) {
	/*
	 * If we're to follow the dynamic flow of instructions,
	 * we must see where the branch leads.  A branchtype of 0
	 * indicates an unconditional branch which we simply take
	 * as the new instruction address.  For a conditional branch,
	 * we continue execution up to the current address, single step,
	 * and keep going.
	 */
	if (branchtype == 0) {
	    instaddr = dest;
	} else if (branchtype == 01) {		/* bsr */
	    if (followcalls) {
		steppast(startingaddr);
		curfunc = whatblock(pc, true);
		if (not isbperr()) {
		    printstatus();
		    /* NOTREACHED */
		}
		bpact();
		if (nosource(curfunc) and canskip(curfunc) and
		  nlhdr.nlines != 0) {
		    stepto(retaddr);
		    instaddr = pc;
		    bpact();
		} else {
		    callnews(/* iscall = */ true);
		}
	    }
	} else {
	    steppast(startingaddr);
	}
    }
}

public odbcc(inst, form)
long inst;
String form;
{
    long disp;
    short w;

    instread(w);
    if (printing) {
    	printf(form, dbname[(int)((inst>>8)&017)], inst&07);
	psymoff(w + sizeof(w));
    }
}

public oscc(inst, dummy)
long inst;
long dummy;
{
    if (printing) {
	printf("\ts%s\t", cname[(int)((inst>>8)&017)]);
    }
    printea((inst>>3)&07, inst&07, 1);
}

public biti(inst, dummy)
long inst;
long dummy;
{
    short w;

    if (printing) {
	printf("\t%s\t", bit[(int)((inst>>6)&03)]);
    }
    if (inst&0x0100) {
	if (printing) {
	    printf("d%D,", inst>>9);
	}
    } else {
	instread(w);
	if (printing) {
	    printf(IMDF, w);
	    printf(",");
	}
    }
    printEA(inst);
}

public opmode(inst, opcode)
long inst;
long opcode;
{
    register int opmode;
    register int reg;
    int size;

    opmode = (int)((inst>>6) & 07);
    reg = (int)((inst>>9) & 07);
    if (opmode == 0 or opmode == 4) {
	size = 1;
    } else if (opmode == 1 or opmode == 3 or opmode == 5) {
	size = 2;
    } else {
	size = 4;
    }
    if (printing) {
	printf("\t%s%c\t", opcode, suffix(size));
    }
    if (opmode >= 4 and opmode <= 6) {
	if (printing) {
	    printf("d%d,", reg);
	}
	printea((inst>>3)&07, inst&07, size);
    } else {
	printea((inst>>3)&07, inst&07, size);
	if (printing) {
	    printf(",%c%d",(opmode<=2) ? 'd' : 'a', reg);
	}
    }
}

public shroi(inst, ds)
long inst;
String ds;
{
    int rx, ry;
    String opcode;

    if ((inst & 0xC0) == 0xC0) {
	opcode = shro[(int)((inst>>9)&03)];
	if (printing) {
	    printf("\t%s%s\t", opcode, ds);
	}
	printEA(inst);
    } else {
	if (printing) {
	    opcode = shro[(int)((inst>>3)&03)];
	    printf("\t%s%s%c\t", opcode, ds, suffix(mapsize(inst)));
	    rx = (int)((inst>>9)&07); ry = (int)(inst&07);
	    if ((inst>>5)&01) {
		printf("d%d,d%d", rx, ry);
	    } else {
		printf(IMDF, (rx ? rx : 8));
		printf(",d%d", ry);
	    }
	}
    }
}		

public oimmed(inst, opcode)
long inst;
register String opcode;
{
    register int size;
    long const;
    short w;

    size = mapsize(inst);
    if (size > 0) {
	if (size == 4) {
	    instread(const);
	} else {
	    instread(w);
	    const = w;
	}
	if (printing) {
	    printf("\t%s%c\t", opcode, suffix(size));
	    printf(IMDF, const);
	    printf(",");
	}
	printEA(inst, size);
    } else {
	if (printing) {
	    printf("\tbadop");
	}
    }
}

public oreg(inst, opcode)
long inst;
register String opcode;
{
    if (printing) {
	printf(opcode, (inst & 07));
    }
}

public extend(inst, opcode)
long inst;
String opcode;
{
    register int size;
    int ry, rx;
    char c;

    if (printing) {
	size = mapsize(inst);
	ry = (inst&07);
	rx = ((inst>>9)&07);
	c = ((inst & 0x1000) ? suffix(size) : ' ');
	printf("\t%s%c\t", opcode, c);
	if (opcode[0] == 'e') {
	    if (inst & 0x0080) {
		printf("d%D,a%D", rx, ry);
	    } else if (inst & 0x0008) {
		printf("a%D,a%D", rx, ry);
	    } else {
		printf("d%D,d%D", rx, ry);
	    }
	} else if ((inst & 0xF000) == 0xB000) {
	    printf("a%D@+,a%D@+", ry, rx);
	} else if (inst & 0x8) {
	    printf("a%D@-,a%D@-", ry, rx);
	} else {
	    printf("d%D,d%D", ry, rx);
	}
    }
}

#ifdef juniper
public olink(inst,size)
long inst;
int size;
{
	short s; long l;
	/* (GB) size is now LONG or WORD */
	if (printing) printf("\tlink%c\ta%D,", (size == LONG)?'l':'w',inst&07);
	if (size == WORD) {
		instread(s);
		l = s;
	}
	else 
		instread(l);
	if (printing) printf("#%D", l);
}

public otrap(inst,opcode)
long inst;
char *opcode;
{
	int type = ((inst & 0x0c00) >> 10);
	char *fmt = "#%d\n";
	int mask = 7;

	/* GB - decode bkpt, trap and rtm instructions */
	if (printing) printf("\t%s\t",opcode);
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
	if (printing) printf(fmt, inst&mask);
}
#endif
#ifdef pmII
public olink(inst, dummy)
long inst;
long dummy;
{
    short w;

    instread(w);
    if (printing) {
	printf("\tlink\ta%D,", inst&07);
	printf(IMDF, w);
    }
}

public otrap(inst, dummy)
long inst;
{
    if (printing) {
	printf("\ttrap\t");
	printf(IMDF, inst&017);
    }
}
#endif

public oneop(inst, opcode)
long inst;
register String opcode;
{
    if (printing) {
	printf("\t%s",opcode);
    }
    printEA(inst,suffix_len(opcode));
}

/** GB FIX jsrop and jmpop **/
public jsrop(inst, opcode)
long inst;
register String opcode;
{
    Address startingaddr;	/* beginning of jsr instruction */
    Address retaddr; /* can't call return_addr (frame not set up yet) */
	Symbol	getwrapee();

    startingaddr = instaddr - 2;
    switch ((inst >> 3) & 07) {
	case 2:
	    retaddr = instaddr;		/* two byte instruction */
	    break;
	case 5:
	case 6:
	    retaddr = instaddr + 2;	/* four byte instruction */
	    break;
	case 7:
	default:
	    switch (inst & 07) {
		case 0:
		case 2:
		case 3:
		    retaddr = instaddr + 2;
		    break;
		case 1:
		default:
		    retaddr = instaddr + 4;	/* six byte instruction */
		    break;
	    }
	    break;
    }
    if (printing) {
	printf("\t%s\t",opcode);
    }
    printEA(inst,suffix_len(opcode));
    if (following and followcalls) {
	steppast(startingaddr);
	curfunc = whatblock(pc, true);
        if (iswrapper(curfunc)) {
		curfunc = getwrapee(curfunc);
	}
	if (not isbperr()) {
	    printstatus();
	    /* NOTREACHED */
	}
	bpact();
	if (nosource(curfunc) and canskip(curfunc) and (nlhdr.nlines != 0)
			and (inst_tracing == false)) {
	    stepto(retaddr);
	    instaddr = pc;
	    bpact();
	} else {
	    callnews(/* iscall = */ true);
	}
    }
}

public jmpop(inst, opcode)
long inst;
register String opcode;
{
    Address startingaddr;	/* beginning of jump instruction */

    startingaddr = instaddr - 2;
    if (printing) {
	printf("\t%s",opcode);
    }
    printEA(inst,suffix_len(opcode));
    if (following) {
	steppast(startingaddr);
    }
}

public pregmask(mask)
register int mask;
{
    register int i;
    register int flag = 0;

    if (printing) {
	printf("#<");
	for (i=0; i<16; i++) {
	    if (mask&1) {
		if (flag) {
		    printf(",");
		} else {
		    ++flag;
		}
		printf("%c%d",(i<8) ? 'd' : 'a', i&07);
	    }
	    mask >>= 1;
	}
	printf(">");
    }
}

public omovem(inst, size)
long inst;
int size;
{
    register int i, list, mask;
    register int reglist;
    short w;

    i = 0;
    list = 0;
    mask = 0100000;
    instread(w);
    reglist = w;
    if ((inst & 070) == 040) {	/* predecrement */
	for (i = 15; i > 0; i -= 2) {
	    list |= ((mask & reglist) >> i);
	    mask >>= 1;
	}
	for (i = 1; i < 16; i += 2) {
	    list |= ((mask & reglist) << i);
	    mask >>= 1;
	}
	reglist = list;
    }
    if (printing) {
	printf("\tmovem%c\t",(inst&100)?'l':'w');
    }
    if (inst&02000) {
	printEA(inst,size);
	if (printing) {
	    printf(",");
	}
	pregmask(reglist);
    } else {
	pregmask(reglist);
	if (printing) {
	    printf(",");
	}
	printEA(inst,size);
    }
}

#ifdef juniper
public ochk(inst,opcode)
long inst;
register char *opcode;
{
	/* GB - used for chkw, chkl, lea, {div,mul}uw and {div,mul}sw. 
	*/
	int size;
	/* size is WORD unless lea or chkl */
	size = (int)WORD;
	if ((*opcode == 'l')||((*opcode == 'c')&&(*(opcode+3)=='l')))
		size = (int)LONG;

	if (printing) printf("\t%s\t",opcode);
	printEA(inst,size);
	if (printing) printf(",%c%D",(*opcode=='l')?'a':'d',(inst>>9)&07);
}
#endif

#ifdef pmII
public ochk(inst, opcode)
long inst;
register String opcode;
{
    if (printing) {
	printf("\t%s\t", opcode);
    }
    printEA(inst, sizeof(Byte));
    if (printing) {
	printf(",%c%D", (opcode[0] == 'l') ? 'a' : 'd', (inst>>9)&07);
    }
}
#endif

public soneop(inst, opcode)
long inst;
register String opcode;
{
    register int size;

    size = mapsize(inst);
    if (size > 0) {
	if (printing) {
	    printf("\t%s%c\t", opcode, suffix(size));
	}
	printEA(inst,size);
    } else {
	if (printing) {
	    printf("\tbadop");
	}
    }
}

public oquick(inst, opcode)
long inst;
register String opcode;
{
    register int size;
    register int data;

/*
	if (following) {
		bpact();
	}
*/
    size = mapsize(inst);
    data = (int)((inst>>9) & 07);
    if (data == 0) {
	data = 8;
    }
    if (size > 0) {
	if (printing) {
	    printf("\t%s%c\t", opcode, suffix(size));
	    printf(IMDF, data);
	    printf(",");
	}
	printEA(inst,size);
    } else {
	if (printing) {
	    printf("\tbadop");
	}
    }
}

public omoveq(inst, dummy)
long inst;
long dummy;
{
    register int data;

    if (printing) {
	data = (int)(inst & 0377);
	if (data > 127) {
	    data |= ~0377;
	}
	printf("\tmoveq\t");
	printf(IMDF, data);
	printf(",d%D", (inst>>9)&07);
    }
}

public oprint(inst, opcode)
long inst;
register String opcode;
{
    if (printing) {
	printf("\t%s",opcode);
    }
}

public ostop(inst, opcode)
long inst;
register String opcode;
{
    short w;

    instread(w);
    if (printing) {
	printf(opcode, w);
    }
}

public orts(inst, opcode)
long inst;
register String opcode;
{
    Address addr;

    if (following) {
	callnews(/* iscall = */ false);
    	if (inst_tracing) {
	    addr = currtnaddr();
    	} else {
	    addr = return_addr();
	    if (addr == 0) {
		stepto(instaddr - 2);
		addr = currtnaddr();
	    }
	}
	stepto(addr);
	instaddr = pc;
    }
    if (printing) {
	printf("\t%s",opcode);
    }
}

/*
 * Not used by C compiler; does an rts but before doing so, pops
 * arg bytes from the stack.
 */

public ortspop(inst, opcode)
long inst;
register String opcode;
{
    Address addr;
    short w;

    instread(w);
    if (following) {
	callnews(/* iscall = */ false);
    	if (inst_tracing) {
	    addr = currtnaddr();
    	} else {
	    addr = return_addr();
	}
	stepto(addr);
	instaddr = pc;
    }
    if (printing) {
	printf(opcode, w);
    }
}

public omovs(inst, opcode)
long inst;
String opcode;
{
    register int size;
    register unsigned int controlword;
    short w;

    size = mapsize(inst);
    instread(w);
    controlword = w >> 11;
    if (printing) {
	printf("\t%s%c\t", opcode, suffix(size));
    }
    if (controlword & 1){
	controlword >>= 1;
	if (printing) {
	    printf((controlword&0x8) ? "a%D," : "d%D,", controlword&7 );
	}
	printEA(inst&0xff, size);
    } else {
	controlword >>= 1;
	printEA(inst&0xff, size);
	if (printing) {
	    printf((controlword&0x8) ? ",a%D" : ",d%D", controlword&7);
	}
    }
}

#ifdef juniper

public omovesr(inst, dummy)
long inst;
{
	int mode, reg;

	if (printing) printf("\tmovw\t");
	mode = (inst >> 3) & 7;
	reg = inst & 7;
	inst &= ~077; /* strip out mode,reg bits */
	switch (inst) {
	case 0x40c0: /* move from sr */
		if(printing) printf("sr,");
		printea(mode, reg, 2);
		break;
	case 0x42c0: /* move from ccr */
		if(printing) printf("ccr,");
		printea(mode, reg, 2);
		break;
	case 0x44c0: /* move to ccr */
		printea(mode, reg, 2);
		if(printing) printf(",ccr");
		break;
	case 0x46c0: /* move to sr */
		printea(mode, reg, 2);
		if(printing) printf(",sr");
		break;
	}
}

public oorsr(inst, dummy)
long inst;
{
	unsigned short s;
	instread(s);
	if(printing) printf("\t%s%c\t#%x,%s",
	    		inst & 0x0200 ? "and" : "or",
	    		inst & 0x0040 ? 'w' : 'b',
	    		s,
	    		inst & 0x0040 ? "sr" : "ccr");
}

public ortd(inst,dummy)
long inst;
{
	short s;
	instread(s);
	if (printing) {
		printf("\trtd\t");
		printf("#%D", s);
	}
}

public omovc(inst,dummy)
long inst;
{
	register long ext, cntrl;
	unsigned short us;

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

	instread(us);
	ext = us;

	/* 
	 * 000...002 -> 000...002
	 * 800...804 -> 003...007
	 *
	*/
	cntrl = (ext & 7);
	if ((ext & 0x800)) cntrl += 3;
	else if (cntrl > 2) cntrl = 0x20;
	if (cntrl > 7) if (printing) printf("\tbadop");
	else {
		if (printing) printf("\tmovec\t");
		if (inst & 1) {
			if (printing) printf("%c%D,%s", (ext & 0x8000)? 'a': 'd',
			    	(ext >> 12) & 07, creg[cntrl]);
		} else {
			if (printing) printf("%s,%c%D", creg[cntrl],
			    	(ext & 0x8000)? 'a': 'd', 
				(ext >> 12) & 07);
		}
	}
}
#endif

#ifdef pmII
public omovc(inst, opcode)
long inst;
String opcode;
{
    register unsigned int controlword;
    String creg;
    short w;

    instread(w);
    if (printing) {
	controlword = w;
	switch (controlword & 0xfff) {
	    case 0:
		creg = "sfc";
		break;

	    case 1:
		creg = "dfc";
		break;

	    case 0x800:
		creg = "usp";
		break;

	    case 0x801:
		creg = "vbr";
		break;

	    default:
		creg = "???";
		break;
	}
	controlword >>= 12;
	if (inst & 1){
	    if (printing) printf((controlword&0x8) ? "%sa%D,%s" : "%sd%D,%s",
		opcode, controlword&7, creg );
	} else {
	    if (printing) printf((controlword&0x8) ? "%s%s,a%D" : "%s%s,d%D",
		opcode, creg, controlword&7 );
	}
    }
}
#endif
#ifdef juniper
public callm(inst,dummy)
long inst;
int dummy;
{
	short argc;
	instread(argc);
	if (printing) printf("\tcallm\t#%d,",argc);
	printEA(inst,0);
}


public cmp2_chk2(inst,size)
long inst;
int size;
{
	unsigned short extension;
	char *str;
	instread(extension);
	if (extension & 0x4000) str = "\tchk2%c\t";
	else str = "\tcmp2%c\t";

	if (printing) printf(str,suffix(size));
	printEA(inst,size);

	if (printing) printf(",%c%d",(extension & 0x8000)?'a':'d',
		((extension & 7000)>>12));

}


public cas(inst,size)
long inst;
int size;
{
	/* decode the cas and cas2 instructions. */
	int dual;
	unsigned short extension0;
	unsigned short extension1;
	/* dual is set if cas2 */
	dual =  ((inst & 0x3f) == 0x3c) ;

	instread(extension0);
	if (dual) {
		instread(extension1); 
		if (printing) printf("\tcas2%c\td%d:d%d,d%d:d%d,",
			suffix(size), (extension0 & 7),
			(extension1 & 7), 
			((extension0 & 0x1c0)>>6),
			((extension1 & 0x1c0)>>6));
		if (printing) printf("%c%d@,%c%d@", 
			(extension0 & 0x8000)?'a':'d',
			((extension0 & 0x7000)>>12),
			(extension1 & 0x8000)?'a':'d',
			((extension1 & 0x7000)>>12));
	}
	else {
		if (printing) printf("\tcas%c\td%d,d%d,",suffix(size),
			(extension0&7), 
			((extension0 & 0x1c0)>>6));
		printEA(inst,size);
	}

}


public long_div(inst,dummy)
long inst;
{
	int is64bitdividend = 0;
	int doublereg = 0;
	unsigned short extension;
	char dq,dr;

	instread(extension);
	is64bitdividend = (extension & 0x400);
	dq = ((extension & 0x7000) >> 12);
	dr = (extension & 7);
	doublereg = (dr != dq);

	if (printing) printf("\tdiv%cl%c\t",(extension & 0x800)?'s':'u',
		((doublereg)&&(!is64bitdividend))?'l':' ');

	printEA(inst,LONG);

	if (printing) {
	if (doublereg) {
		/* two reg form */
		printf(",d%d:d%d",dr,dq);
	}
	else {
		printf(",d%d",dq);
	}
	}

}

public long_mul(inst,dummy)
long inst;
{
	int is64bitproduct = 0;
	unsigned short extension;
	char dh,dl;

	instread(extension);
	is64bitproduct = (extension & 0x400);
	dl = ((extension & 0x7000) >> 12);
	dh = (extension & 7);

	if (printing) printf("\tmul%cl%c\t",(extension & 0x800)?'s':'u');

	printEA(inst,LONG);

	if (is64bitproduct) {
		/* two reg form */
		if (printing) printf(",d%d:d%d",dh,dl);
		if (dh == dl) if (printing) printf(" <illegal>");
	}
	else {
		if (printing) printf(",d%d",dl);
	}
}

public otrapcc(inst,dummy)
long inst;
{
	long data;
	short s;
	int opmode = inst & 07;
	char sz = ' ';
	int nodata = 0;
	/* GB - has to handle 32-bit offsets for Juniper */

	if (opmode == 3) {
		sz = 'l';
		instread(data);
	}
	else if (opmode == 2) {
		sz = 'w';
		instread(s);
		data = s;
	}
	else nodata++;

	if (printing) printf("\ttrap%s%c\t", sccname[(int)((inst>>8)&017)], sz);
	if (!nodata) if (printing) printf("#0x%x",data);
}

typedef enum { BFTST, BFEXTU, BFCHG, BFEXTS, BFCLR, BFFFO, BFSET, BFINS}
	    bftype_t;

char *bfstr[8] = { "bftst", "bfextu", "bfchg", "bfexts", "bfclr",
		   "bfffo", "bfset", "bfins" } ;

public bitfield(inst,dummy)
long inst;
{
	/* bfXXX.  the 3 least significant bits of the
	   most significant byte indicate which type of bitfield it is
	*/

	bftype_t bftype;

	unsigned short extension;
	unsigned short offset, width;
	unsigned char offset_is_reg, width_is_reg;
	unsigned char reg;

	bftype = (bftype_t)((inst & 0x0700)>>8);

	instread(extension);

	/* hack apart the extension word */
	width = (extension  & 0x1f);
	width_is_reg = ((extension & 0x20) != 0);
	offset = ((extension & 0x07c0)>>6);
	offset_is_reg = ((extension & 0x800) != 0);
	reg = ((extension & 0x7000) >> 12); 

	if (printing) printf("\t%s\t",bfstr[(int)bftype]);
	if (bftype == BFINS)
		if (printing) printf("d%d,",reg);

	printEA(inst,0);

	/* now print the offset and width */
	if (printing) printf(" {%s%d:%s%d}", (offset_is_reg)?"d":"",offset,
		(width_is_reg)?"d":"",width);

	if ((bftype == BFFFO)||(bftype == BFEXTU)||(bftype == BFEXTS))
		if (printing) printf(",d%d",reg);

}

public opack(inst,opcode)
unsigned inst;
char *opcode;
{

	/* do the pack and unpk instructions.  These
	   will not be tested, as they are not in
	   as20 yet.
	*/

	short adjustment;
	char yreg, xreg, memory_op;

	if (printing) printf("\t%s\t",opcode);

	instread(adjustment );

	/* get the registers, and whether the source/dest is memory
	   or registers
	*/
	yreg = ((inst & 0xe00)>>8);
	xreg = (inst & 7);
	memory_op = ((inst & 8) != 0);

	if (memory_op)
		if (printing) printf("a%@d-,a%@d-,",xreg,yreg);
	else
		if (printing) printf("d%d,d%d,",xreg,yreg);

	if (printing) printf("#%d",adjustment);

}

public cp_inst(inst,opcode)
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
	short s;
	long disp;
	long operand;

	cpid = ((inst & 0xe00)>>9);
	if (printing) printf("\tcp%d",cpid);

	if (inst & 0x80) {
		/* cpBcc */
		if (inst & 0x40)
			instread(disp)
		else {
			instread(s);
			disp = s;
		}
		condition = (inst & 0x3f);
		if (printing) {
			printf("B%x\t,",condition);
			psymoff(disp+instaddr);
		}
	}
	else if ((inst & 0x1f8) == 0x48) {
		/* cpDBcc */
		instread(condition); 
		condition &= 0x3f;
		instread(s);
		disp = s;
		if (printing) {
			printf("DB%x\t,",condition);
			psymoff(disp+instaddr);
		}
	}
	else if (((inst & 0x1f8) == 0x78) && (inst & 6)) {
		/* cpTRAPcc */
		instread(condition);
		condition &= 0x3f;
		if (printing) printf("TRAP%x\t",condition);
		if (inst & 2) {
			/* get operand word(s) */
			if (inst & 1)
				instread(operand)
			else {
				instread(s);
				operand = s;
			}
			if (printing) printf("#%d",operand);
		}
	}
	else {
		code = ((inst & 0x1c0)>>6);

		if (code == 0) {
			/* cpGEN */
			instread(command );
			if (printing) printf("GEN\t#0x%x,",command);
			printEA(inst,0);
		}
		else if (code == 5) {
			/* cpRESTORE */
			if (printing) printf("RESTORE\t");
			printEA(inst,0);
		}
		else if (code == 4) {
			/* cpSAVE */
			if (printing) printf("SAVE\t");
			printEA(inst,0);
		}
		else if (code == 1) {
			/* cpScc */
			instread(condition );
			condition &= 0x3f;
			if (printing) printf("S%x\t",condition);
			printEA(inst,1);
		}
	}

}
#endif

/*
 * Compute the next address that will be executed from the given one.
 * If "isnext" is true then consider a procedure call as straight line code.
 *
 * Unconditional branches we just follow, for conditional branches
 * we continue execution to the current location and then single step
 * the machine.
 */

public Address nextaddr(startaddr, isnext)
Address startaddr;
Boolean isnext;
{
    Optab *o;
    short inst;

    instaddr = usignal(process);

    if (instaddr == 0 or instaddr == 1 or used_usignal == true) {
	following = true;
	followcalls = (Boolean) (not isnext);
	printing = false;
	iread(&inst, startaddr, sizeof(inst));
	instaddr = startaddr + sizeof(inst);
	o = decode(inst, startaddr);
	if (o->mask == 0) {
	    fprintf(stderr,
		"[internal error: undecodable op at 0x%x]\n", startaddr);
	    fflush(stderr);
	} else {
	    (*o->opfun)(inst, o->farg);
	}
	following = false;
    } else {
	used_usignal = true;
    }
    return instaddr;
}

/*
 * Step to the given address and then execute one instruction past it.
 * Set instaddr to the new instruction address.
 */

private steppast(addr)
Address addr;
{
    stepto(addr);
    pstep(process, DEFSIG);
    pc = reg(PROGCTR);
    instaddr = pc;
    if (not isbperr()) {
	printstatus();
    }
}

/*
 * Enter a procedure by creating and executing a call instruction.
 */

#define CALLSIZE 6	/* size of call instruction */

public beginproc(p)
Symbol p;
{
    char save[CALLSIZE];
    struct {
	short op;
	char addr[sizeof(long)];	/* unaligned long */
    } call;
    long dest;

    pc = codestart + 6;
    iread(save, pc, sizeof(save));
    call.op = 0x4eb9;			/* jsr */
    dest = codeloc(p) - FUNCOFFSET;
    mov(&dest, call.addr, sizeof(call.addr));
    iwrite(&call, pc, sizeof(call));
    setreg(PROGCTR, pc);
    pstep(process, DEFSIG);
    iwrite(save, pc, sizeof(save));
    pc = reg(PROGCTR);
    if (not isbperr()) {
	printstatus();
    }
    /*
     * Execute link instruction so the return addr is visible.
     */
    pstep(process, DEFSIG);
    pc = reg(PROGCTR);
    if (not isbperr()) {
	printstatus();
    }
}

/*
 * Extract a bit field from an integer.
 */

public integer extractField (s)
Symbol s;
{
    integer nbytes, nbits, n, r, off, len;

    off = s->symvalue.field.offset;
    len = s->symvalue.field.length;
    nbytes = size(s);
    n = 0;
    popn(nbytes, ((char *) &n) + (sizeof(Word) - nbytes));
    nbits = nbytes * BITS_PER_BYTE;
    if (((off mod BITS_PER_BYTE) != 0) || ((len mod BITS_PER_BYTE) != 0)) {
    	r = n >> (nbits - ((off mod nbits) + len));
    	r &= ((1 << len) - 1);
    } else {
	r = n;
    }
    return r;
}

/*
 * Change the length of a value in memory according to a given difference
 * in the lengths of its new and old types.
 */

public loophole (oldlen, newlen)
integer oldlen, newlen;
{
    integer i, n;
    Stack *oldsp;

    n = newlen - oldlen;
    oldsp = sp - oldlen;
    if (n > 0) {
	for (i = oldlen - 1; i >= 0; i--) {
	    oldsp[n + i] = oldsp[i];
	}
	for (i = 0; i < n; i++) {
	    oldsp[i] = '\0';
	}
    } else {
	for (i = 0; i < newlen; i++) {
	    oldsp[i] = oldsp[i - n];
	}
    }
    sp += n;
}

#endif
