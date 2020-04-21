/*
	dbg.h -- header file for the workstation prom debugger.

*/
#ifndef SETJMP
#include "setjmp.h"
#define SETJMP
#endif


/* format of the stack frame as saved by <BREAK> 

	-------------------------
	|	d0....		|  <--- FRAMEPTR (lowest address)
	-------------------------
	|	a7		|		|  increasing 
	-------------------------		|  addresses
	|   alignment word	|  		|
	-------------------------		V
	|   status register	|
	-------------------------
	|   pc (high)		|
	-------------------------
	|   pc (low)		|
	-------------------------
*/
#define DATAREGOFFSET	0	/* longword offsets */
#define ADDRREGOFFSET	8
#define SPOFFSET	(ADDRREGOFFSET + 7)
#define SROFFSET	16
#define PCOFFSET	17
