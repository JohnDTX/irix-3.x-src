#include "term.h"
#include "rpc.h"
#include "Vio.h"

#define STACKLONGS 276

extern long 		a_retval;	/* from io.c */
static char 		*cwa_temp;
static char 		*cwa_newsp;
static FUNPTR 		cwa_func;
extern dispatchEntry 	dispatch[];	/* this is the dispatch table */
static long 		args[STACKLONGS];       /* enough space for a stack 
					    frame and 1000 bytes for a stack */
extern gdownload();

/*
**	doprimitive - get a graphics command code and call the primitive	
**
*/
doprimitive()
{
    register unsigned short cmd;
    register short nargs;
    register dispatchEntry *prim;
    register char *newsp;
    char tempstr[20];


    if ( (cmd=recgcmd()) > maxcom) { 
	messagef("iris: %dcmd code out of range\r\n", cmd);
	return;
    }
#ifdef NOTDEF
printf("doprimitive: ");
printf("%x\n", cmd); 
#endif
printf("%d#", cmd); 
    prim = &dispatch[cmd];
    newsp = (char *)(&args[STACKLONGS-1])-(prim->framesize);
    if( receivef(prim->format,newsp) >= 0 ) {
	if(context==TEXT)
	    switchtographics();
	if(prim->func == gdownload)
	    zerodown(newsp,512);
	cwa_newsp = newsp;			
	asm("movl sp, _cwa_temp"); 		/* sorry kipp and bruce */
	asm("movl _cwa_newsp, sp");
	a_retval = (prim->func)();
	asm("movl _cwa_temp, sp");
	if(prim->returnsdata)
	    senddata(prim->format);
    }
    freearrays();
}

zerodown( ptr, n )
register char *ptr;
register int n;
{
    while(n--) 
	*ptr-- = 0;
}
