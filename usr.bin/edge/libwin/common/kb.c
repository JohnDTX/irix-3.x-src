/*
 * Keyboard management stuff
 *
 * $Source: /d2/3.7/src/usr.bin/edge/libwin/common/RCS/kb.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:47:26 $
 */
#include "gl.h"
#include "device.h"
#include "kb.h"
#include "tf.h"
#include "string.h"
#include "message.h"

/*
 * Data for the default function keys
 */
struct key_data funcnumeric[] = {	/* numeric function keys */
	{ "\033P", 2 },	/* PF1 */
	{ "\033Q", 2 },	/* PF2 */
	{ "\033R", 2 },	/* PF3 */
	{ "\033S", 2 },	/* PF4 */
	{ "\033A", 2 },	/* uparrow */
	{ "\033B", 2 },	/* downarrow */
	{ "\033C", 2 },	/* rightarrow */
	{ "\033D", 2 },	/* left arrow */
	{ "0", 1 },	/* 0 key */
	{ "1", 1 },	/* 1 key */
	{ "2", 1 },	/* 2 key */
	{ "3", 1 },	/* 3 key */
	{ "4", 1 },	/* 4 key */
	{ "5", 1 },	/* 5 key */
	{ "6", 1 },	/* 6 key */
	{ "7", 1 },	/* 7 key */
	{ "8", 1 },	/* 8 key */
	{ "9", 1 },	/* 9 key */
	{ "-", 1 },	/* dash key */
	{ ",", 1 },	/* comma key */
	{ ".", 1 },	/* period */
	{ "\r", 1 },	/* return */
};

struct key_data funckeypad[] = {	/* keypadmode functions keys */
	{ "\033P", 2 },	/* PF1 */
	{ "\033Q", 2 },	/* PF2 */
	{ "\033R", 2 },	/* PF3 */
	{ "\033S", 2 },	/* PF4 */
	{ "\033A", 2 },	/* uparrow */
	{ "\033B", 2 },	/* downarrow */
	{ "\033C", 2 },	/* rightarrow */
	{ "\033D", 2 },	/* left arrow */
	{ "\033?p", 3 },	/* 0 key */
	{ "\033?q", 3 },	/* 1 key */
	{ "\033?r", 3 },	/* 2 key */
	{ "\033?s", 3 },	/* 3 key */
	{ "\033?t", 3 },	/* 4 key */
	{ "\033?u", 3 },	/* 5 key */
	{ "\033?v", 3 },	/* 6 key */
	{ "\033?w", 3 },	/* 7 key */
	{ "\033?x", 3 },	/* 8 key */
	{ "\033?y", 3 },	/* 9 key */
	{ "\033?m", 3 },	/* dash key */
	{ "\033?l", 3 },	/* comma key */
	{ "\033?n", 3 },	/* period */
	{ "\033?M", 3 },	/* return */
};
#define	NKEYS		(sizeof(funcnumeric) / sizeof(struct key_data))

/*
 * Base character for binding keys.  When keys are bound, the user
 * sends \E<key><binding><terminator> where <key> is an index into
 * our keypad table, offset by MINBIND.  <binding> is an ascii sequence
 * of characters.  <terminator> is either NULL, RETURN, LINEFEED, or
 * ESCAPE.  For now, only allow PF1 through PF4 to be bound.
 */
#define	MINBIND		'1'
#define	MAXBIND		(MINBIND + 4)

short	kbdstate;

/* soft keyboard states */
#define STATE_LSHIFT	1
#define STATE_RSHIFT	2
#define STATE_CTRL	4
#define STATE_CAPSLOCK	8

int	keypadmode;

/*
 * Handle a keyboard event
 */
void
kbevent(notUsed, event, value)
	int notUsed;
	long event, value;
{
	char buf[100];					/* XXX */
	int n;

	if (event == KBBINDKEY) {
		/* set key binding */
	} else
	if (event == KBGETBINDING) {
		/* return key binding */
	} else {
		n = kbtranslate(event, value, buf);
		if (n) {
			struct kbmsgvalue *kbmsg;
			
			kbmsg = MALLOC(struct kbmsgvalue *,
					      sizeof(struct kbmsgvalue));
			kbmsg->data = MALLOC(short *, n * sizeof(short));
			kbmsg->len = n;
			bcopy(buf, kbmsg->data, n * sizeof(short));
			sendEvent(KBMSG, (long) kbmsg);
		}
	}
}

/*
 * Init the kb stuff
 */
void
kbinit()
{
	register struct key_data *fn, *fk;
	register int i;
	char *name;
	extern char *malloc();
	static char kb_setup;

	if (kb_setup)
		return;
	kb_setup = 1;

	/*
	 * Bind each key, using data in binding table.
	 * Malloc some memory so that later re-bindings can
	 * free it.
	 */
	fn = &funcnumeric[0];
	fk = &funckeypad[0];
	for (i = NKEYS; --i >= 0; fn++, fk++) {
		/*
		 * Malloc some memory to hold the initial value of the
		 * numeric keypad definition.  Copy static value from
		 * table into malloc'd buffer.
		 */
		name = malloc((unsigned) (strlen(fn->k_name) + 1));
		strcpy(name, fn->k_name);
		fn->k_name = name;

		/*
		 * Malloc some memory to hold the initial value of the
		 * keypad definition.  Copy static value from
		 * table into malloc'd buffer.
		 */
		name = malloc((unsigned) (strlen(fk->k_name) + 1));
		strcpy(name, fk->k_name);
		fk->k_name = name;
	}

	/* register events */
	for (i = 1; i <= MAXKBDBUT; i++) {
		qdevice(i);
		catchEvent(i, kbevent, 0);
	}
	catchEvent(KBBINDKEY, kbevent, 0);
	catchEvent(KBGETBINDING, kbevent, 0);
}

/*
 * kbtranslate:
 *	- translate a key code into a given string of characters, returning
 *	  the number of characters decoded into buf
 *	- fill in the buffer supplied with the characters, in 16 bit form
 */
int
kbtranslate(key, stroke, buf)
	register short key;
	register int stroke;
	register short *buf;
{
	register unsigned char ascii;
 	register short i;
 	register unsigned char *cptr;

	/* handle shift, control, and caps lock key */
	if (key == LEFTSHIFTKEY) {
		if (stroke)
			kbdstate |= STATE_LSHIFT;
		else
			kbdstate &= ~STATE_LSHIFT;
	  	return (0);
	} else
	if (key == RIGHTSHIFTKEY) {
		if (stroke)
			kbdstate |= STATE_RSHIFT;
		else
			kbdstate &= ~STATE_RSHIFT;
	  	return (0);
	} else
	if (key == CTRLKEY) {
		if (stroke)
			kbdstate |= STATE_CTRL;
		else
			kbdstate &= ~STATE_CTRL;
	  	return (0);
	} else
	if (key == CAPSLOCKKEY) {
		if (stroke == 0)
			kbdstate ^= STATE_CAPSLOCK;
		return (0);
	}

	/* throw away key releases and invalid key codes */
	if ((stroke == 0) || (key > BUTCOUNT) || (key == SETUPKEY))
		return (0);

	switch (kbdstate) {
	  case 0:			/* normal key */
		ascii = kbuts[key].b_normal;
		break;
	  case STATE_LSHIFT:
	  case STATE_RSHIFT:
	  case STATE_LSHIFT+STATE_RSHIFT:
	  case STATE_LSHIFT+STATE_CAPSLOCK:
	  case STATE_RSHIFT+STATE_CAPSLOCK:
	  case STATE_LSHIFT+STATE_RSHIFT+STATE_CAPSLOCK:
		ascii = kbuts[key].b_shift;
		break;
	  case STATE_CTRL:
	  case STATE_CTRL+STATE_CAPSLOCK:
		ascii = kbuts[key].b_control;
		break;
	  case STATE_LSHIFT+STATE_CTRL:
	  case STATE_LSHIFT+STATE_CTRL+STATE_CAPSLOCK:
	  case STATE_RSHIFT+STATE_CTRL:
	  case STATE_RSHIFT+STATE_CTRL+STATE_CAPSLOCK:
	  case STATE_LSHIFT+STATE_RSHIFT+STATE_CTRL:
	  case STATE_LSHIFT+STATE_RSHIFT+STATE_CTRL+STATE_CAPSLOCK:
		ascii = kbuts[key].b_controlshift;
		break;
	  case STATE_CAPSLOCK:
		ascii = kbuts[key].b_normal;
		if ((ascii >= 'a') && (ascii <= 'z'))
			ascii = ascii - 32;
		break;
	}

	/* return "normal" keycode */
	if ((ascii & 0x80) == 0) {
		*buf = ascii;
		return (1);
	}

	/* if a break key, just return 0 */
	if (ascii == 0x80) {
		*buf = CODE_BREAK;
		return (1);
	}

	/* see if key is a special key (function key) */
	if ((ascii & 0x7f) <= NKEYS) {
		register struct key_data *kp;

		if (keypadmode)				/* HACK */
			kp = &funckeypad[(ascii & 0x7F) - 1];
		else
			kp = &funcnumeric[(ascii & 0x7F) - 1];
		cptr = (unsigned char *) (kp->k_name);
		for (i=kp->k_len; i--; )
			*buf++ = *cptr++;
		return (kp->k_len);
	}
	return (0);
}

/*
 * This procedure is invoked whenever input is changed into this process.
 * What we do is check and see if the user let go of (or pressed)
 * the shift key, the ctrl key, or the caps-lock key while we weren't
 * the input focus.
 */
kbfix()
{
	if (getbutton(LEFTSHIFTKEY))
		kbdstate |= STATE_LSHIFT;
	else
		kbdstate &= ~STATE_LSHIFT;
	if (getbutton(RIGHTSHIFTKEY))
		kbdstate |= STATE_RSHIFT;
	else
		kbdstate &= ~STATE_RSHIFT;
	if (getbutton(CTRLKEY))
		kbdstate |= STATE_CTRL;
	else
		kbdstate &= ~STATE_CTRL;
	/*
	 * We have to punt on the caps lock key, because we can't ask
	 * the keyboard what state it is in, and the button is a toggle.
	 * **SIGH**
	 */
}

/*
 * Bind a function key to a given string
 */
kbbindkey(key, cp)
	register char key;
	register char *cp;
{
	register int len;
	register char *buf;
	extern char *malloc();

	if ((key < MINBIND) || (key >= MAXBIND))
		return;				/* ignore */
	key -= MINBIND;

	cp++;			/* skip starting terminator character */
	len = strlen(cp);
	buf = malloc((unsigned) len + 1);
	(void) strncpy(buf, cp, len);

	if (keypadmode) {
		free(funckeypad[key].k_name);
		funckeypad[key].k_name = buf;
		funckeypad[key].k_len = len;
	} else {
		free(funcnumeric[key].k_name);
		funcnumeric[key].k_name = buf;
		funcnumeric[key].k_len = len;
	}
}

/*
 * Return the current binding of a function key.
 * The "key" argument is offset by MINBIND.
 */
int
kbgetbinding(key, buf)
	char key;
	char *buf;
{
	int len;

	if ((key < MINBIND) || (key >= MAXBIND))
		return (0);			/* ignore */
	key -= MINBIND;
	if (keypadmode) {
		len = funckeypad[key].k_len;
		(void) strncpy(buf, funckeypad[key].k_name, len);
	} else {
		len = funcnumeric[key].k_len;
		(void) strncpy(buf, funcnumeric[key].k_name, len);
	}
	return  (len);
}
