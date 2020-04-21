
|	The C compiler generates a reference to the global
|	fltused when floating point is used in a C program.
|	This module satisfies that symbol 
|
	.globl 	fltused
	.data
fltused: .word	0
