	.text
| _spl commands
	.globl	_splhi,_spl7,_spl6,_spl5,_spl4,_spl3,_spl2,_spl1,_spl0,_splx

_splhi:
_spl7:	movw	sr,d0		| fetch current CPU priority
	movw	#0x2700,sr	| set priority 7
	rts
_spl6:	movw	sr,d0		| fetch current CPU priority
	movw	#0x2600,sr	| set priority 6
	rts
_spl5:	movw	sr,d0		| fetch current CPU priority
	movw	#0x2500,sr	| set priority 5
	rts
_spl4:	movw	sr,d0		| fetch current CPU priority
	movw	#0x2400,sr	| set priority 4
	rts
_spl3:	movw	sr,d0		| fetch current CPU priority
	movw	#0x2300,sr	| set priority 3
	rts
_spl2:	movw	sr,d0		| fetch current CPU priority
	movw	#0x2200,sr	| set priority 2
	rts
_spl1:	movw	sr,d0		| fetch current CPU priority
	movw	#0x2100,sr	| set priority 1
	rts
_spl0:	movw	sr,d0		| fetch current CPU priority
	movw	#0x2000,sr	| set priority 0
	rts

_splx:	movw	sp@(6),sr	| set priority
	rts

