|	This routine contains cerror, the common c error handler
	
	.text
	.globl	cerror, _errno

cerror:
	movl	d0,_errno
	moveq	#-1,d0
	rts
