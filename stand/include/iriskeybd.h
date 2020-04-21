/*
* $Source: /d2/3.7/src/stand/include/RCS/iriskeybd.h,v $
* $Revision: 1.1 $
* $Date: 89/03/27 17:13:46 $
*/
/*
 * definitions for the SGI up/down encoded Keytronics keyboard
 */
#ifndef KEYBOARD
#define KEYBOARD

/*
 * Keyboard definitions.
 *
 * $Source: /d2/3.7/src/stand/include/RCS/iriskeybd.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:13:46 $
 */

#define	TERM_NAME	"iris_ansi"
#define	LAMP_LOCAL	1
#define	LAMP_KBDLOCKED	2
#define	LAMP_CAPSLOCK	3
#define	LAMP_L1		4
#define	LAMP_L2		5
#define	LAMP_L3		6
#define	LAMP_L4		7

#define	TIME_DELAY1	40000	/* time for kb to reset			*/
#define	TIME_DELAY2	10000	/* time for 2nd config byte to arrive	*/

/* this works for both old and new keyboards */
#define	CONFIG_REQUEST	0x10	/* kb request config bytes code		*/

/* config byte returned old kb	*/
#define	CONFIG_BYTE_OLDKB	0xaa

/* config byte returned new kb, clover standard and GS	*/
#define	CONFIG_BYTE_NEWKB	0x6e

#define	CONFIG_KBSWTCH_ISO		0x01	/* second byte code for ISO kbd */
#define	CONFIG_KBSWTCH_STD		0x00	/* second byte code for STD kbd	*/

/* control bits for new keyboard, low bit = 0 */
#define	NEWKB_SHORTBEEP		0x02
#define	NEWKB_LONGBEEP		0x04
#define	NEWKB_NOCLICK		0x08
#define	NEWKB_GETCONFIG		0x10
#define	NEWKB_SETDS1		0x20
#define	NEWKB_SETDS2		0x40
#define	NEWKB_AUTOREPEAT	0x80

/* control bits for new keyboard, low bit = 1 */
#define	NEWKB_DS1ANDDS2		0x03
#define	NEWKB_SETDS3		0x05
#define	NEWKB_SETDS4		0x09
#define	NEWKB_SETDS5		0x11
#define	NEWKB_SETDS6		0x21
#define	NEWKB_SETDS7		0x41
#define	NEWKB_OFFDS		0x01		/* disable most DS's */

/* soft keyboard states */
#define STATE_LSHIFT	0x01
#define STATE_RSHIFT	0x02
#define STATE_LCTRL	0x04
#define	STATE_RCTRL	0x08	/* GS keyboard only	*/
#define	STATE_LALT	0x10	/* GS keyboard only	*/
#define	STATE_RALT	0x20	/* GS keyboard only	*/
#define STATE_CAPSLOCK	0x40
#define STATE_SETUP	0x80	/* IRIS keyboard only	*/

/*
 * We manage an extended ascii keyboard set with this code.  To do this,
 * the standard 256 ascii sequence is represented in a standard fashion.
 * An addtions will be numerically larger than 256.
 */
#define	CODE_BREAK	0x0100		/* break code */


/*
** Decoding structure for translating up/down keyboard strokes into ascii
** key codes. This structure is used to represent the common keys
** between the two (2) keyboards that are supported: IRIS and GS.  This
** structure is also used to hold the keys specific for a keyboard.
*/
struct	kbutton {
	char	b_normal;		/* normal code */
	char	b_shift;		/* shifted code */
	char	b_control;		/* controlified code */
	char	b_controlshift;		/* controlshiftified code */
	char	b_alt ;			/* alt code */
	char	b_gs_index ;		/* GS char index */
};

/*
 * Data structure defining the function keys
 */
struct	key_data {
	char	*k_name;
	short	k_len;
};

#define	CTRL(c)		('c' & 037)

extern	struct kbutton kbuts[];
extern struct kbutton kbuts_std[];
extern struct kbutton kbuts_iso[];

extern	struct key_data keypad_numeric[ 29 ];

#define	 N_KPKEYS	(sizeof(keypad_numeric) / sizeof(struct key_data))

/*
** some special keys (keycodes from the keyboard)
** It just so happens that on the two different keyboards, these following
** keys are the same. NOTE: the values are the keycode + 1
*/
#define	KEY_CTRL	0x03
#define	KEY_RIGHT_CTRL	0x56
#define	KEY_RIGHT_SHIFT	0x05
#define	KEY_LEFT_SHIFT	0x06
#define	KEY_CAPSLOCK	0x04
#define	KEY_LEFT_ALT	0x54	/* GS keyboard only	*/
#define	KEY_RIGHT_ALT	0x55	/* GS keyboard only	*/
#define	KEY_SETUP	0x02	/* IRIS keyboard only	*/

/*
** Keybored types
*/
#define	KBD_4D60STD	1
#define	KBD_4D60ISO	2
#define	KBD_IRIS	3

/* constants for the iris keyboard */
#define	KBD_BEEPCMD	0x00		/* low bit clear --> beep commands */
#define	KBD_LEDCMD	0x01		/* low bit set --> led commands */
#define	KBD_RESET	0x10		/* reset the keyboard */
#define	KBD_REPLY	0xAA		/* reply from keyboard after reset */

/* beep commands */
#define	KBD_CLICK	0x08		/* key click: active low */
#define	KBD_SHORTBEEP	0x02		/* do a short beep */
#define	KBD_LONGBEEP	0x04		/* do a long beep */

/* led commands */
#define	KBD_LED0	0x00
#define	KBD_LED1	0x02		/* led0 and led1 are inverses */
#define	KBD_LED2	0x04
#define	KBD_LED3	0x08
#define	KBD_LED4	0x10
#define	KBD_LED5	0x20
#define	KBD_LED6	0x40
#define	KBD_LED8	0x80

/* defaults */
#define	KBD_BEEPDEFAULT	(KBD_BEEPCMD|KBD_CLICK|KBD_SHORTBEEP)
#define	KBD_LEDDEFAULT	(KBD_LEDCMD|KBD_LED0)

#define BUTCOUNT	144
#endif
