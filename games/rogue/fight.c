static	char	*Fight_c	= "@(#)fight.c	1.11";
/*
 * All the fighting gets done here
 *
 * @(#)fight.c	3.28 (Berkeley) 6/15/81
 */

#include <ctype.h>
#include "rogue.h"

long e_levels[] = {
    10L,20L,40L,80L,160L,320L,640L,1280L,2560L,5120L,10240L,20480L,
    40920L, 81920L, 163840L, 327680L, 655360L, 1310720L, 2621440L, 0L };

/*
 * fight:
 *	The player attacks the monster.
 */

fight(mp, mn, weap, thrown)
reg1 rega1	coord	*mp;
		char	mn;
		struct	object	*weap;
		bool	thrown;
{
    reg1 rega2	struct	thing	*tp;
    reg2 rega3	struct	linked_list *item;
    reg3 regd1	bool	did_hit = TRUE;

    /*
     * Find the monster we want to fight
     */
    if ((item = find_mons(mp->y, mp->x)) == NULL)
	debug("Fight what @ %d,%d", mp->y, mp->x);
    tp = (struct thing *) ldata(item);
    /*
     * Since we are fighting, things are not quiet so no healing takes
     * place.
     */
    quiet = 0;
    runto(mp, &hero);
    /*
     * Let him know it was really a mimic (if it was one).
     */
    if (tp->t_type == 'M' && tp->t_disguise != 'M' && off(player, ISBLIND)) {
#ifdef	COLOR
	acolor(cw, C_magenta, C_gold);
#endif
	msg("Wait! That's a mimic!");
	tp->t_disguise = 'M';
	did_hit = thrown;
    }
    if (did_hit) {
	char	*mname;

	did_hit = FALSE;
	if (on(player, ISBLIND))
	    mname = "it";
	else
	    mname = monsters[mn-'A'].m_name;
	if (roll_em(&pstats, &tp->t_stats, weap, thrown)) {
	    did_hit = TRUE;
	    if (thrown)
		thunk(weap, mname);
	    else
		hit(NULL, mname);
	    if (on(player, CANHUH)) {
#ifdef	COLOR
		acolor(cw, C_blue, Dbackground);
#endif
		msg("Your hands stop glowing red");
#ifdef	COLOR
		acolor(cw, C_blue, Dbackground);
#endif
		msg("The %s appears confused.", mname);
		tp->t_flags |= ISHUH;
		player.t_flags &= ~CANHUH;
	    }
	    if (tp->t_stats.s_hpt <= 0)
		killed(item, TRUE);
	} else
	    if (thrown)
		bounce(weap, mname);
	    else
		miss(NULL, mname);
    }
    count = 0;
    return did_hit;
}

/*
 * attack:
 *	The monster attacks the player
 */

attack(mp)
reg1 rega1	struct	thing	*mp;
{
    reg2 rega2	char	*mname;

    /*
     * Since this is an attack, stop running and any healing that was
     * going on at the time.
     */
    running = FALSE;
    quiet = 0;
    if (mp->t_type == 'M' && off(player, ISBLIND))
	mp->t_disguise = 'M';
    if (on(player, ISBLIND))
	mname = "it";
    else
	mname = monsters[mp->t_type-'A'].m_name;
    if (roll_em(&mp->t_stats, &pstats, NULL, FALSE)) {
	if (mp->t_type != 'E')
	    hit(mname, NULL);
	if (pstats.s_hpt <= 0)
	    death(mp->t_type);	/* Bye bye life ... */
	if (off(*mp, ISCANC))
	    switch (mp->t_type) {
		case 'R':
		    /*
		     * If a rust monster hits, you lose armor
		     */
		    if (cur_armor != NULL && cur_armor->o_ac < 9) {
#ifdef	COLOR
			acolor(cw, C_green, Dbackground);
#endif
			if (!terse) {
#ifdef	iris
			    msg("Your armor appears to be");
			    acolor(cw, C_green, Dbackground);
			    msg("weaker now. Oh my!");
#else
			    msg("Your armor appears to be weaker now. Oh my!");
#endif
			} else
			    msg("Your armor weakens");
			cur_armor->o_ac++;
		    }
		when 'E':
		    /*
		     * The gaze of the floating eye hypnotizes you
		     */
		    if (on(player, ISBLIND))
			break;
		    if (!no_command) {
#ifdef	COLOR
			acolor(cw, C_magenta, Dbackground);
#endif
#ifdef	iris
			msg("You are transfixed by the");
			acolor(cw, C_magenta, Dbackground);
			msg("gaze of the floating eye.");
#else
			msg("You are transfixed%s",
			  terse?"":" by the gaze of the floating eye.");
#endif
		    }
		    no_command += rnd(2)+2;
		when 'A':
		    /*
		     * Ants have poisonous bites
		     */
		    if (!save(VS_POISON))
			if (!ISWEARING(R_SUSTSTR)) {
			    chg_str(-1);
#ifdef	COLOR
			    acolor(cw, C_red, C_magenta);
#endif
			    if (terse)
				msg("A sting has weakened you");
			    else {
#ifdef	iris
			        msg("You feel a sting in your arm.");
			        acolor(cw, C_red, C_magenta);
			        msg("You now feel weaker.");
#else
				msg("You feel a sting in your arm and now %s",
				  "feel weaker");
#endif
			    }
			} else
			    if (!terse)
				msg("A sting momentarily weakens you");
			    else
				msg("Sting has no effect");
		when 'W':
		    /*
		     * Wraiths might drain energy levels
		     */
		    if (rnd(100) < 15) {
			int	fewer;

			if (pstats.s_exp == 0)
			    death('W');		/* All levels gone */
#ifdef	COLOR
			acolor(cw, C_magenta, Dforeground);
#endif
			msg("You suddenly feel weaker.");
			if (--pstats.s_lvl == 0) {
			    pstats.s_exp = 0;
			    pstats.s_lvl = 1;
			} else
			    pstats.s_exp = e_levels[pstats.s_lvl-1]+1;
			fewer = roll(1, 10);
			pstats.s_hpt -= fewer;
			max_hp -= fewer;
			if (pstats.s_hpt < 1)
			    pstats.s_hpt = 1;
			if (max_hp < 1)
			    death('W');
		    }
		when 'F':
		    /*
		     * Violet fungi stops the poor guy from moving
		     */
		    player.t_flags |= ISHELD;
		    sprintf(monsters['F'-'A'].m_stats.s_dmg,"%dd1",++fung_hit);
		when 'L':
		{
		    /*
		     * Leperachaun steals some gold
		     */
		    long lastpurse;

		    lastpurse = purse;
		    purse -= GOLDCALC;
		    if (!save(VS_MAGIC))
			purse -= GOLDCALC + GOLDCALC + GOLDCALC + GOLDCALC;
		    if (purse < 0)
			purse = 0;
		    if (purse != lastpurse) {
#ifdef	COLOR
			acolor(cw, C_gold, Dforeground);
#endif
			msg("Your purse feels lighter");
		    }
		    remove(&mp->t_pos, find_mons(mp->t_pos.y, mp->t_pos.x));
		}
		when 'N':
		{
		    struct	linked_list *list, *steal;
		    struct	object	*obj;
		    int	nobj;

		    /*
		     * Nymph's steal a magic item, look through the pack
		     * and pick out one we like.
		     */
		    steal = NULL;
		    for (nobj=0, list=pack; list != NULL; list=next(list)) {
			obj = (struct object *) ldata(list);
			if (obj != cur_armor && obj != cur_weapon &&
			  is_magic(obj) && rnd(++nobj) == 0)
				steal = list;
		    }
		    if (steal != NULL) {
			struct	object	*obj;

			obj = (struct object *) ldata(steal);
			remove(&mp->t_pos, find_mons(mp->t_pos.y, mp->t_pos.x));
#ifdef	COLOR
			acolor(cw, C_magenta, C_blue);
#endif
			if (obj->o_count > 1 && obj->o_group == 0) {
			    int	oc;

			    oc = obj->o_count--;
			    obj->o_count = 1;
			    msg("She stole %s!", inv_name(obj, TRUE));
			    obj->o_count = oc;
			} else {
			    msg("She stole %s!", inv_name(obj, TRUE));
			    detach(pack, steal);
			    discard(steal);
			}
			inpack--;
		    }
		}
		otherwise:
		    break;
	    }
    } else if (mp->t_type != 'E') {
	if (mp->t_type == 'F') {
	    pstats.s_hpt -= fung_hit;
	    if (pstats.s_hpt <= 0)
		death(mp->t_type);	/* Bye bye life ... */
	}
	miss(mname, NULL);
    }
    /*
     * Check to see if this is a regenerating monster and let it heal if
     * it is.
     */
    if (on(*mp, ISREGEN) && rnd(100) < 33)
	mp->t_stats.s_hpt++;
    if (fight_flush)
	flushi();	/* flush typeahead */
    count = 0;
    status();		/* may want to remove: it may be done in command() */
}

/*
 * swing:
 *	returns true if the swing hits
 */

swing(at_lvl, op_arm, wplus)
int	at_lvl, op_arm, wplus;
{
    reg1 regd1	int	res = rnd(20)+1;
    reg2 regd2	int	need = (21-at_lvl)-op_arm;

    return (res+wplus >= need);
}

/*
 * check_level:
 *	Check to see if the guy has gone up a level.
 */

check_level()
{
    reg1 regd1	int	i;
    reg2 regd2	int	add;

    for (i = 0; e_levels[i] != 0; i++)
	if (e_levels[i] > pstats.s_exp)
	    break;
    i++;
    if (i > pstats.s_lvl) {
	add = roll(i-pstats.s_lvl,10);
	max_hp += add;
	if ((pstats.s_hpt += add) > max_hp)
	    pstats.s_hpt = max_hp;
#ifdef	COLOR
	acolor(cw, C_magenta, Dbackground);
#endif
	msg("Welcome to level %d", i);
    }
    pstats.s_lvl = i;
}

/*
 * roll_em:
 *	Roll several attacks
 */

roll_em(att, def, weap, hurl)
struct	stats *att, *def;
struct	object	*weap;
bool	hurl;
{
    reg1 rega1	char	*cp;
    reg2 regd1	int	ndice;
    reg3 regd2	int	nsides;
    reg4 regd3	int	def_arm;
    reg5 regd4	bool	did_hit = FALSE;
    reg6 regd5	int	prop_hplus;
    reg7 regd6	int	prop_dplus;
		char	*index();

    prop_hplus = prop_dplus = 0;
    if (weap == NULL)
	cp = att->s_dmg;
    else if (hurl)
	if ((weap->o_flags&ISMISL) && cur_weapon != NULL &&
	  cur_weapon->o_which == weap->o_launch) {
	    cp = weap->o_hurldmg;
	    prop_hplus = cur_weapon->o_hplus;
	    prop_dplus = cur_weapon->o_dplus;
	} else
	    cp = (weap->o_flags&ISMISL ? weap->o_damage : weap->o_hurldmg);
    else {
	cp = weap->o_damage;
	/*
	 * Drain a staff of striking
	 */
	if (weap->o_type == WAND && weap->o_which == WS_HIT
	  && weap->o_charges == 0) {
		    weap->o_damage = "0d0";
		    weap->o_hplus = weap->o_dplus = 0;
		}
    }
    for (;;) {
	int	damage;
	int	hplus = prop_hplus + (weap == NULL ? 0 : weap->o_hplus);
	int	dplus = prop_dplus + (weap == NULL ? 0 : weap->o_dplus);

	if (weap == cur_weapon) {
	    if (ISRING(LEFT, R_ADDDAM))
		dplus += cur_ring[LEFT]->o_ac;
	    else if (ISRING(LEFT, R_ADDHIT))
		hplus += cur_ring[LEFT]->o_ac;
	    if (ISRING(RIGHT, R_ADDDAM))
		dplus += cur_ring[RIGHT]->o_ac;
	    else if (ISRING(RIGHT, R_ADDHIT))
		hplus += cur_ring[RIGHT]->o_ac;
	}
	ndice = atoi(cp);
	if ((cp = index(cp, 'd')) == NULL)
	    break;
	nsides = atoi(++cp);
	if (def == &pstats) {
	    if (cur_armor != NULL)
		def_arm = cur_armor->o_ac;
	    else
		def_arm = def->s_arm;
	    if (ISRING(LEFT, R_PROTECT))
		def_arm -= cur_ring[LEFT]->o_ac;
	    else if (ISRING(RIGHT, R_PROTECT))
		def_arm -= cur_ring[RIGHT]->o_ac;
	} else
	    def_arm = def->s_arm;
	if (swing(att->s_lvl, def_arm, hplus+str_plus(&att->s_str))) {
	    int	proll;

	    proll = roll(ndice, nsides);
	    if (ndice + nsides > 0 && proll < 1)
		debug("Damage for %dd%d came out %d.", ndice, nsides, proll);
	    damage = dplus + proll + add_dam(&att->s_str);
	    def->s_hpt -= max(0, damage);
	    did_hit = TRUE;
	}
	if ((cp = index(cp, '/')) == NULL)
	    break;
	cp++;
    }
    return did_hit;
}

/*
 * prname:
 *	The print name of a combatant
 */

char	*
prname(who, upper)
reg1 rega1	char	*who;
		bool	upper;
{
    static	char	tbuf[80];

    *tbuf = '\0';
    if (who == 0)
	strcpy(tbuf, "you"); 
    else if (on(player, ISBLIND))
	strcpy(tbuf, "it");
    else {
	strcpy(tbuf, "the ");
	strcat(tbuf, who);
    }
    if (upper)
	*tbuf = toupper(*tbuf);
    return tbuf;
}

/*
 * hit:
 *	Print a message to indicate a succesful hit
 */

hit(er, ee)
reg1 rega1	char	*er;
reg2 rega2	char	*ee;
{
    reg3 rega3	char	*s;

    strcpy(prbuf, prname(er, TRUE));
    if (terse)
	s = " hit.";
    else
	switch (rnd(4)) {
	    case 0: s = " scored an excellent hit on ";
	    when 1: s = " hit ";
	    when 2: s = (er == NULL ? " have injured " : " has injured ");
	    when 3: s = (er == NULL ? " swing and hit " : " swings and hits ");
	}
    strcat(prbuf, s);
    if (!terse)
	strcat(prbuf, prname(ee, FALSE));
#ifdef	COLOR
    if (er == NULL)
	acolor(cw, C_red, Dbackground);
    else
	acolor(cw, Dbackground, C_red);
#endif
    msg(prbuf);
}

/*
 * miss:
 *	Print a message to indicate a poor swing
 */

miss(er, ee)
reg1 rega1	char	*er;
reg2 rega2	char	*ee;
{
    reg3 rega3	char	*s;

    strcpy(prbuf, prname(er, TRUE));
    switch (terse ? 0 : rnd(4)) {
	case 0: s = (er == NULL ? " miss" : " misses");
	when 1: s = (er == NULL ? " swing and miss" : " swings and misses");
	when 2: s = (er == NULL ? " barely miss" : " barely misses");
	when 3: s = (er == NULL ? " don't hit" : " doesn't hit");
    }
    strcat(prbuf, s);
    if (!terse)
	sprintf(prbuf+strlen(prbuf), " %s", prname(ee, FALSE));
#ifdef	COLOR
    if (er == NULL)
	acolor(cw, C_green, Dbackground);
    else
	acolor(cw, Dbackground, C_green);
#endif
    msg(prbuf);
}

/*
 * save_throw:
 *	See if a creature save against something
 */
save_throw(which, tp)
int	which;
struct	thing	*tp;
{
    reg1 regd1	int	need;

    need = 14 + which - tp->t_stats.s_lvl / 2;
    return (roll(1, 20) >= need);
}
/*
 * save:
 *	See if he saves against various nasty things
 */

save(which)
int	which;
{
    return save_throw(which, &player);
}

/*
 * str_plus:
 *	compute bonus/penalties for strength on the "to hit" roll
 */

str_plus(str)
reg1 rega1	str_t *str;
{
    if (str->st_str == 18) {
	if (str->st_add == 100)
	    return 3;
	if (str->st_add > 50)
	    return 2;
    }
    if (str->st_str >= 17)
	return 1;
    if (str->st_str > 6)
	return 0;
    return str->st_str - 7;
}

/*
 * add_dam:
 *	compute additional damage done for exceptionally high or low strength
 */

add_dam(str)
reg1 rega1	str_t *str;
{
    if (str->st_str == 18) {
	if (str->st_add == 100)
	    return 6;
	if (str->st_add > 90)
	    return 5;
	if (str->st_add > 75)
	    return 4;
	if (str->st_add != 0)
	    return 3;
	return 2;
    }
    if (str->st_str > 15)
	return 1;
    if (str->st_str > 6)
	return 0;
    return str->st_str - 7;
}

/*
 * raise_level:
 *	The guy just magically went up a level.
 */

raise_level()
{
    pstats.s_exp = e_levels[pstats.s_lvl-1] + 1L;
    check_level();
}

/*
 * thunk:
 *	A missile hits a monster
 */

thunk(weap, mname)
reg1 rega1	struct	object	*weap;
reg2 rega2	char	*mname;
{
#ifdef	COLOR
    acolor(cw, C_red, Dbackground);
#endif
    if (weap->o_type == WEAPON)
	msg("The %s hits the %s", w_names[weap->o_which], mname);
    else
	msg("You hit the %s.", mname);
}

/*
 * bounce:
 *	A missile misses a monster
 */

bounce(weap, mname)
reg1 rega1	struct	object	*weap;
reg2 rega2	char	*mname;
{
#ifdef	COLOR
    acolor(cw, C_green, Dbackground);
#endif
    if (weap->o_type == WEAPON)
	msg("The %s misses the %s", w_names[weap->o_which], mname);
    else
	msg("You missed the %s.", mname);
}

/*
 * remove a monster from the screen
 */
remove(mp, item)
reg1 rega1	coord	*mp;
reg2 rega2	struct	linked_list *item;
{
    mvwaddch(mw, mp->y, mp->x, ' ');
#ifdef	COLOR
    Dcolor(cw, ((struct thing *) ldata(item))->t_oldch);
#endif
    mvwaddch(cw, mp->y, mp->x, ((struct thing *) ldata(item))->t_oldch);
#ifdef	COLOR
    acolor(cw, Dforeground, Dbackground);
#endif
    detach(mlist, item);
    discard(item);
}

/*
 * is_magic:
 *	Returns true if an object radiates magic
 */

is_magic(obj)
reg1 rega1	struct	object	*obj;
{
    switch (obj->o_type) {
	case ARMOR:
	    return obj->o_ac != a_class[obj->o_which];
	when WEAPON:
	    return obj->o_hplus != 0 || obj->o_dplus != 0;
	when POTION:
	case SCROLL:
	case WAND:
	case RING:
	case AMULET:
	    return TRUE;
    }
    return FALSE;
}

/*
 * killed:
 *	Called to put a monster to death
 */

killed(item, pr)
reg1 rega1	struct	linked_list *item;
		bool	pr;
{
    reg2 rega2	struct	thing	*tp;
    reg3 rega3	struct	linked_list *pitem;
    reg4 rega4	struct	linked_list *nexti;

    tp = (struct thing *) ldata(item);
    if (pr) {
#ifdef	COLOR
	acolor(cw, Dbackground, Dforeground);
#endif
	msg(
	  on(player, ISBLIND)?"%sit.":"%s%s%s",
	  terse ? "Defeated " : "You have defeated ",
	    terse?"":"the ",
	    monsters[tp->t_type-'A'].m_name);
    }
    pstats.s_exp += tp->t_stats.s_exp;
    /*
     * Do adjustments if he went up a level
     */
    check_level();
    /*
     * If the monster was a violet fungi, un-hold him
     */
    switch (tp->t_type) {
	case 'F':
	    player.t_flags &= ~ISHELD;
	    fung_hit = 0;
	    strcpy(monsters['F'-'A'].m_stats.s_dmg, "000d0");
	when 'L': {
	    struct	room	*rp;

	    if ((rp = roomin(&tp->t_pos)) == NULL)
		break;
	    if (rp->r_goldval != 0 || fallpos(&tp->t_pos,&rp->r_gold,FALSE)) {
		rp->r_goldval += GOLDCALC;
		if (save(VS_MAGIC))
		    rp->r_goldval += GOLDCALC + GOLDCALC
				   + GOLDCALC + GOLDCALC;
		mvwaddch(stdscr, rp->r_gold.y, rp->r_gold.x, GOLD);
		if (!(rp->r_flags & ISDARK)) {
		    light(&hero);
#ifdef	COLOR
		    Dcolor(cw, PLAYER);
#endif
		    mvwaddch(cw, hero.y, hero.x, PLAYER);
#ifdef	COLOR
		    acolor(cw, Dforeground, Dbackground);
#endif
		}
	    }
	}
    }
    /*
     * Empty the monsters pack
     */
    pitem = tp->t_pack;
    /*
     * Get rid of the monster.
     */
    remove(&tp->t_pos, item);
    while (pitem != NULL) {
	struct	object	*obj;

	nexti = next(tp->t_pack);
	obj = (struct object *) ldata(pitem);
	obj->o_pos = tp->t_pos;
	detach(tp->t_pack, pitem);
	fall(pitem, FALSE);
	pitem = nexti;
    }
}
