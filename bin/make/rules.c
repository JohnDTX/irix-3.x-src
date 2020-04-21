/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef	sgi
#ident	"@(#)make:rules.c	1.9.1.1"
#endif
#include "defs"
/* DEFAULT RULES FOR UNIX */

/*
 *	These are the internal rules that "make" trucks around with it at
 *	all times. One could completely delete this entire list and just
 *	conventionally define a global "include" makefile which had these
 *	rules in it. That would make the rules dynamically changeable
 *	without recompiling make. This file may be modified to local
 *	needs. 
 *	The makefile for make (make.mk) is parameterized for a different
 *	rules file. The macro $(RULES) defined in "make.mk" can be set
 *	to another file and when "make" is "made" the procedure will
 *	use the new file. The recommended way to do this is on the
 *	command line as follows:
 *		"make -f make.mk RULES=pwbrules"
 */

CHARSTAR builtin[] =
	{
#ifdef	sgi
	".SUFFIXES: .o .fc .cf .c .c~ .y .y~ .l .l~ .s .s~ .h .h~ .sh .sh~ .f .f~ .p .p~ .c,v .f,v .h,v .p,v .l,v .y,v .obj .obj~ .i",
#else	/* sgi */
	".SUFFIXES: .o .c .c~ .y .y~ .l .l~ .s .s~ .h .h~ .sh .sh~ .f .f~",
#endif	/* sgi */

/* PRESET VARIABLES */
	"MAKE=make",
	"AR=ar",	"ARFLAGS=rv",
	"AS=as",	"ASFLAGS=",
	"CC=cc",	"CFLAGS=-O",
	"F77=f77",	"F77FLAGS=",
	"GET=get",	"GFLAGS=",
	"LEX=lex",	"LFLAGS=",
	"LD=ld",	"LDFLAGS=",
	"YACC=yacc",	"YFLAGS=",
#ifdef	sgi
	"CO=co",	"COFLAGS=-q",
	"PC=pc",	"PCFLAGS=",
	"C2FFLAGS=", "F2CFLAGS=",
#endif


/* SINGLE SUFFIX RULES */
	".c:",
		"\t$(CC) $(CFLAGS) $< $(LDFLAGS) -o $@",
	".c~:",
		"\t$(GET) $(GFLAGS) $<",
		"\t$(CC) $(CFLAGS) $*.c $(LDFLAGS) -o $*",
		"\t-rm -f $*.c",
	".f:",
		"\t$(F77) $(F77FLAGS) $< $(LDFLAGS) -o $@",
	".f~:",
		"\t$(GET) $(GFLAGS) $<",
		"\t$(F77) $(F77FLAGS) $< $(LDFLAGS) -o $*",
		"\t-rm -f $*.f",
	".sh:",
		"\tcp $< $@; chmod 0777 $@",
	".sh~:",
		"\t$(GET) $(GFLAGS) $<",
		"\tcp $*.sh $*; chmod 0777 $@",
		"\t-rm -f $*.sh",
#ifdef	sgi
	".p:",
		"\t$(PC) $(PCFLAGS) $< $(LDFLAGS) -o $@",
	".p~:",
		"\t$(GET) $(GFLAGS) $<",
		"\t$(PC) $(PCFLAGS) $*.p $(LDFLAGS) -o $*",
		"\trm -f $*.p",
	/*
	 * RCS support rules.  Note that MAKEPATH **must** be used to have
	 * RCS work, if the RCS files live in a separate directory than
	 * where the source files live (like ./RCS/*,v).
	 */
#define	CHECKOUT	"\t$(CO) $(COFLAGS) $<"
	".c,v:",
		CHECKOUT,
		"\t$(CC) $(CFLAGS) $*.c $(LDFLAGS) -o $*",
		"\t-rm -f $*.c",
	".f,v:",
		CHECKOUT,
		"\t$(F77) $(F77FLAGS) $< $(LDFLAGS) -o $*",
		"\t-rm -f $*.f",
	".p,v:",
		CHECKOUT,
		"\t$(PC) $(PCFLAGS) $*.p $(LDFLAGS) -o $*",
		"\trm -f $*.p",
	".sh,v:",
		CHECKOUT,
		"\tcp $*.sh $*; chmod 0777 $@",
		"\t-rm -f $*.sh",
#endif

/* DOUBLE SUFFIX RULES */
	".c.o:",
		"\t$(CC) $(CFLAGS) -c $<",
	".c~.c:",
		"\t$(GET) $(GFLAGS) $<",
	".c~.o:",
		"\t$(GET) $(GFLAGS) $<",
		"\t$(CC) $(CFLAGS) -c $*.c",
		"\t-rm -f $*.c",
#ifdef	sgi
	".c.i:",
		"\t$(CC) $(CFLAGS) -E $< > $*.i",
	".c~.i:",
		"\t$(GET) $(GFLAGS) $<",
		"\t$(CC) $(CFLAGS) -E $< > $*.i",
		"\trm -f $*.c",
	".c.s:",
		"\t$(CC) $(CFLAGS) -S $<",
	".c~.s:",
		"\t$(GET) $(GFLAGS) $<",
		"\t$(CC) $(CFLAGS) -S $<",
		"\trm -f $*.c",
	".p.o:",
		"\t$(PC) -c $(PCFLAGS) $<",
	".p~.o:",
		"\t$(GET) $(GFLAGS) $<",
		"\t$(PC) -c $(PCFLAGS) $*.p",
		"\trm -f $*.p",
#endif	/* sgi */
	".f.a:",
		"\t$(F77) $(F77FLAGS) $(LDFLAGS) -c $*.f",
		"\t$(AR) $(ARFLAGS) $@ $*.o",
		"\t-rm -f $*.o",
	".f.o:",
		"\t$(F77) $(F77FLAGS) $(LDFLAGS) -c $*.f",
	".f~.a:",
		"\t$(GET) $(GFLAGS) $<",
		"\t$(F77) $(F77FLAGS) $(LDFLAGS) -c $*.f",
		"\t$(AR) $(ARFLAGS) $@ $*.o",
		"\t-rm -f $*.[fo]",
	".f~.f:",
		"\t$(GET) $(GFLAGS) $<",
	".f~.o:",
		"\t$(GET) $(GFLAGS) $<",
		"\t$(F77) $(F77FLAGS) $(LDFLAGS) -c $*.f",
		"\t-rm -f $*.f",
	".s.o:",
		"\t$(AS) $(ASFLAGS) $< -o $@",
	".s~.o:",
		"\t$(GET) $(GFLAGS) $<",
		"\t$(AS) $(ASFLAGS) $*.s -o $*.o",
		"\t-rm -f $*.s",
	".s~.s:",
		"\t$(GET) $(GFLAGS) $<",
	".sh~.sh:",
		"\t$(GET) $(GFLAGS) $<",
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
	".l~.c:",
		"\t$(GET) $(GFLAGS) $<",
		"\t$(LEX) $(LFLAGS) $*.l",
		"\tmv lex.yy.c $@",
		"\t-rm -f $*.l",
	".l~.l:",
		"\t$(GET) $(GFLAGS) $<",
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
		"\t-rm -f $*.y",
	".y~.y:",
		"\t$(GET) $(GFLAGS) $<",
	".l.c :",
		"\t$(LEX) $(LFLAGS) $<",
		"\tmv lex.yy.c $@",
	".c.a:",
		"\t$(CC) -c $(CFLAGS) $<",
		"\t$(AR) $(ARFLAGS) $@ $*.o",
		"\trm -f $*.o",
	".c~.a:",
		"\t$(GET) $(GFLAGS) $<",
		"\t$(CC) -c $(CFLAGS) $*.c",
		"\t$(AR) $(ARFLAGS) $@ $*.o",
		"\trm -f $*.[co]",
	".s~.a:",
		"\t$(GET) $(GFLAGS) $<",
		"\t$(AS) $(ASFLAGS) $*.s -o $*.o",
		"\t$(AR) $(ARFLAGS) $@ $*.o",
		"\t-rm -f $*.[so]",
	".h~.h:",
		"\t$(GET) $(GFLAGS) $<",
#ifdef	sgi
	/*
	 * RCS support rules.  Note that MAKEPATH **must** be used to have
	 * RCS work, if the RCS files live in a separate directory than
	 * where the source files live (like ./RCS/*,v).
	 */
	".c,v.c:",
		CHECKOUT,
	".c,v.o:",
		CHECKOUT,
		"\t$(CC) $(CFLAGS) -c $*.c",
		"\t-rm -f $*.c",
	".c,v.s:",
		CHECKOUT,
		"\t$(CC) $(CFLAGS) -S $*.c",
		"\t-rm -f $*.c",
	".c,v.i:",
		CHECKOUT,
		"\t$(CC) $(CFLAGS) -E $*.c > $*.i",
		"\t-rm -f $*.c",
	".h,v.h:",
		CHECKOUT,
	".f,v.f:",
		CHECKOUT,
	".f,v.o:",
		CHECKOUT,
		"\t$(F77) $(F77FLAGS) $(LDFLAGS) -c $*.f",
		"\t-rm -f $*.f",
	".p,v.p:",
		CHECKOUT,
	".p,v.o:",
		CHECKOUT,
		"\t$(PC) -c $(PCFLAGS) $*.p",
		"\trm -f $*.p",
	".y,v.y:",
		CHECKOUT,
	".y,v.c:",
		CHECKOUT,
		"\t$(YACC) $(YFLAGS) $*.y",
		"\tmv y.tab.c $@",
		"\t-rm -f $*.y",
	".y,v.o:",
		CHECKOUT,
		"\t$(YACC) $(YFLAGS) $*.y",
		"\t$(CC) $(CFLAGS) -c y.tab.c",
		"\t-rm -f y.tab.c $*.y",
		"\tmv y.tab.o $@",
	".l,v.l:",
		CHECKOUT,
	".l,v.c :",
		CHECKOUT,
		"\t$(LEX) $(LFLAGS) $*.l",
		"\tmv lex.yy.c $@",
		"\t-rm -f $*.l",
	".l,v.o:",
		CHECKOUT,
		"\t$(LEX) $(LFLAGS) $*.l",
		"\t$(CC) $(CFLAGS) -c lex.yy.c",
		"\t-rm -f lex.yy.c $*.l",
		"\tmv lex.yy.o $@",
	".sh,v.sh:",
		CHECKOUT,

			/* wrappers */
	".c.fc:",
		"\textcentry $*.c $*.fc",
	".cf.o:",
		"\t$(F77) $(F77FLAGS) -c $*.f",
		"\tmkc2f $(C2FFLAGS) $< $*.s",
		"\t$(AS) -o $*.wo $*.s",
		"\t$(LD) -r $*.o $*.wo -o $*.tmp",
		"\tmv $*.tmp $*.o",
		"\trm -f $*.s $*.wo",
	".fc.o:",
		"\t$(CC) $(CFLAGS) -c $*.c",
		"\tmkf2c $(F2CFLAGS) $< $*.s",
		"\t$(AS) -o $*.wo $*.s",
		"\t$(LD) -r $*.o $*.wo -o $*.tmp",
		"\tmv $*.tmp $*.o",
		"\trm -f $*.s $*.wo",

#endif	/* sgi */

	"markfile.o:	markfile",
		"\tA=@;echo \"static char _sccsid[] = \\042`grep $$A'(#)' markfile`\\042;\" > markfile.c",
		"\t$(CC) -c markfile.c",
		"\trm -f markfile.c",
	0 };
