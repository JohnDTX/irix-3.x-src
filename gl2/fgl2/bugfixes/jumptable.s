|
|
|	This file includes simple jumps for the gl2
|	routines which are missing from the lib.prim file
|	as they are not in the remote version but are needed
|	in the local version.   These are
|
|		gexit, ginit, gflush
|
		.globl	gexit,ginit,gflush
gexit:
	jmp	_gexit
ginit:
	jmp _ginit
gflush:
	jmp _gflush
