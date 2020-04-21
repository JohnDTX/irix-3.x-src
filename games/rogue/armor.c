static	char	*Armor_c	= "@(#)armor.c	1.6 10/9/84";
/*
 * This file contains misc functions for dealing with armor
 * @(#)armor.c	2.7 (Berkeley) 2/9/81
 * @(#)armor.c	1.6 10/9/84 23:40:21
 */

#include "rogue.h"

/*
 * wear:
 *	The player wants to wear something, so let him/her put it on.
 */

wear()
{
    reg1 rega1	struct	linked_list *item;
    reg2 rega2	struct	object	*obj;

    if (cur_armor != NULL) {
#ifdef	COLOR
	Dcolor(cw, ARMOR);
#endif
#ifdef	iris
	msg("You are already wearing some.");
	if (!terse) {
#ifdef	COLOR
	    Dcolor(cw, ARMOR);
#endif
	    msg("You'll have to take it off first");
	}
#else
	if (!terse)
	    msg("You are already wearing some.  \
You'll have to take it off first");
	else
	    msg("You are already wearing some");
#endif
	after = FALSE;
	return;
    }
    if ((item = get_item("wear", ARMOR)) == NULL)
	return;
    obj = (struct object *) ldata(item);
    if (obj->o_type != ARMOR) {
#ifdef	COLOR
	Dcolor(cw, ARMOR);
#endif
	msg("You can't wear that.");
	return;
    }
    waste_time();
#ifdef	COLOR
    Dcolor(cw, ARMOR);
#endif
    if (!terse)
	msg("You are now wearing %s.", a_names[obj->o_which]);
    else
	msg("Wearing %s.", a_names[obj->o_which]);
    cur_armor = obj;
    obj->o_flags |= ISKNOW;
}

/*
 * take_off:
 *	Get the armor off of the player's back
 */

take_off()
{
    reg1 rega1	struct	object	*obj;

    if (cur_armor == NULL) {
#ifdef	COLOR
	Dcolor(cw, ARMOR);
#endif
	msg("You aren't wearing any");
	return;
    }
    obj = cur_armor;
    if (!dropcheck(cur_armor))
	return;
    cur_armor = NULL;
#ifdef	COLOR
    Dcolor(cw, ARMOR);
#endif
    msg("Was wearing %c) %s", pack_char(obj), inv_name(obj, TRUE));
#ifdef	COLOR
    Dcolor(cw, ARMOR);
#endif
}

/*
 * waste_time:
 *	Do nothing but let other things happen
 */

waste_time()
{
    do_daemons(BEFORE);
    do_fuses(BEFORE);
    do_daemons(AFTER);
    do_fuses(AFTER);
}
