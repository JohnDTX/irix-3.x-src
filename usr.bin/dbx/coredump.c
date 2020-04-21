/* Copyright (c) 1982 Regents of the University of California */

static char sccsid[] = "@(#)coredump.c 1.4 1/25/83";

static char rcsid[] = "$Header: /d2/3.7/src/usr.bin/dbx/RCS/coredump.c,v 1.1 89/03/27 17:44:22 root Exp $";

/*
 * Deal with the core dump anachronism.
 */

#include "defs.h"
#include "coredump.h"
#include "machine.h"
#include "object.h"
#include "main.h"
#include <sys/param.h>
#include <sys/dir.h>
#include <machine/psr.h>
#include <machine/pte.h>
#include <sys/user.h>
#include <sys/vm.h>
#include <machine/reg.h>
#include <machine/cpureg.h>
#include <a.out.h>

#ifndef public
#define coredump_readin(m, r, s) coredump_xreadin(&(m), r, &(s))

#include "machine.h"
#endif

#ifndef UDOT_VBASE
#   define UDOT_VBASE UDOTBASE
#endif

typedef struct {
    Address begin;
    Address end;
    Address seekaddr;
} Map;

private Map datamap, stkmap;
private File objfile;
private struct exec hdr;

/*
 * Special variables for debugging the kernel.
 */

private integer masterpcbb;
private integer slr;
private struct pte *sbr;
private struct pcb pcb;

private getpcb ()
{
    integer i;

    fseek(corefile, masterpcbb & ~0x80000000, 0);
    get(corefile, pcb);
    pcb.pcb_p0lr &= ~AST_CLR;
    printf("p0br %lx p0lr %lx p1br %lx p1lr %lx\n",
	pcb.pcb_p0br, pcb.pcb_p0lr, pcb.pcb_p1br, pcb.pcb_p1lr
    );
#   ifdef vax
	setreg(0, pcb.pcb_r0);
	setreg(1, pcb.pcb_r1);
	setreg(2, pcb.pcb_r2);
	setreg(3, pcb.pcb_r3);
	setreg(4, pcb.pcb_r4);
	setreg(5, pcb.pcb_r5);
	setreg(6, pcb.pcb_r6);
	setreg(7, pcb.pcb_r7);
	setreg(8, pcb.pcb_r8);
	setreg(9, pcb.pcb_r9);
	setreg(10, pcb.pcb_r10);
	setreg(11, pcb.pcb_r11);
	setreg(ARGP, pcb.pcb_ap);
	setreg(FRP, pcb.pcb_fp);
	setreg(STKP, pcb.pcb_ksp);
	setreg(PROGCTR, pcb.pcb_pc);
#   else /* 68000 */
#	ifdef sun
	    for (i = 0; i < 14; i++) {
		setreg(i, pcb.pcb_regs.val[i]);
	    }
#       else /* sgi
	    for (i = 0; i < 14; i++) {
		setreg(i, pcb.pcb_regs[i]);
	    } */
#       endif
#   endif
}

public coredump_getkerinfo ()
{
    Symbol s;

    s = lookup(identname("Sysmap", true));
    if (s == nil) {
	panic("can't find 'Sysmap'");
    }
    sbr = (struct pte *) (s->symvalue.offset);
    s = lookup(identname("Syssize", true));
    if (s == nil) {
	panic("can't find 'Syssize'");
    }
    slr = (integer) (s->symvalue.offset);
    printf("sbr %lx slr %lx\n", sbr, slr);
    s = lookup(identname("masterpaddr", true));
    if (s == nil) {
	panic("can't find 'masterpaddr'");
    }
    fseek(
	corefile,
	datamap.seekaddr + s->symvalue.offset&0x7fffffff - datamap.begin,
	0
    );
    get(corefile, masterpcbb);
#ifdef juniper
    masterpcbb = (masterpcbb&PTE_PFNUM)*512;
#else
    masterpcbb = (masterpcbb&PG_PFNUM)*512;
#endif
    getpcb();
}

private copyregs (savreg, reg)
Word savreg[], reg[];
{
#   ifdef vax
	reg[0] = savreg[R0];
	reg[1] = savreg[R1];
	reg[2] = savreg[R2];
	reg[3] = savreg[R3];
	reg[4] = savreg[R4];
	reg[5] = savreg[R5];
	reg[6] = savreg[R6];
	reg[7] = savreg[R7];
	reg[8] = savreg[R8];
	reg[9] = savreg[R9];
	reg[10] = savreg[R10];
	reg[11] = savreg[R11];
	reg[ARGP] = savreg[AP];
	reg[FRP] = savreg[FP];
	reg[STKP] = savreg[SP];
	reg[PROGCTR] = savreg[PC];
#   endif 
#   if (defined(pmII)||defined(juniper))
	reg[0] = savreg[R0];
	reg[1] = savreg[R1];
	reg[2] = savreg[R2];
	reg[3] = savreg[R3];
	reg[4] = savreg[R4];
	reg[5] = savreg[R5];
	reg[6] = savreg[R6];
	reg[7] = savreg[R7];
	reg[8] = savreg[AR0];
	reg[9] = savreg[AR1];
	reg[10] = savreg[AR2];
	reg[11] = savreg[AR3];
	reg[12] = savreg[AR4];
	reg[13] = savreg[AR5];
	reg[14] = savreg[AR6];
	reg[15] = savreg[AR7];
	reg[PROGCTR] = savreg[PC];
#   endif
}

/*
 * Read the user area information from the core dump.
 */

public coredump_xreadin(mask, reg, signo)
int *mask;
Word reg[];
int *signo;
{
    register struct user *up;
    register Word *savreg;
    union {
	struct user u;
	char dummy[ctob(UPAGES)];
    } ustruct;
    Symbol s;

    objfile = fopen(objname, "r");
    if (objfile == nil) {
	fatal("can't read \"%s\"", objname);
    }
    get(objfile, hdr);
    if (vaddrs) {
	datamap.begin = 0;
	datamap.end = 0xffffffff;
	stkmap.begin = 0xffffffff;
	stkmap.end = 0xffffffff;
    } else {
	up = &(ustruct.u);
	fread(up, ctob(UPAGES), 1, corefile);
#	ifdef vax
	    savreg = (Word *) &(ustruct.dummy[ctob(UPAGES)]);
#	endif
#	ifdef sun
	    savreg = (Word *) (
		&(ustruct.dummy[ctob(UPAGES) - 10] - (NREG * sizeof(Word)))
	    );
#	endif
#	ifdef sgi
	    savreg = (Word *) &(ustruct.dummy[((int) up->u_ar0) - UDOT_VBASE]);
#	endif

#       ifdef sgi
	    *mask = savreg[RPS];
#       else
	    *mask = savreg[PS];
#       endif
	printf("Corefile produced from file \"%s\"\n", ustruct.u.u_comm);
	copyregs(savreg, reg);
	*signo = up->u_arg[0];
	datamap.seekaddr = ctob(UPAGES);
	stkmap.begin = MAXSTKADDR - ctob(up->u_ssize);
	stkmap.end = MAXSTKADDR;
	stkmap.seekaddr = datamap.seekaddr + ctob(up->u_dsize);
#ifdef sgi
	codestart = hdr.a_entry;
#else
	codestart = CODESTART;
#endif
	switch (hdr.a_magic) {
	    case OMAGIC:
		datamap.begin = codestart;
		datamap.end = codestart + ctob(up->u_tsize) + ctob(up->u_dsize);
		break;

	    case NMAGIC:
	    case ZMAGIC:
		datamap.begin = (Address)
		    ptob(btop(ctob(up->u_tsize) - 1) + 1) + codestart;
		datamap.end = datamap.begin + ctob(up->u_dsize);
		break;

	    default:
		fatal("bad magic number 0x%x", hdr.a_magic);
	}
    }
}

public coredump_close()
{
    fclose(objfile);
}

public coredump_readtext(buff, addr, nbytes)
char *buff;
Address addr;
int nbytes;
{
    if (hdr.a_magic == OMAGIC or vaddrs) {
	coredump_readdata(buff, addr, nbytes);
    } else {
	fseek(objfile, N_TXTOFF(hdr) + addr - codestart, 0);
	fread(buff, nbytes, sizeof(Byte), objfile);
    }
}

/*
 * Map a virtual address to a physical address.
 */

private Address vmap (addr)
Address addr;
{
    Address r;
    integer v, n;
    struct pte pte;

    r = addr & ~0xc0000000;
    v = btop(r);
    switch (addr&0xc0000000) {
	case 0xc0000000:
	case 0x80000000:
	    /*
	     * In system space, so get system pte.
	     * If it is valid or reclaimable then the physical address
	     * is the combination of its page number and the page offset
	     * of the original address.
	     */
	    if (v >= slr) {
		error("address %x out of segment", addr);
	    }
	    r = ((long) (sbr + v)) & ~0x80000000;
	    goto simple;

	case 0x40000000:
	    /*
	     * In p1 space, must not be in shadow region.
	     */
	    if (v < pcb.pcb_p1lr) {
		error("address %x out of segment", addr);
	    }
	    r = (Address) (pcb.pcb_p1br + v);
	    break;

	case 0x00000000:
	    /*
	     * In p0 space, must not be off end of region.
	     */
	    if (v >= pcb.pcb_p0lr) {
		error("address %x out of segment", addr);
	    }
	    r = (Address) (pcb.pcb_p0br + v);
	    break;

	default:
	    /* do nothing */
	    break;
    }
    /*
     * For p0/p1 address, user-level page table should be in
     * kernel virtual memory.  Do second-level indirect by recursing.
     */
    if ((r & 0x80000000) == 0) {
	error("bad p0br or p1br in pcb");
    }
    r = vmap(r);
simple:
    /*
     * "r" is now the address of the pte of the page
     * we are interested in; get the pte and paste up the physical address.
     */
    fseek(corefile, r, 0);
    n = fread(&pte, sizeof(pte), 1, corefile);
    if (n != 1) {
	error("page table botch (fread at %x returns %d)", r, n);
    }
    if (pte.pg_v == 0 and (pte.pg_fod != 0 or pte.pg_pfnum == 0)) {
	error("page no valid or reclamable");
    }
    return (addr&PGOFSET) + ((Address) ptob(pte.pg_pfnum));
}

public coredump_readdata(buff, addr, nbytes)
char *buff;
Address addr;
int nbytes;
{
    Address a;

    a = addr;
    if (a < datamap.begin) {
	if (hdr.a_magic == OMAGIC) {
	    error("[data address 0x%x too low (lb = 0x%x)]", a, datamap.begin);
	} else {
	    coredump_readtext(buff, a, nbytes);
	}
    } else if (a > stkmap.end) {
	error("data address 0x%x too high (ub = 0x%x)", a, stkmap.end);
    } else {
	if (vaddrs) {
	    vreadfromfile(corefile, a, buff, nbytes);
	} else {
	    readfromfile(corefile, a, buff, nbytes);
	}
    }
}

/*
 * Read a block of data from a memory image, mapping virtual addresses.
 * Have to watch out for page boundaries.
 */

private vreadfromfile (corefile, v, buff, nbytes)
File corefile;
Address v;
char *buff;
integer nbytes;
{
    Address a;
    integer i, remainder, pagesize;
    char *bufp;

    a = v;
    pagesize = (integer) ptob(1);
    remainder = pagesize - (a mod pagesize);
    if (remainder >= nbytes) {
	readfromfile(corefile, vmap(a), buff, nbytes);
    } else {
	readfromfile(corefile, vmap(a), buff, remainder);
	a += remainder;
	i = nbytes - remainder;
	bufp = buff + remainder;
	while (i > pagesize) {
	    readfromfile(corefile, vmap(a), bufp, pagesize);
	    a += pagesize;
	    bufp += pagesize;
	    i -= pagesize;
	}
	readfromfile(corefile, vmap(a), bufp, i);
    }
}

private readfromfile (f, a, buff, nbytes)
File f;
Address a;
char *buff;
integer nbytes;
{
    integer fileaddr;

    if (a < stkmap.begin) {
	fileaddr = datamap.seekaddr + a - datamap.begin;
    } else {
	fileaddr = stkmap.seekaddr + a - stkmap.begin;
    }
    fseek(f, fileaddr, 0);
    fread(buff, nbytes, sizeof(Byte), f);
}
