/*
 * Keyboard interface structure between user and system:
 *	- user issues a grioctl(GR_[SG]ETKBD, &a_kbd)
 */
struct	kbd {
	short	k_beepmask;		/* mask of beep bits */
	short	k_ledmask;		/* mask of led bits */
};

/* some special keys (keycodes from the keyboard) */
#define	KEY_CTRL	0x02
#define	KEY_RIGHT_SHIFT	0x04
#define	KEY_LEFT_SHIFT	0x05
#define	KEY_CAPSLOCK	0x03
#define	KEY_SETUP	0x01

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

#ifdef	KERNEL
struct	kbd kbd;
#endif
