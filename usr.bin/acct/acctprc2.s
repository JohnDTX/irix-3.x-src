.data
.comm	_pb,24
.comm	_tb,52
.comm	_ub,16000
.lcomm	_usize,4
.text
LL0:.align	1
.globl	_main
.data	1
L28:.ascii	"%hu\11%s\11%lu\11%lu\11%u\0"
.text
.set	L23,0x0
.data
.text
_main:.word	L23
jbr	L29
L2000001:pushal	_pb
calls	$1,_enter
L29:pushal	_pb+20
pushal	_pb+16
pushal	_pb+12
pushal	_pb+2
pushal	_pb
pushal	L28
calls	$6,_scanf
incl	r0
jneq	L2000001
calls	$0,_squeeze
pushal	_ucmp
pushl	$32
pushl	_usize
pushal	_ub
calls	$4,_qsort
calls	$0,_output
ret
.align	1
.globl	_enter
.data	1
L53:.ascii	"acctprc2\72 INCREASE USIZE\12\0"
.text
.data
.align	2
L57:.double	0d3.60000000000000000000e+03
.text
.data
.align	2
L58:.double	0d3.60000000000000000000e+03
.text
.data
.align	2
L60:.double	0d3.60000000000000000000e+03
.text
.data
.align	2
L61:.double	0d3.60000000000000000000e+03
.text
.set	L35,0xc00
.data
.text
_enter:.word	L35
subl2	$20,sp
movl	4(ap),r11
addl3	$2,r11,-(sp)
calls	$1,_strlen
addl3	$1,r0,r10
jbr	L42
L2000003:clrb	2(r11)[r10]
incl	r10
L42:cmpl	r10,$8
jlssu	L2000003
clrl	-4(fp)
movl	-4(fp),r10
jbr	L45
L2000005:mull3	$7,r10,r0
addl3	$2,r11,r1
movl	-4(fp),r2
cvtbl	(r1)[r2],r1
addl3	r1,r0,r10
incl	-4(fp)
L45:addl3	$2,r11,r0
movl	-4(fp),r1
tstb	(r0)[r1]
jneq	L2000005
mull3	$7,r10,r0
movzwl	(r11),r1
addl3	r1,r0,r10
clrl	-4(fp)
pushl	$500
pushl	r10
L2000007:calls	$2,urem
movl	r0,r10
ashl	$5,r10,r0
tstb	_ub+2[r0]
jneq	L2000009
L47:cmpl	-4(fp),$500
jlss	L51
pushal	L53
pushal	__iob+32
calls	$2,_fprintf
pushl	$1
calls	$1,_exit
L51:ashl	$5,r10,r0
tstb	_ub+2[r0]
jneq	L55
ashl	$5,r10,r0
movw	(r11),_ub(r0)
pushl	$8
addl3	$2,r11,-(sp)
ashl	$5,r10,r0
pushab	_ub+2[r0]
calls	$3,_strncpy
L55:cvtld	12(r11),r0
divd2	L57,r0
ashl	$5,r10,r2
cvtfd	_ub+12(r2),r4
addd2	r0,r4
cvtdf	r4,_ub+12(r2)
cvtld	16(r11),r0
divd2	L58,r0
ashl	$5,r10,r2
cvtfd	_ub+16(r2),r4
addd2	r0,r4
cvtdf	r4,_ub+16(r2)
pushl	_pb+20
calls	$1,_kcore
cvtld	r0,-20(fp)
movd	-20(fp),-12(fp)
cvtld	12(r11),r0
muld3	r0,-12(fp),r2
divd2	L60,r2
ashl	$5,r10,r0
cvtfd	_ub+20(r0),r4
addd2	r2,r4
cvtdf	r4,_ub+20(r0)
cvtld	16(r11),r0
muld3	r0,-12(fp),r2
divd2	L61,r2
ashl	$5,r10,r0
cvtfd	_ub+24(r0),r4
addd2	r2,r4
cvtdf	r4,_ub+24(r0)
ashl	$5,r10,r0
incl	_ub+28(r0)
ret
L2000009:movl	-4(fp),r0
incl	-4(fp)
cmpl	r0,$500
jgeq	L47
ashl	$5,r10,r0
cmpw	(r11),_ub(r0)
jneq	L46
pushl	$8
ashl	$5,r10,r0
pushab	_ub+2[r0]
addl3	$2,r11,-(sp)
calls	$3,_strncmp
tstl	r0
jeql	L47
L46:pushl	$500
addl3	$1,r10,-(sp)
jbr	L2000007
.align	1
.globl	_squeeze
.set	L62,0xc00
.data
.text
_squeeze:.word	L62
clrl	r10
movl	r10,r11
L2000011:ashl	$5,r11,r0
tstb	_ub+2[r0]
jeql	L66
ashl	$5,r11,r0
ashl	$5,r10,r1
movw	_ub(r0),_ub(r1)
pushl	$8
ashl	$5,r11,r0
pushab	_ub+2[r0]
ashl	$5,r10,r0
pushab	_ub+2[r0]
calls	$3,_strncpy
ashl	$5,r11,r0
ashl	$5,r10,r1
movf	_ub+12(r0),_ub+12(r1)
ashl	$5,r11,r0
ashl	$5,r10,r1
movf	_ub+16(r0),_ub+16(r1)
ashl	$5,r11,r0
ashl	$5,r10,r1
movf	_ub+20(r0),_ub+20(r1)
ashl	$5,r11,r0
ashl	$5,r10,r1
movf	_ub+24(r0),_ub+24(r1)
ashl	$5,r11,r0
ashl	$5,r10,r1
movl	_ub+28(r0),_ub+28(r1)
incl	r10
L66:incl	r11
cmpl	r11,$500
jlss	L2000011
movl	r10,_usize
ret
.align	1
.globl	_ucmp
.set	L70,0xc00
.data
.text
_ucmp:.word	L70
movl	4(ap),r11
movl	8(ap),r10
cmpw	(r11),(r10)
jeql	L74
movzwl	(r11),r0
movzwl	(r10),r1
subl2	r1,r0
ret
L74:pushl	$8
addl3	$2,r10,-(sp)
addl3	$2,r11,-(sp)
calls	$3,_strncmp
ret
.align	1
.globl	_output
.set	L75,0x800
.data
.text
_output:.word	L75
clrl	r11
jbr	L81
L2000013:ashl	$5,r11,r0
movw	_ub(r0),_tb
pushl	$8
ashl	$5,r11,r0
pushab	_ub+2[r0]
pushal	_tb+2
calls	$3,_strncpy
ashl	$5,r11,r0
movf	_ub+12(r0),_tb+12
ashl	$5,r11,r0
movf	_ub+16(r0),_tb+16
ashl	$5,r11,r0
movf	_ub+20(r0),_tb+20
ashl	$5,r11,r0
movf	_ub+24(r0),_tb+24
ashl	$5,r11,r0
movl	_ub+28(r0),_tb+40
pushal	__iob+16
pushl	$1
pushl	$52
pushal	_tb
calls	$4,_fwrite
incl	r11
L81:cmpl	r11,_usize
jlss	L2000013
ret

