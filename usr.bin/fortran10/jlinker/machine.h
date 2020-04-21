#ifndef machine_h
#define machine_h
typedef unsigned int Address;
typedef unsigned char Byte;
typedef unsigned int Word;

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

printinst(/* lowaddr, highaddr */);
printninst(/* count, addr */);
#ifdef sgi /* GB - -Ddouble='long float' wont change the print format! */
#else
#endif
#ifdef GB_FIXES
#else
#endif
Address printdata(/* lowaddr, highaddr, format */);
printndata(/* count, startaddr, format */);
printvalue(/* v, format */);
printerror(/*  */);
printsig (/* signo */);
endprogram(/*  */);
dostep(/* isnext */);
#ifdef vax
#endif
#if (defined(juniper)||defined(pmII))
#endif
setbp(/* addr */);
unsetbp(/* addr */);
#if (defined(juniper)||defined(pmII))
#ifdef pmII
#endif
#ifdef juniper
#endif
omove(/* inst, s */);
obranch(/* inst, dummy */);
#ifdef juniper
#endif
#ifdef juniper
#endif
odbcc(/* inst, form */);
oscc(/* inst, dummy */);
biti(/* inst, dummy */);
opmode(/* inst, opcode */);
shroi(/* inst, ds */);
oimmed(/* inst, opcode */);
oreg(/* inst, opcode */);
extend(/* inst, opcode */);
#ifdef juniper
olink(/* inst,size */);
otrap(/* inst,opcode */);
#endif
#ifdef pmII
olink(/* inst, dummy */);
otrap(/* inst, dummy */);
#endif
oneop(/* inst, opcode */);
jsrop(/* inst, opcode */);
jmpop(/* inst, opcode */);
pregmask(/* mask */);
omovem(/* inst, size */);
#ifdef juniper
ochk(/* inst,opcode */);
#endif
#ifdef pmII
ochk(/* inst, opcode */);
#endif
soneop(/* inst, opcode */);
oquick(/* inst, opcode */);
omoveq(/* inst, dummy */);
oprint(/* inst, opcode */);
ostop(/* inst, opcode */);
orts(/* inst, opcode */);
ortspop(/* inst, opcode */);
omovs(/* inst, opcode */);
#ifdef juniper
omovesr(/* inst, dummy */);
oorsr(/* inst, dummy */);
ortd(/* inst,dummy */);
omovc(/* inst,dummy */);
#endif
#ifdef pmII
omovc(/* inst, opcode */);
#endif
#ifdef juniper
callm(/* inst,dummy */);
cmp2_chk2(/* inst,size */);
cas(/* inst,size */);
long_div(/* inst,dummy */);
#ifdef NOTDEF /* fix for scr1131 */
#else
#endif
long_mul(/* inst,dummy */);
otrapcc(/* inst,dummy */);
bitfield(/* inst,dummy */);
opack(/* inst,opcode */);
cp_inst(/* inst,opcode */);
#endif
/*
Address nextaddr(/* startaddr, isnext );
*/
beginproc(/* p */);
integer extractField (/* s */);
loophole (/* oldlen, newlen */);
#else ifdef vax
Address nextaddr(/* startaddr, isnext */);
beginproc(/* p, argc */);
integer extractField (/* s */);
loophole (/* oldlen, newlen */);
#endif
#endif
