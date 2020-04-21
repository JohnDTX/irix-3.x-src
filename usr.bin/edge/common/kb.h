/*
 * Keyboard definitions.
 *
 * $Source: /d2/3.7/src/usr.bin/edge/common/RCS/kb.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:45:54 $
 */

/*
 * We manage an extended ascii keyboard set with this code.  To do this,
 * the standard 256 ascii sequence is represented in a standard fashion.
 * An addtions will be numerically larger than 256.
 */
#define	CODE_BREAK	0x0100		/* break code */

/*
 * Decoding structure for translating up/down keyboard strokes into ascii
 * key codes.
 */
struct	kbutton {
	char	b_normal;		/* normal code */
	char	b_shift;		/* shifted code */
	char	b_control;		/* controlified code */
	char	b_controlshift;		/* controlshiftified code */
};

/*
 * Data structure defining the function keys
 */
struct	key_data {
	char	*k_name;
	short	k_len;
};

extern	struct kbutton kbuts[];
extern	struct key_data funcnumeric[];
extern	struct key_data funckeypad[];
