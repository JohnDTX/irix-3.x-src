/*
 * $Source: /d2/3.7/src/stand/lib/dev/RCS/keybd.c,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:14:35 $
 */
#include "iriskeybd.h"

#ifdef notdef
#undef isascii
#define isascii(x)	(!((x)&0200))
#endif

extern short	kbdstate;
extern char	kbdcntrlstate;
extern unsigned char	kbdtype;

/*
** _duputchar
**   print a character on the keyboard (duart 0 port A)
**   If this routine is called we assume this must be the console, so
**   we even support flow control of a sorts
*/
keyputchar( c )
register	c;
{


	_duputc( c, 0 );

}

/*
** These two variables represent the total count and current count of
** characters translated.  This has to be done because some keys
** can be xlated into more that one character
*/
int	_cntt,
	_cntc;
char	buff0[ 10 ],
	buff1[ 10 ];

/*
** _keygetchar
**   get a character on the console (duart 0 port A)
**	XXX - check nowait stuff
*/   
keygetchar(nowaitflag)
{
	int	c;

	if ( _cntt == 0 || _cntc >= _cntt )
	{
		_cntt = _cntc = 0;
		if ( nowaitflag )
		{
			if ( ( c = _dugetc( 0, 2 ) ) < 0 )
				return ( -1 );
				
			_cntt = dutranslate( c, buff0 );
		}
		else
		{
			do
			{
				_cntt = dutranslate( _dugetc( 0, 0 ) & 0xFF, buff0 );
			} while ( _cntt == 0 );
		}
	}
	return( (unsigned)buff0[ _cntc++ ] );
}

int
dutranslate( key, buf )
register int	key;
register char	*buf;
{
    register int ascii;
    register int stroke;
    register int indx;

    stroke = ( ( key & 0x80 ) != 0x80 );
    key &= ~0x80;
    key++;

    /*
     * Caps locked toggles when RIGHTSHIFT pressed while CAPSLOCK is down 
     */
    if (key == KEY_RIGHT_SHIFT) {
	if (stroke == 1 && (kbdstate&STATE_CAPSLOCK)) {
	    kbdstate ^= STATE_CAPSLOCK;
	    if (kbdstate & STATE_CAPSLOCK)
		lampon(LAMP_CAPSLOCK);
	    else
		lampoff(LAMP_CAPSLOCK);
	}
	/* return(0);  -- can''t return: CAPSLOCK == META */
    }

    /*
     * Handle shift, control, and caps lock keys.
     */
    if (key == KEY_LEFT_SHIFT) {
	if (stroke)
	    kbdstate |= STATE_LSHIFT;
	else
	    kbdstate &= ~STATE_LSHIFT;
	return (0);
    }
    else if (key == KEY_RIGHT_SHIFT) {
	if (stroke)
	    kbdstate |= STATE_RSHIFT;
	else
	    kbdstate &= ~STATE_RSHIFT;
	return (0);
    }
    else if (key == KEY_CTRL) {
	if (stroke)
	    kbdstate |= STATE_LCTRL;
	else
	    kbdstate &= ~STATE_LCTRL;
	return (0);
    }
    else if (key == (KEY_RIGHT_CTRL)) {
	if (stroke)
	    kbdstate |= STATE_RCTRL;
	else
	    kbdstate &= ~STATE_RCTRL;
	return (0);
    }
    else if (key == KEY_CAPSLOCK) {
	if (stroke == 0)
	{
	    kbdstate ^= STATE_CAPSLOCK;
	    if ( kbdstate & STATE_CAPSLOCK )
		lampon( LAMP_CAPSLOCK );
	    else
		lampoff( LAMP_CAPSLOCK );
	}
	return (0);
    }
    if ( kbdtype == KBD_4D60ISO )
    {
	    if ( key == KEY_LEFT_ALT )
	    {
		if ( stroke )
			kbdstate |= STATE_LALT;
		else
			kbdstate &= ~STATE_LALT;
		return ( 0 );
	    }
	    else
	    if ( key == KEY_RIGHT_ALT )
	    {
		if ( stroke )
			kbdstate |= STATE_RALT;
		else
			kbdstate &= ~STATE_RALT;
		return ( 0 );
	    }
    }
    if ( kbdtype == KBD_IRIS )
    {
	    if (key == KEY_SETUP) {
		if (stroke)
		    kbdstate |= STATE_SETUP;
		else {
		    kbdstate &= ~STATE_SETUP;
		}
		return (0);
	    }
    }


    /* throw away key releases and invalid key codes */
    if ((stroke == 0) || (key > BUTCOUNT))
	return (0);

    /* XXX add special SETUP actions */
    switch (kbdstate)
    {
      case 0:			/* normal key */
	if ( ( indx = kbuts[ key ].b_gs_index ) != 0 )
	{
		switch ( kbdtype )
		{
		   case KBD_IRIS:
			ascii = kbuts[ key ].b_normal;
			break;

		   case KBD_4D60ISO:
			ascii = kbuts_iso[ indx ].b_normal;
			break;

		   case KBD_4D60STD:
			ascii = kbuts_std[ indx ].b_normal;
			break;
		}
	}
	else
		ascii = kbuts[key].b_normal;
	break;
      case STATE_LSHIFT:
      case STATE_RSHIFT:
      case STATE_LSHIFT + STATE_RSHIFT:
      case STATE_LSHIFT + STATE_CAPSLOCK:
      case STATE_RSHIFT + STATE_CAPSLOCK:
      case STATE_LSHIFT + STATE_RSHIFT + STATE_CAPSLOCK:
	if ( ( indx = kbuts[ key ].b_gs_index ) != 0 )
	{
		switch ( kbdtype )
		{
		   case KBD_IRIS:
			ascii = kbuts[ key ].b_shift;
			break;

		   case KBD_4D60ISO:
			ascii = kbuts_iso[ indx ].b_shift;
			break;

		   case KBD_4D60STD:
			ascii = kbuts_std[ indx ].b_shift;
			break;
		}
	}
	else
		ascii = kbuts[key].b_shift;
	break;
      case STATE_LCTRL:
      case STATE_LCTRL + STATE_CAPSLOCK:
      case STATE_RCTRL:
      case STATE_RCTRL + STATE_CAPSLOCK:
      case STATE_RCTRL + STATE_LCTRL:
      case STATE_RCTRL + STATE_LCTRL + STATE_CAPSLOCK:
	if ( ( indx = kbuts[ key ].b_gs_index ) != 0 )
	{
		switch ( kbdtype )
		{
		   case KBD_IRIS:
			ascii = kbuts[ key ].b_control;
			break;

		   case KBD_4D60ISO:
			ascii = kbuts_iso[ indx ].b_control;
			break;

		   case KBD_4D60STD:
			ascii = kbuts_std[ indx ].b_control;
			break;
		}
	}
	else
		ascii = kbuts[key].b_control;
	break;
      case STATE_LSHIFT + STATE_LCTRL:
      case STATE_LSHIFT + STATE_LCTRL + STATE_CAPSLOCK:
      case STATE_RSHIFT + STATE_LCTRL:
      case STATE_RSHIFT + STATE_LCTRL + STATE_CAPSLOCK:
      case STATE_LSHIFT + STATE_RSHIFT + STATE_LCTRL:
      case STATE_LSHIFT + STATE_RSHIFT + STATE_LCTRL + STATE_CAPSLOCK:
      case STATE_LSHIFT + STATE_RCTRL:
      case STATE_LSHIFT + STATE_RCTRL + STATE_CAPSLOCK:
      case STATE_RSHIFT + STATE_RCTRL:
      case STATE_RSHIFT + STATE_RCTRL + STATE_CAPSLOCK:
      case STATE_LSHIFT + STATE_RSHIFT + STATE_RCTRL:
      case STATE_LSHIFT + STATE_RSHIFT + STATE_RCTRL + STATE_CAPSLOCK:
      case STATE_LSHIFT + STATE_RSHIFT + STATE_LCTRL + STATE_RCTRL + STATE_CAPSLOCK:
	if ( ( indx = kbuts[ key ].b_gs_index ) != 0 )
	{
		switch ( kbdtype )
		{
		   case KBD_IRIS:
			ascii = kbuts[ key ].b_controlshift;
			break;

		   case KBD_4D60ISO:
			ascii = kbuts_iso[ indx ].b_controlshift;
			break;

		   case KBD_4D60STD:
			ascii = kbuts_std[ indx ].b_controlshift;
			break;
		}
	}
	else
		ascii = kbuts[key].b_controlshift;
	break;
      case STATE_CAPSLOCK:
	if ( ( indx = kbuts[ key ].b_gs_index ) != 0 )
	{
		switch ( kbdtype )
		{
		   case KBD_IRIS:
			ascii = kbuts[ key ].b_normal;
			break;

		   case KBD_4D60ISO:
			ascii = kbuts_iso[ indx ].b_normal;
			break;

		   case KBD_4D60STD:
			ascii = kbuts_std[ indx ].b_normal;
			break;
		}
	}
	else
		ascii = kbuts[key].b_normal;
	if ( islower( ascii ) )
		ascii = _toupper( ascii );
	break;

     case STATE_LALT:
     case STATE_LALT + STATE_RSHIFT:
     case STATE_LALT + STATE_LSHIFT:
     case STATE_LALT + STATE_RSHIFT + STATE_LSHIFT:
     case STATE_LALT + STATE_CAPSLOCK:
     case STATE_LALT + STATE_RSHIFT + STATE_CAPSLOCK:
     case STATE_LALT + STATE_LSHIFT + STATE_CAPSLOCK:
     case STATE_LALT + STATE_RSHIFT + STATE_LSHIFT + STATE_CAPSLOCK:

     case STATE_RALT:
     case STATE_RALT + STATE_RSHIFT:
     case STATE_RALT + STATE_LSHIFT:
     case STATE_RALT + STATE_RSHIFT + STATE_LSHIFT:
     case STATE_RALT + STATE_CAPSLOCK:
     case STATE_RALT + STATE_RSHIFT + STATE_CAPSLOCK:
     case STATE_RALT + STATE_LSHIFT + STATE_CAPSLOCK:
     case STATE_RALT + STATE_RSHIFT + STATE_LSHIFT + STATE_CAPSLOCK:

     case STATE_RALT + STATE_LALT:
     case STATE_RALT + STATE_LALT + STATE_RSHIFT:
     case STATE_RALT + STATE_LALT + STATE_LSHIFT:
     case STATE_RALT + STATE_LALT + STATE_RSHIFT + STATE_LSHIFT:
     case STATE_RALT + STATE_LALT + STATE_CAPSLOCK:
     case STATE_RALT + STATE_LALT + STATE_RSHIFT + STATE_CAPSLOCK:
     case STATE_RALT + STATE_LALT + STATE_LSHIFT + STATE_CAPSLOCK:
     case STATE_RALT + STATE_LALT + STATE_RSHIFT + STATE_LSHIFT + STATE_CAPSLOCK:

	if ( ( indx = kbuts[ key ].b_gs_index ) != 0 )
	{
		switch ( kbdtype )
		{
		   case KBD_IRIS:
			return ( 0 );

		   case KBD_4D60ISO:
			ascii = kbuts_iso[ indx ].b_alt;
			break;

		   case KBD_4D60STD:
			ascii = kbuts_iso[ indx ].b_alt;
			break;
		}
	}
	else
		ascii = kbuts[key].b_alt;
	break;

	default:
		return ( 0 );
    }

    /* return "normal" keycode */
	if (((ascii & 0x80) == 0)
	   || ((ascii & 0x7f) > N_KPKEYS)) {
	    *buf = ascii;
	    return (1);
    }

    /* if a break key, return special code */
    if (ascii == 0x80) {
	*buf = CODE_BREAK;
	return (1);
    }

    /*
     * See if key is from the keypad
     */
    if ((ascii & 0x7f) <= N_KPKEYS) {
	register struct key_data *kp;
	register int i;
	register char *cp;

	kp = &keypad_numeric[ ( ascii & 0x7f ) - 1 ];
	i = kp->k_len;
	cp = kp->k_name;
	while (i-- > 0)
	    *buf++ = *cp++;
	return (kp->k_len);
    }

    return (0);
}

beep()
{
	if ( kbdtype == KBD_4D60STD )
	{
		keyputchar(kbdcntrlstate | NEWKB_SHORTBEEP);
	}
	else
	if ( kbdtype == KBD_IRIS )
	{
		keyputchar(kbdcntrlstate | KBD_BEEPCMD | KBD_SHORTBEEP);
	}
}

lampon( val )
{
	lampfunc( 1, val );
}

lampoff( val )
{
	lampfunc( 0, val );
}

static
lampfunc( action, val )
{
	switch ( val )
	{
	   case LAMP_CAPSLOCK:
		if ( action )
			kbdcntrlstate |= 0x40;
		else
			kbdcntrlstate &= ~0x40;
		keyputchar( kbdcntrlstate & 0xff );
		break;

	   case LAMP_LOCAL:
	   case LAMP_KBDLOCKED:
		break;
	}
}
