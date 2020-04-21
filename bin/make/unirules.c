/*	@(#)rules.c	3.1	*/
static	char	*Sccsid = "@(#)$Header: /d2/3.7/src/bin/make/RCS/unirules.c,v 1.1 89/03/27 14:54:21 root Exp $";
/*
 * $Log:	unirules.c,v $
 * Revision 1.1  89/03/27  14:54:21  root
 * Initial check-in for 3.7
 * 
 * Revision 1.4  86/02/20  09:11:50  gb
 * updated for 2.4/3.4, both iris and turbo use .o's for fortran/pascal
 * 
 * Revision 1.3  85/11/27  15:10:40  gb
 * updated for juniper 
 * 
 * Revision 1.2  85/06/20  11:28:56  bob
 * Added rules for our FORTRAN and PASCAL compilers.
 * Cleaned up some rules, e.g. SCCS won't clobber version checked out for
 * editing.
 * 
 */
#include "defs"
/* DEFAULT RULES FOR UNISOFT SYSTEMS UNIPLUS+ UNIX */

/*
 *	These are the internal rules that "make" trucks around with it at
 *	all times. One could completely delete this entire list and just
 *	conventionally define a global "include" makefile which had these
 *	rules in it. That would make the rules dynamically changeable
 *	without recompiling make. This file may be modified to local
 *	needs. There are currently two versions of this file with the
 *	source; namely, rules.c (which is the version running in Columbus)
 *	and pwbrules.c which is my attempt at satisfying the requirements
 *	of PWB systems.
 *	The makefile for make (make.mk) is parameterized for a different
 *	rules file. The macro $(RULES) defined in "make.mk" can be set
 *	to another file and when "make" is "made" the procedure will
 *	use the new file. The recommended way to do this is on the
 *	command line as follows:
 *		"make -f make.mk RULES=pwbrules"
 */

CHARSTAR builtin[] =
	{
	".SUFFIXES: .obj .obj~ .fc .cf .f .p .o .c .c~ .y .y~ .l .l~ .s .s~ .sh .sh~ .h .h~ .i",


/* PRESET VARIABLES */
	"MAKE=make",
	"YACC=yacc",
	"YFLAGS=",
	"LEX=lex",
	"LFLAGS=",
	"LD=ld",
	"LDFLAGS=",
	"CC=cc",
	"CFLAGS=-O",
	"AS=as",
	"ASFLAGS=",
	"GET=get",
	"GFLAGS=",
	"AR=ar",

	"F77=f77",
	"F77FLAGS=",
	"PC=pc",
	"PCFLAGS=",

/* SINGLE SUFFIX RULES */
	".c:",
		"\t$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@",
	".c~:",
		"\t$(GET) $(GFLAGS) $<",
		"\t$(CC) $(CFLAGS) $*.c $(LDFLAGS) -o $*",
		"\trm -f $*.c",
	".sh:",
		"\tcp $< $@;chmod ugo+x $@",
	".sh~:",
		"\t$(GET) $(GFLAGS) $<",
		"\tcp $*.sh $*;chmod ugo+x $@",
		"\trm -f $*.sh",
	".f:",
		"\t$(F77) $(F77FLAGS) $< $(LDFLAGS) -o $@",
	".f~:",
		"\t$(GET) $(GFLAGS) $<",
		"\t$(F77) $(F77FLAGS) $*.f $(LDFLAGS) -o $*",
		"\trm -f $*.f",
	".p:",
		"\t$(PC) $(PCFLAGS) $< $(LDFLAGS) -o $@",
	".p~:",
		"\t$(GET) $(GFLAGS) $<",
		"\t$(PC) $(PCFLAGS) $*.p $(LDFLAGS) -o $*",
		"\trm -f $*.p",

/* DOUBLE SUFFIX RULES */
	".c.o:",
		"\t$(CC) $(CFLAGS) -c $<",
	".c~.o:",
		"\t$(GET) $(GFLAGS) $<",
		"\t$(CC) $(CFLAGS) -c $*.c",
		"\trm -f $*.c",
	".c~.c:",
		"\t$(GET) $(GFLAGS) $<",
	".s.o:",
		"\t$(AS) $(ASFLAGS) -o $@ $<",
	".s~.o:",
		"\t$(GET) $(GFLAGS) $<",
		"\t$(AS) $(ASFLAGS) -o $*.o $*.s",
		"\trm -f $*.s",
	".c.i:",
		"\t$(CC) $(CFLAGS) -P $<",
	".c~.i:",
		"\t$(GET) $(GFLAGS) $<",
		"\t$(CC) $(CFLAGS) -P $<",
		"\trm -f $*.c",
	".c.s:",
		"\t$(CC) $(CFLAGS) -S $<",
	".y.o:",
		"\t$(YACC) $(YFLAGS) $<",
		"\t$(CC) $(CFLAGS) -c y.tab.c",
		"\trm y.tab.c",
		"\tmv y.tab.o $@",
	".y~.o:",
		"\t$(GET) $(GFLAGS) $<",
		"\t$(YACC) $(YFLAGS) $*.y",
		"\t$(CC) $(CFLAGS) -c y.tab.c",
		"\trm -f y.tab.c $*.y",
		"\tmv y.tab.o $*.o",
	".l.o:",
		"\t$(LEX) $(LFLAGS) $<",
		"\t$(CC) $(CFLAGS) -c lex.yy.c",
		"\trm lex.yy.c",
		"\tmv lex.yy.o $@",
	".l~.o:",
		"\t$(GET) $(GFLAGS) $<",
		"\t$(LEX) $(LFLAGS) $*.l",
		"\t$(CC) $(CFLAGS) -c lex.yy.c",
		"\trm -f lex.yy.c $*.l",
		"\tmv lex.yy.o $*.o",
	".y.c :",
		"\t$(YACC) $(YFLAGS) $<",
		"\tmv y.tab.c $@",
	".y~.c :",
		"\t$(GET) $(GFLAGS) $<",
		"\t$(YACC) $(YFLAGS) $*.y",
		"\tmv y.tab.c $*.c",
		"\trm -f $*.y",
	".l.c :",
		"\t$(LEX) $<",
		"\tmv lex.yy.c $@",
	".c.a:",
		"\t$(CC) -c $(CFLAGS) $<",
		"\t$(AR) rv $@ $*.o",
		"\trm -f $*.o",
	".c~.a:",
		"\t$(GET) $(GFLAGS) $<",
		"\t$(CC) -c $(CFLAGS) $*.c",
		"\t$(AR) rv $@ $*.o",
		"\trm -f $*.[co]",
	".s~.a:",
		"\t$(GET) $(GFLAGS) $<",
		"\t$(AS) $(ASFLAGS) -o $*.o $*.s",
		"\t$(AR) rv $@ $*.o",
		"\trm -f $*.[so]",
	".h~.h:",
		"\t$(GET) $(GFLAGS) $<",

	".f.o:",
		"\t$(F77) -c $(F77FLAGS) $<",
	".f~.o:",
		"\t$(GET) $(GFLAGS) $<",
		"\t$(F77) -c $(F77FLAGS) $*.f",
		"\trm -f $*.f",

				/* wrappers */
	".c.fc:",
		"\textcentry $*.c $*.fc",
	".cf.o:",
		"\t$(F77) $(F77FLAGS) -c $*.f",
		"\tmkc2f $< $*.s",
		"\t$(AS) -o $*.wo $*.s",
		"\t$(LD) -r $*.o $*.wo -o $*.tmp",
		"\tmv $*.tmp $*.o",
		"\trm -f $*.s $*.wo",
	".fc.o:",
		"\t$(CC) $(CFLAGS) -c $*.c",
		"\tmkf2c $< $*.s",
		"\t$(AS) -o $*.wo $*.s",
		"\t$(LD) -r $*.o $*.wo -o $*.tmp",
		"\tmv $*.tmp $*.o",
		"\trm -f $*.s $*.wo",
	".p.o:",
		"\t$(PC) -c $(PCFLAGS) $<",
	".p~.o:",
		"\t$(GET) $(GFLAGS) $<",
		"\t$(PC) -c $(PCFLAGS) $*.p",
		"\trm -f $*.p",

	"markfile.o:	markfile",
		"\tA=@;echo \"static char _sccsid[] = \\042`grep $$A'(#)' markfile`\\042;\" > markfile.c",
		"\tcc -c markfile.c",
		"\trm -f markfile.c",
	0 };

