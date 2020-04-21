/*                      STORE AND LOAD MULTIPLE ROUTINES
/*
/*  These routines are used to save and then subsequently restore the contents
/*  of the 2903's general purpose registers to RAM storage.  The routines are
/*  enterable at any of 16 locations to specify the saving (restoring) of any
/*  set of last "n" registers.  The caller is responsible for initializing the
/*  RAM's MAR to point to the desired save area before executing the store
/*  routine, and then setting the MAR to the same value before restoring the
/*  registers with a call to the load routine.
/*
/*  For instance, to free up registers 11-15 for use by a subroutine, the
/*  caller would load MAR with the address of his save area, call STM11UP,
/*  call the subroutine with no worries about the destruction of data in those
/*  registers, and then restore the registers by calling LM11UP.
/*
*/

#include "mas.h"
#include "fbcparams.h"
#include "fbc.h"
#include "consts.h"


stm()
{
newfile("stm.c");
label(STM0UP)
	_NS RAM(RAMWR,0,INC);_ES         /* Store register 0 and bump MAR.  */

label(STM1UP)
	_NS RAM(RAMWR,1,INC);_ES         /* All the rest are the same.      */

label(STM2UP)
	_NS RAM(RAMWR,2,INC);_ES

label(STM3UP)
	_NS RAM(RAMWR,3,INC);_ES

label(STM4UP)
	_NS RAM(RAMWR,4,INC);_ES

label(STM5UP)
	_NS RAM(RAMWR,5,INC);_ES

label(STM6UP)
	_NS RAM(RAMWR,6,INC);_ES

label(STM7UP)
	_NS RAM(RAMWR,7,INC);_ES

label(STM8UP)
	_NS RAM(RAMWR,8,INC);_ES

label(STM9UP)
	_NS RAM(RAMWR,9,INC);_ES

label(STM10UP)
	_NS RAM(RAMWR,10,INC);_ES

label(STM11UP)
	_NS RAM(RAMWR,11,INC);_ES

label(STM12UP)
	_NS RAM(RAMWR,12,INC);_ES

label(STM13UP)
	_NS RAM(RAMWR,13,INC);_ES

label(STM14UP)
	_NS RAM(RAMWR,14,INC);_ES

label(STM15UP)
	_NS RAM(RAMWR,15,HOLD);
	   SEQ(RETN);_ES

label(LM0UP)
	_NS RAM(RAMRD,0,INC);_ES         /* Load register 0 and bump MAR.   */

label(LM1UP)
	_NS RAM(RAMRD,1,INC);_ES         /* All the rest are the same.      */

label(LM2UP)
	_NS RAM(RAMRD,2,INC);_ES

label(LM3UP)
	_NS RAM(RAMRD,3,INC);_ES

label(LM4UP)
	_NS RAM(RAMRD,4,INC);_ES

label(LM5UP)
	_NS RAM(RAMRD,5,INC);_ES

label(LM6UP)
	_NS RAM(RAMRD,6,INC);_ES

label(LM7UP)
	_NS RAM(RAMRD,7,INC);_ES

label(LM8UP)
	_NS RAM(RAMRD,8,INC);_ES

label(LM9UP)
	_NS RAM(RAMRD,9,INC);_ES

label(LM10UP)
	_NS RAM(RAMRD,10,INC);_ES

label(LM11UP)
	_NS RAM(RAMRD,11,INC);_ES

label(LM12UP)
	_NS RAM(RAMRD,12,INC);_ES

label(LM13UP)
	_NS RAM(RAMRD,13,INC);_ES

label(LM14UP)
	_NS RAM(RAMRD,14,INC);_ES

label(LM15UP)
	_NS RAM(RAMRD,15,HOLD);
	   SEQ(RETN);_ES
}
