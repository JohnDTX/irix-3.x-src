/* relocate.h - Macro to move code compiled to start from _start
 * but mapped in memory at another address (eg. on a prom board) to 
 * _start.  It should be the first statement in main(), after any
 * declarations (no register declarations of pointers can be made, as
 * the macro assumes that first address register available is a5).
 * Since ld68 always puts a call to ddt before calling main(),
 * the program should always be started from the address of main(), which
 * is usually 18 hex from the beginning of the program.  The code will 
 * happily copy itself onto itself, so nothing need be changed if the code
 * is actually loaded at _start.  NB. Nothing past _end is 
 * transferred.
 *
 * Mon Nov  1 14:08:55 1982  Charles (Herb) Kuta  (kuta at Olympus)
 */

#ifndef JMPCODE
#define JMPCODE		0x4ef9
#endif

#define _SUB_ADDR(addr1,addr2) 	( ((int)addr1 - (int)addr2) >> 1 )
		/* trickiness to prevent call to ldiv */ 

#define _JUMP(addr)		asm("	jmp	addr")

#ifdef PM1
#define START		_start
#endif
#ifdef PM2
#define START		start
#endif

#define RELOCATE 							\
{ 									\
    extern short START,_end,_relocstart,_relocend;			\
    register short *here /* a5 */,*addr, *newAddr; 			\
									\
    asm("_relocstart:"); 						\
    asm("	lea	pc@(-2),a5");	/* get where we are */ 		\
    newAddr = &START;	 		/* get where we'll put it */	\
									\
    /* transfer portion from start of program up to invocation of RELOCATE */\
    addr = here - _SUB_ADDR(&_relocstart,newAddr); 			\
    for (; addr < here; addr++,newAddr++)				\
    	*newAddr = *addr;						\
    									\
    /* replace ourself with a jump to the code which follows us */	\
    *newAddr++ = JMPCODE;						\
    *newAddr++ = (short) ((int) &_relocend >> 16); 			\
    *newAddr++ = (short) &_relocend; 					\
									\
    /* transfer remainder; NB. _end is location of program end when	\
     * it starts at &START */ 					\
    newAddr = &_relocend; 						\
    addr = here + _SUB_ADDR(&_relocend,&_relocstart); 			\
    for (; newAddr <= &_end; addr++,newAddr++)				\
    	*newAddr = *addr;						\
									\
    /* now jump to the start to the code we've just moved */		\
    _JUMP(START);	 						\
    asm("_relocend:"); 							\
}
