	.text
| spl commands
	.globl	splhi,spl7,spl6,spl5,spl4,spl3,spl2,spl1,spl0,splx

splhi:
spl7:	movw	sr,d0		| fetch current CPU priority
	movw	#0x2700,sr	| set priority 7
	rts
spl6:	movw	sr,d0		| fetch current CPU priority
	movw	#0x2600,sr	| set priority 6
	rts
spl5:	movw	sr,d0		| fetch current CPU priority
	movw	#0x2500,sr	| set priority 5
	rts
spl4:	movw	sr,d0		| fetch current CPU priority
	movw	#0x2400,sr	| set priority 4
	rts
spl3:	movw	sr,d0		| fetch current CPU priority
	movw	#0x2300,sr	| set priority 3
	rts
spl2:	movw	sr,d0		| fetch current CPU priority
	movw	#0x2200,sr	| set priority 2
	rts
spl1:	movw	sr,d0		| fetch current CPU priority
	movw	#0x2100,sr	| set priority 1
	rts
spl0:	movw	sr,d0		| fetch current CPU priority
	movw	#0x2000,sr	| set priority 0
	rts

splx:	movw	sp@(6),sr	| set priority
	rts

