/*	add_sentinels
/*
/*	puts flag values and addresses on either end of the vertex list
/*	and send the stipple pattern address to the BPC 
/*
*/
#include "mas.h"
#include "fbcparams.h"
#include "fbc.h"
#include "consts.h"
#include "polydefs.h"

/* registers used
		_TEMP,
		_LASTPT

   constants used
		_STARTLIST
		_BIGNEG
*/

add_sentinels()
{
newfile("addsentinels.c");

label(ADD_SENTINELS)

    _NS /* load _TEMP with the address of the start of the list and
	increment the address to the correct location, this is done
	so BIGNEG is only loaded once */
	IMMREG(RONLYOP, P0, _STARTLIST+4, _TEMP);
	CONST(_STARTLIST+4);
	DOTOMAR(INC);
    _ES

    _NS /* write the address of the beginning of the list next, and
	decrement the address to the correct position for the flag
	value */
	RAM(RAMWR, _TEMP, DEC);
    _ES

    _NS /* load _TEMP with BIGNEG */
	IMMREG(RONLYOP, P0, _BIGNEG, _TEMP);
	CONST(_BIGNEG);
    _ES

    _NS /* write BIGNEG to the next location to flag the end of the
        vertex list */
	RAM(RAMWR, _TEMP, INC);
    _ES

    _NS /* load the MAR with the address of the start of the list */
	LOADMAR(_STARTLIST);
	CONST(_STARTLIST);
    _ES

    _NS /* write the flag value in there */
	RAM(RAMWR, _TEMP, INC);
    _ES

    _NS /* write the address of the end of the list now, also
	increment _LASTPT in case we have to reverse the order of Z and
	shade */
	ALUOP(RONLYOP, P1);
	SETROP(_LASTPT, NONE);
	SETSOP(NONQOP, _LASTPT, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRE);
	SEQ(RETN);
    _ES
}
