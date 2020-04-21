|
|  boost.s --
|
|  old proms would only diskboot correctly from 0x1000.
|  this program relocates whatever occurs after its END,
|  to the TGT location, and jumps to it, thereby correcting
|  this prom defect.
| 
|  designed to be used in conjunction with makedboot.
|  by mucking with this program's a.out after linking and
|  stripping, we can cause the old proms to load the stuff
|  that is to be relocated.
|
|	IMPORTS:
	.globl	_edata
A_SIZE		= 0x20
A_MAGIC		= 0x00
A_TEXT		= 0x04
A_DATA		= 0x08
A_ENTRY		= 0x1C

start:
	jra	start2
	.asciz	"WE WILL BE HAVING FUN"
	.even

start2:
	movl	#_edata,a0		| point to the tacked-on header
	movl	a0@(A_TEXT),d0		| get its text+data size
	addl	a0@(A_DATA),d0		|
	addql	#3,d0			| round up and
	asrl	#2,d0			| convert to longword count
	movl	a0@(A_ENTRY),a2		| where to copy it all
	movl	a2,a1			| and where to launch it
	addl	#8,a2			| skip reset vector
	addl	#A_SIZE,a0		| where text+data is, now
1$:
	movl	a0@+,a1@+		| relocate
	subql	#1,d0
	jne	1$

	jmp	a2@			| launch!
