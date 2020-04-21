|S$TAN           

|
|	GB - patches installed to make consistent with double version.
|		use _fperror structure reference with offset of zero
|		for saving operand on error.
|
SIN = 1
COS = 	2
TAN = 	3
LOG = 	4
EXP = 	5
SQRT = 6
POW = 	7
ASIN = 8
ACOS = 9
SINH = 10
COSH = 11
ATAN2 = 12
ATAN = 13
UP_I = 14
GAMMA = 15
HYPOT = 16
J0 = 	17
J1 = 	18
Y0 = 	19
Y1 = 	20
YN = 	21
LOG10 = 22
	.globl	__fperror

|S_TAN           
|Starting peephole.  Msize=    ; Value = 12
|Maxtmp=                       ; Value = 128

	.globl	s_tan            
s_tan:
	link    a6,#-140

|  42	   $pi_reduce(arg, x, multiple, error);
	movl    a6@(16),sp@-
	pea     a6@(-4)
	pea     a6@(-8)
	movl    a6@(12),sp@-
	jsr     _pi_reduce
	addl    #16,sp

|  43	   IF error = none THEN BEGIN
	movl    a6@(16),a0
	tstl    a0@
	jne     _1

|  44	      xsq := sqr(x);
	movl    a6@(-8),sp@-
	movl    a6@(-8),sp@-
	jsr     _f_mul
	addql   #8,sp
	movl    d0,a6@(-12)

|  45	      result := x + x * xsq * (0.333335034 -
	movl    d0,sp@-
	movl    a6@(-8),sp@-
	jsr     _f_mul
	addql   #8,sp
	movl    #0x401E0939,sp@-
	movl    a6@(-12),sp@-
	moveml  #0x0001,a6@(-140)
	jsr     _f_sub
	addql   #8,sp
	movl    d0,d1
	moveml  a6@(-140),#0x0001
	movl    d1,sp@-
	movl    #0x3EA88E8C,sp@-
	moveml  #0x0001,a6@(-140)
	jsr     _f_div
	addql   #8,sp
	movl    d0,d1
	moveml  a6@(-140),#0x0001
	movl    d1,sp@-
	movl    a6@(-12),sp@-
	moveml  #0x0001,a6@(-140)
	jsr     _f_mul
	addql   #8,sp
	movl    d0,d1
	moveml  a6@(-140),#0x0001
	movl    d1,sp@-
	movl    #0x3EAAAAE3,sp@-
	moveml  #0x0001,a6@(-140)
	jsr     _f_sub
	addql   #8,sp
	movl    d0,d1
	moveml  a6@(-140),#0x0001
	movl    d1,sp@-
	movl    d0,sp@-
	jsr     _f_mul
	addql   #8,sp
	movl    d0,sp@-
	movl    a6@(-8),sp@-
	jsr     _f_add
	addql   #8,sp
	movl    a6@(8),a1
	movl    d0,a1@

|  47	      IF odd(multiple) THEN
	movl    a6@(-4),d0
	andl    #1,d0
	tstl    d0
	jeq     _2

|  48	         result := -1.0 / result;
	movl    a6@(8),a0
	movl    a0@,sp@-
	movl    #0x3F800000,sp@-
	jsr     _f_div
	addql   #8,sp
	tstl    d0
	jeq     .L2
	bchg    #31,d0
.L2:
	movl    a6@(8),a1
	movl    d0,a1@
_2:
_1:

|  50	END;
	unlk    a6
	rts     
|TAN             

|TAN             
|Starting peephole.  Msize=    ; Value = 12
|Maxtmp=                       ; Value = 0

	.globl	_tan              
_tan:
	link    a6,#-12
|
|	GB - patch
	movl	a6@(8),__fperror
|

|  57	   s_tan(result, arg, error);
	pea     a6@(-8)
	movl    a6@(8),sp@-
	pea     a6@(-12)
	jsr     s_tan
	addl    #12,sp

|  58	   IF error = none THEN tan := result
	tstl    a6@(-8)
	jne     _3

|  58	
	movl    a6@(-12),a6@(-4)
	jra     _4
_3:

|  59	                   ELSE tan := $t_error('tan   ', error);
	movl    a6@(-8),sp@-
	movl    #TAN,sp@-
	jsr     _t_error
	addql   #8,sp
	movl    d0,a6@(-4)
_4:
	movl    a6@(-4),d0

|  60	END;
	unlk    a6
	rts     
	.data
	.even
	.text
|LOG10           

|LOG10           
|Starting peephole.  Msize=    ; Value = 12
|Maxtmp=                       ; Value = 0

	.globl	_log10            
_log10:
	link    a6,#-12
|
|	GB - patch
	movl	a6@(8),__fperror
|

|  67	   $t_log(result, arg, error);
	pea     a6@(-8)
	movl    a6@(8),sp@-
	pea     a6@(-12)
	jsr     _t_log
	addl    #12,sp

|  68	   IF error = none THEN log10 := result / 2.30258509
	tstl    a6@(-8)
	jne     _5

|  68	
	movl    #0x40135D8D,sp@-
	movl    a6@(-12),sp@-
	jsr     _f_div
	addql   #8,sp
	movl    d0,a6@(-4)
	jra     _6
_5:

|  69	                   ELSE log10 := $t_error('alog10', error);
	movl    a6@(-8),sp@-
	movl    #LOG10,sp@-
	jsr     _t_error
	addql   #8,sp
	movl    d0,a6@(-4)
_6:
	movl    a6@(-4),d0

|  70	END;
	unlk    a6
	rts     
	.data
	.even
	.text
|    3 Instructions Emitted.
| Init = 66 Read = 1035 Asm = 382 Write = 733
