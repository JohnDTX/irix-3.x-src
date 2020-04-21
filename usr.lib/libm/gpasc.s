	.globl	__raise_fperror
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
LOG10 = 	22
	.globl	__fperror

|$T_SQRT         
|Starting peephole.  Msize=    ; Value = 12
|Maxtmp=                       ; Value = 128

	.globl	_t_sqrt          
_t_sqrt:
	link    a6,#-140

|  59	   IF $t_check(error, x) = none THEN BEGIN
	movl    a6@(12),sp@-
	movl    a6@(16),sp@-
	jsr     _t_check
	addql   #8,sp
	tstl    d0
	jne     _1

|
|	**** PATCH by GB - scr1321.  Test for -0.
|
	movl	a6@(12),d0
	cmpl	#0x80000000,d0
	bne		.L0
	clrl	a6@(12)
.L0:

|  60	      IF x < 0 THEN
	movl    #0,sp@-
	jsr     _i_2_f
	addql   #4,sp
	tstl    d0
	jpl     .L2
	eorl    #0x7FFFFFFF,d0
.L2:
	movl    a6@(12),d1
	jpl     .L3
	eorl    #0x7FFFFFFF,d1
.L3:
	cmpl    d1,d0
	jle     _2

|  61	         error := domain_error
	movl    a6@(16),a0
	movl    #3,a0@
| 
|	*** PATCH by GB - domain error should signal exception.
|
|	movl	#INVALID_OP_F1,sp@-
|	movl	#SQRT,sp@-
|	jbsr	__raise_fperror
|	addql	#8,sp
	jra     _3
_2:

|  62	      ELSE IF x = 0 THEN
	movl    #0,sp@-
	jsr     _i_2_f
	addql   #4,sp
	cmpl    a6@(12),d0
	jne     _4

|  63	         y := 0.0
	movl    a6@(8),a0
	movl    #0,a0@
	jra     _5
_4:

|  65	         $t_unpack(x,mantissa,exponent);
	pea     a6@(-12)
	pea     a6@(-4)
	movl    a6@(12),sp@-
	jsr     _t_unpack
	addl    #12,sp

|  66	         IF odd(exponent) THEN BEGIN
	movl    a6@(-12),d0
	andl    #1,d0
	tstl    d0
	jeq     _6

|  67	            mantissa := mantissa / 2.0;
	movl    #0x40000000,sp@-
	movl    a6@(-4),sp@-
	jsr     _f_div
	addql   #8,sp
	movl    d0,a6@(-4)

|  68	            exponent := exponent + 1;
	addql   #1,a6@(-12)

|  69	            approx := 0.5901621 * mantissa + 0.4173076;
	movl    d0,sp@-
	movl    #0x3F1714DD,sp@-
	jsr     _f_mul
	addql   #8,sp
	movl    #0x3ED5A957,sp@-
	movl    d0,sp@-
	jsr     _f_add
	addql   #8,sp
	movl    d0,a6@(-8)
	jra     _7
_6:

|  71	            approx := 0.4173076 * mantissa + 0.5901621;
	movl    a6@(-4),sp@-
	movl    #0x3ED5A957,sp@-
	jsr     _f_mul
	addql   #8,sp
	movl    #0x3F1714DD,sp@-
	movl    d0,sp@-
	jsr     _f_add
	addql   #8,sp
	movl    d0,a6@(-8)
_7:

|  73	         approx := approx + (mantissa/approx);
	movl    a6@(-8),sp@-
	movl    a6@(-4),sp@-
	jsr     _f_div
	addql   #8,sp
	movl    d0,sp@-
	movl    a6@(-8),sp@-
	jsr     _f_add
	addql   #8,sp
	movl    d0,a6@(-8)

|  74	         approx := approx + (4.0*mantissa/approx);
	movl    a6@(-4),sp@-
	movl    #0x40800000,sp@-
	jsr     _f_mul
	addql   #8,sp
	movl    a6@(-8),sp@-
	movl    d0,sp@-
	jsr     _f_div
	addql   #8,sp
	movl    d0,sp@-
	movl    a6@(-8),sp@-
	jsr     _f_add
	addql   #8,sp
	movl    d0,a6@(-8)

|  75	         $t_pack(approx, (exponent DIV 2)-2, y, error);
	movl    #2,sp@-
	movl    a6@(-12),sp@-
	moveml  #0x0001,a6@(-140)
	jsr     ldiv
	addql   #8,sp
	movl    d0,d1
	moveml  a6@(-140),#0x0001
	subql   #2,d1
	movl    a6@(16),sp@-
	movl    a6@(8),sp@-
	movl    d1,sp@-
	movl    d0,sp@-
	jsr     _t_pack
	addl    #16,sp
_5:
_3:
_1:

|  78	END;
	unlk    a6
	rts     
|$PI$REDUCE      

|$PI_REDUCE      
|Starting peephole.  Msize=    ; Value = 68
|Maxtmp=                       ; Value = 128

	.globl	_pi_reduce       
_pi_reduce:
	link    a6,#-196
	moveml  #0x0100,sp@-

| 105	   IF $t_check(error, arg) = none THEN BEGIN
	movl    a6@(8),sp@-
	movl    a6@(20),sp@-
	jsr     _t_check
	addql   #8,sp
	tstl    d0
	jne     _8

| 106	      IF abs(arg) > max_input THEN
	movl    a6@(8),d0
	bclr    #31,d0
	tstl    d0
	jpl     .L5
	eorl    #0x7FFFFFFF,d0
.L5:
	cmpl    #0x4EFFFFFF,d0
	jle     _9

| 107	         error := infeasible
	movl    a6@(20),a0
	movl    #5,a0@
	jra     _10
_9:

| 108	      ELSE IF abs(arg) < 0.7854 THEN BEGIN  { This constant may }
	movl    a6@(8),d0
	bclr    #31,d0
	cmpl    #0x3F490FF9,d0
	jge     _11

| 109	         result := arg;                     { be perturbed to   }
	movl    a6@(12),a0
	movl    a6@(8),a0@

| 110	         multiple := 0;                   { assure monotonicity }
	movl    a6@(16),a1
	movl    #0,a1@
	jra     _12
_11:

| 113	         upper := $t_halfsz(arg);         lower := arg - upper;
	movl    a6@(8),sp@-
	jsr     _t_halfsz
	addql   #4,sp
	movl    d0,a6@(-4)

| 113	
	movl    d0,sp@-
	movl    a6@(8),sp@-
	jsr     _f_sub
	addql   #8,sp
	movl    d0,a6@(-8)

| 116	         pi[1].ii := 1059250176  { 0.63623047 } ;
	movl    #1059250176,a6@(-68)

| 117	         pi[2].ii :=  969670656  { 0.38909912E-03 } ;
	movl    #969670656,a6@(-64)

| 118	         pi[3].ii :=  878411776  { 0.20442531E-06 } ;
	movl    #878411776,a6@(-60)

| 119	         pi[4].ii :=  782008320  { 0.71167960E-10 } ;
	movl    #782008320,a6@(-56)

| 120	         pi[5].ii :=  679641088  { 0.14488410E-13 } ;
	movl    #679641088,a6@(-52)

| 121	         pi[6].ii :=  596959232  { 0.16141060E-16 } ;
	movl    #596959232,a6@(-48)

| 122	         pi[7].ii :=  510689280  { 0.12731964E-19 } ;
	movl    #510689280,a6@(-44)

| 123	         pi[8].ii :=  418045952  { 0.60713764E-23 } ;
	movl    #418045952,a6@(-40)

| 124	         pi[8].ii :=  324132864  { 0.26489949E-26 } ;
	movl    #324132864,a6@(-40)

| 125	         pi[10].ii:=  229220352  { 0.10453948E-29 } ;
	movl    #229220352,a6@(-32)

| 126	         result := pi[1].rr * upper;
	movl    a6@(-4),sp@-
	movl    a6@(-68),sp@-
	jsr     _f_mul
	addql   #8,sp
	movl    a6@(12),a0
	movl    d0,a0@

| 127	         cutoff := $t_logb(result) - (half_word_size - 2);
	movl    a6@(12),a1
	movl    a1@,sp@-
	jsr     _t_logb
	addql   #4,sp
	moveq   #11,d1
	subql   #2,d1
	subl    d1,d0
	movl    d0,a6@(-16)

| 128	         multiple := round(result);
	movl    a6@(12),a0
	movl    a0@,sp@-
	jsr     _round
	addql   #4,sp
	movl    a6@(16),a1
	movl    d0,a1@

| 129	         result := result - multiple;
	movl    a6@(12),a0
	movl    a6@(16),a1
	movl    a1@,sp@-
	moveml  #0x0100,a6@(-196)
	jsr     _i_2_f
	addql   #4,sp
	moveml  a6@(-196),#0x0100
	movl    d0,sp@-
	movl    a0@,sp@-
	jsr     _f_sub
	addql   #8,sp
	movl    a6@(12),a0
	movl    d0,a0@

| 130	         r_lower := 0.0;
	movl    #0,a6@(-12)

| 131	         i := 1;
	movl    #1,a6@(-24)

| 132	         WHILE ($t_logb(result) < cutoff) AND
_13:
	movl    a6@(12),a0
	movl    a0@,sp@-
	jsr     _t_logb
	addql   #4,sp
	cmpl    a6@(-16),d0
	slt     d0
	movl    a6@(-24),d1
	addql   #2,d1
	cmpl    #10,d1
	slt     d1
	andb    d1,d0
	jeq     _14

| 134	            r_lower := r_lower + (lower*pi[i].rr + upper*pi[i+1].rr);
	movl    a6@(-24),d0
	asll    #2,d0
	movl    a6@(-72,d0:L),sp@-
	movl    a6@(-8),sp@-
	jsr     _f_mul
	addql   #8,sp
	movl    a6@(-24),d1
	addql   #1,d1
	asll    #2,d1
	movl    a6@(-72,d1:L),sp@-
	movl    a6@(-4),sp@-
	moveml  #0x0001,a6@(-196)
	jsr     _f_mul
	addql   #8,sp
	movl    d0,d1
	moveml  a6@(-196),#0x0001
	movl    d1,sp@-
	movl    d0,sp@-
	jsr     _f_add
	addql   #8,sp
	movl    d0,sp@-
	movl    a6@(-12),sp@-
	jsr     _f_add
	addql   #8,sp
	movl    d0,a6@(-12)

| 135	            rtemp := result + r_lower;
	movl    a6@(12),a1
	movl    d0,sp@-
	movl    a1@,sp@-
	jsr     _f_add
	addql   #8,sp
	movl    d0,a6@(-20)

| 136	            IF $t_logb(rtemp) >= (-1) THEN BEGIN   { >= 0.5 }
	movl    d0,sp@-
	jsr     _t_logb
	addql   #4,sp
	cmpl    #-1,d0
	jlt     _15

| 137	               itemp := round(rtemp);
	movl    a6@(-20),sp@-
	jsr     _round
	addql   #4,sp
	movl    d0,a6@(-28)

| 138	               multiple := multiple + itemp;
	movl    a6@(16),a0
	addl    a0@,d0
	movl    d0,a0@

| 139	               result := result - itemp;
	movl    a6@(12),a1
	movl    a6@(-28),sp@-
	moveml  #0x0200,a6@(-196)
	jsr     _i_2_f
	addql   #4,sp
	moveml  a6@(-196),#0x0200
	movl    d0,sp@-
	movl    a1@,sp@-
	jsr     _f_sub
	addql   #8,sp
	movl    a6@(12),a0
	movl    d0,a0@

| 140	               rtemp := result + r_lower;
	movl    a6@(12),a1
	movl    a6@(-12),sp@-
	movl    a1@,sp@-
	jsr     _f_add
	addql   #8,sp
	movl    d0,a6@(-20)
_15:

| 142	            r_lower := (result - rtemp) + r_lower;
	movl    a6@(12),a0
	movl    a6@(-20),sp@-
	movl    a0@,sp@-
	jsr     _f_sub
	addql   #8,sp
	movl    a6@(-12),sp@-
	movl    d0,sp@-
	jsr     _f_add
	addql   #8,sp
	movl    d0,a6@(-12)

| 143	            result := rtemp;
	movl    a6@(12),a1
	movl    a6@(-20),a1@

| 144	            cutoff := cutoff - half_word_size;
	subl    #11,a6@(-16)

| 145	            i := i+1;
	addql   #1,a6@(-24)
	jra     _13
_14:

| 147	         result := 1.5707963 * (( (lower * pi[i].rr
	movl    a6@(-24),d0
	asll    #2,d0
	movl    a6@(-72,d0:L),sp@-
	movl    a6@(-8),sp@-
	jsr     _f_mul
	addql   #8,sp
	movl    a6@(-24),d1
	addql   #1,d1
	asll    #2,d1
	movl    a6@(-24),d7
	addql   #2,d7
	asll    #2,d7
	movl    a6@(-72,d7:L),sp@-
	movl    a6@(-72,d1:L),sp@-
	moveml  #0x0001,a6@(-196)
	jsr     _f_add
	addql   #8,sp
	movl    d0,d1
	moveml  a6@(-196),#0x0001
	movl    d1,sp@-
	movl    a6@(8),sp@-
	moveml  #0x0001,a6@(-196)
	jsr     _f_mul
	addql   #8,sp
	movl    d0,d1
	moveml  a6@(-196),#0x0001
	movl    d1,sp@-
	movl    d0,sp@-
	jsr     _f_add
	addql   #8,sp
	movl    a6@(-12),sp@-
	movl    d0,sp@-
	jsr     _f_add
	addql   #8,sp
	movl    a6@(12),a0
	movl    a0@,sp@-
	movl    d0,sp@-
	jsr     _f_add
	addql   #8,sp
	movl    d0,sp@-
	movl    #0x3FC90FDA,sp@-
	jsr     _f_mul
	addql   #8,sp
	movl    a6@(12),a1
	movl    d0,a1@
_12:
_10:
_8:

| 152	END;
	moveml  sp@+,#0x0080
	unlk    a6
	rts     
|S$SNCS          

|S_SNCS          
|Starting peephole.  Msize=    ; Value = 12
|Maxtmp=                       ; Value = 128

	.globl	s_sncs           
s_sncs:
	link    a6,#-140

| 164	   $pi_reduce(arg, x, multiple, error);
	movl    a6@(16),sp@-
	pea     a6@(-4)
	pea     a6@(-8)
	movl    a6@(12),sp@-
	jsr     _pi_reduce
	addl    #16,sp

| 165	   IF error = none THEN BEGIN
	movl    a6@(16),a0
	tstl    a0@
	jne     _16

| 166	      xsq := sqr(x);
	movl    a6@(-8),sp@-
	movl    a6@(-8),sp@-
	jsr     _f_mul
	addql   #8,sp
	movl    d0,a6@(-12)

| 167	      multiple := multiple + sin_cos_flag;
	movl    a6@(20),d0
	addl    d0,a6@(-4)

| 168	      IF odd(multiple) THEN BEGIN
	movl    a6@(-4),d1
	andl    #1,d1
	tstl    d1
	jeq     _17

| 169	         result := 1.0 + xsq * (-4.99998424e-1 +       { cos }
	movl    #0xBAB1FCEA,sp@-
	movl    a6@(-12),sp@-
	jsr     _f_mul
	addql   #8,sp
	movl    d0,sp@-
	movl    #0x3D2A9DD3,sp@-
	jsr     _f_add
	addql   #8,sp
	movl    d0,sp@-
	movl    a6@(-12),sp@-
	jsr     _f_mul
	addql   #8,sp
	movl    d0,sp@-
	movl    #0xBEFFFFCB,sp@-
	jsr     _f_add
	addql   #8,sp
	movl    d0,sp@-
	movl    a6@(-12),sp@-
	jsr     _f_mul
	addql   #8,sp
	movl    d0,sp@-
	movl    #0x3F800000,sp@-
	jsr     _f_add
	addql   #8,sp
	movl    a6@(8),a1
	movl    d0,a1@

| 172	         multiple := multiple - 1;
	subql   #1,a6@(-4)
	jra     _18
_17:

| 174	         result := x + x * xsq * (-1.66666502e-1 +     { sin }
	movl    a6@(-12),sp@-
	movl    a6@(-8),sp@-
	jsr     _f_mul
	addql   #8,sp
	movl    #0xB94C7DC9,sp@-
	movl    a6@(-12),sp@-
	moveml  #0x0001,a6@(-140)
	jsr     _f_mul
	addql   #8,sp
	movl    d0,d1
	moveml  a6@(-140),#0x0001
	movl    d1,sp@-
	movl    #0x3C088302,sp@-
	moveml  #0x0001,a6@(-140)
	jsr     _f_add
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
	movl    #0xBE2AAA9F,sp@-
	moveml  #0x0001,a6@(-140)
	jsr     _f_add
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
	movl    a6@(8),a0
	movl    d0,a0@
_18:

| 177	      IF odd(multiple DIV 2) THEN
	movl    #2,sp@-
	movl    a6@(-4),sp@-
	jsr     ldiv
	addql   #8,sp
	andl    #1,d0
	tstl    d0
	jeq     _19

| 178	         result := - result;
	movl    a6@(8),a0
	movl    a0@,d0
	tstl    d0
	jeq     .L7
	bchg    #31,d0
.L7:
	movl    d0,a0@
_19:
_16:

| 180	END;
	unlk    a6
	rts     
|$T$ATAN         

|$T_ATAN         
|Starting peephole.  Msize=    ; Value = 12
|Maxtmp=                       ; Value = 128

	.globl	_t_atan          
_t_atan:
	link    a6,#-140

| 194	   IF $t_check(error, arg) = none THEN BEGIN
	movl    a6@(12),sp@-
	movl    a6@(16),sp@-
	jsr     _t_check
	addql   #8,sp
	tstl    d0
	jne     _20

| 195	      x := abs(arg);
	movl    a6@(12),d0
	bclr    #31,d0
	movl    d0,a6@(-4)

| 196	      IF x < 1.0 THEN
	cmpl    #0x3F800000,d0
	jge     _21

| 197	         reduced := false
	movb    #0,a6@(-12)
	jra     _22
_21:

| 199	         x := 1.0 / x;
	movl    a6@(-4),sp@-
	movl    #0x3F800000,sp@-
	jsr     _f_div
	addql   #8,sp
	movl    d0,a6@(-4)

| 200	         reduced := true;
	movb    #-1,a6@(-12)
_22:

| 202	      xsq := sqr(x);
	movl    a6@(-4),sp@-
	movl    a6@(-4),sp@-
	jsr     _f_mul
	addql   #8,sp
	movl    d0,a6@(-8)

| 203	      result := x + x*xsq *
	movl    d0,sp@-
	movl    a6@(-4),sp@-
	jsr     _f_mul
	addql   #8,sp
	movl    #0xBC2BFF24,sp@-
	movl    a6@(-8),sp@-
	moveml  #0x0001,a6@(-140)
	jsr     _f_mul
	addql   #8,sp
	movl    d0,d1
	moveml  a6@(-140),#0x0001
	movl    d1,sp@-
	movl    #0xBF2CDDBB,sp@-
	moveml  #0x0001,a6@(-140)
	jsr     _f_add
	addql   #8,sp
	movl    d0,d1
	moveml  a6@(-140),#0x0001
	movl    d1,sp@-
	movl    a6@(-8),sp@-
	moveml  #0x0001,a6@(-140)
	jsr     _f_mul
	addql   #8,sp
	movl    d0,d1
	moveml  a6@(-140),#0x0001
	movl    d1,sp@-
	movl    #0xBF9B537A,sp@-
	moveml  #0x0001,a6@(-140)
	jsr     _f_add
	addql   #8,sp
	movl    d0,d1
	moveml  a6@(-140),#0x0001
	movl    d1,sp@-
	movl    d0,sp@-
	jsr     _f_mul
	addql   #8,sp
	movl    a6@(-8),sp@-
	movl    #0x4086B4EA,sp@-
	moveml  #0x0001,a6@(-140)
	jsr     _f_add
	addql   #8,sp
	movl    d0,d1
	moveml  a6@(-140),#0x0001
	movl    d1,sp@-
	movl    a6@(-8),sp@-
	moveml  #0x0001,a6@(-140)
	jsr     _f_mul
	addql   #8,sp
	movl    d0,d1
	moveml  a6@(-140),#0x0001
	movl    d1,sp@-
	movl    #0x4068FDB5,sp@-
	moveml  #0x0001,a6@(-140)
	jsr     _f_add
	addql   #8,sp
	movl    d0,d1
	moveml  a6@(-140),#0x0001
	movl    d1,sp@-
	movl    d0,sp@-
	jsr     _f_div
	addql   #8,sp
	movl    d0,sp@-
	movl    a6@(-4),sp@-
	jsr     _f_add
	addql   #8,sp
	movl    a6@(8),a0
	movl    d0,a0@

| 206	      IF reduced THEN
	tstb    a6@(-12)
	jeq     _23

| 207	         result := piover2 - result;
	movl    a6@(8),a1
	movl    a1@,sp@-
	movl    #0x3FC90FDA,sp@-
	jsr     _f_sub
	addql   #8,sp
	movl    a6@(8),a0
	movl    d0,a0@
_23:

| 208	      IF arg < 0 THEN
	movl    #0,sp@-
	jsr     _i_2_f
	addql   #4,sp
	tstl    d0
	jpl     .L9
	eorl    #0x7FFFFFFF,d0
.L9:
	movl    a6@(12),d1
	jpl     .La
	eorl    #0x7FFFFFFF,d1
.La:
	cmpl    d1,d0
	jle     _24

| 209	         result := -result;
	movl    a6@(8),a0
	movl    a0@,d0
	tstl    d0
	jeq     .Lb
	bchg    #31,d0
.Lb:
	movl    d0,a0@
_24:
_20:

| 211	END;
	unlk    a6
	rts     
|$E$REDUCE       

|$E_REDUCE       
|Starting peephole.  Msize=    ; Value = 24
|Maxtmp=                       ; Value = 128

	.globl	_e_reduce        
_e_reduce:
	link    a6,#-152
	moveml  #0x0100,sp@-

| 233	   IF $t_check(error, arg) = none THEN 
	movl    a6@(8),sp@-
	movl    a6@(24),sp@-
	jsr     _t_check
	addql   #8,sp
	tstl    d0
	jne     _25

| 234	      IF arg < min_input THEN 
	movl    a6@(8),d0
	jpl     .Ld
	eorl    #0x7FFFFFFF,d0
.Ld:
	movl    #0xC61C4000,d1
	jpl     .Le
	eorl    #0x7FFFFFFF,d1
.Le:
	cmpl    d1,d0
	jge     _26

| 235	         error := underflow
	movl    a6@(24),a0
	movl    #1,a0@
	jra     _27
_26:

| 236	      ELSE IF arg > max_input THEN
	movl    a6@(8),d0
	jpl     .Lf
	eorl    #0x7FFFFFFF,d0
.Lf:
	cmpl    #0x461C4000,d0
	jle     _28

| 237	         error := overflow
	movl    a6@(24),a0
	movl    #2,a0@
	jra     _29
_28:

| 239	         upper := $t_halfsz(arg);
	movl    a6@(8),sp@-
	jsr     _t_halfsz
	addql   #4,sp
	movl    d0,a6@(-4)

| 240	         lower := arg - upper;
	movl    d0,sp@-
	movl    a6@(8),sp@-
	jsr     _f_sub
	addql   #8,sp
	movl    d0,a6@(-8)

| 241	         x := upper * upper_log_2_inv;
	movl    #0x3FB8A000,sp@-
	movl    a6@(-4),sp@-
	jsr     _f_mul
	addql   #8,sp
	movl    d0,a6@(-12)

| 242	         multiple := round(x);
	movl    d0,sp@-
	jsr     _round
	addql   #4,sp
	movl    a6@(20),a0
	movl    d0,a0@

| 243	         x := (x - multiple) +
	movl    a6@(20),a1
	movl    a1@,sp@-
	jsr     _i_2_f
	addql   #4,sp
	movl    d0,sp@-
	movl    a6@(-12),sp@-
	jsr     _f_sub
	addql   #8,sp
	movl    #0x3FB8A000,sp@-
	movl    a6@(-8),sp@-
	moveml  #0x0001,a6@(-152)
	jsr     _f_mul
	addql   #8,sp
	movl    d0,d1
	moveml  a6@(-152),#0x0001
	movl    #0x39A3B295,sp@-
	movl    a6@(8),sp@-
	moveml  #0x0003,a6@(-152)
	jsr     _f_mul
	addql   #8,sp
	movl    d0,d7
	moveml  a6@(-152),#0x0003
	movl    d7,sp@-
	movl    d1,sp@-
	moveml  #0x0001,a6@(-152)
	jsr     _f_add
	addql   #8,sp
	movl    d0,d1
	moveml  a6@(-152),#0x0001
	movl    d1,sp@-
	movl    d0,sp@-
	jsr     _f_add
	addql   #8,sp
	movl    d0,a6@(-12)

| 245	         xsq := sqr(x);
	movl    d0,sp@-
	movl    d0,sp@-
	jsr     _f_mul
	addql   #8,sp
	movl    d0,a6@(-16)

| 246	         p := x * (7.21504804 + 0.05769958151 * xsq);
	movl    d0,sp@-
	movl    #0x3D6C5665,sp@-
	jsr     _f_mul
	addql   #8,sp
	movl    d0,sp@-
	movl    #0x40E6E1AC,sp@-
	jsr     _f_add
	addql   #8,sp
	movl    d0,sp@-
	movl    a6@(-12),sp@-
	jsr     _f_mul
	addql   #8,sp
	movl    a6@(12),a0
	movl    d0,a0@

| 247	         q := 20.81822806+xsq;
	movl    a6@(-16),sp@-
	movl    #0x41A68BBB,sp@-
	jsr     _f_add
	addql   #8,sp
	movl    a6@(16),a1
	movl    d0,a1@
_29:
_27:
_25:

| 249	END;
	moveml  sp@+,#0x0080
	unlk    a6
	rts     
|$T$EXP          

|$T_EXP          
|Starting peephole.  Msize=    ; Value = 12
|Maxtmp=                       ; Value = 0

	.globl	_t_exp           
_t_exp:
	link    a6,#-12

| 260	   $e_reduce(arg, p, q, multiple, error);
	movl    a6@(16),sp@-
	pea     a6@(-4)
	pea     a6@(-12)
	pea     a6@(-8)
	movl    a6@(12),sp@-
	jsr     _e_reduce
	addl    #20,sp

| 261	   IF error = none THEN BEGIN
	movl    a6@(16),a0
	tstl    a0@
	jne     _30

| 262	      result := 0.5 + p/(q-p);
	movl    a6@(-8),sp@-
	movl    a6@(-12),sp@-
	jsr     _f_sub
	addql   #8,sp
	movl    d0,sp@-
	movl    a6@(-8),sp@-
	jsr     _f_div
	addql   #8,sp
	movl    d0,sp@-
	movl    #0x3F000000,sp@-
	jsr     _f_add
	addql   #8,sp
	movl    a6@(8),a1
	movl    d0,a1@

| 263	      $t_pack(result, multiple+1, result, error);
	movl    a6@(8),a0
	movl    a6@(-4),d0
	addql   #1,d0
	movl    a6@(16),sp@-
	movl    a6@(8),sp@-
	movl    d0,sp@-
	movl    a0@,sp@-
	jsr     _t_pack
	addl    #16,sp
_30:

| 265	END;
	unlk    a6
	rts     
|$T$LOG          

|$T_LOG          
|Starting peephole.  Msize=    ; Value = 16
|Maxtmp=                       ; Value = 128

	.globl	_t_log           
_t_log:
	link    a6,#-144
	moveml  #0x0100,sp@-

| 279	   IF $t_check(error, arg) = none THEN
	movl    a6@(12),sp@-
	movl    a6@(16),sp@-
	jsr     _t_check
	addql   #8,sp
	tstl    d0
	jne     _31

| 280	      IF arg <= 0.0 THEN
	cmpl    #0,a6@(12)
	jgt     _32

| 281	         error := domain_error
	movl    a6@(16),a0
	movl    #3,a0@
	jra     _33
_32:

| 283	         $t_unpack(arg, x, n);
	pea     a6@(-16)
	pea     a6@(-4)
	movl    a6@(12),sp@-
	jsr     _t_unpack
	addl    #12,sp

| 284	         IF x > 1.414213 THEN BEGIN
	movl    a6@(-4),d0
	jpl     .Li
	eorl    #0x7FFFFFFF,d0
.Li:
	cmpl    #0x3FB504EE,d0
	jle     _34

| 285	            x := x / 2.0;
	movl    #0x40000000,sp@-
	movl    a6@(-4),sp@-
	jsr     _f_div
	addql   #8,sp
	movl    d0,a6@(-4)

| 286	            n := n + 1;
	addql   #1,a6@(-16)
_34:

| 288	         z := (x-1.0) / (x+1.0);
	movl    #0x3F800000,sp@-
	movl    a6@(-4),sp@-
	jsr     _f_sub
	addql   #8,sp
	movl    #0x3F800000,sp@-
	movl    a6@(-4),sp@-
	moveml  #0x0001,a6@(-144)
	jsr     _f_add
	addql   #8,sp
	movl    d0,d1
	moveml  a6@(-144),#0x0001
	movl    d1,sp@-
	movl    d0,sp@-
	jsr     _f_div
	addql   #8,sp
	movl    d0,a6@(-8)

| 289	         zsq := sqr(z);
	movl    d0,sp@-
	movl    d0,sp@-
	jsr     _f_mul
	addql   #8,sp
	movl    d0,a6@(-12)

| 290	         result := n*log_2 + (z+z) +
	movl    a6@(-16),sp@-
	jsr     _i_2_f
	addql   #4,sp
	movl    #0x3F317217,sp@-
	movl    d0,sp@-
	jsr     _f_mul
	addql   #8,sp
	movl    a6@(-8),sp@-
	movl    a6@(-8),sp@-
	moveml  #0x0001,a6@(-144)
	jsr     _f_add
	addql   #8,sp
	movl    d0,d1
	moveml  a6@(-144),#0x0001
	movl    d1,sp@-
	movl    d0,sp@-
	jsr     _f_add
	addql   #8,sp
	movl    a6@(-12),sp@-
	movl    a6@(-8),sp@-
	moveml  #0x0001,a6@(-144)
	jsr     _f_mul
	addql   #8,sp
	movl    d0,d1
	moveml  a6@(-144),#0x0001
	movl    a6@(-12),sp@-
	movl    #0x3FD4114D,sp@-
	moveml  #0x0003,a6@(-144)
	jsr     _f_sub
	addql   #8,sp
	movl    d0,d7
	moveml  a6@(-144),#0x0003
	movl    d7,sp@-
	movl    #0x3F8D5EEC,sp@-
	moveml  #0x0003,a6@(-144)
	jsr     _f_div
	addql   #8,sp
	movl    d0,d7
	moveml  a6@(-144),#0x0003
	movl    d7,sp@-
	movl    d1,sp@-
	moveml  #0x0001,a6@(-144)
	jsr     _f_mul
	addql   #8,sp
	movl    d0,d1
	moveml  a6@(-144),#0x0001
	movl    d1,sp@-
	movl    d0,sp@-
	jsr     _f_add
	addql   #8,sp
	movl    a6@(8),a0
	movl    d0,a0@
_33:
_31:

| 293	END;
	moveml  sp@+,#0x0080
	unlk    a6
	rts     
|SIN             

|SIN             
|Starting peephole.  Msize=    ; Value = 12
|Maxtmp=                       ; Value = 0

	.globl	_sin              
_sin:
	link    a6,#-12
|
|	GB - patch
	movl	a6@(8),__fperror
|

| 300	   s_sncs(result, arg, error, 0);
	movl    #0,sp@-
	pea     a6@(-8)
	movl    a6@(8),sp@-
	pea     a6@(-12)
	jsr     s_sncs
	addl    #16,sp

| 301	   IF error = none THEN sin := result
	tstl    a6@(-8)
	jne     _35

| 301	
	movl    a6@(-12),a6@(-4)
	jra     _36
_35:

| 302	                   ELSE sin := $t_error('sin   ', error);
	movl    a6@(-8),sp@-
	movl    #SIN,sp@-
	jsr     _t_error
	addql   #8,sp
	movl    d0,a6@(-4)
_36:
	movl    a6@(-4),d0

| 303	END;
	unlk    a6
	rts     
	.data
	.even
	.text
|COS             

|COS             
|Starting peephole.  Msize=    ; Value = 12
|Maxtmp=                       ; Value = 0

	.globl	_cos              
_cos:
	link    a6,#-12
|
|	GB - patch
	movl	a6@(8),__fperror
|

| 311	   s_sncs(result, arg, error, 1);
	movl    #1,sp@-
	pea     a6@(-8)
	movl    a6@(8),sp@-
	pea     a6@(-12)
	jsr     s_sncs
	addl    #16,sp

| 312	   IF error = none THEN cos := result
	tstl    a6@(-8)
	jne     _37

| 312	
	movl    a6@(-12),a6@(-4)
	jra     _38
_37:

| 313	                   ELSE cos := $t_error('cos   ', error);
	movl    a6@(-8),sp@-
	movl    #COS,sp@-
	jsr     _t_error
	addql   #8,sp
	movl    d0,a6@(-4)
_38:
	movl    a6@(-4),d0

| 314	END;
	unlk    a6
	rts     
	.data
	.even
	.text
|ATAN            

|ATAN            
|Starting peephole.  Msize=    ; Value = 12
|Maxtmp=                       ; Value = 0

	.globl	_atan             
_atan:
	link    a6,#-12
|
|	GB - patch
	movl	a6@(8),__fperror
|

| 322	   $t_atan(result, arg, error);
	pea     a6@(-8)
	movl    a6@(8),sp@-
	pea     a6@(-12)
	jsr     _t_atan
	addl    #12,sp

| 323	   IF error = none THEN atan := result
	tstl    a6@(-8)
	jne     _39

| 323	
	movl    a6@(-12),a6@(-4)
	jra     _40
_39:

| 324	                   ELSE atan := $t_error('atan  ', error);
	movl    a6@(-8),sp@-
	movl    #ATAN,sp@-
	jsr     _t_error
	addql   #8,sp
	movl    d0,a6@(-4)
_40:
	movl    a6@(-4),d0

| 325	END;
	unlk    a6
	rts     
	.data
	.even
	.text
|EXP             

|EXP             
|Starting peephole.  Msize=    ; Value = 12
|Maxtmp=                       ; Value = 0

	.globl	_exp              
_exp:
	link    a6,#-12
|
|	GB - patch
	movl	a6@(8),__fperror
|

| 333	   $t_exp(result, arg, error);
	pea     a6@(-8)
	movl    a6@(8),sp@-
	pea     a6@(-12)
	jsr     _t_exp
	addl    #12,sp

| 334	   IF error = none THEN exp := result
	tstl    a6@(-8)
	jne     _41

| 334	
	movl    a6@(-12),a6@(-4)
	jra     _42
_41:

| 335	                   ELSE exp := $t_error('exp   ', error);
	movl    a6@(-8),sp@-
	movl    #EXP,sp@-
	jsr     _t_error
	addql   #8,sp
	movl    d0,a6@(-4)
_42:
	movl    a6@(-4),d0

| 336	END;
	unlk    a6
	rts     
	.data
	.even
	.text
|LOG             

|LOG             
|Starting peephole.  Msize=    ; Value = 12
|Maxtmp=                       ; Value = 0

	.globl	_log              
_log:
	link    a6,#-12
|
|	GB - patch
	movl	a6@(8),__fperror
|

| 344	   $t_log(result, arg, error);
	pea     a6@(-8)
	movl    a6@(8),sp@-
	pea     a6@(-12)
	jsr     _t_log
	addl    #12,sp

| 345	   IF error = none THEN log := result
	tstl    a6@(-8)
	jne     _43

| 345	
	movl    a6@(-12),a6@(-4)
	jra     _44
_43:

| 346	                   ELSE log := $t_error('alog  ', error);
	movl    a6@(-8),sp@-
	movl    #LOG,sp@-
	jsr     _t_error
	addql   #8,sp
	movl    d0,a6@(-4)
_44:
	movl    a6@(-4),d0

| 347	END;
	unlk    a6
	rts     
	.data
	.even
	.text
|SQRT            

|SQRT            
|Starting peephole.  Msize=    ; Value = 12
|Maxtmp=                       ; Value = 0

	.globl	_sqrt             
_sqrt:
	link    a6,#-12
|
|	GB - patch
	movl	a6@(8),__fperror
|

| 354	   $t_sqrt(result, arg, error);
	pea     a6@(-8)
	movl    a6@(8),sp@-
	pea     a6@(-12)
	jsr     _t_sqrt
	addl    #12,sp

| 355	   IF error = none THEN sqrt := result
	tstl    a6@(-8)
	jne     _45

| 355	
	movl    a6@(-12),a6@(-4)
	jra     _46
_45:

| 356	                   ELSE sqrt := $t_error('sqrt  ', error);
	movl    a6@(-8),sp@-
	movl    #SQRT,sp@-
	jsr     _t_error
	addql   #8,sp
	movl    d0,a6@(-4)
_46:
	movl    a6@(-4),d0

| 357	END;
	unlk    a6
	rts     
	.data
	.even
	.text
|   13 Instructions Emitted.
| Init = 50 Read = 7931 Asm = 3040 Write = 5197
