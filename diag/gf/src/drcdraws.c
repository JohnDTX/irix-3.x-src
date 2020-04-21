/* gl2draws.c		*/
/*   used in gecmd.c	*/

#include "geofdef.h"

unsigned short drawtests[][256] = {

    {		/* setup ---	gt0			*/
	0x208,0x16,1,0xdf,
	0x208,0x14,0,0,
	0x208,0x15,0xffff,0xffff,
	0x408,2,0,0,0x3ff,0x3ff,
	0x1108,0x17,0, -1,-1,-1,-1,-1,-1,-1,-1,
			-1,-1,-1,-1,-1,-1,-1,-1,
	0x108,0x21,0,
	0x208,0x20,0,0xffff,
    GEOF},

    {		/* clear screen -- gt1			*/
	0x408,0x39,0,0,0x3ff,0x3ff,
    GEOF},
};


unsigned short expect[][64] = {
/*  format: {n, data, data, ...,data}	  :n = # of interrupts */
{0},	/* gt0 */
{0},	/* gt1 */
{0},	/* gt2 */
{0},	/* gt3 */
{0},	/* gt4 */
{0},	/* gt5 */
{0},	/* gt6 */
{0},	/* gt7 */


};
