|
| $Source: /d2/3.7/src/stand/lib/dev/RCS/spl.s,v $
| $Revision: 1.1 $
| $Date: 89/03/27 17:14:49 $

	.globl	_spl7, _spl6, _spl5, _spl4, _spl3, _spl2, _spl1, _spl0, _splx
_spl7:
_spl6:
_spl5:
_spl4:
_spl3:
_spl2:
_spl1:
_spl0:
_splx:
	rts

	.globl _halt
_halt:
	stop	#0x2700
	jra	_halt
