
/* advance.c
/*	advance
/*
*/
#include "mas.h"
#include "fbcparams.h"
#include "fbc.h"
#include "consts.h"
#include "polydefs.h"

/* registers used

	ADVANCE_S2D
		_XLEFT_HI
		_XLEFT_LO
		_COLOR_LEFT_HI
		_COLOR_LEFT_LO
		_XRIGHT_HI
		_XRIGHT_LO
		_COLOR_RIGHT_HI
		_COLOR_RIGHT_LO

    constants used
		none
    
    scratch locations used
		_SCR_DEL_XLEFT_HI
		_SCR_DEL_XLEFT_LO
		_SCR_DEL_COLOR_LEFT_HI
		_SCR_DEL_COLOR_LEFT_LO
		_SCR_DEL_XRIGHT_HI
		_SCR_DEL_XRIGHT_LO
		_SCR_DEL_COLOR_RIGHT_HI
		_SCR_DEL_COLOR_RIGHT_LO

*/

advance()
{
newfile("advance.c");

label(ADVANCE_S2D)
    _NS	/* finish updating the deltas */
	REGRAM(ADDOP, P0, _COLOR_RIGHT_HI, _COLOR_RIGHT_HI, DEC);
	PROPIN;
    _ES

    _NS	/* finish updating the deltas */
	REGRAM(ADDOP, P0, _XRIGHT_LO, _XRIGHT_LO, DEC);
	PROPOUT16;
    _ES

    _NS	/* finish updating the deltas */
	REGRAM(ADDOP, P0, _XRIGHT_HI, _XRIGHT_HI, DEC);
	PROPIN;
    _ES

    _NS	/* finish updating the deltas */
	REGRAM(ADDOP, P0, _COLOR_LEFT_LO, _COLOR_LEFT_LO, DEC);
	PROPOUT16;
    _ES

    _NS	/* finish updating the deltas */
	REGRAM(ADDOP, P0, _COLOR_LEFT_HI, _COLOR_LEFT_HI, DEC);
	PROPIN;
    _ES

    _NS	/* finish updating the deltas */
	REGRAM(ADDOP, P0, _XLEFT_LO, _XLEFT_LO, DEC);
	PROPOUT16;
    _ES

    _NS	/* finish updating the deltas */
	REGRAM(ADDOP, P0, _XLEFT_HI, _XLEFT_HI, DEC);
	PROPIN;
	SEQ(RETN);
    _ES

/*****************************************************/
/*****************************************************/
/*****************************************************/

label(ADVANCE_S_Z)
    _NS	/* finish updating the deltas */
	REGRAM(ADDOP, P0, _Z_RIGHT_HI_ZS, _Z_RIGHT_HI_ZS, DEC);
	PROPIN;
    _ES

    _NS	/* finish updating the deltas */
	REGRAM(ADDOP, P0, _COLOR_RIGHT_LO, _COLOR_RIGHT_LO, DEC);
	PROPOUT16;
    _ES

    _NS	/* finish updating the deltas */
	REGRAM(ADDOP, P0, _COLOR_RIGHT_HI, _COLOR_RIGHT_HI, DEC);
	PROPIN;
    _ES

    _NS	/* finish updating the deltas */
	REGRAM(ADDOP, P0, _XRIGHT_LO, _XRIGHT_LO, DEC);
	PROPOUT16;
    _ES

    _NS	/* finish updating the deltas */
	REGRAM(ADDOP, P0, _XRIGHT_HI, _XRIGHT_HI, DEC);
	PROPIN;
    _ES

    _NS	/* finish updating the deltas */
	REGRAM(ADDOP, P0, _Z_LEFT_LO_ZS, _Z_LEFT_LO_ZS, DEC);
	PROPOUT16;
    _ES

    _NS	/* finish updating the deltas */
	REGRAM(ADDOP, P0, _Z_LEFT_HI_ZS, _Z_LEFT_HI_ZS, DEC);
	PROPIN;
    _ES

    _NS	/* finish updating the deltas */
	REGRAM(ADDOP, P0, _COLOR_LEFT_LO, _COLOR_LEFT_LO, DEC);
	PROPOUT16;
    _ES

    _NS	/* finish updating the deltas */
	REGRAM(ADDOP, P0, _COLOR_LEFT_HI, _COLOR_LEFT_HI, DEC);
	PROPIN;
    _ES

    _NS	/* finish updating the deltas */
	REGRAM(ADDOP, P0, _XLEFT_LO, _XLEFT_LO, DEC);
	PROPOUT16;
    _ES

    _NS	/* finish updating the deltas */
	REGRAM(ADDOP, P0, _XLEFT_HI, _XLEFT_HI, DEC);
	PROPIN;
	SEQ(RETN);
    _ES
}

