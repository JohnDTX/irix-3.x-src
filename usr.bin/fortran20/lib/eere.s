|	inline e reduction routine
|	called with "arg" in f1,
|	returns mult in d1, p in f1, q in f2
|	trashes registers f1, f2, f3
|
|	f2 = upper = (double)(single)arg
		movb	d0,fpaop(DBLSGL,0,1)
		movb	d0,fpaop(SGLDBLUPD,2,f)
|	f3 = lower = arg - upper
		movb	d0,fpaop(SUBDBL,1,2)
		movb	d0,fpaop(COPY,3,f)
|	f2 = x = upper * log2_u
		movl	#0x40000000,fpaop(DOUBLELO,0,0)
		movl	#0x3FF71547,fpaop(MULDBLUPD,2,0)
|	d1 = mult = round(x)
		movb	fpaop(OR,0,0),d0
		andb	#0xF1,fpaop(OR,0,0)
		movb	d0,fpaop(DBLINT,0,2)
		movb	d0,fpaop(OR,0,0)
		movl	fpaop(INTEGER,f,0),d1
|	x = calculation
|	f2 = x - mult
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
|	f1 = p = x * (p0 + xsq*(p1 + xsq*p2))
		movl	#0xAA5CD009,fpaop(DOUBLELO,0,0)
		movl	#0x3F97A609,fpaop(MULDBL,2,0)
		movl	#0x9C957777,fpaop(DOUBLELO,0,0)
		movl	#0x403433A2,fpaop(ADDDBL,f,0)
		movb	d0,fpaop(MULDBL,2,f)
		movl	#0xE9C773D2,fpaop(DOUBLELO,0,0)
		movl	#0x4097A774,fpaop(ADDDBL,f,0)
		movb	d0,fpaop(MULDBLUPD,1,f)
|	f2 = q = q0 + xsq*(q1 + xsq)
		movl	#0x13B3FFDA,fpaop(DOUBLELO,0,0)
		movl	#0x406D25B4,fpaop(ADDDBL,2,0)
		movb	d0,fpaop(MULDBLUPD,2,f)
		movl	#0xB314DFB1,fpaop(DOUBLELO,0,0)
		movl	#0x40B11016,fpaop(ADDDBLUPD,2,0)
