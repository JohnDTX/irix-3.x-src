/*
**	These routines do special things
**
**
*/
#include "gl.h"
#include "term.h"
#include <types.h>
#include <Venviron.h>


void xginit()
{
    ginit();
    switchtographics();
    tpoff();
    cursoff();
}


void xgreset()
{
    greset();
    tpoff();
    cursoff();
}


void xgexit()
{

    greset();
    cursoff();
    tpoff();
    lampoff(LAMP_KBDLOCKED);
    context = TEXT;
}


void xcharstr(ptr)
u_char *ptr;
{
	charstr(ptr);
}

