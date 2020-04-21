/*
 * Keyboard manipuation code
 *
 * TODO:
 *	- add in a "meta" key
 */
#include "../h/param.h"
#include "../h/systm.h"
#include "machine/cpureg.h"
#include "../gl1/kgl.h"
#include "../gl1/shmem.h"
#include "../gl1/kbd.h"
#include "../gl1/device.h"

#include "debug.h"
#if NDEBUG > 0
extern	short kdebug;
#endif

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
 * Structure describing each key in each distinct state
 */
struct	kbutton kbuts[] = {
	{0x80, 0x80, 0x80, 0x80},	/* BUTTON0 - BREAKKEY */
	{0xff, 0xff, 0xff, 0xff},	/* BUTTON1 - SETUPKEY */
	{0xff, 0xff, 0xff, 0xff},	/* BUTTON2 - CTRLKEY */
	{0xff, 0xff, 0xff, 0xff},	/* BUTTON3 - CAPSLOCKKEY */
	{0xff, 0xff, 0xff, 0xff},	/* BUTTON4 - RIGHTSHIFTKEY */
	{0xff, 0xff, 0xff, 0xff},	/* BUTTON5 - LEFTSHIFTKEY */
	{0x1b, 0x1b, 0x1b, 0x1b},	/* BUTTON6 - ESCKEY */
	{0x31, 0x21, 0x31, 0x21},	/* BUTTON7 - ONEKEY */
	{0x09, 0x09, 0x09, 0x09},	/* BUTTON8 - TABKEY */
	{0x71, 0x51, 0x11, 0x11},	/* BUTTON9 - QKEY */
	{0x61, 0x41, 0x01, 0x01},	/* BUTTON10 - AKEY */
	{0x73, 0x53, 0x13, 0x13},	/* BUTTON11 - SKEY */
	{0xff, 0xff, 0xff, 0xff},	/* BUTTON12 - NOSCRLKEY */
	{0x32, 0x40, 0x32, 0x00},	/* BUTTON13 - TWOKEY */
	{0x33, 0x23, 0x33, 0x23},	/* BUTTON14 - THREEKEY */
	{0x77, 0x57, 0x17, 0x17},	/* BUTTON15 - WKEY */
	{0x65, 0x45, 0x05, 0x05},	/* BUTTON16 - EKEY */
	{0x64, 0x44, 0x04, 0x04},	/* BUTTON17 - DKEY */
	{0x66, 0x46, 0x06, 0x06},	/* BUTTON18 - FKEY */
	{0x7a, 0x5a, 0x1a, 0x1a},	/* BUTTON19 - ZKEY */
	{0x78, 0x58, 0x18, 0x18},	/* BUTTON20 - XKEY */
	{0x34, 0x24, 0x34, 0x24},	/* BUTTON21 - FOURKEY */
	{0x35, 0x25, 0x35, 0x25},	/* BUTTON22 - FIVEKEY */
	{0x72, 0x52, 0x12, 0x12},	/* BUTTON23 - RKEY */
	{0x74, 0x54, 0x14, 0x14},	/* BUTTON24 - TKEY */
	{0x67, 0x47, 0x07, 0x07},	/* BUTTON25 - GKEY */
	{0x68, 0x48, 0x08, 0x08},	/* BUTTON26 - HKEY */
	{0x63, 0x43, 0x03, 0x03},	/* BUTTON27 - CKEY */
	{0x76, 0x56, 0x16, 0x16},	/* BUTTON28 - VKEY */
	{0x36, 0x5e, 0x36, 0x1e},	/* BUTTON29 - SIXKEY */
	{0x37, 0x26, 0x37, 0x26},	/* BUTTON30 - SEVENKEY */
	{0x79, 0x59, 0x19, 0x19},	/* BUTTON31 - YKEY */
	{0x75, 0x55, 0x15, 0x15},	/* BUTTON32 - UKEY */
	{0x6a, 0x4a, 0x0a, 0x0a},	/* BUTTON33 - JKEY */
	{0x6b, 0x4b, 0x0b, 0x0b},	/* BUTTON34 - KKEY */
	{0x62, 0x42, 0x02, 0x02},	/* BUTTON35 - BKEY */
	{0x6e, 0x4e, 0x0e, 0x0e},	/* BUTTON36 - NKEY */
	{0x38, 0x2a, 0x38, 0x2a},	/* BUTTON37 - EIGHTKEY */
	{0x39, 0x28, 0x39, 0x28},	/* BUTTON38 - NINEKEY */
	{0x69, 0x49, 0x09, 0x09},	/* BUTTON39 - IKEY */
	{0x6f, 0x4f, 0x0f, 0x0f},	/* BUTTON40 - OKEY */
	{0x6c, 0x4c, 0x0c, 0x0c},	/* BUTTON41 - LKEY */
	{0x3b, 0x3a, 0x3b, 0x3a},	/* BUTTON42 - SEMICOLONKEY */
	{0x6d, 0x4d, 0x0d, 0x0d},	/* BUTTON43 - MKEY */
	{0x2c, 0x3c, 0x2c, 0x3c},	/* BUTTON44 - COMMAKEY */
	{0x30, 0x29, 0x30, 0x29},	/* BUTTON45 - ZEROKEY */
	{0x2d, 0x5f, 0x2d, 0x1f},	/* BUTTON46 - MINUSKEY */
	{0x70, 0x50, 0x10, 0x10},	/* BUTTON47 - PKEY */
	{0x5b, 0x7b, 0x1b, 0x7b},	/* BUTTON48 - LEFTBRACKET */
	{0x27, 0x22, 0x27, 0x22},	/* BUTTON49 - QUOTEKEY */
	{0x0d, 0x0d, 0x0d, 0x0d},	/* BUTTON50 - RETURNKEY */
	{0x2e, 0x3e, 0x2e, 0x3e},	/* BUTTON51 - PERIODKEY */
	{0x2f, 0x3f, 0x2f, 0x3f},	/* BUTTON52 - VIRGULEKEY */
	{0x3d, 0x2b, 0x3d, 0x2b},	/* BUTTON53 - EQUALKEY */
	{0x60, 0x7e, 0x60, 0x7e},	/* BUTTON54 - ACCENTGRAVEKEY */
	{0x5d, 0x7d, 0x1d, 0x7d},	/* BUTTON55 - RIGHTBRACKETKEY */
	{0x5c, 0x7c, 0x1c, 0x7c},	/* BUTTON56 - BACKSLASHKEY */
	{0xb1, 0xb1, 0xb1, 0xb1},	/* BUTTON57 - PADONEKEY */
	{0xb0, 0xb0, 0xb0, 0xb0},	/* BUTTON58 - PADZEROKEY */
	{0x0a, 0x0a, 0x0a, 0x0a},	/* BUTTON59 - LINEFEEDKEY */
	{0x08, 0x08, 0x08, 0x08},	/* BUTTON60 - BACKSPACEKEY */
	{0x7f, 0x7f, 0x7f, 0x7f},	/* BUTTON61 - DELETEKEY */
	{0xb4, 0xb4, 0xb4, 0xb4},	/* BUTTON62 - PADFOURKEY */
	{0xb2, 0xb2, 0xb2, 0xb2},	/* BUTTON63 - PADTWOKEY */
	{0xb3, 0xb3, 0xb3, 0xb3},	/* BUTTON64 - PADTHREEKEY */
	{0xae, 0xae, 0xae, 0xae},	/* BUTTON65 - PADPERIODKEY */
	{0xb7, 0xb7, 0xb7, 0xb7},	/* BUTTON66 - PADSEVENKEY */
	{0xb8, 0xb8, 0xb8, 0xb8},	/* BUTTON67 - PADEIGHTKEY */
	{0xb5, 0xb5, 0xb5, 0xb5},	/* BUTTON68 - PADFIVEKEY */
	{0xb6, 0xb6, 0xb6, 0xb6},	/* BUTTON69 - PADSIXKEY */
	{0x82, 0x82, 0x82, 0x82},	/* BUTTON70 - PADPF2KEY */
	{0x81, 0x81, 0x81, 0x81},	/* BUTTON71 - PADPF1KEY */
	{0x88, 0x88, 0x88, 0x88},	/* BUTTON72 - LEFTARROWKEY */
	{0x86, 0x86, 0x86, 0x86},	/* BUTTON73 - DOWNARROWKEY */
	{0xb9, 0xb9, 0xb9, 0xb9},	/* BUTTON74 - PADNINEKEY */
	{0xad, 0xad, 0xad, 0xad},	/* BUTTON75 - PADMINUSKEY */
	{0xac, 0xac, 0xac, 0xac},	/* BUTTON76 - PADCOMMAKEY */
	{0x84, 0x84, 0x84, 0x84},	/* BUTTON77 - PADPF4KEY */
	{0x83, 0x83, 0x83, 0x83},	/* BUTTON78 - PADPF3KEY */
	{0x87, 0x87, 0x87, 0x87},	/* BUTTON79 - RIGHTARROWKEY */
	{0x85, 0x85, 0x85, 0x85},	/* BUTTON80 - UPARROWKEY */
	{0x8d, 0x8d, 0x8d, 0x8d},	/* BUTTON81 - PADENTERKEY */
	{0x20, 0x20, 0x20, 0x20},	/* BUTTON82 - SPACKKEY */
};

/*
 * Data structure defining the programmable function keys
 *	- each function key is individually programmable via unix
 *	  ioctl's of the window device XXX
 *	- the "hkey_data" stuff is for the hardwired keys; that is,
 *	  when the keyboard is initialized, the hkey_data is used
 *	  to initialize the function keys
 */
struct	hkey_data {
	char	*k_name;
	short	k_len;
};

struct	key_data {
	char	k_name[100];
	short	k_len;
};

/*
 * Data for the default hardwired function keys
 *	- this data is copied into the real key data structure whenever
 *	  the keyboard is initialized
 */
struct	key_data keys[8];			/* real keys */
struct	hkey_data hardkeys[] = {		/* default keys */
	{ "\033P", 2 },		/* PF1 */
	{ "\033Q", 2 },		/* PF2 */
	{ "\033R", 2 },		/* PF3 */
	{ "\033S", 2 },		/* PF4 */
	{ "\033A", 2 },		/* uparrow */
	{ "\033B", 2 },		/* downarrow */
	{ "\033C", 2 },		/* rightarrow */
	{ "\033D", 2 },		/* left arrow */
};
#define	NHARDKEYS	(sizeof(hardkeys) / sizeof(struct hkey_data))
#define	NKEYS		(sizeof(keys) / sizeof(struct key_data))

/*
 * kbinit:
 *	- perform whatever initialization is needed to reset the keyboard
 *	  to a known state
 */
kbinit()
{
	register short i;
	register int s;

	s = spl6();
	for (i = 0; i < NHARDKEYS; i++) {
		bcopy(hardkeys[i].k_name, keys[i].k_name, hardkeys[i].k_len);
		keys[i].k_len = hardkeys[i].k_len;
	}
	kbd.k_beepmask = KBD_BEEPDEFAULT;
	kbd.k_ledmask = KBD_LEDDEFAULT;
	kb_putc(kbd.k_ledmask);
	kb_putc(kbd.k_beepmask & ~(KBD_SHORTBEEP|KBD_LONGBEEP));
	splx(s);
}

/*
 * kb_softintr:
 *	- call kb_translate from the duart routine
 */
kb_softintr(c)
	char c;
{
	char buf[100];

	wn_translate(buf, kb_translate(c, buf));
}

/*
 * kb_translate:
 *	- translate a key code into a given string of characters, returning
 *	  the number of characters returned, or -1 indicating no key to send
 *	- return 0 for the break key
 *	- holding down SETUP inhibits queue entries
 * TODO	- add in SETUP RESET for zapping running graphics process and
 *	  reset'ing things
 * TODO	- add in funny key support for queue's
 */
kb_translate(c, buf)
	register char c;
	char *buf;
{
	register struct shmem *sh = (struct shmem *)SHMEM_VBASE;
	register short stroke;
	register unsigned char key, ascii;
	register short i;
	static short state;

	if (!kbsetup) {
		kbinit();
		kbsetup = 1;
	}

	grlastupdate = time;
	unblankscreen();
	stroke = !(c & 0x80);
	key = c & 0x7f;

    /* handle shift, control, and caps lock key */
	if ((key == KEY_LEFT_SHIFT) || (key == KEY_RIGHT_SHIFT)) {
		if (stroke)
			state |= 1;
		else
			state &= ~1;
	}
	if (key == KEY_CTRL) {
		if (stroke)
			state |= 2;
		else
			state &= ~2;
	}
	if (key == KEY_CAPSLOCK) {
		if (stroke == 0)
			state ^= 4;
	}

    /* if button is queue'd add it to the queue unless SETUP key is down */
	key += BUTOFFSET;
	sh->Buttons[key].state = stroke;
	if (sh->Buttons[key].queue 
		    && (!sh->Buttons[SETUPKEY].state || key == SETUPKEY)) {
		qenter(key, stroke);
		if (stroke = sh->Buttons[key].tiedevice1)
			qenter(stroke, sh->valuators[stroke - VALOFFSET]);
		if (stroke = sh->Buttons[key].tiedevice2)
			qenter(stroke, sh->valuators[stroke - VALOFFSET]);
		return -1;
	}
	key -= BUTOFFSET;

    /* throw away key releases and unwanted key codes */
	if ((stroke == 0) || (key > NBUTTONS) ||
	    (key == KEY_LEFT_SHIFT) || (key == KEY_RIGHT_SHIFT) ||
	    (key == KEY_CTRL) || (key == KEY_CAPSLOCK) ||
	    (key == KEY_SETUP))
		return -1;

	switch (state) {
	  case 0:			/* normal key */
		ascii = kbuts[key].b_normal;
		break;
	  case 1:			/* shift key only */
	  case 5:			/* capslock + shift */
		ascii = kbuts[key].b_shift;
		break;
	  case 2:			/* control key only */
	  case 6:			/* capslock + control */
		ascii = kbuts[key].b_control;
		break;
	  case 3:			/* control + shift */
	  case 7:			/* control + shift + capslock */
		ascii = kbuts[key].b_controlshift;
		break;
	  case 4:			/* capslock only */
		ascii = kbuts[key].b_normal;
		if ((ascii >= 'a') && (ascii <= 'z'))
			ascii = ascii - 32;
		break;
	}
#if NDEBUG > 0
	if (kdebug && (key == 66))	/* PADSEVENKEY */
		debug();
#endif

    /* return "normal" keycode */
	if ((ascii & 0x80) == 0) {
		/* don't queue it if SETUP key is down */
		if (sh->queuedkeyboard && !sh->Buttons[SETUPKEY].state) {
			qenter(KEYBD, ascii);
			return -1;
		} else {
			buf[0] = ascii;
			return 1;
		}
	}

    /* if a break key, just return 0 */
	if (ascii == 0x80)
		return 0;

    /* see if key is a sepcial key (function key) */
	if ((ascii & 0x7f) <= NKEYS) {
		register struct key_data *kp;

		kp = &keys[(ascii & 0x7F) - 1];
		bcopy(kp->k_name, buf, kp->k_len);
		if (sh->queuedkeyboard) {
			for (i = 0; i < kp->k_len; i++)
				qenter(KEYBD, kp->k_name[i]);
			return -1;
		}
		return kp->k_len;
	}

    /* key must be on keypad */
	ascii &= 0x7f;				/* strip off high bit */
	if (keypadmode) {
		if (sh->queuedkeyboard) {
			qenter(KEYBD, '\033');
			qenter(KEYBD, ascii);
			return -1;
		}
		buf[0] = '\033';
		buf[1] = ascii;
		return 2;
	}
	if (sh->queuedkeyboard) {
		qenter(KEYBD, ascii);
		return -1;
	}
	buf[0] = ascii;
	return 1;
}

/*
 * kbbell:
 *	- ring the keyboard's bell
 */
kbbell()
{
	kb_putc(kbd.k_beepmask);
}
