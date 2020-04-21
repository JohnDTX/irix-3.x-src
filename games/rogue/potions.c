/*
 * 	@(#)potions.c	3.1	3.1	5/7/81
 *	%W% %G%
 * Function(s) for dealing with potions
 */

#include "rogue.h"

quaff()
{
    reg1 rega1	struct	object	*obj;
    reg2 rega2	struct	linked_list *item;
    reg3 rega3	struct	linked_list *titem;
    reg4 rega4	struct	thing	*th;
		char	buf[80];

    item = get_item("quaff", POTION);
    /*
     * Make certain that it is somethings that we want to drink
     */
    if (item == NULL)
	return;
    obj = (struct object *) ldata(item);
    if (obj->o_type != POTION) {
	if (!terse)
	    msg("Yuk! Why would you want to drink that?");
	else
	    msg("That's undrinkable");
	return;
    }
    if (obj == cur_weapon)
	cur_weapon = NULL;

    /*
     * Calculate the effect it has on the poor guy.
     */
    switch (obj->o_which) {
	case P_CONFUSE:
	    if (off(player, ISHUH)) {
#ifdef	iris
		msg("Wait, what's going on here");
		msg("Huh? What? Who?");
#else
		msg("Wait, what's going on here. Huh? What? Who?");
#endif
		if (on(player, ISHUH))
		    lengthen(unconfuse, rnd(8)+HUHDURATION);
		else
		    fuse(unconfuse, 0, rnd(8)+HUHDURATION, AFTER);
		player.t_flags |= ISHUH;
	    }
	    p_know[P_CONFUSE] = TRUE;
	when P_POISON:
#ifdef	COLOR
	    acolor(cw, C_yellow, Dbackground);
#endif
	    if (!ISWEARING(R_SUSTSTR)) {
		chg_str(-(rnd(3)+1));
		msg("You feel very sick now.");
	    } else
		msg("You feel momentarily sick");
	    p_know[P_POISON] = TRUE;
	when P_HEALING:
	    if ((pstats.s_hpt += roll(pstats.s_lvl, 4)) > max_hp)
		pstats.s_hpt = ++max_hp;
#ifdef	COLOR
	    acolor(cw, C_green, Dbackground);
#endif
	    msg("You begin to feel better.");
	    sight();
	    p_know[P_HEALING] = TRUE;
	when P_STRENGTH:
#ifdef	iris
	    msg("You feel stronger, now");
	    msg("Gosh, what bulging muscles");
#else
	    msg("You feel stronger, now. Gosh, what bulging muscles");
#endif
	    chg_str(1);
	    p_know[P_STRENGTH] = TRUE;
	when P_MFIND:
	    /*
	     * Potion of MONSTER detection, if there are monsters, detect them
	     */
	    if (mlist != NULL) {
		wclear(hw);
		overmerge(mw, hw);	/* overlay with correct colors */
#ifdef	iris
		if (_iris) {
		    touchwin(hw);
/*		    wmove(hw, hero.y, hero.x); */
		    Draw(hw);
		    color(C_red);
		    icursor(0,0);
		    font(_nowfont=_linfont[0]);	/* Old English	*/
		    charstr("You feel the presence of monsters");
		    color(Dforeground);
		    font(_nowfont=_linfont[1]);	/* Standard	*/
		    charstr(more);
		    color(Dforeground);
		    wait_for(' ');
		    clearok(cw, TRUE);
		    touchwin(cw);
		    Draw(cw);
		} else {
#endif
		    mvwaddstr(hw, 0, 0,
		      "You feel the presence of monsters-More-");
		    show_win(hw, NULL);
#ifdef	iris
		}
#endif
		p_know[P_MFIND] = TRUE;
#ifdef	COLOR
		acolor(hw, Dforeground, Dbackground);
#endif
	    } else {
		msg("You have a strange feeling");
		msg("It passes");
	    }
	when P_TFIND:
	    /*
	     * Potion of magic detection.  Show the potions and scrolls
	     */
	    if (lvl_obj != NULL) {
		struct	linked_list *mobj;
		struct	object	*tp;
		int	magic;

		magic = 0;
		wclear(hw);
		for (mobj = lvl_obj; mobj != NULL; mobj = next(mobj)) {
		    tp = (struct object *) ldata(mobj);
		    if (is_magic(tp)) {
			magic++;
#ifdef	iris
			if (!_iris)
#endif
			    mvwaddch(hw, tp->o_pos.y, tp->o_pos.x, MAGIC);
			p_know[P_TFIND] = TRUE;
		    }
		}
		if (magic) {
#ifdef	COLOR
		    acolor(hw, C_blue, Dbackground);
#endif
#ifdef	iris
		    msg(" ");
		    mpos = 0;
		    if (!_iris)
			wmsg(hw, "You sense magic on this level.-More-");
#else
		    mvwaddstr(hw, 0, 0,
		      "You sense the presence of magic on this level.--More--");
#endif
#ifdef	iris
		    if (_iris) {
			touchwin(hw);
			Draw(hw);
			Imfind(magic);
		    } else
#endif
			show_win(hw, NULL);
#ifdef	COLOR
		    acolor(hw, Dforeground, Dbackground);
#endif
		} else {
#ifdef	iris
		    msg("You have a strange feeling");
		    msg("In a moment it passes");
#else
		    msg("You have a strange feeling for a moment, %s",
		      "then it passes");
#endif
		}
	    }
	when P_PARALYZE:
	    msg("You can't move.");
	    no_command = HOLDTIME;
	    p_know[P_PARALYZE] = TRUE;
	when P_SEEINVIS:
	    msg("This potion tastes like %s juice.", fruit);
	    if (off(player, CANSEE)) {
		player.t_flags |= CANSEE;
		fuse(unsee, 0, SEEDURATION, AFTER);
		light(&hero);
	    }
	    sight();
	when P_RAISE:
	    msg("You suddenly feel much more skillful");
	    p_know[P_RAISE] = TRUE;
	    raise_level();
	when P_XHEAL:
	    if ((pstats.s_hpt += roll(pstats.s_lvl, 8)) > max_hp)
		pstats.s_hpt = ++max_hp;
	    msg("You begin to feel much better.");
	    p_know[P_XHEAL] = TRUE;
	    sight();
	when P_HASTE:
	    add_haste(TRUE);
	    msg("You feel yourself moving much faster.");
	    p_know[P_HASTE] = TRUE;
	when P_RESTORE:
#ifdef	iris
	    msg("Hey, this tastes great");
	    msg("It make you feel warm all over");
#else
	    msg("Hey, this tastes great; it make you feel warm all over");
#endif
	    if (pstats.s_str.st_str < max_stats.s_str.st_str ||
		(pstats.s_str.st_str == 18 &&
		 pstats.s_str.st_add < max_stats.s_str.st_add))
	    pstats.s_str = max_stats.s_str;
	when P_BLIND:
	    msg("A cloak of darkness falls around you.");
	    if (off(player, ISBLIND)) {
		player.t_flags |= ISBLIND;
		fuse(sight, 0, SEEDURATION, AFTER);
		look(FALSE);
	    }
	    p_know[P_BLIND] = TRUE;
	when P_NOP:
	    msg("This potion tastes extremely dull.");
	otherwise:
	    msg("What an odd tasting potion!");
	    return;
    }
    status();
    if (p_know[obj->o_which] && p_guess[obj->o_which]) {
	cfree(p_guess[obj->o_which]);
	p_guess[obj->o_which] = NULL;
    } else
	if (!p_know[obj->o_which]&&askme&&p_guess[obj->o_which] == NULL) {
#ifdef	iris
	    msg("Call it?");
#else
	    msg(terse ? "Call it: " : "What do you want to call it?");
#endif
	    if (get_str(buf, cw) == NORM) {
		p_guess[obj->o_which] = malloc((unsigned int) strlen(buf) + 1);
		strcpy(p_guess[obj->o_which], buf);
	    }
	}
    /*
     * Throw the item away
     */
    inpack--;
    if (obj->o_count > 1)
	obj->o_count--;
    else {
	detach(pack, item);
        discard(item);
    }
}
