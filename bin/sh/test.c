/*	@(#)test.c	1.6	*/
/*
 *      test expression
 *      [ expression ]
 */

#include	"defs.h"
#include <sys/types.h>
#include <sys/stat.h>

INT	ap, ac;
STRING	*av;

test(argn, com)
	STRING com[];
	INT argn;
{


        ac = argn; av = com; ap = 1;
        IF eq(com[0],"[")
	THEN	IF !eq(com[--ac], "]")
		THEN	failed("test", "] missing");
		FI
	FI
        com[ac] = 0;
	IF ac <= 1 THEN return(1) FI
        return(exp()?0:1);
}

STRING	nxtarg(mt)	{

	IF ap >= ac
	THEN	IF mt
		THEN	ap++;
                        return(0);
		FI
		failed("test", "argument expected");
	FI
        return(av[ap++]);
}

exp() {
        INT p1;
	STRING	p2;

        p1 = e1();
        p2 = nxtarg(1);
	IF (p2!=0) THEN
		IF eq(p2, "-o")	THEN return(p1 | exp()) FI
	        IF eq(p2,"]")&&!eq(p2,")") THEN	failed("test", synmsg) FI
	FI
        ap--;
        return(p1);
}

e1() {
        INT p1;
	STRING p2;

        p1 = e2();
	p2 = nxtarg(1);
	IF (p2!=0)&&eq(p2, "-a") THEN return(p1 & e1()) FI
        ap--;
        return(p1);
}

e2() {
	STRING p2;

        p2 = nxtarg(0);
        IF (p2!=0)&&eq(p2, "!")
	THEN	return(!e3())
	FI
        ap--;
        return(e3());
}

e3() {
        INT p1;
	REG STRING	a;
	REG STRING	p2;
	REG STRING	p3;
	L_INT	atol();
        L_INT int1, int2;

        a=nxtarg(0);
        IF eq(a, "(")
	THEN	p1 = exp();
		p3 = nxtarg(0);
                IF (p3==0)||!eq(p3, ")") THEN failed("test",") expected") FI
                return(p1);
	FI

        p2 = nxtarg(1);
        ap--;
        IF (p2==0)||(!eq(p2,"=")&&!eq(p2,"!="))
	THEN	IF eq(a, "-r")
		THEN	p3 = nxtarg(0);
			IF p3==0 THEN return(0) FI
			return(tio(p3, 4))
		FI
		IF eq(a, "-w")
		THEN	p3 = nxtarg(0);
			IF p3==0 THEN return(0) FI
			return(tio(p3, 2))
		FI
		IF eq(a, "-x")
		THEN	p3 = nxtarg(0);
			IF p3==0 THEN return(0) FI
			return(tio(p3, 1))
		FI
		IF eq(a, "-d")
		THEN	p3 = nxtarg(0);
			IF p3==0 THEN return(0) FI
			return(filtyp(p3, S_IFDIR))
		FI
		IF eq(a, "-c")
		THEN	p3 = nxtarg(0);
			IF p3==0 THEN return(0) FI
			return(filtyp(p3, S_IFCHR))
		FI
		IF eq(a, "-b")
		THEN	p3 = nxtarg(0);
			IF p3==0 THEN return(0) FI
			return(filtyp(p3, S_IFBLK))
		FI
		IF eq(a, "-f")
		THEN	p3 = nxtarg(0);
			IF p3==0 THEN return(0) FI
			return(filtyp(p3, S_IFREG))
		FI
		IF eq(a, "-u")
		THEN	p3 = nxtarg(0);
			IF p3==0 THEN return(0) FI
			return(ftype(p3, S_ISUID))
		FI
		IF eq(a, "-g")
		THEN	p3 = nxtarg(0);
			IF p3==0 THEN return(0) FI
			return(ftype(p3, S_ISGID))
		FI
		IF eq(a, "-k")
		THEN	p3 = nxtarg(0);
			IF p3==0 THEN return(0) FI
			return(ftype(p3, S_ISVTX))
		FI
		IF eq(a, "-s")
		THEN	p3 = nxtarg(0);
			IF p3==0 THEN return(0) FI
			return(fsizep(p3))
		FI
		IF eq(a, "-p")
		THEN	p3 = nxtarg(0);
			IF p3==0 THEN return(0) FI
			return(filtyp(p3, S_IFIFO))
		FI
		IF eq(a, "-t")
		THEN	IF ap >= ac THEN return(isatty(1)) FI /* no args */
			IF (a=nxtarg(0))==0 ORF eq(a, "-a") ORF eq(a, "-o")
			THEN	ap--;
				return(isatty(1));
			FI
			return(isatty(atoi(a)));
		FI
		/* THEN	IF ap >= ac
			THEN return(isatty(1));
			ELIF eq((a=nxtarg(0)), "-a")
				ORF eq(a, "-o")
			    THEN	ap--;
					return(isatty(1));
			ELSE return(isatty(atoi(a)));
			FI
		FI */
		IF eq(a, "-n")
		THEN	p3 = nxtarg(0);
			IF p3==0 THEN return(0) FI
			return(!eq(p3, ""))
		FI
		IF eq(a, "-z")
		THEN	p3 = nxtarg(0);
			IF p3==0 THEN return(1) FI
			return(eq(p3, ""))
		FI
	FI

        p2 = nxtarg(1);
	IF p2==0 THEN return(!eq(a, "")) FI
	IF eq(p2, "-a") ORF eq(p2, "-o") 
	THEN	ap--;
		return(!eq(a, ""))
	FI
	IF eq(p2, "=")
	THEN	p3 = nxtarg(0);
		IF p3==0 THEN return(0) FI
		return(eq(p3, a))
	FI
	IF eq(p2, "!=")
	THEN	p3 = nxtarg(0);
		IF p3==0 THEN return(1) FI
		return(!eq(p3, a))
	FI
        int1 = atol(a);
        p3 = nxtarg(0);
        IF p3!=0
	THEN	int2 = atol(p3);
		IF eq(p2, "-eq") THEN return(int1==int2) FI
		IF eq(p2, "-ne") THEN return(int1!=int2) FI
		IF eq(p2, "-gt") THEN return(int1>int2) FI
		IF eq(p2, "-lt") THEN return(int1<int2) FI
		IF eq(p2, "-ge") THEN return(int1>=int2) FI
		IF eq(p2, "-le") THEN return(int1<=int2) FI
	FI

	bfailed(btest, badop, p2);
	/*NOTREACHED*/
}

tio(a, f)
	STRING	a;
	INT f;
{
	register int accessible = !access(a, f);
	register int mode;
	struct stat stb;

	if (f != 1 || !accessible)
		return accessible;
	/*
	 * f == 1 && accessible
	 */
	if (stat(a, &stb) < 0)
		return 0;
	mode = stb.st_mode;
	return ((mode & S_IFMT) == S_IFREG
	    && (mode & (S_IEXEC|(S_IEXEC>>3)|(S_IEXEC>>6))) != 0);
}

ftype(f,field)
	STRING	f;
	INT field;
{
        struct stat statb;

	IF stat(f,&statb)<0 THEN return(0) FI
	IF (statb.st_mode&field)==field
	THEN	return(1);
	ELSE	return(0)
	FI
}

filtyp(f,field)
	STRING	f;
	INT field;
{
        struct stat statb;

	IF stat(f,&statb)<0 THEN return(0) FI
	IF (statb.st_mode&S_IFMT)==field
	THEN	return(1);
	ELSE	return(0)
	FI
}

fsizep(f)
	STRING	f;
{
        struct stat statb;
	IF stat(f, &statb) <0 THEN return(0) FI
        return(statb.st_size>0);
}
