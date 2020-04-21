
|
|	getbstart is a C-callable routine which returns
|	the address of the start of the bss segment as
|	set up by the loader.  getend is similar, and
|	returns the end of the program (end of bss), as
|	getedata returns the value of _edata.
|
	.globl	getbstart,_getbstart
getbstart:
_getbstart:
	movl	#_bstart,d0
	rts
	.globl	getedata,_getedata
getedata:
_getedata:
	movl	#_edata,d0
	rts
	.globl	getend,_getend
getend:
_getend:
	movl	#_end,d0
	rts
