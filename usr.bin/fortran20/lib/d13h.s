#include "fpa.h"

	.text
%_EERE:		linkw	a6,#-48
|	a1@ = mult
		movl	a6@(8),a1
|	f1 = arg
		lea	a6@(28),a0
		movl	a0@-,fpaop(DOUBLELO,1,0)
		movl	a0@-,fpaop(DOUBLEHI,1,0)
|	f2 = upper = (double)(single)arg
		movb	d0,fpaop(DBLSGL,0,1)
		movb	d0,fpaop(SGLDBLUPD,2,f)
|	f3 = lower = arg - upper
		movb	d0,fpaop(SUBDBL,1,2)
		movb	d0,fpaop(COPY,3,f)
|	f2 = x = upper * log2_u
		movl	#0x40000000,fpaop(DOUBLELO,0,0)
		movl	#0x3FF71547,fpaop(MULDBLUPD,2,0)
|	d1 = longmult = round(x)
		movb	fpaop(OR,0,0),d0
		andb	#0xF1,fpaop(OR,0,0)
		movb	d0,fpaop(DBLINT,0,2)
		movb	d0,fpaop(OR,0,0)
		movl	fpaop(INTEGER,f,0),d1
|	a1@ = mult = longmult
		movw	d1,a1@
|	x = calculation
|	f2 = x - longmult
		movb	d0,fpaop(INTDBL,f,f)
		movb	d0,fpaop(SUBDBLUPD,2,f)
|	f3 = lower * log2_u
		movl	#0x40000000,fpaop(DOUBLELO,3,0)
		movl	#0x3FF71547,fpaop(MULDBLUPD,3,0)
|	f1 = arg * log2_l
		movl	#0x7F0BBBE8,fpaop(DOUBLELO,0,0)
		movl	#0x3E8295C1,fpaop(MULDBLUPD,1,0)
|	f1 = f1 + f3
		movb	d0,fpaop(ADDDBLUPD,1,3)
|	f1 = x = f2 + f1
		movb	d0,fpaop(ADDDBLUPD,1,2)
|	f2 = xsq = x**2
		movb	d0,fpaop(MULDBL,1,1)
		movb	d0,fpaop(COPY,2,f)
|	a6@(16)@ = p = x * (p0 + xsq*(p1 + xsq*p2))
		movl	#0xAA5CD009,fpaop(DOUBLELO,0,0)
		movl	#0x3F97A609,fpaop(MULDBL,2,0)
		movl	#0x9C957777,fpaop(DOUBLELO,0,0)
		movl	#0x403433A2,fpaop(ADDDBL,f,0)
		movb	d0,fpaop(MULDBL,2,f)
		movl	#0xE9C773D2,fpaop(DOUBLELO,0,0)
		movl	#0x4097A774,fpaop(ADDDBL,f,0)
		movb	d0,fpaop(MULDBLUPD,1,f)
		movl	a6@(16),a0
		movl	fpaop(DOUBLEHI,1,0),a0@+
		movl	fpaop(DOUBLELO,1,0),a0@+
|	a6@(12)@ = q = q0 + xsq*(q1 + xsq)
		movl	#0x13B3FFDA,fpaop(DOUBLELO,0,0)
		movl	#0x406D25B4,fpaop(ADDDBL,2,0)
		movb	d0,fpaop(MULDBL,f,2)
		movl	#0xB314DFB1,fpaop(DOUBLELO,0,0)
		movl	#0x40B11016,fpaop(ADDDBL,f,0)
		movl	a6@(12),a0
		movl	fpaop(DOUBLEHI,f,0),a0@+
		movl	fpaop(DOUBLELO,f,0),a0@+
endeere:
		unlk	a6
		rtd	#20

	.globl	%_DSQT
%_DSQT:
		linkw	a6,#-44
		moveml	#0x0108,a7@-
|		moveml	#<d7,a4>,a7@-
|	if(arg = 0)
		movl	a6@(8),a4
		lea	a4@(8),a0
		movl	a0@-,fpaop(DOUBLELO,0,0)
		movl	a0@-,fpaop(TSTDBL,0,0)
		tstb	fpaop(CR,0,0)
		bnes	%_DSQT2E
|	then return 0
		lea	a4@(8),a0
		movl	a0@-,fpaop(DOUBLELO,1,0)
		movl	a0@-,fpaop(DOUBLEHI,1,0)
		bra	%_DSQT1D4
%_DSQT2E:
|	else
		lea	a6@(-44),a0
		movl	a4,a1
		movl	a1@+,a0@+
		movl	a1@+,a0@+
|	d7 = expon = arg's exponent - 1023
		movw	#0x7FF0,d0
		andw	a6@(-44),d0
		extl	d0
		divs	#0x10,d0
		movw	d0,d7
		subw	#0x3FF,d7
|	mant = arg with 3ff as new exponent
		movw	a6@(-44),d0
		extl	d0
		andl	#0x800F,d0
		orl	#0x3FF0,d0
		movw	d0,a6@(-44)
		addql	#4,a0
		lea	a0@(-12),a1
		movl	a1@+,a0@+
		movl	a1@+,a0@+
|	if odd(expon)
		movl	d7,d1
		andw	#0x1,d1
		beqs	%_DSQTCA
|	then
		movl	a0@-,fpaop(DOUBLELO,1,0)
		movl	a0@-,fpaop(DOUBLEHI,1,0)
		movl	#0x0,fpaop(DOUBLELO,0,0)
		movl	#0x3FE00000,fpaop(MULDBLUPD,1,0)
		movl	fpaop(DOUBLEHI,1,0),a0@+
		movl	fpaop(DOUBLELO,1,0),a0@+
		addqw	#1,d7
		movl	#0xA0DAD67B,fpaop(DOUBLELO,1,0)
		movl	#0x3FE2E29B,fpaop(DOUBLEHI,1,0)
		movl	a0@-,fpaop(DOUBLELO,0,0)
		movl	a0@-,fpaop(MULDBLUPD,1,0)
		movl	#0xEF97D2F0,fpaop(DOUBLELO,0,0)
		movl	#0x3FDAB52A,fpaop(ADDDBLUPD,1,0)
		addql	#8,a0
		movl	fpaop(DOUBLEHI,1,0),a0@+
		movl	fpaop(DOUBLELO,1,0),a0@+
		bras	%_DSQT100
%_DSQTCA:
|	else
		movl	#0xEF97D2F0,fpaop(DOUBLELO,1,0)
		movl	#0x3FDAB52A,fpaop(DOUBLEHI,1,0)
		lea	a6@(-24),a0
		movl	a0@-,fpaop(DOUBLELO,0,0)
		movl	a0@-,fpaop(MULDBLUPD,1,0)
		movl	#0xA0DAD67B,fpaop(DOUBLELO,0,0)
		movl	#0x3FE2E29B,fpaop(ADDDBLUPD,1,0)
		addql	#8,a0
		movl	fpaop(DOUBLEHI,1,0),a0@+
		movl	fpaop(DOUBLELO,1,0),a0@+
%_DSQT100:
|	three step algorithm:
		lea	a6@(-24),a0
		movl	a0@-,fpaop(DOUBLELO,1,0)
		movl	a0@-,fpaop(DOUBLEHI,1,0)
		lea	a6@(-16),a1
		movl	a1@-,fpaop(DOUBLELO,0,0)
		movl	a1@-,fpaop(DIVDBLUPD,1,0)
		addql	#8,a1
		movl	a1@-,fpaop(DOUBLELO,0,0)
		movl	a1@-,fpaop(ADDDBLUPD,1,0)
		movl	fpaop(DOUBLEHI,1,0),a1@+
		movl	fpaop(DOUBLELO,1,0),a1@+
		movl	#0x0,fpaop(DOUBLELO,1,0)
		movl	#0x40100000,fpaop(DOUBLEHI,1,0)
		subql	#8,a1
		movl	a1@-,fpaop(DOUBLELO,0,0)
		movl	a1@-,fpaop(MULDBLUPD,1,0)
		lea	a6@(-16),a0
		movl	a0@-,fpaop(DOUBLELO,0,0)
		movl	a0@-,fpaop(DIVDBLUPD,1,0)
		addql	#8,a0
		movl	a0@-,fpaop(DOUBLELO,0,0)
		movl	a0@-,fpaop(ADDDBLUPD,1,0)
		movl	fpaop(DOUBLEHI,1,0),a0@+
		movl	fpaop(DOUBLELO,1,0),a0@+
		movl	#0x0,fpaop(DOUBLELO,1,0)
		movl	#0x40300000,fpaop(DOUBLEHI,1,0)
		addql	#8,a1
		movl	a1@-,fpaop(DOUBLELO,0,0)
		movl	a1@-,fpaop(MULDBLUPD,1,0)
		movl	a0@-,fpaop(DOUBLELO,0,0)
		movl	a0@-,fpaop(DIVDBLUPD,1,0)
		addql	#8,a0
		movl	a0@-,fpaop(DOUBLELO,0,0)
		movl	a0@-,fpaop(ADDDBLUPD,1,0)
		movl	fpaop(DOUBLEHI,1,0),a0@+
		movl	fpaop(DOUBLELO,1,0),a0@+
|	expon = (expon div 2) - 3
		movl	d7,d0
		extl	d0
		divs	#0x2,d0
		movw	d0,d7
		subqw	#3,d7
		lea	a1@(-12),a0
		addql	#8,a1
		movl	a1@+,a0@+
		movl	a1@+,a0@+
		tstw	d7
		blts	%_DSQT1BA
		movl	d7,d0
		lslw	#0x4,d0
		addw	d0,a6@(-44)
		bras	%_DSQT1C8
%_DSQT1BA:
		movw	d7,d0
		negw	d0
		movw	d0,d7
		movl	d7,d0
		lslw	#0x4,d0
		subw	d0,a6@(-44)
%_DSQT1C8:
		lea	a6@(-36),a0
		movl	a0@-,fpaop(DOUBLELO,1,0)
		movl	a0@-,fpaop(DOUBLEHI,1,0)
%_DSQT1D4:
		moveml	a7@+,#0x1080
|		moveml	a7@+,#<d7,a4>
		unlk	a6
		rtd	#4

	.globl	%_DSIN
%_DSIN:			linkw	a6,#-8
%_DSIN4:		movl	a6@(8),a0
			addql	#8,a0
|	f1 = x = ourpi_reduce(arg, mult)
|	d1 = mult
|	f1 = arg
		movl	a0@-,fpaop(DOUBLELO,1,0)
		movl	a0@-,fpaop(DOUBLEHI,1,0)
|	f2 = abs(arg)
		movb	d0,fpaop(COPY,2,1)
		movb	d0,fpaop(TSTDBL,2,2)
		tstb	fpaop(CR,0,0)
		bges	$1
		movb	d0,fpaop(NEGDBLUPD,2,2)
$1:
		movl	#0x2E48E8A7,fpaop(DOUBLELO,0,0)
		movl	#0x3FE921FF,fpaop(CMPDBL,2,0)
		tstb	fpaop(CR,0,0)
		bges	%MYSNPI_REDUCE30
		clrw	d1
		bra	%MYSNPI_REDUCE13A
%MYSNPI_REDUCE30:
|	arg + POFOUR
		movl	#0x54442D19,fpaop(DOUBLELO,0,0)
		movl	#0x3FE921FB,fpaop(ADDDBL,1,0)
|	ff = x = (arg+POFOUR)*TWOOP
		movl	#0x6DC9C882,fpaop(DOUBLELO,0,0)
		movl	#0x3FE45F30,fpaop(MULDBL,f,0)
|	if(x < 0.0)
		movb	d0,fpaop(TSTDBL,f,f)
		tstb	fpaop(CR,0,0)
		bges	%MYSNPI_REDUCE8A

		clrl	fpaop(DOUBLELO,0,0)
		movl	#0x3FF00000,fpaop(SUBDBL,f,0)
|	f2 = multiple = trunc(x-1)
%MYSNPI_REDUCE8A:
		movb	d0,fpaop(DBLINTUPD,2,f)
		movl	fpaop(INTEGER,2,0),d0
		movw	d0,d1
		movb	d0,fpaop(INTDBLUPD,2,2)
%MYSNPI_REDUCE9C:
|	f3 = i = trunc(arg)
		movb	d0,fpaop(DBLINTUPD,3,1)
		movb	d0,fpaop(INTDBLUPD,3,3)
|	f1 = x = arg - i
		movb	d0,fpaop(SUBDBLUPD,1,3)
|	f3 = i - multiple * (POTWO & 0xffffffff)
		clrl	fpaop(DOUBLELO,0,0)
		movl	#0x3FF921FB,fpaop(MULDBL,2,0)
		movb	d0,fpaop(SUBDBLUPD,3,f)
|	f1 = x - multiple * y	/* Need some more bits here: */
		movl	#0x4611A400,fpaop(DOUBLELO,0,0)
		movl	#0x3E95110B,fpaop(MULDBL,2,0)
		movb	d0,fpaop(SUBDBLUPD,1,f)
|	f1 = f1 + f3
		movb	d0,fpaop(ADDDBLUPD,1,3)
%MYSNPI_REDUCE13A:
|	f2 = xsq = x**2
|	d1 = mult = mult + sincosflag
		movb	a0@-,fpaop(MULDBL,1,1)
|	if(not odd(mult))
		movw	d1,d0
		movb	d0,fpaop(COPY,2,f)
		andw	#0x1,d0
		bne	%_ESN110
|	then f1 = result = long calculation
|	f3 = numerator:
		movl	#0x939A4320,fpaop(DOUBLELO,0,0)
		movl	#0xC03A9A34,fpaop(MULDBL,2,0)
		movl	#0x71B00EAF,fpaop(DOUBLELO,0,0)
		movl	#0x40A4BFC6,fpaop(ADDDBL,f,0)
		movb	d0,fpaop(MULDBL,f,2)
		movl	#0x19E78297,fpaop(DOUBLELO,0,0)
		movl	#0x40F920F0,fpaop(SUBDBL,f,0)
		movb	d0,fpaop(MULDBL,f,2)
		movb	d0,fpaop(MULDBL,f,1)
		movb	d0,fpaop(COPY,3,f)
|	ff = denominator:
		movl	#0x610CC36D,fpaop(DOUBLELO,0,0)
		movl	#0x406571EC,fpaop(ADDDBL,2,0)
		movb	d0,fpaop(MULDBL,f,2)
		movl	#0x6070A8EF,fpaop(DOUBLELO,0,0)
		movl	#0x40CD2F63,fpaop(ADDDBL,f,0)
		movb	d0,fpaop(MULDBL,f,2)
		movl	#0x136DA1E4,fpaop(DOUBLELO,0,0)
		movl	#0x4122D8B4,fpaop(ADDDBL,f,0)
|	quotient:
		movb	d0,fpaop(DIVDBL,3,f)
		movb	d0,fpaop(ADDDBLUPD,1,f)
		bra	%_ESN1D0
%_ESN110:
|	else result = different long calculation
|	f3 = numerator:
		movl	#0xA1A690EC,fpaop(DOUBLELO,0,0)
		movl	#0xC05C5464,fpaop(MULDBL,2,0)
		movl	#0x54C66210,fpaop(DOUBLELO,0,0)
		movl	#0x40BFDCA2,fpaop(ADDDBL,f,0)
		movb	d0,fpaop(MULDBL,f,2)
		movl	#0x8399F561,fpaop(DOUBLELO,0,0)
		movl	#0x41027D14,fpaop(SUBDBL,f,0)
		movb	d0,fpaop(MULDBL,f,2)
		movb	d0,fpaop(COPY,3,f)
|	ff = denominator:
		movl	#0x81C62DF4,fpaop(DOUBLELO,0,0)
		movl	#0x40602B4B,fpaop(ADDDBL,2,0)
		movb	d0,fpaop(MULDBL,f,2)
		movl	#0xB4D57133,fpaop(DOUBLELO,0,0)
		movl	#0x40C170E9,fpaop(ADDDBL,f,0)
		movb	d0,fpaop(MULDBL,f,2)
		movl	#0x8399F4FA,fpaop(DOUBLELO,0,0)
		movl	#0x41127D14,fpaop(ADDDBL,f,0)
|	f1 = quotient:
		movb	d0,fpaop(DIVDBL,3,f)
		clrl	fpaop(DOUBLELO,0,0)
		movl	#0x3FF00000,fpaop(ADDDBL,f,0)
		movb	d0,fpaop(COPY,1,f)
		subqw	#1,d1
%_ESN1D0:
|	if(odd(mult div 2))
		andw	#0x2,d1
		beqs	%_ESN1FA
|	then return -result
		movb	d0,fpaop(NEGDBLUPD,1,1)
%_ESN1FA:
%_DSIN16:		unlk	a6
%_DSIN18:		rtd	#4
	.globl	%_DCOS
%_DCOS:			linkw	a6,#-8
%_DCOS4:		movl	a6@(8),a0
%_DCOS8:		addql	#8,a0
|	f1 = x = ourpi_reduce(arg, mult)
|	d1 = mult
|	f1 = arg
		movl	a0@-,fpaop(DOUBLELO,1,0)
		movl	a0@-,fpaop(DOUBLEHI,1,0)
|	f2 = abs(arg)
		movb	d0,fpaop(COPY,2,1)
		movb	d0,fpaop(TSTDBL,2,2)
		tstb	fpaop(CR,0,0)
		bges	$3
		movb	d0,fpaop(NEGDBLUPD,2,2)
$3:
		movl	#0x2E48E8A7,fpaop(DOUBLELO,0,0)
		movl	#0x3FE921FF,fpaop(CMPDBL,2,0)
		tstb	fpaop(CR,0,0)
		bges	%MYCPI_REDUCE30
		clrw	d1
		bra	%MYCPI_REDUCE13A
%MYCPI_REDUCE30:
|	arg + POFOUR
		movl	#0x54442D19,fpaop(DOUBLELO,0,0)
		movl	#0x3FE921FB,fpaop(ADDDBL,1,0)
|	ff = x = (arg+POFOUR)*TWOOP
		movl	#0x6DC9C882,fpaop(DOUBLELO,0,0)
		movl	#0x3FE45F30,fpaop(MULDBL,f,0)
|	if(x < 0.0)
		movb	d0,fpaop(TSTDBL,f,f)
		tstb	fpaop(CR,0,0)
		bges	%MYCPI_REDUCE8A

		clrl	fpaop(DOUBLELO,0,0)
		movl	#0x3FF00000,fpaop(SUBDBL,f,0)
|	f2 = multiple = trunc(x-1)
%MYCPI_REDUCE8A:
		movb	d0,fpaop(DBLINTUPD,2,f)
		movl	fpaop(INTEGER,2,0),d0
		movw	d0,d1
		movb	d0,fpaop(INTDBLUPD,2,2)
%MYCPI_REDUCE9C:
|	f3 = i = trunc(arg)
		movb	d0,fpaop(DBLINTUPD,3,1)
		movb	d0,fpaop(INTDBLUPD,3,3)
|	f1 = x = arg - i
		movb	d0,fpaop(SUBDBLUPD,1,3)
|	f3 = i - multiple * (POTWO & 0xffffffff)
		clrl	fpaop(DOUBLELO,0,0)
		movl	#0x3FF921FB,fpaop(MULDBL,2,0)
		movb	d0,fpaop(SUBDBLUPD,3,f)
|	f1 = x - multiple * y	/* Need some more bits here: */
		movl	#0x4611A400,fpaop(DOUBLELO,0,0)
		movl	#0x3E95110B,fpaop(MULDBL,2,0)
		movb	d0,fpaop(SUBDBLUPD,1,f)
|	f1 = f1 + f3
		movb	d0,fpaop(ADDDBLUPD,1,3)
%MYCPI_REDUCE13A:
|	f2 = xsq = x**2
|	d1 = mult = mult + sincosflag
		movb	a0@-,fpaop(MULDBL,1,1)
		addw	#1,d1
|	if(not odd(mult))
		movw	d1,d0
		movb	d0,fpaop(COPY,2,f)
		andw	#0x1,d0
		bne	%_EC110
|	then f1 = result = long calculation
|	f3 = numerator:
		movl	#0x939A4320,fpaop(DOUBLELO,0,0)
		movl	#0xC03A9A34,fpaop(MULDBL,2,0)
		movl	#0x71B00EAF,fpaop(DOUBLELO,0,0)
		movl	#0x40A4BFC6,fpaop(ADDDBL,f,0)
		movb	d0,fpaop(MULDBL,f,2)
		movl	#0x19E78297,fpaop(DOUBLELO,0,0)
		movl	#0x40F920F0,fpaop(SUBDBL,f,0)
		movb	d0,fpaop(MULDBL,f,2)
		movb	d0,fpaop(MULDBL,f,1)
		movb	d0,fpaop(COPY,3,f)
|	ff = denominator:
		movl	#0x610CC36D,fpaop(DOUBLELO,0,0)
		movl	#0x406571EC,fpaop(ADDDBL,2,0)
		movb	d0,fpaop(MULDBL,f,2)
		movl	#0x6070A8EF,fpaop(DOUBLELO,0,0)
		movl	#0x40CD2F63,fpaop(ADDDBL,f,0)
		movb	d0,fpaop(MULDBL,f,2)
		movl	#0x136DA1E4,fpaop(DOUBLELO,0,0)
		movl	#0x4122D8B4,fpaop(ADDDBL,f,0)
|	quotient:
		movb	d0,fpaop(DIVDBL,3,f)
		movb	d0,fpaop(ADDDBLUPD,1,f)
		bra	%_EC1D0
%_EC110:
|	else result = different long calculation
|	f3 = numerator:
		movl	#0xA1A690EC,fpaop(DOUBLELO,0,0)
		movl	#0xC05C5464,fpaop(MULDBL,2,0)
		movl	#0x54C66210,fpaop(DOUBLELO,0,0)
		movl	#0x40BFDCA2,fpaop(ADDDBL,f,0)
		movb	d0,fpaop(MULDBL,f,2)
		movl	#0x8399F561,fpaop(DOUBLELO,0,0)
		movl	#0x41027D14,fpaop(SUBDBL,f,0)
		movb	d0,fpaop(MULDBL,f,2)
		movb	d0,fpaop(COPY,3,f)
|	ff = denominator:
		movl	#0x81C62DF4,fpaop(DOUBLELO,0,0)
		movl	#0x40602B4B,fpaop(ADDDBL,2,0)
		movb	d0,fpaop(MULDBL,f,2)
		movl	#0xB4D57133,fpaop(DOUBLELO,0,0)
		movl	#0x40C170E9,fpaop(ADDDBL,f,0)
		movb	d0,fpaop(MULDBL,f,2)
		movl	#0x8399F4FA,fpaop(DOUBLELO,0,0)
		movl	#0x41127D14,fpaop(ADDDBL,f,0)
|	f1 = quotient:
		movb	d0,fpaop(DIVDBL,3,f)
		clrl	fpaop(DOUBLELO,0,0)
		movl	#0x3FF00000,fpaop(ADDDBL,f,0)
		movb	d0,fpaop(COPY,1,f)
		subqw	#1,d1
%_EC1D0:
|	if(odd(mult div 2))
		andw	#0x2,d1
		beqs	%_EC1FA
|	then return -result
		movb	d0,fpaop(NEGDBLUPD,1,1)
%_EC1FA:
%_DCOS18:		unlk	a6
%_DCOS1A:		rtd	#4
	.globl	%_DATN
%_DATN:
		linkw	a6,#-44
			movl	a6@(8),a0
			movl	a0@,d0
			cmpl	#0x7ff00000,d0
			beqs	%_DATN18
			cmpl	#0xfff00000,d0
			bnes	%_DATN20
%_DATN18:	lea		a6@(-16),a0
|
|	GB - return -pi/2 if -INF, +pi/2 if INF, as does s/w case.
|
|			movl	#0x3FF921FB,fpaop(DOUBLEHI,1,0)
|
			andl	#0x80000000,d0
			eorl	#0x3ff921fb,d0
			movl	d0,fpaop(DOUBLEHI,1,0)
			movl	#0x54442D18,fpaop(DOUBLELO,1,0)
			bra	%_DATN22C
%_DATN20:
		movl	a6@(8),a0
		addql	#8,a0
|	f1 = x = abs(arg)
		movl	a0@-,fpaop(DOUBLELO,1,0)
		movl	a0@-,fpaop(DOUBLEHI,1,0)
		movb	d0,fpaop(TSTDBL,1,1)
		tstb	fpaop(CR,0,0)
		bpls	%_DATN22
		movb	d0,fpaop(NEGDBLUPD,1,1)
%_DATN22:
|	if x >= 2
		movl	#0x0,fpaop(DOUBLELO,0,0)
		movl	#0x40000000,fpaop(CMPDBL,1,0)
		tstb	fpaop(CR,0,0)
		blts	%_DATN84
|	then
		movl	#0x0,fpaop(DOUBLELO,0,0)
		movl	#0xBFF00000,fpaop(REVDIVDBLUPD,1,0)
|	f2 = quad = pi_2, d1 = reduced = TRUE
		movl	#0x3FF921FB,fpaop(DOUBLEHI,2,0)
		movl	#0x54442D18,fpaop(DOUBLELO,2,0)
		moveq	#1,d1
		bras	%_DATNFA
|	else
%_DATN84:
|	if(x > .5)
		movl	#0x0,fpaop(DOUBLELO,0,0)
		movl	#0x3FE00000,fpaop(CMPDBL,1,0)
		tstb	fpaop(CR,0,0)
		bles	%_DATNF8
|	then
		movl	#0x0,fpaop(DOUBLELO,0,0)
		movl	#0x3FF00000,fpaop(SUBDBLUPD,1,0)
		movl	#0x0,fpaop(DOUBLELO,0,0)
		movl	#0x40000000,fpaop(ADDDBL,1,0)
		movb	d0,fpaop(DIVDBLUPD,1,f)
		movl	#0x3FE921FB,fpaop(DOUBLEHI,2,0)
		movl	#0x54442D18,fpaop(DOUBLELO,2,0)
		moveq	#1,d1
		bras	%_DATNFA
%_DATNF8:
|	else
		clrb	d1
%_DATNFA:
|	f3 = xsq = x**2
		movb	d0,fpaop(MULDBL,1,1)
		movb	d0,fpaop(COPY,3,f)
|	f4 = numerator
		movl	#0xABB1127,fpaop(DOUBLELO,0,0)
		movl	#0x3FEAFDFC,fpaop(MULDBL,3,0)
		movl	#0x77A1DB86,fpaop(DOUBLELO,0,0)
		movl	#0x402237D6,fpaop(ADDDBL,f,0)
		movb	d0,fpaop(MULDBL,f,3)
		movl	#0x266ED388,fpaop(DOUBLELO,0,0)
		movl	#0x4036ED3B,fpaop(ADDDBL,f,0)
		movb	d0,fpaop(MULDBL,f,3)
		movl	#0xF9D8F82A,fpaop(DOUBLELO,0,0)
		movl	#0x402F8669,fpaop(ADDDBL,f,0)
		movb	d0,fpaop(MULDBL,f,3)
		movb	d0,fpaop(MULDBL,f,1)
		movb	d0,fpaop(COPY,4,f)
|	ff = denominator
		movl	#0x85349ABF,fpaop(DOUBLELO,0,0)
		movl	#0x402FBC4E,fpaop(ADDDBL,3,0)
		movb	d0,fpaop(MULDBL,f,3)
		movl	#0x760B164F,fpaop(DOUBLELO,0,0)
		movl	#0x40505691,fpaop(ADDDBL,f,0)
		movb	d0,fpaop(MULDBL,f,3)
		movl	#0x35098C6A,fpaop(DOUBLELO,0,0)
		movl	#0x405849C4,fpaop(ADDDBL,f,0)
		movb	d0,fpaop(MULDBL,f,3)
		movl	#0x7B62BD8F,fpaop(DOUBLELO,0,0)
		movl	#0x4047A4CF,fpaop(ADDDBL,f,0)
|	ff = quotient:
		movb	d0,fpaop(DIVDBL,4,f)
		movb	d0,fpaop(SUBDBLUPD,1,f)
		tstb	d1
		beqs	%_DATN22C
		movb	d0,fpaop(ADDDBLUPD,1,2)
%_DATN22C:
|	give result same sign as arg:
		movl	fpaop(DOUBLEHI,1,0),d0
		movl	a6@(8),a0
		movl	a0@,d1
		roxll	#1,d0
		roxll	#1,d1
		roxrl	#1,d0
		movl	d0,fpaop(DOUBLEHI,1,0)
		unlk	a6
		rtd	#4

	.globl	%_DEXP
%_DEXP:
		linkw	a6,#-44
		movl	a6@(8),a0
		addql	#8,a0
		movl	a0@-,fpaop(DOUBLELO,1,0)
		movl	a0@-,fpaop(DOUBLEHI,1,0)
|	set d1 = mult, f1 = p, f2 = q:
#include "eere.s"
|	mult = mult + 1
		addqw	#1,d1
|	r = .5 + p/(q-p)
		movb	d0,fpaop(SUBDBLUPD,2,1)
		movb	d0,fpaop(DIVDBLUPD,1,2)
		movl	#0x0,fpaop(DOUBLELO,0,0)
		movl	#0x3FE00000,fpaop(ADDDBLUPD,1,0)
|	d0 = exponent
		movl	fpaop(DOUBLEHI,1,0),d0
		swap	d0
|	if mult >= 0
		tstl	d1
		blts	%_DEXP74
|	then
		lslw	#0x4,d1
		addl	d1,d0
		bras	%_DEXP84
%_DEXP74:
|	else
		negw	d1
		lslw	#0x4,d1
		subw	d1,d0
%_DEXP84:
|	return r
		swap	d0
		movl	d0,fpaop(DOUBLEHI,1,0)
		unlk	a6
		rtd	#4

	.globl	%_DLOG
%_DLOG:
		linkw	a6,#-52
		movl	a6@(8),a0
		movl	a0@,d0
		swap	d0
|	d1 = n = r.ia and 0x7ff0 >> 4
		movw	#0x7FF0,d1
		andw	d0,d1
		asrw	#4,d1
|	d1 = n = n - 1023
		subw	#0x3FF,d1
|	r.ia = (r.ia & 0x800f) | 0x3ff0
		andw	#0x800F,d0
		orw	#0x3FF0,d0
		swap	d0
|	f1 = x = r.ia
		movl	a0@(4),fpaop(DOUBLELO,1,0)
		movl	d0,fpaop(DOUBLEHI,1,0)
|	if(x > 1.414213)
		movl	#0xCF893FAF,fpaop(DOUBLELO,0,0)
		movl	#0x3FF6A09D,fpaop(CMPDBL,1,0)
		tstb	fpaop(CR,0,0)
		bles	%_DLOGAA
|	then f1 = z = (x-2)/(x+2) and n = n+1
		movl	#0x0,fpaop(DOUBLELO,0,0)
		movl	#0x40000000,fpaop(SUBDBLUPD,1,0)
		movl	#0x0,fpaop(DOUBLELO,0,0)
		movl	#0x40100000,fpaop(ADDDBL,1,0)
		movb	d0,fpaop(DIVDBLUPD,1,f)
		addqw	#1,d1
		bras	%_DLOGEE
%_DLOGAA:
|	else f1 = z = (x-1)/(x+1)
		movl	#0x0,fpaop(DOUBLELO,0,0)
		movl	#0x3FF00000,fpaop(SUBDBLUPD,1,0)
		movl	#0x0,fpaop(DOUBLELO,0,0)
		movl	#0x40000000,fpaop(ADDDBL,1,0)
		movb	d0,fpaop(DIVDBLUPD,1,f)
%_DLOGEE:
|	f2 = zsq = z**2
		movb	d0,fpaop(MULDBL,1,1)
		movb	d0,fpaop(COPY,2,f)
|	result = long calculation
|	f3 = numerator
		movl	#0xD8397D8E,fpaop(DOUBLELO,0,0)
		movl	#0xBFE9441A,fpaop(MULDBL,2,0)
		movl	#0x829541A,fpaop(DOUBLELO,0,0)
		movl	#0x40106261,fpaop(ADDDBLUPD,f,0)
		movb	d0,fpaop(MULDBL,f,2)
		movl	#0xE2D5CA65,fpaop(DOUBLELO,0,0)
		movl	#0x40100820,fpaop(SUBDBLUPD,f,0)
		movb	d0,fpaop(MULDBL,f,2)
		movb	d0,fpaop(MULDBL,f,1)
		movb	d0,fpaop(COPY,3,f)
|	ff = denominator
		movl	#0x65A42680,fpaop(DOUBLELO,0,0)
		movl	#0x4021D592,fpaop(SUBDBL,2,0)
		movb	d0,fpaop(MULDBL,f,2)
		movl	#0x5F65AAD1,fpaop(DOUBLELO,0,0)
		movl	#0x403380A4,fpaop(ADDDBL,f,0)
		movb	d0,fpaop(MULDBL,f,2)
		movl	#0x5440AFA8,fpaop(DOUBLELO,0,0)
		movl	#0x40280C31,fpaop(SUBDBL,f,0)
|	f3 = quotient
		movb	d0,fpaop(DIVDBLUPD,3,f)
|	f1 = (f3 + z) * 2
		movb	d0,fpaop(ADDDBLUPD,1,3)
		movl	#0x0,fpaop(DOUBLELO,0,0)
		movl	#0x40000000,fpaop(MULDBLUPD,1,0)
|	ff = n*log2
		extl	d1
		movl	d1,fpaop(INTDBL,0,0)
		movl	#0xFEFA39F0,fpaop(DOUBLELO,0,0)
		movl	#0x3FE62E42,fpaop(MULDBL,f,0)
|	result = f1 = ff + f1
		movb	d0,fpaop(ADDDBLUPD,1,f)
endlog:
		unlk	a6
		rtd	#4

	.globl	%_DUPI
%_DUPI:			linkw	a6,#-8
%_DUPI4:		movl	a6@(12),a0
%_DUPI8:		addql	#8,a0
%_DUPIA:		movl	a0@-,a7@-
%_DUPIC:		movl	a0@-,a7@-
%_DUPIE:		movl	a6@(8),a0
%_DUPI12:		movl	a0@,a7@-
%_DUPI14:		jsr	%_DUPIV
%_DUPI1A:		unlk	a6
%_DUPI1C:		rtd	#8
	.globl	%_DUPIV
%_DUPIV:		linkw	a6,#-24
			moveml	#0x0700,a7@-
|			moveml	#<d5,d6,d7>,a7@-
%_DUPIV8:		movl	a6@(8),d6
%_DUPIVC:		slt	d5
%_DUPIVE:		negb	d5
%_DUPIV10:		movl	d6,d0
%_DUPIV12:		bpls	%_DUPIV16
%_DUPIV14:		negl	d0
%_DUPIV16:		movl	d0,d6
%_DUPIV18:		lea	a6@(-16),a0
%_DUPIV1C:		movl	#0x3FF00000,a0@+
%_DUPIV22:		clrl	a0@+
%_DUPIV24:		tstl	d6
%_DUPIV26:		bles	%_DUPIV84
%_DUPIV28:		movl	d6,d0
%_DUPIV2A:		andw	#0x1,d0
%_DUPIV2E:		beqs	%_DUPIV3A
%_DUPIV30:		subql	#8,a0
%_DUPIV32:		lea	a6@(12),a1
%_DUPIV36:		movl	a1@+,a0@+
%_DUPIV38:		movl	a1@+,a0@+
%_DUPIV3A:		moveq	#2,d7
%_DUPIV3C:		bras	%_DUPIV80
%_DUPIV3E:		lea	a6@(20),a0
%_DUPIV42:		movl	a0@-,fpaop(DOUBLELO,1,0)
%_DUPIV46:		movl	a0@-,fpaop(DOUBLEHI,1,0)
%_DUPIV4A:		addql	#8,a0
%_DUPIV4C:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_DUPIV50:		movl	a0@-,fpaop(MULDBLUPD,1,0)
%_DUPIV54:		movl	fpaop(DOUBLEHI,1,0),a0@+
%_DUPIV58:		movl	fpaop(DOUBLELO,1,0),a0@+
%_DUPIV5C:		movl	d6,d0
%_DUPIV5E:		andl	d7,d0
%_DUPIV60:		beqs	%_DUPIV7E
%_DUPIV62:		lea	a6@(-8),a1
%_DUPIV66:		movl	a1@-,fpaop(DOUBLELO,1,0)
%_DUPIV6A:		movl	a1@-,fpaop(DOUBLEHI,1,0)
%_DUPIV6E:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_DUPIV72:		movl	a0@-,fpaop(MULDBLUPD,1,0)
%_DUPIV76:		movl	fpaop(DOUBLEHI,1,0),a1@+
%_DUPIV7A:		movl	fpaop(DOUBLELO,1,0),a1@+
%_DUPIV7E:		addl	d7,d7
%_DUPIV80:		cmpl	d6,d7
%_DUPIV82:		bles	%_DUPIV3E
%_DUPIV84:		tstb	d5
%_DUPIV86:		beqs	%_DUPIVAC
%_DUPIV88:		movl	#0x0,fpaop(DOUBLELO,1,0)
%_DUPIV90:		movl	#0x3FF00000,fpaop(DOUBLEHI,1,0)
%_DUPIV98:		lea	a6@(-8),a0
%_DUPIV9C:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_DUPIVA0:		movl	a0@-,fpaop(DIVDBLUPD,1,0)
%_DUPIVA4:		movl	fpaop(DOUBLEHI,1,0),a0@+
%_DUPIVA8:		movl	fpaop(DOUBLELO,1,0),a0@+
%_DUPIVAC:		lea	a6@(-8),a0
%_DUPIVB0:		movl	a0@-,fpaop(DOUBLELO,1,0)
%_DUPIVB4:		movl	a0@-,fpaop(DOUBLEHI,1,0)
%_DUPIVB8:		moveml	a7@+,#0x00E0
|			moveml	a7@+,#<d5,d6,d7>
%_DUPIVBC:		unlk	a6
%_DUPIVBE:		rtd	#12
	.globl	%_DUPD
%_DUPD:			linkw	a6,#-20
%_DUPD4:		movl	a4,a7@-
%_DUPD6:		movl	a6@(12),a4
%_DUPDA:		lea	a4@(8),a0
%_DUPDE:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_DUPD12:		movl	a0@-,fpaop(TSTDBL,0,0)
%_DUPD16:		tstb	fpaop(CR,0,0)
%_DUPD1A:		bnes	%_DUPD2A
%_DUPD1C:		lea	a4@(8),a0
%_DUPD20:		movl	a0@-,fpaop(DOUBLELO,1,0)
%_DUPD24:		movl	a0@-,fpaop(DOUBLEHI,1,0)
%_DUPD28:		bras	%_DUPD56
%_DUPD2A:		movl	a4,a7@-
%_DUPD2C:		jsr	%_DLOG
%_DUPD32:		movl	a6@(8),a0
%_DUPD36:		addql	#8,a0
%_DUPD38:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_DUPD3C:		movl	a0@-,fpaop(MULDBLUPD,1,0)
%_DUPD40:		lea	a6@(-16),a0
%_DUPD44:		movl	fpaop(DOUBLEHI,1,0),a0@+
%_DUPD48:		movl	fpaop(DOUBLELO,1,0),a0@+
%_DUPD4C:		pea	a6@(-16)
%_DUPD50:		jsr	%_DEXP
%_DUPD56:		movl	a7@+,a4
%_DUPD58:		unlk	a6
%_DUPD5A:		rtd	#8
%_EASC:			linkw	a6,#-30
			moveml	#0x0108,a7@-
|			moveml	#<d7,a4>,a7@-
%_EASC8:		movl	a6@(18),a4
%_EASCC:		lea	a6@(18),a0
%_EASC10:		movl	a0@-,fpaop(DOUBLELO,1,0)
%_EASC14:		movl	a0@-,fpaop(DOUBLEHI,1,0)
%_EASC18:		movb	d0,fpaop(TSTDBL,0,1)
%_EASC1C:		tstb	fpaop(CR,0,0)
%_EASC20:		bpls	%_EASC26
%_EASC22:		movb	d0,fpaop(NEGDBLUPD,1,1)
%_EASC26:		lea	a6@(-26),a0
%_EASC2A:		movl	fpaop(DOUBLEHI,1,0),a0@+
%_EASC2E:		movl	fpaop(DOUBLELO,1,0),a0@+
%_EASC32:		movl	a0@-,fpaop(DOUBLELO,1,0)
%_EASC36:		movl	a0@-,fpaop(DOUBLEHI,1,0)
%_EASC3A:		movl	#0x2DE00D1B,fpaop(DOUBLELO,0,0)
%_EASC42:		movl	#0x3FE6A090,fpaop(CMPDBL,1,0)
%_EASC4A:		tstb	fpaop(CR,0,0)
%_EASC4E:		bges	%_EASC54
%_EASC50:		clrb	d7
%_EASC52:		bras	%_EASCB2
%_EASC54:		movl	#0x0,fpaop(DOUBLELO,1,0)
%_EASC5C:		movl	#0x3FF00000,fpaop(DOUBLEHI,1,0)
%_EASC64:		lea	a6@(-18),a0
%_EASC68:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_EASC6C:		movl	a0@-,fpaop(SUBDBLUPD,1,0)
%_EASC70:		movl	#0x0,fpaop(DOUBLELO,2,0)
%_EASC78:		movl	#0x3FF00000,fpaop(DOUBLEHI,2,0)
%_EASC80:		addql	#8,a0
%_EASC82:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_EASC86:		movl	a0@-,fpaop(ADDDBLUPD,2,0)
%_EASC8A:		movb	d0,0x9912:w
%_EASC8E:		lea	a6@(-10),a1
%_EASC92:		movl	fpaop(DOUBLEHI,1,0),a1@+
%_EASC96:		movl	fpaop(DOUBLELO,1,0),a1@+
%_EASC9A:		pea	a6@(-10)
%_EASC9E:		jsr	%_DSQT
%_EASCA4:		lea	a6@(-26),a0
%_EASCA8:		movl	fpaop(DOUBLEHI,1,0),a0@+
%_EASCAC:		movl	fpaop(DOUBLELO,1,0),a0@+
%_EASCB0:		moveq	#1,d7
%_EASCB2:		lea	a6@(-18),a0
%_EASCB6:		movl	a0@-,fpaop(DOUBLELO,1,0)
%_EASCBA:		movl	a0@-,fpaop(DOUBLEHI,1,0)
%_EASCBE:		addql	#8,a0
%_EASCC0:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_EASCC4:		movl	a0@-,fpaop(MULDBLUPD,1,0)
%_EASCC8:		addql	#8,a0
%_EASCCA:		movl	fpaop(DOUBLEHI,1,0),a0@+
%_EASCCE:		movl	fpaop(DOUBLELO,1,0),a0@+
%_EASCD2:		movl	#0x74CB69A4,fpaop(DOUBLELO,1,0)
%_EASCDA:		movl	#0x3F737670,fpaop(DOUBLEHI,1,0)
%_EASCE2:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_EASCE6:		movl	a0@-,fpaop(MULDBLUPD,1,0)
%_EASCEA:		movl	#0x2368D8A7,fpaop(DOUBLELO,0,0)
%_EASCF2:		movl	#0x3FE2F50D,fpaop(SUBDBLUPD,1,0)
%_EASCFA:		addql	#8,a0
%_EASCFC:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_EASC100:		movl	a0@-,fpaop(MULDBLUPD,1,0)
%_EASC104:		movl	#0x678D8743,fpaop(DOUBLELO,0,0)
%_EASC10C:		movl	#0x4014208C,fpaop(ADDDBLUPD,1,0)
%_EASC114:		addql	#8,a0
%_EASC116:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_EASC11A:		movl	a0@-,fpaop(MULDBLUPD,1,0)
%_EASC11E:		movl	#0x74A263CE,fpaop(DOUBLELO,0,0)
%_EASC126:		movl	#0x402CB368,fpaop(SUBDBLUPD,1,0)
%_EASC12E:		addql	#8,a0
%_EASC130:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_EASC134:		movl	a0@-,fpaop(MULDBLUPD,1,0)
%_EASC138:		movl	#0xBD0C7221,fpaop(DOUBLELO,0,0)
%_EASC140:		movl	#0x4030A5D2,fpaop(ADDDBLUPD,1,0)
%_EASC148:		addql	#8,a0
%_EASC14A:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_EASC14E:		movl	a0@-,fpaop(MULDBLUPD,1,0)
%_EASC152:		movl	#0xB89359E1,fpaop(DOUBLELO,0,0)
%_EASC15A:		movl	#0x401B1843,fpaop(SUBDBLUPD,1,0)
%_EASC162:		addql	#8,a0
%_EASC164:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_EASC168:		movl	a0@-,fpaop(MULDBLUPD,1,0)
%_EASC16C:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_EASC170:		movl	a0@-,fpaop(MULDBLUPD,1,0)
%_EASC174:		lea	a6@(-10),a1
%_EASC178:		movl	a1@-,fpaop(DOUBLELO,2,0)
%_EASC17C:		movl	a1@-,fpaop(DOUBLEHI,2,0)
%_EASC180:		movl	#0x3F41F7DC,fpaop(DOUBLELO,0,0)
%_EASC188:		movl	#0x402BD9B5,fpaop(SUBDBLUPD,2,0)
%_EASC190:		addql	#8,a1
%_EASC192:		movl	a1@-,fpaop(DOUBLELO,0,0)
%_EASC196:		movl	a1@-,fpaop(MULDBLUPD,2,0)
%_EASC19A:		movl	#0x6AFE0D48,fpaop(DOUBLELO,0,0)
%_EASC1A2:		movl	#0x404FDC8E,fpaop(ADDDBLUPD,2,0)
%_EASC1AA:		addql	#8,a1
%_EASC1AC:		movl	a1@-,fpaop(DOUBLELO,0,0)
%_EASC1B0:		movl	a1@-,fpaop(MULDBLUPD,2,0)
%_EASC1B4:		movl	#0xA46999E4,fpaop(DOUBLELO,0,0)
%_EASC1BC:		movl	#0x40600CA4,fpaop(SUBDBLUPD,2,0)
%_EASC1C4:		addql	#8,a1
%_EASC1C6:		movl	a1@-,fpaop(DOUBLELO,0,0)
%_EASC1CA:		movl	a1@-,fpaop(MULDBLUPD,2,0)
%_EASC1CE:		movl	#0xBC517145,fpaop(DOUBLELO,0,0)
%_EASC1D6:		movl	#0x405D8B3A,fpaop(ADDDBLUPD,2,0)
%_EASC1DE:		addql	#8,a1
%_EASC1E0:		movl	a1@-,fpaop(DOUBLELO,0,0)
%_EASC1E4:		movl	a1@-,fpaop(MULDBLUPD,2,0)
%_EASC1E8:		movl	#0xCA6E7FED,fpaop(DOUBLELO,0,0)
%_EASC1F0:		movl	#0x40445232,fpaop(SUBDBLUPD,2,0)
%_EASC1F8:		movb	d0,fpaop(DIVDBLUPD,1,2)
%_EASC1FC:		movl	a1@-,fpaop(DOUBLELO,0,0)
%_EASC200:		movl	a1@-,fpaop(ADDDBLUPD,1,0)
%_EASC204:		movl	a4,a0
%_EASC206:		movl	fpaop(DOUBLEHI,1,0),a0@+
%_EASC20A:		movl	fpaop(DOUBLELO,1,0),a0@+
%_EASC20E:		movl	a4@(4),a7@-
%_EASC212:		movl	a4@,a7@-
%_EASC214:		lea	a6@(18),a0
%_EASC218:		movl	a0@-,a7@-
%_EASC21A:		movl	a0@-,a7@-
%_EASC21C:		jsr	%D_SIGN
%_EASC222:		movl	a4,a0
%_EASC224:		movl	fpaop(DOUBLEHI,1,0),a0@+
%_EASC228:		movl	fpaop(DOUBLELO,1,0),a0@+
%_EASC22C:		tstb	d7
%_EASC22E:		beqs	%_EASC29C
%_EASC230:		lea	a6@(18),a0
%_EASC234:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_EASC238:		movl	a0@-,fpaop(TSTDBL,0,0)
%_EASC23C:		tstb	fpaop(CR,0,0)
%_EASC240:		bges	%_EASC29C
%_EASC242:		movb	a6@(8),d0
%_EASC246:		beqs	%_EASC274
%_EASC248:		movl	#0x54442D18,fpaop(DOUBLELO,1,0)
%_EASC250:		movl	#0x3FF921FB,fpaop(DOUBLEHI,1,0)
%_EASC258:		lea	a4@(8),a1
%_EASC25C:		movl	a1@-,fpaop(DOUBLELO,0,0)
%_EASC260:		movl	a1@-,fpaop(ADDDBLUPD,1,0)
%_EASC264:		movb	d0,fpaop(NEGDBLUPD,2,1)
%_EASC268:		movl	a4,a1
%_EASC26A:		movl	fpaop(DOUBLEHI,2,0),a1@+
%_EASC26E:		movl	fpaop(DOUBLELO,2,0),a1@+
%_EASC272:		bras	%_EASC29A
%_EASC274:		movl	#0x54442D1A,fpaop(DOUBLELO,1,0)
%_EASC27C:		movl	#0x400921FB,fpaop(DOUBLEHI,1,0)
%_EASC284:		lea	a4@(8),a0
%_EASC288:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_EASC28C:		movl	a0@-,fpaop(ADDDBLUPD,1,0)
%_EASC290:		movl	a4,a0
%_EASC292:		movl	fpaop(DOUBLEHI,1,0),a0@+
%_EASC296:		movl	fpaop(DOUBLELO,1,0),a0@+
%_EASC29A:		bras	%_EASC2C8
%_EASC29C:		cmpb	a6@(8),d7
%_EASC2A0:		bnes	%_EASC2C8
%_EASC2A2:		movl	#0x54442D18,fpaop(DOUBLELO,1,0)
%_EASC2AA:		movl	#0x3FF921FB,fpaop(DOUBLEHI,1,0)
%_EASC2B2:		lea	a4@(8),a0
%_EASC2B6:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_EASC2BA:		movl	a0@-,fpaop(SUBDBLUPD,1,0)
%_EASC2BE:		movl	a4,a0
%_EASC2C0:		movl	fpaop(DOUBLEHI,1,0),a0@+
%_EASC2C4:		movl	fpaop(DOUBLELO,1,0),a0@+
%_EASC2C8:
			moveml	a7@+,#0x1080
|			moveml	a7@+,#<d7,a4>
%_EASC2CC:		unlk	a6
%_EASC2CE:		rtd	#14
%_ESCH:			linkw	a6,#-46
		moveml	#0x0308,a7@-
|		moveml	#<d6,d7,a4>,a7@-
%_ESCH8:		movl	a6@(18),a4
%_ESCHC:		movb	a6@(8),d6
%_ESCH10:		lea	a6@(18),a0
%_ESCH14:		movl	a0@-,fpaop(DOUBLELO,1,0)
%_ESCH18:		movl	a0@-,fpaop(DOUBLEHI,1,0)
%_ESCH1C:		movb	d0,fpaop(TSTDBL,0,1)
%_ESCH20:		tstb	fpaop(CR,0,0)
%_ESCH24:		bpls	%_ESCH2A
%_ESCH26:		movb	d0,fpaop(NEGDBLUPD,1,1)
%_ESCH2A:		lea	a6@(-34),a0
%_ESCH2E:		movl	fpaop(DOUBLEHI,1,0),a0@+
%_ESCH32:		movl	fpaop(DOUBLELO,1,0),a0@+
%_ESCH36:		movl	a0@-,fpaop(DOUBLELO,1,0)
%_ESCH3A:		movl	a0@-,fpaop(DOUBLEHI,1,0)
%_ESCH3E:		movl	#0x0,fpaop(DOUBLELO,0,0)
%_ESCH46:		movl	#0x3FE80000,fpaop(CMPDBL,1,0)
%_ESCH4E:		tstb	fpaop(CR,0,0)
%_ESCH52:		bge	%_ESCH1F4
%_ESCH56:		addql	#8,a0
%_ESCH58:		movl	a0@-,fpaop(DOUBLELO,1,0)
%_ESCH5C:		movl	a0@-,fpaop(DOUBLEHI,1,0)
%_ESCH60:		addql	#8,a0
%_ESCH62:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_ESCH66:		movl	a0@-,fpaop(MULDBLUPD,1,0)
%_ESCH6A:		addql	#8,a0
%_ESCH6C:		movl	fpaop(DOUBLEHI,1,0),a0@+
%_ESCH70:		movl	fpaop(DOUBLELO,1,0),a0@+
%_ESCH74:		tstb	d6
%_ESCH76:		beq	%_ESCH138
%_ESCH7A:		movl	#0x809EDA4B,fpaop(DOUBLELO,1,0)
%_ESCH82:		movl	#0x3DE6473C,fpaop(DOUBLEHI,1,0)
%_ESCH8A:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_ESCH8E:		movl	a0@-,fpaop(MULDBLUPD,1,0)
%_ESCH92:		movl	#0xCAF0D8A6,fpaop(DOUBLELO,0,0)
%_ESCH9A:		movl	#0x3E5AE5F3,fpaop(ADDDBLUPD,1,0)
%_ESCHA2:		addql	#8,a0
%_ESCHA4:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_ESCHA8:		movl	a0@-,fpaop(MULDBLUPD,1,0)
%_ESCHAC:		movl	#0xE3996DA7,fpaop(DOUBLELO,0,0)
%_ESCHB4:		movl	#0x3EC71DE3,fpaop(ADDDBLUPD,1,0)
%_ESCHBC:		addql	#8,a0
%_ESCHBE:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_ESCHC2:		movl	a0@-,fpaop(MULDBLUPD,1,0)
%_ESCHC6:		movl	#0x19D0EEA8,fpaop(DOUBLELO,0,0)
%_ESCHCE:		movl	#0x3F2A01A0,fpaop(ADDDBLUPD,1,0)
%_ESCHD6:		addql	#8,a0
%_ESCHD8:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_ESCHDC:		movl	a0@-,fpaop(MULDBLUPD,1,0)
%_ESCHE0:		movl	#0x1111232B,fpaop(DOUBLELO,0,0)
%_ESCHE8:		movl	#0x3F811111,fpaop(ADDDBLUPD,1,0)
%_ESCHF0:		addql	#8,a0
%_ESCHF2:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_ESCHF6:		movl	a0@-,fpaop(MULDBLUPD,1,0)
%_ESCHFA:		movl	#0x5555554B,fpaop(DOUBLELO,0,0)
%_ESCH102:		movl	#0x3FC55555,fpaop(ADDDBLUPD,1,0)
%_ESCH10A:		lea	a6@(18),a1
%_ESCH10E:		movl	a1@-,fpaop(DOUBLELO,0,0)
%_ESCH112:		movl	a1@-,fpaop(MULDBLUPD,1,0)
%_ESCH116:		addql	#8,a0
%_ESCH118:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_ESCH11C:		movl	a0@-,fpaop(MULDBLUPD,1,0)
%_ESCH120:		addql	#8,a1
%_ESCH122:		movl	a1@-,fpaop(DOUBLELO,0,0)
%_ESCH126:		movl	a1@-,fpaop(ADDDBLUPD,1,0)
%_ESCH12A:		movl	a4,a0
%_ESCH12C:		movl	fpaop(DOUBLEHI,1,0),a0@+
%_ESCH130:		movl	fpaop(DOUBLELO,1,0),a0@+
%_ESCH134:		bra	%_ESCH1F0
%_ESCH138:		movl	#0x8E37B8D4,fpaop(DOUBLELO,1,0)
%_ESCH140:		movl	#0x3E22203B,fpaop(DOUBLEHI,1,0)
%_ESCH148:		lea	a6@(-18),a0
%_ESCH14C:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_ESCH150:		movl	a0@-,fpaop(MULDBLUPD,1,0)
%_ESCH154:		movl	#0x236F2B8D,fpaop(DOUBLELO,0,0)
%_ESCH15C:		movl	#0x3E927E04,fpaop(ADDDBLUPD,1,0)
%_ESCH164:		addql	#8,a0
%_ESCH166:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_ESCH16A:		movl	a0@-,fpaop(MULDBLUPD,1,0)
%_ESCH16E:		movl	#0x8C597EB2,fpaop(DOUBLELO,0,0)
%_ESCH176:		movl	#0x3EFA01A0,fpaop(ADDDBLUPD,1,0)
%_ESCH17E:		addql	#8,a0
%_ESCH180:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_ESCH184:		movl	a0@-,fpaop(MULDBLUPD,1,0)
%_ESCH188:		movl	#0x1668ECC9,fpaop(DOUBLELO,0,0)
%_ESCH190:		movl	#0x3F56C16C,fpaop(ADDDBLUPD,1,0)
%_ESCH198:		addql	#8,a0
%_ESCH19A:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_ESCH19E:		movl	a0@-,fpaop(MULDBLUPD,1,0)
%_ESCH1A2:		movl	#0x55559647,fpaop(DOUBLELO,0,0)
%_ESCH1AA:		movl	#0x3FA55555,fpaop(ADDDBLUPD,1,0)
%_ESCH1B2:		addql	#8,a0
%_ESCH1B4:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_ESCH1B8:		movl	a0@-,fpaop(MULDBLUPD,1,0)
%_ESCH1BC:		movl	#0xFFFFFFBA,fpaop(DOUBLELO,0,0)
%_ESCH1C4:		movl	#0x3FDFFFFF,fpaop(ADDDBLUPD,1,0)
%_ESCH1CC:		addql	#8,a0
%_ESCH1CE:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_ESCH1D2:		movl	a0@-,fpaop(MULDBLUPD,1,0)
%_ESCH1D6:		movl	#0x0,fpaop(DOUBLELO,0,0)
%_ESCH1DE:		movl	#0x3FF00000,fpaop(ADDDBLUPD,1,0)
%_ESCH1E6:		movl	a4,a1
%_ESCH1E8:		movl	fpaop(DOUBLEHI,1,0),a1@+
%_ESCH1EC:		movl	fpaop(DOUBLELO,1,0),a1@+
%_ESCH1F0:		bra	%_ESCH36C
%_ESCH1F4:		lea	a6@(-26),a0
%_ESCH1F8:		movl	a0@-,a7@-
%_ESCH1FA:		movl	a0@-,a7@-
%_ESCH1FC:		pea	a6@(-18)
%_ESCH200:		pea	a6@(-10)
%_ESCH204:		pea	a6@(-38)
%_ESCH208:		jsr	%_EERE
%_ESCH20E:		lea	a6@(-2),a0
%_ESCH212:		movl	a0@-,fpaop(DOUBLELO,1,0)
%_ESCH216:		movl	a0@-,fpaop(DOUBLEHI,1,0)
%_ESCH21A:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_ESCH21E:		movl	a0@-,fpaop(SUBDBLUPD,1,0)
%_ESCH222:		addql	#8,a0
%_ESCH224:		movl	a0@-,fpaop(DOUBLELO,2,0)
%_ESCH228:		movl	a0@-,fpaop(DOUBLEHI,2,0)
%_ESCH22C:		movb	d0,fpaop(DIVDBLUPD,2,1)
%_ESCH230:		lea	a0@(-16),a1
%_ESCH234:		movl	fpaop(DOUBLEHI,2,0),a1@+
%_ESCH238:		movl	fpaop(DOUBLELO,2,0),a1@+
%_ESCH23C:		cmpw	#0x1C,a6@(-38)
%_ESCH242:		bge	%_ESCH2F6
%_ESCH246:		movw	a6@(-38),d0
%_ESCH24A:		lslw	#0x1,d0
%_ESCH24C:		negw	d0
%_ESCH24E:		movw	d0,d7
%_ESCH250:		subqw	#1,d7
%_ESCH252:		movl	a1@-,fpaop(DOUBLELO,1,0)
%_ESCH256:		movl	a1@-,fpaop(DOUBLEHI,1,0)
%_ESCH25A:		movl	#0x0,fpaop(DOUBLELO,0,0)
%_ESCH262:		movl	#0x3FE00000,fpaop(ADDDBLUPD,1,0)
%_ESCH26A:		movl	#0x0,fpaop(DOUBLELO,2,0)
%_ESCH272:		movl	#0x3FE00000,fpaop(DOUBLEHI,2,0)
%_ESCH27A:		movb	d0,fpaop(DIVDBLUPD,2,1)
%_ESCH27E:		lea	a1@(-12),a0
%_ESCH282:		movl	fpaop(DOUBLEHI,2,0),a0@+
%_ESCH286:		movl	fpaop(DOUBLELO,2,0),a0@+
%_ESCH28A:		tstw	d7
%_ESCH28C:		blts	%_ESCH298
%_ESCH28E:		movl	d7,d0
%_ESCH290:		lslw	#0x4,d0
%_ESCH292:		addw	d0,a6@(-46)
%_ESCH296:		bras	%_ESCH2A6
%_ESCH298:		movw	d7,d0
%_ESCH29A:		negw	d0
%_ESCH29C:		movw	d0,d7
%_ESCH29E:		movl	d7,d0
%_ESCH2A0:		lslw	#0x4,d0
%_ESCH2A2:		subw	d0,a6@(-46)
%_ESCH2A6:		lea	a6@(-26),a0
%_ESCH2AA:		lea	a0@(-20),a1
%_ESCH2AE:		movl	a1@+,a0@+
%_ESCH2B0:		movl	a1@+,a0@+
%_ESCH2B2:		tstb	d6
%_ESCH2B4:		beqs	%_ESCH2D6
%_ESCH2B6:		subql	#8,a0
%_ESCH2B8:		movl	a0@-,fpaop(DOUBLELO,1,0)
%_ESCH2BC:		movl	a0@-,fpaop(DOUBLEHI,1,0)
%_ESCH2C0:		lea	a0@(16),a1
%_ESCH2C4:		movl	a1@-,fpaop(DOUBLELO,0,0)
%_ESCH2C8:		movl	a1@-,fpaop(SUBDBLUPD,1,0)
%_ESCH2CC:		movl	fpaop(DOUBLEHI,1,0),a0@+
%_ESCH2D0:		movl	fpaop(DOUBLELO,1,0),a0@+
%_ESCH2D4:		bras	%_ESCH2F6
%_ESCH2D6:		lea	a6@(-26),a0
%_ESCH2DA:		movl	a0@-,fpaop(DOUBLELO,1,0)
%_ESCH2DE:		movl	a0@-,fpaop(DOUBLEHI,1,0)
%_ESCH2E2:		lea	a0@(16),a1
%_ESCH2E6:		movl	a1@-,fpaop(DOUBLELO,0,0)
%_ESCH2EA:		movl	a1@-,fpaop(ADDDBLUPD,1,0)
%_ESCH2EE:		movl	fpaop(DOUBLEHI,1,0),a0@+
%_ESCH2F2:		movl	fpaop(DOUBLELO,1,0),a0@+
%_ESCH2F6:		movl	#0x0,fpaop(DOUBLELO,1,0)
%_ESCH2FE:		movl	#0x3FE00000,fpaop(DOUBLEHI,1,0)
%_ESCH306:		lea	a6@(-26),a0
%_ESCH30A:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_ESCH30E:		movl	a0@-,fpaop(ADDDBLUPD,1,0)
%_ESCH312:		lea	a0@(-12),a1
%_ESCH316:		movl	fpaop(DOUBLEHI,1,0),a1@+
%_ESCH31A:		movl	fpaop(DOUBLELO,1,0),a1@+
%_ESCH31E:		tstw	a6@(-38)
%_ESCH322:		blts	%_ESCH330
%_ESCH324:		movw	a6@(-38),d0
%_ESCH328:		lslw	#0x4,d0
%_ESCH32A:		addw	d0,a6@(-46)
%_ESCH32E:		bras	%_ESCH340
%_ESCH330:		movw	a6@(-38),d0
%_ESCH334:		negw	d0
%_ESCH336:		movw	d0,a6@(-38)
%_ESCH33A:		lslw	#0x4,d0
%_ESCH33C:		subw	d0,a6@(-46)
%_ESCH340:		movl	a4,a0
%_ESCH342:		lea	a6@(-46),a1
%_ESCH346:		movl	a1@+,a0@+
%_ESCH348:		movl	a1@+,a0@+
%_ESCH34A:		tstb	d6
%_ESCH34C:		beqs	%_ESCH36C
%_ESCH34E:		movl	a4@(4),a7@-
%_ESCH352:		movl	a4@,a7@-
%_ESCH354:		lea	a6@(18),a0
%_ESCH358:		movl	a0@-,a7@-
%_ESCH35A:		movl	a0@-,a7@-
%_ESCH35C:		jsr	%D_SIGN
%_ESCH362:		movl	a4,a0
%_ESCH364:		movl	fpaop(DOUBLEHI,1,0),a0@+
%_ESCH368:		movl	fpaop(DOUBLELO,1,0),a0@+
%_ESCH36C:
			moveml	a7@+,#0x10C0
|			moveml	a7@+,#<d6,d7,a4>
%_ESCH370:		unlk	a6
%_ESCH372:		rtd	#14
	.globl	%_DTAN
%_DTAN:			linkw	a6,#-36
%_DTAN4:		movl	a6@(8),a0
%_DTAN8:		addql	#8,a0
|	f1 = arg
		movl	a0@-,fpaop(DOUBLELO,1,0)
		movl	a0@-,fpaop(DOUBLEHI,1,0)
|	f2 = abs(arg)
		movb	d0,fpaop(COPY,2,1)
		movb	d0,fpaop(TSTDBL,2,2)
		tstb	fpaop(CR,0,0)
		bges	$2
		movb	d0,fpaop(NEGDBLUPD,2,2)
$2:
		movl	#0x2E48E8A7,fpaop(DOUBLELO,0,0)
		movl	#0x3FE921FF,fpaop(CMPDBL,2,0)
		tstb	fpaop(CR,0,0)
		bges	%MYTPI_REDUCE30
		clrw	d1
		bra	%MYTPI_REDUCE13A
%MYTPI_REDUCE30:
|	arg + POFOUR
		movl	#0x54442D19,fpaop(DOUBLELO,0,0)
		movl	#0x3FE921FB,fpaop(ADDDBL,1,0)
|	ff = x = (arg+POFOUR)*TWOOP
		movl	#0x6DC9C882,fpaop(DOUBLELO,0,0)
		movl	#0x3FE45F30,fpaop(MULDBL,f,0)
|	if(x < 0.0)
		movb	d0,fpaop(TSTDBL,f,f)
		tstb	fpaop(CR,0,0)
		bges	%MYTPI_REDUCE8A

		clrl	fpaop(DOUBLELO,0,0)
		movl	#0x3FF00000,fpaop(SUBDBL,f,0)
|	f2 = multiple = trunc(x-1)
%MYTPI_REDUCE8A:
		movb	d0,fpaop(DBLINTUPD,2,f)
		movl	fpaop(INTEGER,2,0),d0
		movw	d0,d1
		movb	d0,fpaop(INTDBLUPD,2,2)
%MYTPI_REDUCE9C:
|	f3 = i = trunc(arg)
		movb	d0,fpaop(DBLINTUPD,3,1)
		movb	d0,fpaop(INTDBLUPD,3,3)
|	f1 = x = arg - i
		movb	d0,fpaop(SUBDBLUPD,1,3)
|	f3 = i - multiple * (POTWO & 0xffffffff)
		clrl	fpaop(DOUBLELO,0,0)
		movl	#0x3FF921FB,fpaop(MULDBL,2,0)
		movb	d0,fpaop(SUBDBLUPD,3,f)
|	f1 = x - multiple * y	/* Need some more bits here: */
		movl	#0x4611A400,fpaop(DOUBLELO,0,0)
		movl	#0x3E95110B,fpaop(MULDBL,2,0)
		movb	d0,fpaop(SUBDBLUPD,1,f)
|	f1 = f1 + f3
		movb	d0,fpaop(ADDDBLUPD,1,3)
%MYTPI_REDUCE13A:
%_DTAN1E:		lea	a6@(-32),a0
			movl	fpaop(DOUBLEHI,1,0),a0@+
			movl	fpaop(DOUBLELO,1,0),a0@+
%_DTAN22:		movl	a0@-,fpaop(DOUBLELO,1,0)
%_DTAN26:		movl	a0@-,fpaop(DOUBLEHI,1,0)
%_DTAN2A:		addql	#8,a0
%_DTAN2C:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_DTAN30:		movl	a0@-,fpaop(MULDBLUPD,1,0)
%_DTAN34:		addql	#8,a0
%_DTAN36:		movl	fpaop(DOUBLEHI,1,0),a0@+
%_DTAN3A:		movl	fpaop(DOUBLELO,1,0),a0@+
%_DTAN3E:		movl	#0x5681936D,fpaop(DOUBLELO,1,0)
%_DTAN46:		movl	#0xBFEED9E5,fpaop(DOUBLEHI,1,0)
%_DTAN4E:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_DTAN52:		movl	a0@-,fpaop(MULDBLUPD,1,0)
%_DTAN56:		movl	#0x8E09B7BF,fpaop(DOUBLELO,0,0)
%_DTAN5E:		movl	#0x4058A222,fpaop(ADDDBLUPD,1,0)
%_DTAN66:		addql	#8,a0
%_DTAN68:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_DTAN6C:		movl	a0@-,fpaop(MULDBLUPD,1,0)
%_DTAN70:		movl	#0xAF153DAC,fpaop(DOUBLELO,0,0)
%_DTAN78:		movl	#0x4098FC89,fpaop(SUBDBLUPD,1,0)
%_DTAN80:		addql	#8,a0
%_DTAN82:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_DTAN86:		movl	a0@-,fpaop(MULDBLUPD,1,0)
%_DTAN8A:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_DTAN8E:		movl	a0@-,fpaop(MULDBLUPD,1,0)
%_DTAN92:		lea	a6@(-16),a1
%_DTAN96:		movl	a1@-,fpaop(DOUBLELO,2,0)
%_DTAN9A:		movl	a1@-,fpaop(DOUBLEHI,2,0)
%_DTAN9E:		movl	#0x81168C68,fpaop(DOUBLELO,0,0)
%_DTANA6:		movl	#0x405BFFA4,fpaop(SUBDBLUPD,2,0)
%_DTANAE:		addql	#8,a1
%_DTANB0:		movl	a1@-,fpaop(DOUBLELO,0,0)
%_DTANB4:		movl	a1@-,fpaop(MULDBLUPD,2,0)
%_DTANB8:		movl	#0x732A7D4A,fpaop(DOUBLELO,0,0)
%_DTANC0:		movl	#0x40A14D1F,fpaop(ADDDBLUPD,2,0)
%_DTANC8:		addql	#8,a1
%_DTANCA:		movl	a1@-,fpaop(DOUBLELO,0,0)
%_DTANCE:		movl	a1@-,fpaop(MULDBLUPD,2,0)
%_DTAND2:		movl	#0x434FEE6D,fpaop(DOUBLELO,0,0)
%_DTANDA:		movl	#0x40B2BD67,fpaop(SUBDBLUPD,2,0)
%_DTANE2:		movb	d0,fpaop(DIVDBLUPD,1,2)
%_DTANE6:		movl	a1@-,fpaop(DOUBLELO,0,0)
%_DTANEA:		movl	a1@-,fpaop(ADDDBLUPD,1,0)
%_DTANEE:		lea	a6@(-16),a0
%_DTANF2:		movl	fpaop(DOUBLEHI,1,0),a0@+
%_DTANF6:		movl	fpaop(DOUBLELO,1,0),a0@+
			andw	#0x1,d1
%_DTAN102:		beqs	%_DTAN128
%_DTAN104:		movl	#0x0,fpaop(DOUBLELO,1,0)
%_DTAN10C:		movl	#0x3FF00000,fpaop(DOUBLEHI,1,0)
%_DTAN114:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_DTAN118:		movl	a0@-,fpaop(DIVDBLUPD,1,0)
%_DTAN11C:		movb	d0,fpaop(NEGDBLUPD,2,1)
%_DTAN120:		movl	fpaop(DOUBLEHI,2,0),a0@+
%_DTAN124:		movl	fpaop(DOUBLELO,2,0),a0@+
%_DTAN128:		lea	a6@(-8),a0
%_DTAN12C:		movl	a0@-,fpaop(DOUBLELO,1,0)
%_DTAN130:		movl	a0@-,fpaop(DOUBLEHI,1,0)
%_DTAN134:		unlk	a6
%_DTAN136:		rtd	#4
	.globl	%_DASN
%_DASN:			linkw	a6,#-16
%_DASN4:		pea	a6@(-16)
%_DASN8:		movl	a6@(8),a0
%_DASNC:		addql	#8,a0
%_DASNE:		movl	a0@-,a7@-
%_DASN10:		movl	a0@-,a7@-
%_DASN12:		movw	#0x100,a7@-
%_DASN16:		jsr	%_EASC
%_DASN1C:		lea	a6@(-8),a0
%_DASN20:		movl	a0@-,fpaop(DOUBLELO,1,0)
%_DASN24:		movl	a0@-,fpaop(DOUBLEHI,1,0)
%_DASN28:		unlk	a6
%_DASN2A:		rtd	#4
	.globl	%_DACS
%_DACS:			linkw	a6,#-16
%_DACS4:		pea	a6@(-16)
%_DACS8:		movl	a6@(8),a0
%_DACSC:		addql	#8,a0
%_DACSE:		movl	a0@-,a7@-
%_DACS10:		movl	a0@-,a7@-
%_DACS12:		clrw	a7@-
%_DACS14:		jsr	%_EASC
%_DACS1A:		lea	a6@(-8),a0
%_DACS1E:		movl	a0@-,fpaop(DOUBLELO,1,0)
%_DACS22:		movl	a0@-,fpaop(DOUBLEHI,1,0)
%_DACS26:		unlk	a6
%_DACS28:		rtd	#4
	.globl	%_DAT2
%_DAT2:			linkw	a6,#-24
			movl	a6@(8),a0
			cmpl	#0x80000000,a0@
			beqs	%_DAT21
			tstl	a0@
			bnes	%_DAT24
%_DAT21: 		tstl	a0@(4)
			bnes	%_DAT24
|
|	The second arg is zero.  If the first is zero, we raise an
|	exception. Otherwise, the result is pi/2 * sign(first arg)
|
			movl	a6@(12),a1
			cmpl	#0x80000000,a1@
			beqs	%_DAT21A
			tstl	a1@
			bnes	%_DAT21X
%_DAT21A:		tstl	a1@(4)
			bnes	%_DAT21X
|			bclr	#7,a1@
|
|	Both arguments are zero.  Give datan of INF.
|
			movl	#0x7ff00000,a6@(-16)
			movl	#0,a6@(-12)
|			btst	#7,a1@
|			beqs	%_DAT22C
|			bset	#7,a6@(-16)
			bras	%_DAT22C
%_DAT21X:
|
|	The second arg is zero, but not the first.  Return pi/2 * sign(first)
|
			movl	#0x3ff921fb,a6@(-24)
			movl	#0x54442d18,a6@(-20)
%_DAT21D:		btst	#7,a1@
			beqs	%_DAT280
			bset	#7,a6@(-24)
			bras	%_DAT280
%_DAT24:		movl	a6@(12),a0
%_DAT28:		addql	#8,a0
%_DAT2A:		movl	a0@-,fpaop(DOUBLELO,1,0)
%_DAT2E:		movl	a0@-,fpaop(DOUBLEHI,1,0)
%_DAT212:		movl	a6@(8),a0
%_DAT216:		addql	#8,a0
%_DAT218:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_DAT21C:		movl	a0@-,fpaop(DIVDBLUPD,1,0)
%_DAT220:		lea	a6@(-16),a0
%_DAT224:		movl	fpaop(DOUBLEHI,1,0),a0@+
%_DAT228:		movl	fpaop(DOUBLELO,1,0),a0@+
%_DAT22C:		pea	a6@(-16)
%_DAT230:		jsr	%_DATN
%_DAT236:		lea	a6@(-24),a0
%_DAT23A:		movl	fpaop(DOUBLEHI,1,0),a0@+
%_DAT23E:		movl	fpaop(DOUBLELO,1,0),a0@+
%_DAT242:		movl	a6@(8),a1
%_DAT246:		addql	#8,a1
%_DAT248:		movl	a1@-,fpaop(DOUBLELO,0,0)
%_DAT24C:		movl	a1@-,fpaop(TSTDBL,0,0)
%_DAT250:		tstb	fpaop(CR,0,0)
%_DAT254:		bges	%_DAT280
%_DAT256:		movl	#0x54442D1A,a7@-
%_DAT25C:		movl	#0x400921FB,a7@-
%_DAT262:		movl	a6@(12),a1
%_DAT266:		addql	#8,a1
%_DAT268:		movl	a1@-,a7@-
%_DAT26A:		movl	a1@-,a7@-
%_DAT26C:		jsr	%D_SIGN
%_DAT272:		lea	a6@(-16),a0
%_DAT276:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_DAT27A:		movl	a0@-,fpaop(ADDDBLUPD,1,0)
%_DAT27E:		bras	%_DAT28C
%_DAT280:		lea	a6@(-16),a0
%_DAT284:		movl	a0@-,fpaop(DOUBLELO,1,0)
%_DAT288:		movl	a0@-,fpaop(DOUBLEHI,1,0)
%_DAT28C:		unlk	a6
%_DAT28E:		rtd	#8
	.globl	%_DSNH
%_DSNH:			linkw	a6,#-16
%_DSNH4:		pea	a6@(-16)
%_DSNH8:		movl	a6@(8),a0
%_DSNHC:		addql	#8,a0
%_DSNHE:		movl	a0@-,a7@-
%_DSNH10:		movl	a0@-,a7@-
%_DSNH12:		movw	#0x100,a7@-
%_DSNH16:		jsr	%_ESCH
%_DSNH1C:		lea	a6@(-8),a0
%_DSNH20:		movl	a0@-,fpaop(DOUBLELO,1,0)
%_DSNH24:		movl	a0@-,fpaop(DOUBLEHI,1,0)
%_DSNH28:		unlk	a6
%_DSNH2A:		rtd	#4
	.globl	%_DCSH
%_DCSH:			linkw	a6,#-16
%_DCSH4:		pea	a6@(-16)
%_DCSH8:		movl	a6@(8),a0
%_DCSHC:		addql	#8,a0
%_DCSHE:		movl	a0@-,a7@-
%_DCSH10:		movl	a0@-,a7@-
%_DCSH12:		clrw	a7@-
%_DCSH14:		jsr	%_ESCH
%_DCSH1A:		lea	a6@(-8),a0
%_DCSH1E:		movl	a0@-,fpaop(DOUBLELO,1,0)
%_DCSH22:		movl	a0@-,fpaop(DOUBLEHI,1,0)
%_DCSH26:		unlk	a6
%_DCSH28:		rtd	#4
	.globl	%_DTNH
%_DTNH:			linkw	a6,#-68
%_DTNH4:		movl	a4,a7@-
%_DTNH6:		movl	a6@(8),a4
%_DTNHA:		lea	a4@(8),a0
%_DTNHE:		movl	a0@-,fpaop(DOUBLELO,1,0)
%_DTNH12:		movl	a0@-,fpaop(DOUBLEHI,1,0)
%_DTNH16:		movb	d0,fpaop(TSTDBL,0,1)
%_DTNH1A:		tstb	fpaop(CR,0,0)
%_DTNH1E:		bpls	%_DTNH24
%_DTNH20:		movb	d0,fpaop(NEGDBLUPD,1,1)
%_DTNH24:		movl	#0x9999999A,fpaop(DOUBLELO,0,0)
%_DTNH2C:		movl	#0x40331999,fpaop(CMPDBL,1,0)
%_DTNH34:		tstb	fpaop(CR,0,0)
%_DTNH38:		bles	%_DTNH4A
%_DTNH3A:		lea	a6@(-16),a0
%_DTNH3E:		movl	#0x3FF00000,a0@+
%_DTNH44:		clrl	a0@+
%_DTNH46:		bra	%_DTNH250
%_DTNH4A:		lea	a4@(8),a0
%_DTNH4E:		movl	a0@-,fpaop(DOUBLELO,1,0)
%_DTNH52:		movl	a0@-,fpaop(DOUBLEHI,1,0)
%_DTNH56:		movb	d0,fpaop(TSTDBL,0,1)
%_DTNH5A:		tstb	fpaop(CR,0,0)
%_DTNH5E:		bpls	%_DTNH64
%_DTNH60:		movb	d0,fpaop(NEGDBLUPD,1,1)
%_DTNH64:		lea	a6@(-56),a0
%_DTNH68:		movl	fpaop(DOUBLEHI,1,0),a0@+
%_DTNH6C:		movl	fpaop(DOUBLELO,1,0),a0@+
%_DTNH70:		movl	a0@-,fpaop(DOUBLELO,1,0)
%_DTNH74:		movl	a0@-,fpaop(DOUBLEHI,1,0)
%_DTNH78:		movl	#0x0,fpaop(DOUBLELO,0,0)
%_DTNH80:		movl	#0x3FE80000,fpaop(CMPDBL,1,0)
%_DTNH88:		tstb	fpaop(CR,0,0)
%_DTNH8C:		bge	%_DTNH16E
%_DTNH90:		addql	#8,a0
%_DTNH92:		movl	a0@-,fpaop(DOUBLELO,1,0)
%_DTNH96:		movl	a0@-,fpaop(DOUBLEHI,1,0)
%_DTNH9A:		addql	#8,a0
%_DTNH9C:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_DTNHA0:		movl	a0@-,fpaop(MULDBLUPD,1,0)
%_DTNHA4:		addql	#8,a0
%_DTNHA6:		movl	fpaop(DOUBLEHI,1,0),a0@+
%_DTNHAA:		movl	fpaop(DOUBLELO,1,0),a0@+
%_DTNHAE:		movl	#0x96444324,fpaop(DOUBLELO,1,0)
%_DTNHB6:		movl	#0xBFEEDCC6,fpaop(DOUBLEHI,1,0)
%_DTNHBE:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_DTNHC2:		movl	a0@-,fpaop(MULDBLUPD,1,0)
%_DTNHC6:		movl	#0xB304DEF0,fpaop(DOUBLELO,0,0)
%_DTNHCE:		movl	#0x4058DAC3,fpaop(SUBDBLUPD,1,0)
%_DTNHD6:		addql	#8,a0
%_DTNHD8:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_DTNHDC:		movl	a0@-,fpaop(MULDBLUPD,1,0)
%_DTNHE0:		movl	#0xA7D1B192,fpaop(DOUBLELO,0,0)
%_DTNHE8:		movl	#0x40994587,fpaop(SUBDBLUPD,1,0)
%_DTNHF0:		addql	#8,a0
%_DTNHF2:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_DTNHF6:		movl	a0@-,fpaop(MULDBLUPD,1,0)
%_DTNHFA:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_DTNHFE:		movl	a0@-,fpaop(MULDBLUPD,1,0)
%_DTNH102:		lea	a0@(16),a1
%_DTNH106:		movl	a1@-,fpaop(DOUBLELO,2,0)
%_DTNH10A:		movl	a1@-,fpaop(DOUBLEHI,2,0)
%_DTNH10E:		movl	#0xECD4A78F,fpaop(DOUBLELO,0,0)
%_DTNH116:		movl	#0x405C3CFD,fpaop(ADDDBLUPD,2,0)
%_DTNH11E:		addql	#8,a1
%_DTNH120:		movl	a1@-,fpaop(DOUBLELO,0,0)
%_DTNH124:		movl	a1@-,fpaop(MULDBLUPD,2,0)
%_DTNH128:		movl	#0x23DFDB06,fpaop(DOUBLELO,0,0)
%_DTNH130:		movl	#0x40A17E3A,fpaop(ADDDBLUPD,2,0)
%_DTNH138:		addql	#8,a1
%_DTNH13A:		movl	a1@-,fpaop(DOUBLELO,0,0)
%_DTNH13E:		movl	a1@-,fpaop(MULDBLUPD,2,0)
%_DTNH142:		movl	#0xBDDD4541,fpaop(DOUBLELO,0,0)
%_DTNH14A:		movl	#0x40B2F425,fpaop(ADDDBLUPD,2,0)
%_DTNH152:		movb	d0,fpaop(DIVDBLUPD,1,2)
%_DTNH156:		movl	a1@-,fpaop(DOUBLELO,0,0)
%_DTNH15A:		movl	a1@-,fpaop(ADDDBLUPD,1,0)
%_DTNH15E:		lea	a6@(-16),a0
%_DTNH162:		movl	fpaop(DOUBLEHI,1,0),a0@+
%_DTNH166:		movl	fpaop(DOUBLELO,1,0),a0@+
%_DTNH16A:		bra	%_DTNH250
%_DTNH16E:		lea	a6@(-48),a0
%_DTNH172:		movl	a0@-,fpaop(DOUBLELO,1,0)
%_DTNH176:		movl	a0@-,fpaop(DOUBLEHI,1,0)
%_DTNH17A:		addql	#8,a0
%_DTNH17C:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_DTNH180:		movl	a0@-,fpaop(ADDDBLUPD,1,0)
%_DTNH184:		movl	fpaop(DOUBLELO,1,0),a7@-
%_DTNH188:		movl	fpaop(DOUBLEHI,1,0),a7@-
%_DTNH18C:		pea	a6@(-40)
%_DTNH190:		pea	a6@(-32)
%_DTNH194:		pea	a6@(-58)
%_DTNH198:		jsr	%_EERE
%_DTNH19E:		lea	a6@(-24),a0
%_DTNH1A2:		movl	a0@-,fpaop(DOUBLELO,1,0)
%_DTNH1A6:		movl	a0@-,fpaop(DOUBLEHI,1,0)
%_DTNH1AA:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_DTNH1AE:		movl	a0@-,fpaop(ADDDBLUPD,1,0)
%_DTNH1B2:		lea	a0@(-28),a1
%_DTNH1B6:		movl	fpaop(DOUBLEHI,1,0),a1@+
%_DTNH1BA:		movl	fpaop(DOUBLELO,1,0),a1@+
%_DTNH1BE:		tstw	a6@(-58)
%_DTNH1C2:		blts	%_DTNH1D0
%_DTNH1C4:		movw	a6@(-58),d0
%_DTNH1C8:		lslw	#0x4,d0
%_DTNH1CA:		addw	d0,a6@(-68)
%_DTNH1CE:		bras	%_DTNH1E0
%_DTNH1D0:		movw	a6@(-58),d0
%_DTNH1D4:		negw	d0
%_DTNH1D6:		movw	d0,a6@(-58)
%_DTNH1DA:		lslw	#0x4,d0
%_DTNH1DC:		subw	d0,a6@(-68)
%_DTNH1E0:		lea	a6@(-24),a0
%_DTNH1E4:		lea	a0@(-44),a1
%_DTNH1E8:		movl	a1@+,a0@+
%_DTNH1EA:		movl	a1@+,a0@+
%_DTNH1EC:		subql	#8,a0
%_DTNH1EE:		movl	a0@-,fpaop(DOUBLELO,1,0)
%_DTNH1F2:		movl	a0@-,fpaop(DOUBLEHI,1,0)
%_DTNH1F6:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_DTNH1FA:		movl	a0@-,fpaop(SUBDBLUPD,1,0)
%_DTNH1FE:		movl	#0x0,fpaop(DOUBLELO,0,0)
%_DTNH206:		movl	#0x40000000,fpaop(MULDBLUPD,1,0)
%_DTNH20E:		lea	a0@(16),a1
%_DTNH212:		movl	a1@-,fpaop(DOUBLELO,2,0)
%_DTNH216:		movl	a1@-,fpaop(DOUBLEHI,2,0)
%_DTNH21A:		movl	a1@-,fpaop(DOUBLELO,0,0)
%_DTNH21E:		movl	a1@-,fpaop(SUBDBLUPD,2,0)
%_DTNH222:		lea	a6@(-16),a0
%_DTNH226:		movl	a0@-,fpaop(DOUBLELO,0,0)
%_DTNH22A:		movl	a0@-,fpaop(ADDDBLUPD,2,0)
%_DTNH22E:		movb	d0,fpaop(DIVDBLUPD,1,2)
%_DTNH232:		movl	#0x0,fpaop(DOUBLELO,2,0)
%_DTNH23A:		movl	#0x3FF00000,fpaop(DOUBLEHI,2,0)
%_DTNH242:		movb	d0,fpaop(SUBDBLUPD,2,1)
%_DTNH246:		addql	#8,a0
%_DTNH248:		movl	fpaop(DOUBLEHI,2,0),a0@+
%_DTNH24C:		movl	fpaop(DOUBLELO,2,0),a0@+
%_DTNH250:		lea	a6@(-8),a0
%_DTNH254:		movl	a0@-,a7@-
%_DTNH256:		movl	a0@-,a7@-
%_DTNH258:		movl	a4@(4),a7@-
%_DTNH25C:		movl	a4@,a7@-
%_DTNH25E:		jsr	%D_SIGN
%_DTNH264:		movl	a7@+,a4
%_DTNH266:		unlk	a6
%_DTNH268:		rtd	#4
	.globl	%_DL10
%_DL10:			linkw	a6,#-8
%_DL104:		movl	a6@(8),a7@-
%_DL108:		jsr	%_DLOG
%_DL10E:		movl	#0xBBB55517,fpaop(DOUBLELO,0,0)
%_DL1016:		movl	#0x40026BB1,fpaop(DIVDBLUPD,1,0)
%_DL101E:		unlk	a6
%_DL1020:		rtd	#4
	.globl	%_PDATN
%_PDATN:		linkw	a6,#-8
%_PDATN4:		pea	a6@(8)
%_PDATN8:		jsr	%_DATN
%_PDATNE:		unlk	a6
%_PDATN10:		rtd	#8
	.globl	%_PDCOS
%_PDCOS:		linkw	a6,#-8
%_PDCOS4:		pea	a6@(8)
%_PDCOS10:		jsr	%_DCOS
%_PDCOS16:		unlk	a6
%_PDCOS18:		rtd	#8
	.globl	%_PDSIN
%_PDSIN:		linkw	a6,#-8
%_PDSIN4:		pea	a6@(8)
%_PDSINE:		jsr	%_DSIN
%_PDSIN14:		unlk	a6
%_PDSIN16:		rtd	#8
	.globl	%_PDEXP
%_PDEXP:		linkw	a6,#-8
%_PDEXP4:		pea	a6@(8)
%_PDEXP8:		jsr	%_DEXP
%_PDEXPE:		unlk	a6
%_PDEXP10:		rtd	#8
	.globl	%_PDLN
%_PDLN:			linkw	a6,#-8
%_PDLN4:		pea	a6@(8)
%_PDLN8:		jsr	%_DLOG
%_PDLNE:		unlk	a6
%_PDLN10:		rtd	#8
	.globl	%_PDSQT
%_PDSQT:		linkw	a6,#-8
%_PDSQT4:		pea	a6@(8)
%_PDSQT8:		jsr	%_DSQT
%_PDSQTE:		unlk	a6
%_PDSQT10:		rtd	#8
	.globl	%D_DPROD
%D_DPROD:		linkw	a6,#-16
%D_DPROD4:		movl	a6@(12),fpaop(SGLDBLUPD,1,0)
%D_DPRODA:		lea	a6@(-16),a0
%D_DPRODE:		movl	fpaop(DOUBLEHI,1,0),a0@+
%D_DPROD12:		movl	fpaop(DOUBLELO,1,0),a0@+
%D_DPROD16:		movl	a6@(8),fpaop(SGLDBLUPD,1,0)
%D_DPROD1C:		movl	a0@-,fpaop(DOUBLELO,0,0)
%D_DPROD20:		movl	a0@-,fpaop(MULDBLUPD,1,0)
%D_DPROD24:		unlk	a6
%D_DPROD26:		rtd	#8
