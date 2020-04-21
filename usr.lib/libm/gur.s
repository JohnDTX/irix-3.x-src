|$T$UNPACK       

|$T_UNPACK       
|Starting peephole.  Msize=    ; Value = 12
|Maxtmp=                       ; Value = 0

	.globl	_t_unpack        
_t_unpack:
	link    a6,#-12

|  43	   $pf_unpk(value, u);
	pea     a6@(-12)
	movl    a6@(8),sp@-
	jsr     _pf_unpk
	addql   #8,sp

|  44	   exponent := u.exp + 23;
	movw    a6@(-4),d0
	extl    d0
	addl    #23,d0
	movl    a6@(16),a0
	movl    d0,a0@

|  45	   u.exp := -23;
	movw    #-23,a6@(-4)

|  46	   $pf_pack(mantissa, u);
	pea     a6@(-12)
	movl    a6@(12),sp@-
	jsr     _pf_pack
	addql   #8,sp

|  47	END;
	unlk    a6
	rts     
|$T$LOGB         

|$T_LOGB         
|Starting peephole.  Msize=    ; Value = 16
|Maxtmp=                       ; Value = 0

	.globl	_t_logb          
_t_logb:
	link    a6,#-16

|  57	   $pf_unpk(arg, u);
	pea     a6@(-16)
	movl    a6@(8),sp@-
	jsr     _pf_unpk
	addql   #8,sp

|  58	   $t_logb := u.exp + 23;
	movw    a6@(-8),d0
	extl    d0
	addl    #23,d0
	movl    d0,a6@(-4)
	movl    a6@(-4),d0

|  59	END;
	unlk    a6
	rts     
|$T$PACK         

|$T_PACK         
|Starting peephole.  Msize=    ; Value = 12
|Maxtmp=                       ; Value = 0

	.globl	_t_pack          
_t_pack:
	link    a6,#-12

|  73	   $pf_unpk(value, u);
	pea     a6@(-12)
	movl    a6@(8),sp@-
	jsr     _pf_unpk
	addql   #8,sp

|  74	   u.exp := u.exp + power_of_2;
	movw    a6@(-4),d0
	extl    d0
	addl    a6@(12),d0
	movl    d0,d1
	subl    #-32767,d1
	cmpl    #65534,d1
	jle     .L4
	chkw     #-1,d1
.L4:
	movw    d0,a6@(-4)

|  75	   IF          u.exp < r_min_exp - 23 THEN BEGIN
	moveq   #-127,d0
	subl    #23,d0
	movw    a6@(-4),d1
	extl    d1
	cmpl    d0,d1
	jge     _1

|  76	      error := underflow ;
	movl    a6@(20),a0
	movl    #1,a0@

|  77	      result := 0;
	movl    #0,sp@-
	jsr     _i_2_f
	addql   #4,sp
	movl    a6@(16),a1
	movl    d0,a1@
	jra     _2
_1:

|  78	   END ELSE IF u.exp > r_max_exp - 23 THEN BEGIN
	moveq   #127,d0
	subl    #23,d0
	movw    a6@(-4),d1
	extl    d1
	cmpl    d0,d1
	jle     _3

|  79	      error := overflow;
	movl    a6@(20),a0
	movl    #2,a0@

|  80	      result :=(* $7F800000*) 2139095040;
	movl    #2139095040,sp@-
	jsr     _i_2_f
	addql   #4,sp
	movl    a6@(16),a1
	movl    d0,a1@
	jra     _4
_3:

|  81	   END ELSE      $pf_pack(result, u);
	pea     a6@(-12)
	movl    a6@(16),sp@-
	jsr     _pf_pack
	addql   #8,sp
_4:
_2:

|  82	END;
	unlk    a6
	rts     
|$T$HALFSZ       

|$T_HALFSZ       
|Starting peephole.  Msize=    ; Value = 20
|Maxtmp=                       ; Value = 0

	.globl	_t_halfsz        
_t_halfsz:
	link    a6,#-20

|  93	   $pf_unpk(arg, u);
	pea     a6@(-20)
	movl    a6@(8),sp@-
	jsr     _pf_unpk
	addql   #8,sp

|  94	   u.lower := (u.lower + 2048) DIV 4096;
	movl    a6@(-16),d0
	addl    #2048,d0
	movl    #4096,sp@-
	movl    d0,sp@-
	jsr     ldiv
	addql   #8,sp
	movl    d0,a6@(-16)

|  95	   u.exp := u.exp + 12;
	movw    a6@(-12),d1
	extl    d1
	addl    #12,d1
	movl    d1,d0
	subl    #-32767,d0
	cmpl    #65534,d0
	jle     .L6
	chkw     #-1,d0
.L6:
	movw    d1,a6@(-12)

|  96	   $pf_pack(temp, u);
	pea     a6@(-20)
	pea     a6@(-8)
	jsr     _pf_pack
	addql   #8,sp

|  97	   $t_halfsz := temp;
	movl    a6@(-8),a6@(-4)
	movl    a6@(-4),d0

|  98	END;
	unlk    a6
	rts     
	.text
|$T$ERROR        

|$T_ERROR        
|Starting peephole.  Msize=    ; Value = 12
|Maxtmp=                       ; Value = 0

	.globl	__mathfunc_id
	.data
	.even
_terrs:
	.word	0
	.word	0x400			| UNDERFL
	.word	0x300			| OVERFL
	.word	0x600			| DOMAIN_ERROR
	.word	0x110			| INVALID_OP_A
	.word	0x700			| CANT_REDUCE_RANGE
	.word	0x162			| INFINITY
	.text

	.globl	_t_error         
_t_error:
	link    a6,#-12
	lea     a6@(-12),a0
|	
|	copy the string
|
|	movl    a6@(8),a1
|	movl    a1@+,a0@+
|	movw    a1@+,a0@+
|	clrb	a0@

| 117	   $rep_error(routine_name, error);
|	movl    a6@(12),sp@-
|	pea     a6@(-12)
|	jsr     _rep_error
|	addql   #8,sp
|
|| 118	   $t_error := 0.0;
|	movl    #0,a6@(-4)
|	movl    a6@(-4),d0
|
| 119	END;
|
|	GB - patch for fp error handling.
|
|		first, translate the error code:
|
|			underflow (1) -> UNDERFL (0x400)
|			overflow (2)  -> OVERFL	 (0x300)
|			domain (3)	  -> DOMAIN_ERROR (0x600)
|			not_a_number(4)-> INVALID_OP_A (0x110)
|			infeasible (5) -> CANT_REDUCE_RANGE (0x700)
|			infinity (6) -> INVALID_OP_F2	(0x162)
|
	movl	a6@(12),d0
	movl	#_terrs,a0
	lsll		#1,d0
	movw	a0@(0,d0:w),d0
	extl	d0
|
|	move the routine id to the global
|
|	lea		a6@(-12),a0
	movl	a6@(8),__mathfunc_id
|
|	_raise_fperror(MATH,<translated error>);
|
	movl	d0,sp@-
	movl	#0xa,sp@-		| MATH
	jbsr	__raise_fperror
	addql	#8,sp
|
|	d0 has the result
|
	unlk    a6
	rts     
|$T$CHECK        


|$T_CHECK        
|Starting peephole.  Msize=    ; Value = 16
|Maxtmp=                       ; Value = 0

	.globl	_t_check         
_t_check:
	link    a6,#-16

| 127	   $pf_unpk(value, u);
	pea     a6@(-16)
	movl    a6@(12),sp@-
	jsr     _pf_unpk
	addql   #8,sp

| 128	   IF u.knd = 5 THEN  error:=not_a_number
	cmpb    #5,a6@(-5)
	jne     _7

| 128	
	movl    a6@(8),a0
	movl    #4,a0@
	jra     _8
_7:
|
|	GB - return error = 6 if input is INF.
|
	cmpb	#4,a6@(-5)		| code for INF
	jne		_7a7a7
	movl	a6@(8),a0
	movl	#6,a0@
	jra		_8

_7a7a7:

| 129	                ELSE  error:=none;
	movl    a6@(8),a0
	movl    #0,a0@
_8:

| 130	   $t_check := error;
	movl    a6@(8),a0
	movl    a0@,a6@(-4)
	movl    a6@(-4),d0

| 131	END;
	unlk    a6
	rts     
|    7 Instructions Emitted.
| Init = 33 Read = 2135 Asm = 599 Write = 1232
