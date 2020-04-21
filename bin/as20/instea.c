#include "sym.h"

#include "inst.h"

/* 
    instea contains the mode and register field encoding for
    each of the possible addressing modes, as enumerated in
    addrmodes.h.  The special entry I_NOMODE is used to indicate
    that the field is empty.  The special entry I_NEEDSREG is
    used to indicate that the register field should be filled in
    with the register in use in the addressing mode.
*/
struct instea_s instea[] =
{
	{ 0,0 },
	{ 0x7, 0x3,},		/* A_PCINDPOSTX (pc rel, indir, post-indexed) */
	{ 0x7, 0x3,},		/* A_PCINDPREX (pc rel, pre-indexed, indirect) */
	{ 0x7, 0x3,},		/* A_PCIXNI (pc relative, indexed, non-indirect */
	{ 0x7, 0x3,},		/* A_PCIXBD (pc relative, with byte disp) */

	/* 5 */
	{ 0x7, 0x2,},		/* A_PCDISP (pc relative, with disp) */
	{ I_NOMODE, I_NOMODE,}, /* unused */
	{ I_NOMODE, I_NOMODE,},	/* unused */
	{ I_NOMODE, I_NOMODE,}, /* A_SPECIALREG (usp,vbr,cacr...) */
	{ 0x7, 0x4,},		/* A_IS_IMML (imm, long value) */

	/* 10 */
	{ 0x7, 0x4,},		/* A_IS_IMMW (imm, word value) */
	{ 0x7, 0x4,},		/* A_IS_IMMB (imm, byte value (32-127, -(1..128) */
	{ 0x7, 0x4,},		/* A_IS_IMM9T31 (imm, value 9-31, incl) */
	{ 0x7, 0x4,},		/* A_IS_IMM8 (imm, value = 8 ) */

	/* 14 */
	{ 0x7, 0x4,},		/* A_IS_IMM1T7 (imm, value 1-7, incl.) */
	{ 0x7, 0x4,},		/* A_IS_IMM0 (immediate, value = zero) */
	{ I_NOMODE, I_NOMODE,},	/* A_IS_DISPL (displacement, long) */
	{ I_NOMODE, I_NOMODE,}, /* A_IS_DISPW (displacement, word) */

	/* 18 */
	{ I_NOMODE, I_NOMODE,},	/* A_IS_DISPB (displacement, byte) */
	{ 0x7, 0x1,},		/* A_IS_ABSL (absolute long) */
	{ 0x7, 0x0,},		/* A_IS_ABSW (absolute word) */
	{ 0x6, I_NEEDSREG,},	/* A_ANINDPOSTX (aN indirect, post-indexed) */

	/* 22 */
	{ 0x6, I_NEEDSREG,},	/* A_ANINDPREX (aN pre-indexed indirect) */
	{ 0x6, I_NEEDSREG,},	/* A_ANIXNI (aN indexed, no indirection) */
	{ 0x6, I_NEEDSREG,},	/* A_ANIXBD (aN ind with byte disp) */
	{ 0x5, I_NEEDSREG,},	/* A_ANINDDISP (aN ind with disp) */

	/* 26 */
	{ 0x4, I_NEEDSREG,},	/* A_ANINDPD (aN ind post decr) */
	{ 0x3, I_NEEDSREG,},	/* A_ANINDPI (aN ind pre incr) */
	{ 0x2, I_NEEDSREG,},	/* A_ANIND (address register indirect) */
	{ I_NOMODE, 0,},	/* A_SR */

	/* 30 */
	{ I_NOMODE, 0,},	/* A_CCR */
	{ 0x1, I_NEEDSREG,}, 	/* A_AN */
	{ 0x0, I_NEEDSREG,} 	/* A_DN */
};
