|$S$G$N          
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
TANH = 23
|
	.globl	__fperror

|$S_G_N          
|Starting peephole.  Msize=    ; Value = 4
|Maxtmp=                       ; Value = 0

	.globl	_s_g_n           
_s_g_n:
	link    a6,#-4

|  39	   IF arg < 0 THEN
	movl    #0,sp@-
	jsr     _i_2_f
	addql   #4,sp
	tstl    d0
	jpl     .L2
	eorl    #0x7FFFFFFF,d0
.L2:
	movl    a6@(8),d1
	jpl     .L3
	eorl    #0x7FFFFFFF,d1
.L3:
	cmpl    d1,d0
	jle     _1

|  40	      $s_g_n := -1.0
	movl    #0xBF800000,a6@(-4)
	jra     _2
_1:

|  42	      $s_g_n := +1.0;
	movl    #0x3F800000,a6@(-4)
_2:
	movl    a6@(-4),d0

|  43	END;
	unlk    a6
	rts     
|$S$ASNCS        

|$S_ASNCS        
|Starting peephole.  Msize=    ; Value = 8
|Maxtmp=                       ; Value = 128

	.globl	_s_asncs         
_s_asncs:
	link    a6,#-136

|  56	   IF $t_check(error, arg) = none THEN BEGIN
	movl    a6@(8),sp@-
	movl    a6@(20),sp@-
	jsr     _t_check
	addql   #8,sp
	tstl    d0
	jne     _3

|  57	      x := abs(arg);
	movl    a6@(8),d0
	bclr    #31,d0
	movl    d0,a6@(-4)

|  58	      IF x > 1.0 THEN
	tstl    d0
	jpl     .L5
	eorl    #0x7FFFFFFF,d0
.L5:
	cmpl    #0x3F800000,d0
	jle     _4

|  59	         error := domain_error
	movl    a6@(20),a0
	movl    #3,a0@
	jra     _5
_4:

|  60	      ELSE BEGIN         IF x < 0.7071 THEN
	cmpl    #0x3F350481,a6@(-4)
	jge     _6

|  61	            reduced := false
	movl    a6@(16),a0
	movb    #0,a0@
	jra     _7
_6:

|  63	            $t_sqrt(x, (1.0-x)*(1.0+x), error);
	movl    a6@(-4),sp@-
	movl    #0x3F800000,sp@-
	jsr     _f_sub
	addql   #8,sp
	movl    a6@(-4),sp@-
	movl    #0x3F800000,sp@-
	moveml  #0x0001,a6@(-136)
	jsr     _f_add
	addql   #8,sp
	movl    d0,d1
	moveml  a6@(-136),#0x0001
	movl    d1,sp@-
	movl    d0,sp@-
	jsr     _f_mul
	addql   #8,sp
	movl    a6@(20),sp@-
	movl    d0,sp@-
	pea     a6@(-4)
	jsr     _t_sqrt
	addl    #12,sp

|  64	            reduced := true;
	movl    a6@(16),a0
	movb    #-1,a0@
_7:

|  66	         xsq := sqr(x);
	movl    a6@(-4),sp@-
	movl    a6@(-4),sp@-
	jsr     _f_mul
	addql   #8,sp
	movl    d0,a6@(-8)

|  67	         result := x + x*xsq *
	movl    d0,sp@-
	movl    a6@(-4),sp@-
	jsr     _f_mul
	addql   #8,sp
	movl    #0x3CC32E35,sp@-
	movl    a6@(-8),sp@-
	moveml  #0x0001,a6@(-136)
	jsr     _f_mul
	addql   #8,sp
	movl    d0,d1
	moveml  a6@(-136),#0x0001
	movl    d1,sp@-
	movl    #0xBEC79049,sp@-
	moveml  #0x0001,a6@(-136)
	jsr     _f_add
	addql   #8,sp
	movl    d0,d1
	moveml  a6@(-136),#0x0001
	movl    d1,sp@-
	movl    a6@(-8),sp@-
	moveml  #0x0001,a6@(-136)
	jsr     _f_mul
	addql   #8,sp
	movl    d0,d1
	moveml  a6@(-136),#0x0001
	movl    d1,sp@-
	movl    #0x3EFBD00F,sp@-
	moveml  #0x0001,a6@(-136)
	jsr     _f_add
	addql   #8,sp
	movl    d0,d1
	moveml  a6@(-136),#0x0001
	movl    d1,sp@-
	movl    d0,sp@-
	jsr     _f_mul
	addql   #8,sp
	movl    a6@(-8),sp@-
	movl    #0xC06AA2BE,sp@-
	moveml  #0x0001,a6@(-136)
	jsr     _f_add
	addql   #8,sp
	movl    d0,d1
	moveml  a6@(-136),#0x0001
	movl    d1,sp@-
	movl    a6@(-8),sp@-
	moveml  #0x0001,a6@(-136)
	jsr     _f_mul
	addql   #8,sp
	movl    d0,d1
	moveml  a6@(-136),#0x0001
	movl    d1,sp@-
	movl    #0x403CDBB7,sp@-
	moveml  #0x0001,a6@(-136)
	jsr     _f_add
	addql   #8,sp
	movl    d0,d1
	moveml  a6@(-136),#0x0001
	movl    d1,sp@-
	movl    d0,sp@-
	jsr     _f_div
	addql   #8,sp
	movl    d0,sp@-
	movl    a6@(-4),sp@-
	jsr     _f_add
	addql   #8,sp
	movl    a6@(12),a0
	movl    d0,a0@
_5:
_3:

|  72	END;
	unlk    a6
	rts     
|$S$$F$ASIN      

|$S_$F_ASIN      
|Starting peephole.  Msize=    ; Value = 4
|Maxtmp=                       ; Value = 0

	.globl	_s__f_asin       
_s__f_asin:
	link    a6,#-4

|  82	   $s_asncs(arg, result, reduced, error);
	movl    a6@(16),sp@-
	pea     a6@(-4)
	movl    a6@(8),sp@-
	movl    a6@(12),sp@-
	jsr     _s_asncs
	addl    #16,sp

|  83	   IF error = none THEN BEGIN
	movl    a6@(16),a0
	tstl    a0@
	jne     _8

|  84	      IF reduced THEN
	tstb    a6@(-4)
	jeq     _9

|  85	         result := piover2 - result;
	movl    a6@(8),a1
	movl    a1@,sp@-
	movl    #0x3FC90FDA,sp@-
	jsr     _f_sub
	addql   #8,sp
	movl    a6@(8),a0
	movl    d0,a0@
_9:

|  86	      IF arg < 0 THEN
	movl    #0,sp@-
	jsr     _i_2_f
	addql   #4,sp
	tstl    d0
	jpl     .L7
	eorl    #0x7FFFFFFF,d0
.L7:
	movl    a6@(12),d1
	jpl     .L8
	eorl    #0x7FFFFFFF,d1
.L8:
	cmpl    d1,d0
	jle     _10

|  87	         result := -result;
	movl    a6@(8),a0
	movl    a0@,d0
	tstl    d0
	jeq     .L9
	bchg    #31,d0
.L9:
	movl    d0,a0@
_10:
_8:

|  89	END;
	unlk    a6
	rts     
|$S$ACOS         

|$S_ACOS         
|Starting peephole.  Msize=    ; Value = 4
|Maxtmp=                       ; Value = 0

	.globl	_s_acos          
_s_acos:
	link    a6,#-4

| 103	   $s_asncs(arg, result, reduced, error);
	movl    a6@(16),sp@-
	pea     a6@(-4)
	movl    a6@(8),sp@-
	movl    a6@(12),sp@-
	jsr     _s_asncs
	addl    #16,sp

| 104	   IF error = none THEN BEGIN
	movl    a6@(16),a0
	tstl    a0@
	jne     _11

| 105	      IF arg < 0 THEN
	movl    #0,sp@-
	jsr     _i_2_f
	addql   #4,sp
	tstl    d0
	jpl     .Lb
	eorl    #0x7FFFFFFF,d0
.Lb:
	movl    a6@(12),d1
	jpl     .Lc
	eorl    #0x7FFFFFFF,d1
.Lc:
	cmpl    d1,d0
	jle     _12

| 106	         IF reduced THEN
	tstb    a6@(-4)
	jeq     _13

| 107	            result := pi - result
	movl    a6@(8),a1
	movl    a1@,sp@-
	movl    #0x40490FDA,sp@-
	jsr     _f_sub
	addql   #8,sp
	movl    a6@(8),a0
	movl    d0,a0@
	jra     _14
_13:

| 109	            result := piover2 + result
	movl    a6@(8),a0
	movl    a0@,sp@-
	movl    #0x3FC90FDA,sp@-
	jsr     _f_add
	addql   #8,sp
	movl    a6@(8),a1
	movl    d0,a1@
_14:
	jra     _15
_12:

| 111	         IF NOT reduced THEN
	tstb    a6@(-4)
	jne     _16

| 112	            result := piover2 - result;
	movl    a6@(8),a0
	movl    a0@,sp@-
	movl    #0x3FC90FDA,sp@-
	jsr     _f_sub
	addql   #8,sp
	movl    a6@(8),a1
	movl    d0,a1@
_16:
_15:
_11:

| 116	END;
	unlk    a6
	rts     
|$S$ATAN2        

|$S_ATAN2        
|Starting peephole.  Msize=    ; Value = 0
|Maxtmp=                       ; Value = 128

	.globl	_s_atan2         
_s_atan2:
	link    a6,#-128
	moveml  #0x0100,sp@-

| 126	   IF $t_check(error, y) = none THEN
	movl    a6@(12),sp@-
	movl    a6@(20),sp@-
	jsr     _t_check
	addql   #8,sp
	tstl    d0
	jne     _17

| 127	      IF $t_check(error, x) = none THEN
	movl    a6@(16),sp@-
	movl    a6@(20),sp@-
	jsr     _t_check
	addql   #8,sp
	tstl    d0
	jne     _18

| 128	         IF (x=0) AND (y=0) THEN
	movl    #0,sp@-
	jsr     _i_2_f
	addql   #4,sp
	cmpl    a6@(16),d0
	seq     d0
	movl    #0,sp@-
	moveml  #0x0001,a6@(-128)
	jsr     _i_2_f
	addql   #4,sp
	movl    d0,d1
	moveml  a6@(-128),#0x0001
	cmpl    a6@(12),d1
	seq     d1
	andb    d1,d0
	jeq     _19

| 129	            error := domain_error
	movl    a6@(20),a0
	movl    #3,a0@
	jra     _20
_19:

| 131	            IF (x=0) OR (($t_logb(y)-$t_logb(x)) > 26.0) THEN
	movl    #0,sp@-
	jsr     _i_2_f
	addql   #4,sp
	cmpl    a6@(16),d0
	seq     d0
	movl    a6@(12),sp@-
	moveml  #0x0001,a6@(-128)
	jsr     _t_logb
	addql   #4,sp
	movl    d0,d1
	moveml  a6@(-128),#0x0001
	movl    a6@(16),sp@-
	moveml  #0x0003,a6@(-128)
	jsr     _t_logb
	addql   #4,sp
	movl    d0,d7
	moveml  a6@(-128),#0x0003
	subl    d7,d1
	movl    d1,sp@-
	moveml  #0x0001,a6@(-128)
	jsr     _i_2_f
	addql   #4,sp
	movl    d0,d1
	moveml  a6@(-128),#0x0001
	cmpl    #0x41D00000,d1
	sgt     d1
	orb     d1,d0
	jeq     _21

| 132	               result := $s_g_n(y) * piover2
	movl    a6@(12),sp@-
	jsr     _s_g_n
	addql   #4,sp
	movl    #0x3FC90FDA,sp@-
	movl    d0,sp@-
	jsr     _f_mul
	addql   #8,sp
	movl    a6@(8),a0
	movl    d0,a0@
	jra     _22
_21:

| 134	               $t_atan(result, y/x, error);
	movl    a6@(16),sp@-
	movl    a6@(12),sp@-
	jsr     _f_div
	addql   #8,sp
	movl    a6@(20),sp@-
	movl    d0,sp@-
	movl    a6@(8),sp@-
	jsr     _t_atan
	addl    #12,sp

| 135	               IF x < 0 THEN
	movl    #0,sp@-
	jsr     _i_2_f
	addql   #4,sp
	tstl    d0
	jpl     .Le
	eorl    #0x7FFFFFFF,d0
.Le:
	movl    a6@(16),d1
	jpl     .Lf
	eorl    #0x7FFFFFFF,d1
.Lf:
	cmpl    d1,d0
	jle     _23

| 136	                  result := $s_g_n(y) * pi + result;
	movl    a6@(12),sp@-
	jsr     _s_g_n
	addql   #4,sp
	movl    #0x40490FDA,sp@-
	movl    d0,sp@-
	jsr     _f_mul
	addql   #8,sp
	movl    a6@(8),a0
	movl    a0@,sp@-
	movl    d0,sp@-
	jsr     _f_add
	addql   #8,sp
	movl    a6@(8),a1
	movl    d0,a1@
_23:
_22:
_20:
_18:
_17:

| 138	END;
	moveml  sp@+,#0x0080
	unlk    a6
	rts     
|$S$TANH         

|$S_TANH         
|Starting peephole.  Msize=    ; Value = 24
|Maxtmp=                       ; Value = 128

	.globl	_s_tanh          
_s_tanh:
	link    a6,#-152

| 149	   IF $t_check(error, arg) = none THEN BEGIN
	movl    a6@(12),sp@-
	movl    a6@(16),sp@-
	jsr     _t_check
	addql   #8,sp
	tstl    d0
	jne     _24

| 150	      x := abs(arg);
	movl    a6@(12),d0
	bclr    #31,d0
	movl    d0,a6@(-8)

| 151	      IF x < 0.75 THEN BEGIN
	cmpl    #0x3F400000,d0
	jge     _25

| 152	         xsq := sqr(x);
	movl    d0,sp@-
	movl    d0,sp@-
	jsr     _f_mul
	addql   #8,sp
	movl    d0,a6@(-12)

| 153	         result := arg + arg*xsq *
	movl    d0,sp@-
	movl    a6@(12),sp@-
	jsr     _f_mul
	addql   #8,sp
	movl    #0xBB763D73,sp@-
	movl    a6@(-12),sp@-
	moveml  #0x0001,a6@(-152)
	jsr     _f_mul
	addql   #8,sp
	movl    d0,d1
	moveml  a6@(-152),#0x0001
	movl    d1,sp@-
	movl    #0xBF52F16B,sp@-
	moveml  #0x0001,a6@(-152)
	jsr     _f_add
	addql   #8,sp
	movl    d0,d1
	moveml  a6@(-152),#0x0001
	movl    d1,sp@-
	movl    d0,sp@-
	jsr     _f_mul
	addql   #8,sp
	movl    a6@(-12),sp@-
	movl    #0x401E352F,sp@-
	moveml  #0x0001,a6@(-152)
	jsr     _f_add
	addql   #8,sp
	movl    d0,d1
	moveml  a6@(-152),#0x0001
	movl    d1,sp@-
	movl    d0,sp@-
	jsr     _f_div
	addql   #8,sp
	movl    d0,sp@-
	movl    a6@(12),sp@-
	jsr     _f_add
	addql   #8,sp
	movl    a6@(8),a0
	movl    d0,a0@
	jra     _26
_25:

| 158	         IF x > 9.011 THEN
	movl    a6@(-8),d0
	jpl     .Lh
	eorl    #0x7FFFFFFF,d0
.Lh:
	cmpl    #0x41102D0E,d0
	jle     _27

| 159	            result := 1.0
	movl    a6@(8),a0
	movl    #0x3F800000,a0@
	jra     _28
_27:

| 161	            x := x * 2.88539008;       { 2/ln(2) }
	movl    #0x4038AA3B,sp@-
	movl    a6@(-8),sp@-
	jsr     _f_mul
	addql   #8,sp
	movl    d0,a6@(-8)

| 162	            multiple := round(x);
	movl    d0,sp@-
	jsr     _round
	addql   #4,sp
	movl    d0,a6@(-4)

| 163	            x := x - multiple;
	movl    d0,sp@-
	jsr     _i_2_f
	addql   #4,sp
	movl    d0,sp@-
	movl    a6@(-8),sp@-
	jsr     _f_sub
	addql   #8,sp
	movl    d0,a6@(-8)

| 164	            xsq := sqr(x);
	movl    d0,sp@-
	movl    d0,sp@-
	jsr     _f_mul
	addql   #8,sp
	movl    d0,a6@(-12)

| 165	            p := x * (7.21504804 + xsq * 0.0576995815);
	movl    #0x3D6C5665,sp@-
	movl    d0,sp@-
	jsr     _f_mul
	addql   #8,sp
	movl    d0,sp@-
	movl    #0x40E6E1AC,sp@-
	jsr     _f_add
	addql   #8,sp
	movl    d0,sp@-
	movl    a6@(-8),sp@-
	jsr     _f_mul
	addql   #8,sp
	movl    d0,a6@(-16)

| 166	            q := 20.8182281 + xsq;
	movl    a6@(-12),sp@-
	movl    #0x41A68BBB,sp@-
	jsr     _f_add
	addql   #8,sp
	movl    d0,a6@(-20)

| 167	            $t_pack(q+p, multiple, temp, error);
	movl    a6@(-16),sp@-
	movl    d0,sp@-
	jsr     _f_add
	addql   #8,sp
	movl    a6@(16),sp@-
	pea     a6@(-24)
	movl    a6@(-4),sp@-
	movl    d0,sp@-
	jsr     _t_pack
	addl    #16,sp

| 168	            result := 1.0 - 2.0*(q-p) / (temp+(q-p))
	movl    a6@(-16),sp@-
	movl    a6@(-20),sp@-
	jsr     _f_sub
	addql   #8,sp
	movl    d0,sp@-
	movl    #0x40000000,sp@-
	jsr     _f_mul
	addql   #8,sp
	movl    a6@(-16),sp@-
	movl    a6@(-20),sp@-
	moveml  #0x0001,a6@(-152)
	jsr     _f_sub
	addql   #8,sp
	movl    d0,d1
	moveml  a6@(-152),#0x0001
	movl    d1,sp@-
	movl    a6@(-24),sp@-
	moveml  #0x0001,a6@(-152)
	jsr     _f_add
	addql   #8,sp
	movl    d0,d1
	moveml  a6@(-152),#0x0001
	movl    d1,sp@-
	movl    d0,sp@-
	jsr     _f_div
	addql   #8,sp
	movl    d0,sp@-
	movl    #0x3F800000,sp@-
	jsr     _f_sub
	addql   #8,sp
	movl    a6@(8),a0
	movl    d0,a0@
_28:

| 170	         IF arg < 0.0 THEN
	cmpl    #0,a6@(12)
	jge     _29

| 171	            result := -result;
	movl    a6@(8),a0
	movl    a0@,d0
	tstl    d0
	jeq     .Li
	bchg    #31,d0
.Li:
	movl    d0,a0@
_29:
_26:
_24:

| 174	END;
	unlk    a6
	rts     
|$S$SNCSH        

|$S_SNCSH        
|Starting peephole.  Msize=    ; Value = 20
|Maxtmp=                       ; Value = 128

	.globl	_s_sncsh         
_s_sncsh:
	link    a6,#-148

| 188	   IF $t_check(error, arg) = none THEN BEGIN
	movl    a6@(12),sp@-
	movl    a6@(16),sp@-
	jsr     _t_check
	addql   #8,sp
	tstl    d0
	jne     _30

| 189	      x := abs(arg);
	movl    a6@(12),d0
	bclr    #31,d0
	movl    d0,a6@(-8)

| 190	      IF x < 0.75 THEN BEGIN
	cmpl    #0x3F400000,d0
	jge     _31

| 191	         xsq := sqr(x);
	movl    d0,sp@-
	movl    d0,sp@-
	jsr     _f_mul
	addql   #8,sp
	movl    d0,a6@(-12)

| 192	         IF sinh_cosh_flag = 0 THEN
	tstl    a6@(20)
	jne     _32

| 193	            result := x + x * xsq * (0.166666786
	movl    d0,sp@-
	movl    a6@(-8),sp@-
	jsr     _f_mul
	addql   #8,sp
	movl    #0x39534C5A,sp@-
	movl    a6@(-12),sp@-
	moveml  #0x0001,a6@(-148)
	jsr     _f_mul
	addql   #8,sp
	movl    d0,d1
	moveml  a6@(-148),#0x0001
	movl    d1,sp@-
	movl    #0x3C088402,sp@-
	moveml  #0x0001,a6@(-148)
	jsr     _f_add
	addql   #8,sp
	movl    d0,d1
	moveml  a6@(-148),#0x0001
	movl    d1,sp@-
	movl    a6@(-12),sp@-
	moveml  #0x0001,a6@(-148)
	jsr     _f_mul
	addql   #8,sp
	movl    d0,d1
	moveml  a6@(-148),#0x0001
	movl    d1,sp@-
	movl    #0x3E2AAAB2,sp@-
	moveml  #0x0001,a6@(-148)
	jsr     _f_add
	addql   #8,sp
	movl    d0,d1
	moveml  a6@(-148),#0x0001
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
	jra     _33
_32:

| 197	            result := 1.0 + xsq * (0.500001028
	movl    #0x3AB9AA6A,sp@-
	movl    a6@(-12),sp@-
	jsr     _f_mul
	addql   #8,sp
	movl    d0,sp@-
	movl    #0x3D2AA0B7,sp@-
	jsr     _f_add
	addql   #8,sp
	movl    d0,sp@-
	movl    a6@(-12),sp@-
	jsr     _f_mul
	addql   #8,sp
	movl    d0,sp@-
	movl    #0x3F000011,sp@-
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
	movl    a6@(8),a0
	movl    d0,a0@
_33:
	jra     _34
_31:

| 201	         $e_reduce(x, p, q, multiple, error);
	movl    a6@(16),sp@-
	pea     a6@(-4)
	pea     a6@(-20)
	pea     a6@(-16)
	movl    a6@(-8),sp@-
	jsr     _e_reduce
	addl    #20,sp

| 202	         IF error = none THEN BEGIN
	movl    a6@(16),a0
	tstl    a0@
	jne     _35

| 203	            x := p / (q-p);
	movl    a6@(-16),sp@-
	movl    a6@(-20),sp@-
	jsr     _f_sub
	addql   #8,sp
	movl    d0,sp@-
	movl    a6@(-16),sp@-
	jsr     _f_div
	addql   #8,sp
	movl    d0,a6@(-8)

| 204	            IF multiple < 14 THEN BEGIN
	cmpl    #14,a6@(-4)
	jge     _36

| 205	               $t_pack(0.5/(x+0.5), -2*multiple-1, xsq, error);
	movl    #0x3F000000,sp@-
	movl    d0,sp@-
	jsr     _f_add
	addql   #8,sp
	movl    d0,sp@-
	movl    #0x3F000000,sp@-
	jsr     _f_div
	addql   #8,sp
	movl    a6@(-4),sp@-
	movl    #2,sp@-
	moveml  #0x0001,a6@(-148)
	jsr     lmul
	addql   #8,sp
	movl    d0,d1
	moveml  a6@(-148),#0x0001
	negl    d1
	subql   #1,d1
	movl    a6@(16),sp@-
	pea     a6@(-12)
	movl    d1,sp@-
	movl    d0,sp@-
	jsr     _t_pack
	addl    #16,sp

| 206	               IF sinh_cosh_flag = 0 THEN
	tstl    a6@(20)
	jne     _37

| 207	                  x := x-xsq
	movl    a6@(-12),sp@-
	movl    a6@(-8),sp@-
	jsr     _f_sub
	addql   #8,sp
	movl    d0,a6@(-8)
	jra     _38
_37:

| 208	               ELSE                         x := x+xsq;
	movl    a6@(-12),sp@-
	movl    a6@(-8),sp@-
	jsr     _f_add
	addql   #8,sp
	movl    d0,a6@(-8)
_38:
_36:

| 210	            $t_pack(0.5+x, multiple, result, error);
	movl    a6@(-8),sp@-
	movl    #0x3F000000,sp@-
	jsr     _f_add
	addql   #8,sp
	movl    a6@(16),sp@-
	movl    a6@(8),sp@-
	movl    a6@(-4),sp@-
	movl    d0,sp@-
	jsr     _t_pack
	addl    #16,sp
_35:
_34:

| 213	      IF (arg<0) AND (error=none) AND (sinh_cosh_flag=0) THEN
	movl    #0,sp@-
	jsr     _i_2_f
	addql   #4,sp
	tstl    d0
	jpl     .Lk
	eorl    #0x7FFFFFFF,d0
.Lk:
	movl    a6@(12),d1
	jpl     .Ll
	eorl    #0x7FFFFFFF,d1
.Ll:
	cmpl    d1,d0
	sgt     d0
	movl    a6@(16),a0
	tstl    a0@
	seq     d1
	andb    d1,d0
	tstl    a6@(20)
	seq     d1
	andb    d1,d0
	jeq     _39

| 214	         result := -result;
	movl    a6@(8),a1
	movl    a1@,d0
	tstl    d0
	jeq     .Lm
	bchg    #31,d0
.Lm:
	movl    d0,a1@
_39:
_30:

| 216	END;
	unlk    a6
	rts     
|$S$UP$S         

|$S_UP_S         
|Starting peephole.  Msize=    ; Value = 4
|Maxtmp=                       ; Value = 0

	.globl	_s_up_s          
_s_up_s:
	link    a6,#-4

| 224	   IF $t_check(error, base) = none THEN
	movl    a6@(12),sp@-
	movl    a6@(20),sp@-
	jsr     _t_check
	addql   #8,sp
	tstl    d0
	jne     _40

| 225	   IF $t_check(error, pow) = none THEN BEGIN
	movl    a6@(16),sp@-
	movl    a6@(20),sp@-
	jsr     _t_check
	addql   #8,sp
	tstl    d0
	jne     _41

| 226	      IF base=0 THEN BEGIN
	movl    #0,sp@-
	jsr     _i_2_f
	addql   #4,sp
	cmpl    a6@(12),d0
	jne     _42

| 227	         IF pow>0 THEN result:=0.0
	movl    #0,sp@-
	jsr     _i_2_f
	addql   #4,sp
	tstl    d0
	jpl     .Lo
	eorl    #0x7FFFFFFF,d0
.Lo:
	movl    a6@(16),d1
	jpl     .Lp
	eorl    #0x7FFFFFFF,d1
.Lp:
	cmpl    d1,d0
	jge     _43

| 227	
	movl    a6@(8),a0
	movl    #0,a0@
	jra     _44
_43:

| 228	                  ELSE error:=overflow;
|
|	GB - this is really a domain error.
|
	movl    a6@(20),a0
	movl    #3,a0@
_44:
	jra     _45
_42:

| 230	         $t_log(base_log, base, error);
	movl    a6@(20),sp@-
	movl    a6@(12),sp@-
	pea     a6@(-4)
	jsr     _t_log
	addl    #12,sp

| 231	         IF error = none THEN $t_exp(result, base_log * pow, error);
	movl    a6@(20),a0
	tstl    a0@
	jne     _46

| 231	
	movl    a6@(16),sp@-
	movl    a6@(-4),sp@-
	jsr     _f_mul
	addql   #8,sp
	movl    a6@(20),sp@-
	movl    d0,sp@-
	movl    a6@(8),sp@-
	jsr     _t_exp
	addl    #12,sp
_46:
_45:
_41:
_40:

| 234	END;
	unlk    a6
	rts     
|$S$UP$I         

|$S_UP_I         
|Starting peephole.  Msize=    ; Value = 8
|Maxtmp=                       ; Value = 0

	.globl	_s_up_i          
_s_up_i:
	link    a6,#-8

| 244	   IF $t_check(error, base) = none THEN
	movl    a6@(12),sp@-
	movl    a6@(20),sp@-
	jsr     _t_check
	addql   #8,sp
	tstl    d0
	jne     _47

| 245	   IF (-1<=pow) AND (pow<=2) THEN BEGIN
	cmpl    #-1,a6@(16)
	sge     d0
	cmpl    #2,a6@(16)
	sle     d1
	andb    d1,d0
	jeq     _48

| 246	      CASE pow OF
	movl    a6@(16),a6@(-8)
	jra     _50
_51:

| 247	        -1:  result := 1.0/base;
	movl    a6@(12),sp@-
	movl    #0x3F800000,sp@-
	jsr     _f_div
	addql   #8,sp
	movl    a6@(8),a0
	movl    d0,a0@
	jra     _49
_52:

| 248	         0:  IF base=0 THEN error:=domain_error
	movl    #0,sp@-
	jsr     _i_2_f
	addql   #4,sp
	cmpl    a6@(12),d0
	jne     _53

| 248	
	movl    a6@(20),a0
	movl    #3,a0@
	jra     _54
_53:

| 249	                       ELSE result:=1;
	movl    #1,sp@-
	jsr     _i_2_f
	addql   #4,sp
	movl    a6@(8),a0
	movl    d0,a0@
_54:
	jra     _49
_55:

| 250	         1:  result:=base;
	movl    a6@(8),a0
	movl    a6@(12),a0@
	jra     _49
_56:

| 251	         2:  result:=base*base
	movl    a6@(12),sp@-
	movl    a6@(12),sp@-
	jsr     _f_mul
	addql   #8,sp
	movl    a6@(8),a0
	movl    d0,a0@
	jra     _49
_57:
	movl    #246,sp@-
	movl    #1,sp@-
	jsr     _caseerr
	addql   #8,sp
	jra     _49
_58:
	.long	_51
	.long   _52
	.long   _55
	.long   _56
_50:
	cmpl    #-1,a6@(-8)
	jlt     _57
	cmpl    #2,a6@(-8)
	jgt     _57
	movl    a6@(-8),d0
	subl    #-1,d0
|
|	GB - 10/16/85 - the branch table is not as20 compatible.  Lets
|	change the format so that it is....
|
	asll    #2,d0
	movl    #_58,a0
	movl	a0@(0,d0:L),a0
	jmp		a0@
_49:
	jra     _59
_48:

| 254	      temp:=pow;
	movl    a6@(16),sp@-
	jsr     _i_2_f
	addql   #4,sp
	movl    d0,a6@(-4)

| 255	      $s_up_s(result, abs(base), temp, error);
	movl    a6@(12),d0
	bclr    #31,d0
	movl    a6@(20),sp@-
	movl    a6@(-4),sp@-
	movl    d0,sp@-
	movl    a6@(8),sp@-
	jsr     _s_up_s
	addl    #16,sp

| 256	      IF (base<0) and (odd(abs(pow))) THEN result:=-result;
	movl    #0,sp@-
	jsr     _i_2_f
	addql   #4,sp
	tstl    d0
	jpl     .Lr
	eorl    #0x7FFFFFFF,d0
.Lr:
	movl    a6@(12),d1
	jpl     .Ls
	eorl    #0x7FFFFFFF,d1
.Ls:
	cmpl    d1,d0
	sgt     d0
	movl    a6@(16),d1
	tstl    d1
	jge     .Lt
	negl    d1
.Lt:
	andl    #1,d1
	tstl    d1
	sne     d1
	andb    d1,d0
	jeq     _60

| 256	
	movl    a6@(8),a0
	movl    a0@,d0
	tstl    d0
	jeq     .Lu
	bchg    #31,d0
.Lu:
	movl    d0,a0@
_60:
_59:
_47:

| 258	END;
	unlk    a6
	rts     
|ASIN            

|ASIN            
|Starting peephole.  Msize=    ; Value = 12
|Maxtmp=                       ; Value = 0

	.globl	_asin             
_asin:
	link    a6,#-12
|
|	GB - patch
	movl	a6@(8),__fperror
|
| 265	   $s_$f_asin(result, arg, error);
	pea     a6@(-8)
	movl    a6@(8),sp@-
	pea     a6@(-12)
	jsr     _s__f_asin
	addl    #12,sp

| 266	   IF error = none THEN asin := result
	tstl    a6@(-8)
	jne     _61

| 266	
	movl    a6@(-12),a6@(-4)
	jra     _62
_61:

| 267	                   ELSE asin := $t_error('asin  ', error);
	movl    a6@(-8),sp@-
	movl    #ASIN,sp@-
	jsr     _t_error
	addql   #8,sp
	movl    d0,a6@(-4)
_62:
	movl    a6@(-4),d0

| 268	END;
	unlk    a6
	rts     
	.data
	.even
	.text
|ACOS            

|ACOS            
|Starting peephole.  Msize=    ; Value = 12
|Maxtmp=                       ; Value = 0

	.globl	_acos             
_acos:
	link    a6,#-12
|
|	GB - patch
	movl	a6@(8),__fperror
|

| 276	   $s_acos(result, arg, error);
	pea     a6@(-8)
	movl    a6@(8),sp@-
	pea     a6@(-12)
	jsr     _s_acos
	addl    #12,sp

| 277	   IF error = none THEN acos := result
	tstl    a6@(-8)
	jne     _63

| 277	
	movl    a6@(-12),a6@(-4)
	jra     _64
_63:

| 278	                   ELSE acos := $t_error('acos  ', error);
	movl    a6@(-8),sp@-
	movl    #ACOS,sp@-
	jsr     _t_error
	addql   #8,sp
	movl    d0,a6@(-4)
_64:
	movl    a6@(-4),d0

| 279	END;
	unlk    a6
	rts     
	.data
	.even
	.text
|ATAN2           

|ATAN2           
|Starting peephole.  Msize=    ; Value = 12
|Maxtmp=                       ; Value = 0

	.globl	_atan2            
_atan2:
	link    a6,#-12
|
|	GB - patch
	movl	a6@(8),__fperror
|

| 287	   $s_atan2(result, y, x, error);
	pea     a6@(-8)
	movl    a6@(12),sp@-
	movl    a6@(8),sp@-
	pea     a6@(-12)
	jsr     _s_atan2
	addl    #16,sp

| 288	   IF error = none THEN atan2 := result
	tstl    a6@(-8)
	jne     _65

| 288	
	movl    a6@(-12),a6@(-4)
	jra     _66
_65:

| 289	                   ELSE atan2 := $t_error('atan2 ', error);
	movl    a6@(-8),sp@-
	movl    #ATAN2,sp@-
	jsr     _t_error
	addql   #8,sp
	movl    d0,a6@(-4)
_66:
	movl    a6@(-4),d0

| 290	END;
	unlk    a6
	rts     
	.data
	.even
	.text
|SINH            

|SINH            
|Starting peephole.  Msize=    ; Value = 12
|Maxtmp=                       ; Value = 0

	.globl	_sinh             
_sinh:
	link    a6,#-12
|
|	GB - patch
	movl	a6@(8),__fperror
|

| 297	   $s_sncsh(result, arg, error, 0);
	movl    #0,sp@-
	pea     a6@(-8)
	movl    a6@(8),sp@-
	pea     a6@(-12)
	jsr     _s_sncsh
	addl    #16,sp

| 298	   IF error = none THEN sinh := result
	tstl    a6@(-8)
	jne     _67

| 298	
	movl    a6@(-12),a6@(-4)
	jra     _68
_67:

| 299	                   ELSE sinh := $t_error('sinh  ', error);
	movl    a6@(-8),sp@-
	movl    #SINH,sp@-
	jsr     _t_error
	addql   #8,sp
	movl    d0,a6@(-4)
_68:
	movl    a6@(-4),d0

| 300	END;
	unlk    a6
	rts     
	.data
	.even
	.text
|COSH            

|COSH            
|Starting peephole.  Msize=    ; Value = 12
|Maxtmp=                       ; Value = 0

	.globl	_cosh             
_cosh:
	link    a6,#-12
|
|	GB - patch
	movl	a6@(8),__fperror
|

| 308	   $s_sncsh(result, arg, error, 1);
	movl    #1,sp@-
	pea     a6@(-8)
	movl    a6@(8),sp@-
	pea     a6@(-12)
	jsr     _s_sncsh
	addl    #16,sp

| 309	   IF error = none THEN cosh := result
	tstl    a6@(-8)
	jne     _69

| 309	
	movl    a6@(-12),a6@(-4)
	jra     _70
_69:

| 310	                   ELSE cosh := $t_error('cosh  ', error);
	movl    a6@(-8),sp@-
	movl    #COSH,sp@-
	jsr     _t_error
	addql   #8,sp
	movl    d0,a6@(-4)
_70:
	movl    a6@(-4),d0

| 311	END;
	unlk    a6
	rts     
	.data
	.even
	.text
|TANH            

|TANH            
|Starting peephole.  Msize=    ; Value = 12
|Maxtmp=                       ; Value = 0

	.globl	_tanh             
_tanh:
	link    a6,#-12

|
|	GB - patch
	movl	a6@(8),__fperror
|
| 319	   $s_tanh(result, arg, error);
	pea     a6@(-8)
	movl    a6@(8),sp@-
	pea     a6@(-12)
	jsr     _s_tanh
	addl    #12,sp

| 320	   IF error = none THEN tanh := result
	tstl    a6@(-8)
	jne     _71

| 320	
	movl    a6@(-12),a6@(-4)
	jra     _72
_71:

| 321	                   ELSE tanh := $t_error('tanh  ', error);
	movl    a6@(-8),sp@-
	movl    #TANH,sp@-
	jsr     _t_error
	addql   #8,sp
	movl    d0,a6@(-4)
_72:
	movl    a6@(-4),d0

| 322	END;
	unlk    a6
	rts     
	.data
	.even
	.text
|$UP$I           

|$UP_I           
|Starting peephole.  Msize=    ; Value = 12
|Maxtmp=                       ; Value = 0

	.globl	_up_i            
_up_i:
	link    a6,#-12
|
|	GB - patch
	movl	a6@(12),__fperror
|

| 339	   $s_up_i(result, base,pow, error);
	pea     a6@(-8)
	movl    a6@(12),sp@-
	movl    a6@(8),sp@-
	pea     a6@(-12)
	jsr     _s_up_i
	addl    #16,sp

| 340	   IF error = none THEN $up_i := result
	tstl    a6@(-8)
	jne     _75

| 340	
	movl    a6@(-12),a6@(-4)
	jra     _76
_75:

| 341	                   ELSE $up_i := $t_error('r**r  ', error);
	movl    a6@(-8),sp@-
	movl    #UP_I,sp@-
	jsr     _t_error
	addql   #8,sp
	movl    d0,a6@(-4)
_76:
	movl    a6@(-4),d0

| 342	END;
	unlk    a6
	rts     
	.data
	.even
	.text
|   17 Instructions Emitted.
| Init = 33 Read = 8048 Asm = 2799 Write = 4750
