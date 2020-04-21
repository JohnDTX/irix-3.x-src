static	char	*Use_c	= "@(#)use.c	1.18";
/*
 * The routines are here for manipulating objects
 * @(#)use.c	3.5 (Berkeley) 6/15/81
 *
 * @(#)use.c	1.18 4/3/85
 */

#include <ctype.h>
#include "rogue.h"

char	*malloc();

/*
 * Read a scroll and let it happen
 */

read_scroll()
{
    reg1 rega1	struct	object	*obj;
    reg2 rega2	struct	linked_list *item;
    reg3 rega3	struct	room	*rp;
    reg4 regd1	int	i;
    reg5 regd2	int	j;
    reg6 regd3	char	ch;
    reg7 regd4	char	nch;
    reg8 rega4	struct	linked_list *titem;
		char	buf[80];
		int	used;		/* to see if we used it */

    used = 1;
    item = get_item("read", SCROLL);
#ifdef	COLOR
    Dcolor(cw, SCROLL);
#endif
    if (item == NULL)
	return;
    obj = (struct object *) ldata(item);
    if (obj->o_type != SCROLL) {
	if (!terse)
	    msg("There is nothing on it to read");
	else
	    msg("Nothing to read");
	return;
    }
    if (on(player,ISBLIND)) {
	msg("Blind %s can't read",rnd(1)?"men":"women");
	return;
    }
    msg("As you read the scroll, it vanishes");
    if (obj == cur_weapon)
	cur_weapon = NULL;
    /*
     * Calculate the effect it has on the poor guy.
     */
    switch (obj->o_which) {
	case S_CONFUSE:
	    /*
	     * Scroll of monster confusion.  Give him that power.
	     */
	    msg("Your hands begin to glow red");
	    player.t_flags |= CANHUH;
	when S_LIGHT:
	    s_know[S_LIGHT] = TRUE;
	    lights();
	when S_ARMOR:
	    if (cur_armor != NULL) {
		msg("Your armor glows faintly for a moment");
		cur_armor->o_ac--;
		cur_armor->o_flags &= ~ISCURSED;
	    }
	when S_HOLD:
	    /*
	     * Hold monster scroll.  Stop all monsters within two spaces
	     * from chasing after the hero.
	     */
	    {
		int	x, y;
		struct	linked_list *mon;

		for (x = hero.x-2; x <= hero.x+2; x++)
		    for (y = hero.y-2; y <= hero.y+2; y++)
			if (y >= 0 && x >= 0 && y < LINES && x < COLS
			  && isupper(mvwinch(mw, y, x)))
			    if ((mon = find_mons(y, x)) != NULL) {
				struct	thing	*th;

				th = (struct thing *) ldata(mon);
				th->t_flags &= ~ISRUN;
				th->t_flags |= ISHELD;
			    }
	    }
	when S_SLEEP:
	    /*
	     * Scroll which makes you fall asleep
	     */
	    s_know[S_SLEEP] = TRUE;
	    msg("You fall asleep");
	    no_command += 4 + rnd(SLEEPTIME);
	when S_CREATE:
	    /*
	     * Create a monster
	     * First look in a circle around him, next try his room
	     * otherwise give up
	     */
	    {
		int	x, y;
		bool	appear = 0;
		coord	mp;

		/*
		 * Search for an open place
		 */
		for (y = hero.y-1; y <= hero.y+1; y++)
		    for (x = hero.x-1; x <= hero.x+1; x++) {
			/*
			 * Don't put a monster in top of the player
			 * or anything else nasty
			 */
			if (y >= 0 && x >= 0 && y < LINES && x < COLS
			  && (y != hero.y || x != hero.x)
			  && step_ok(winat(y, x))) {
			    if (rnd(++appear) == 0) {
				mp.y = y;
				mp.x = x;
			    }
			}
		    }
		if (appear) {
		    titem = new_item(sizeof (struct thing));
		    new_monster(titem, randmonster(FALSE), &mp);
		} else
		    msg("You hear a faint cry of anguish in the distance");
	    }
	when S_IDENT:
	    /*
	     * Identify, let the rogue figure something out
	     */
	    msg("This scroll is an identify scroll");
	    s_know[S_IDENT] = TRUE;
	    used = whatis();
	when S_MAP:
	    /*
	     * Scroll of magic mapping.
	     */
	    s_know[S_MAP] = TRUE;
	    msg("Oh, now this scroll has a map on it");
#ifdef	COLOR
	    acolor(hw, C_magenta, Dbackground);
#endif
	    overwrite(stdscr, hw);	/* uncommented 06/10/84 */
#ifdef	COLOR
	    acolor(hw, Dforeground, Dbackground);
#endif
	    /*
	     * Take all the things we want to keep hidden out of the window
	     */
	    for (i = 0; i < LINES; i++)
		for (j = 0; j < COLS; j++) {
		    switch (nch = ch = mvwinch(hw, i, j)) {
		      case SECRETDOOR:
			mvaddch(i, j, nch = door);
		      when TRAP:
		      case POTION:
		      case SCROLL:
		      case GOLD:
		      case FOOD:
		      case WEAPON:
		      case ARMOR:
			nch = ' ';
			break;
		      default:
			if (ch == floor)
			    nch = ' ';
			break;
		    }
		    if (nch != ch)
			waddch(hw, nch);
		}
	    /*
	     * Copy in what he has discovered
	     */
	    overlay(cw, hw);
	    /*
	     * And set up for display
	     */
/*	    overwrite(hw, cw); */
			/* leave first and last lines alone */
	    for (i = 1; i < LINES - 1; i++)
		for (j = 0; j < COLS; j++) {
		    ch = mvwinch(hw, i, j);
		    Dcolor(cw, ch);
		    mvwaddch(cw, i, j, ch);
		}
	when S_GFIND:
	    /*
	     * Potion of gold detection
	     */
	    {
		int	gtotal = 0;
#ifdef	iris
		int	gpiles = 0;
#endif

		wclear(hw);
#ifdef	COLOR
		acolor(hw, C_gold, Dbackground);
#endif
		for (i = 0; i < MAXROOMS; i++) {
		    gtotal += rooms[i].r_goldval;
		    if (rooms[i].r_goldval != 0 &&
		      mvwinch(stdscr, rooms[i].r_gold.y, rooms[i].r_gold.x)
		      == GOLD) {
#ifdef	iris
			gpiles++;
			if (!_iris)
#endif
			    mvwaddch(hw,rooms[i].r_gold.y,rooms[i].r_gold.x,
			      GOLD);
		    }
		}
		if (gtotal) {
#ifdef	iris
		    msg(" ");
		    mpos = 0;
		    if (!_iris)
			wmsg(hw, "You begin feeling greedy.");
#else
		    mvwaddstr(hw, 0, 0,
		      "You begin to feel greedy and you sense gold.--More--");
#endif
		    s_know[S_GFIND] = TRUE;
#ifdef	iris
		    if (_iris) {
			touchwin(hw);
			Draw(hw);
			Igfind(gpiles);
		    } else
#endif
			show_win(hw, NULL);
		} else
		    msg("You begin to feel a pull downward");
#ifdef	COLOR
		acolor(hw, Dforeground, Dbackground);
#endif
	    }
	when S_TELEP:
	    /*
	     * Scroll of teleportation:
	     * Make him dissapear and reappear
	     */
	    {
		int	rm;
		struct	room	*cur_room;
		char	ch;

		cur_room = roomin(&hero);
		ch = mvwinch(stdscr, hero.y, hero.x);
#ifdef	COLOR
		Dcolor(cw, ch);
#endif
		mvwaddch(cw, hero.y, hero.x, ch);
#ifdef	COLOR
		acolor(cw, Dforeground, Dbackground);
#endif
		do {
		    rm = rnd_room();
		    rnd_pos(&rooms[rm], &hero);
		} until(winat(hero.y, hero.x) == floor);
		if (cur_room != &rooms[rm])
		    s_know[S_TELEP] = TRUE;
		light(&hero);
#ifdef	COLOR
		Dcolor(cw, PLAYER);
#endif
		mvwaddch(cw, hero.y, hero.x, PLAYER);
#ifdef	COLOR
		acolor(cw, Dforeground, Dbackground);
#endif
	    }
	when S_ENCH:
	    if (cur_weapon == NULL)
		msg("You feel a strange sense of loss");
	    else {
		if (rnd(100) > 50)
		    cur_weapon->o_hplus++;
		else
		    cur_weapon->o_dplus++;
		msg("Your %s glows blue for a moment",
		  w_names[cur_weapon->o_which]);
	    }
	when S_SCARE:
	    /*
	     * A monster will refuse to step on a scare monster scroll
	     * if it is dropped.  Thus reading it is a mistake and produces
	     * laughter at the poor rogue's boo boo.
	     */
#ifdef	iris
	    msg("You hear maniacal laughter");
#else
	    msg("You hear maniacal laughter in the distance");
#endif
	when S_REMOVE:
	    if (cur_armor != NULL)
	    	cur_armor->o_flags &= ~ISCURSED;
	    if (cur_weapon != NULL)
	    	cur_weapon->o_flags &= ~ISCURSED;
	    if (cur_ring[LEFT] != NULL)
	    	cur_ring[LEFT]->o_flags &= ~ISCURSED;
	    if (cur_ring[RIGHT] != NULL)
	    	cur_ring[RIGHT]->o_flags &= ~ISCURSED;
#ifdef	iris
	    msg("You feel somebody watching over you");
#else
	    msg("You feel as if somebody is watching over you");
#endif
	when S_AGGR:
	    /*
	     * This scroll aggravates all the monsters on the current
	     * level and sets them running towards the hero
	     */
	    aggravate();
	    msg("You hear a high pitched humming noise.");
	when S_NOP:
	    msg("This scroll seems to be blank.");
	when S_GENOCIDE:
	    msg("You have been granted the power of genocide");
	    genocide();
	    s_know[S_GENOCIDE] = TRUE;
	otherwise:
	    msg("What a puzzling scroll");
	    return;
    }
    look(TRUE);	/* put the result of the scroll on the screen */
/* status(); */
    if (s_know[obj->o_which] && s_guess[obj->o_which]) {
	cfree(s_guess[obj->o_which]);
	s_guess[obj->o_which] = NULL;
    } else
	if (!s_know[obj->o_which] && askme && s_guess[obj->o_which] == NULL) {
	    status();
#ifdef	iris
	    msg("Call it?");
#else
	    msg(terse ? "Call it: " : "What do you want to call it?");
#endif
/*	    mpos = 0; */
	    if (get_str(buf, cw) == NORM) {
		s_guess[obj->o_which] = malloc(strlen(buf) + 1);
		strcpy(s_guess[obj->o_which], buf);
	    }
        }
    /*
     * Get rid of the thing
     */
    if (used) {
        inpack--;
	if (obj->o_count > 1)
	    obj->o_count--;
	else {
            detach(pack, item);
            discard(item);
	}
    }
}

/*
 * Iris Gold FIND graphic display
 */

#ifdef	iris

Igfind(piles)
{
    short *x;		/* X coordinate  of a pile of gold */
    short *y;		/* X coordinate  of a pile of gold */
    short *t;		/* twinkle state of a pile of gold */
    short i;
    short j;

    x = (short *) new(piles*sizeof (int));
    y = (short *) new(piles*sizeof (int));
    t = (short *) new(piles*sizeof (int));
    for (i = j = 0; i < MAXROOMS; i++) {
        if (rooms[i].r_goldval != 0 &&
          mvwinch(stdscr, rooms[i].r_gold.y, rooms[i].r_gold.x)
          == GOLD) {
	    x[j] = rooms[i].r_gold.x;
	    y[j] = rooms[i].r_gold.y;
	    t[j] = 0;
	    j++;
        }
    }
    doublebuffer();
    gconfig();
    i = -1;
    while (!Qchar() || readchar() != ' ') {
	color(Dbackground);
	glclear();
	color(C_gold);
	icursor(0,0);
	font(_nowfont=_linfont[0]);	/* Old English	*/
	charstr("You begin feeling greedy.");
	color(Dforeground);
	font(_nowfont=_linfont[1]);	/* Standard	*/
	charstr(more);
	color(C_gold);
	while ((j = rnd(piles)) == i && piles > 1)
		continue;
	i = j;
	t[i] = rnd(16);
	for (j=0; j<piles; j++) {
	    if (t[j] & 010)
		continue;
	    pushmatrix();
	    translate((Coord)(x[j]*CWIDTH),(Coord)_linblit[y[j]+1],(Coord)0.0);
	    if (t[j] & 01)
		translate((Coord)2.,(Coord)2.,(Coord)0.0);
	    if (t[j] & 02)
		rotate((Angle)450,'z');
	    if (t[j] & 04)
		scale((Coord).5,(Coord).5,(Coord)0.0);
	    rectfi(-7,0,7,0);
	    rotate((Angle)900,'z');
	    rectfi(-7,0,7,0);
	    rotate((Angle)450,'z');
	    rectfi(-4,0,4,0);
	    rotate((Angle)900,'z');
	    rectfi(-4,0,4,0);
	    popmatrix();
	}
	swapbuffers();
	tick(4);
    }
    singlebuffer();
    gconfig();
    color(Dforeground);
    FREE(x);
    FREE(y);
    FREE(t);
    clearok(cw, TRUE);
    touchwin(cw);
}

#endif	iris

/*
 * Iris Magic FIND graphic display
 */

#ifdef	iris

Imfind(piles)
{
    short *x;		/* X coordinate  of a magic item */
    short *y;		/* X coordinate  of a magic item */
    short *t;		/* twinkle state of a magic item */
    short i;
    short j;
    double fabs();
    int		th;

    x = (short *) new(piles*sizeof (int));
    y = (short *) new(piles*sizeof (int));
    t = (short *) new(piles*sizeof (int));
    if (lvl_obj != NULL) {
	struct	linked_list *mobj;
	struct	object	*tp;

	for (j = 0, mobj = lvl_obj; mobj != NULL; mobj = next(mobj)) {
	    tp = (struct object *) ldata(mobj);
	    switch (tp->o_type) {
	      case POTION:
	      case SCROLL:
	      case WAND:
	      case RING:
		x[j] = tp->o_pos.x;
		y[j] = tp->o_pos.y;
		t[j] = 0;
		j++;
	    }
	}
    }
    doublebuffer();
    gconfig();
    i = -1;
    mapcolor(C_vary,(Colorindex)0,(Colorindex)0,(Colorindex)255);
    mapcolor(C_vary+1,(Colorindex)0,(Colorindex)255,(Colorindex)0);
    mapcolor(C_vary+2,(Colorindex)255,(Colorindex)0,(Colorindex)0);
    while (!Qchar() || readchar() != ' ') {
	color(Dbackground);
	glclear();
	color(C_blue);
	icursor(0,0);
	font(_nowfont=_linfont[0]);	/* Old English	*/
	charstr("You sense magic.");
	color(Dforeground);
	font(_nowfont=_linfont[1]);	/* Standard	*/
	charstr(more);
	color(C_vary);
	while ((j = rnd(piles)) == i && piles > 1)
		continue;
	i = j;
	t[i] = (t[i] + 50);
	if (t[i] > 3600)
	    t[i] = 0;
	if (t[0] == 1200)
	    color(C_vary+1);
	else if (t[0] == 2400)
	    color(C_vary+2);
	else
	    color(C_vary);
#ifdef	notdefatall
	th = t[0]/10;
	th *= 255;
	th /= 360;
	if (th <= 85) {
	    mapcolor(C_vary,
	      (Colorindex)(th-0),
	      (Colorindex)(0),
	      (Colorindex)(85-th));
	} else if (th <= 170) {
	    mapcolor(C_vary,
	      (Colorindex)(170-th),
	      (Colorindex)(th-85),
	      (Colorindex)(0));
	} else {
	    mapcolor(C_vary,
	      (Colorindex)(0),
	      (Colorindex)(255-th),
	      (Colorindex)(th-170));
	}
#endif
	for (j=0; j<piles; j++) {
	    pushmatrix();
	    translate((Coord)(x[j]*CWIDTH),(Coord)_linblit[y[j]+1],(Coord)0.0);
	    rotate((Angle)t[j],'z');
	    scale((Coord)(10.-fabs((double)t[i]-1800.)/360.+1.),
	      (Coord)(10.-fabs((double)t[i]-1800.)/360.+1.),(Coord)0.0);
				/* draw a $ */
	    rectfi(-1,-9,-1,9);			/* left  vertical bar	*/
	    rectfi( 1,-9, 1,9);			/* right vertical bar	*/
	    arc((Coord)0.,(Coord) 3.5,		/* top    curve		*/
	      (Coord)3.5,(Angle)900,(Angle)2700);
	    arc((Coord)0.,(Coord)-3.5,		/* bottom curve		*/
	      (Coord)3.5,(Angle)2700,(Angle)900);
	    recti(0, 7, 4, 7);			/* top    -		*/
	    recti(0,-7,-4,-7);			/* bottom -		*/
	    popmatrix();
	}
	swapbuffers();
/*	tick(1); */
    }
    singlebuffer();
    gconfig();
    color(Dforeground);
    FREE(x);
    FREE(y);
    FREE(t);
    clearok(cw, TRUE);
    touchwin(cw);
}

#endif	iris

/*
 * lights:
 *	Light up a corridor or room as a result of reading a scroll of light
 *	or zapping a wand of light. Do blue shimmer on Iris.
 */
lights()
{
    reg1 rega1	struct	room	*rp;
    reg2 regd1	int i;
    reg3 regd2	int j;

#ifdef	iris
    			/*
			 * Tell Curses to write changes even if
			 * colors & chars seem to not have changed.
			 * Used for diddling color map and shimmering.
			 */
    if (_iris) {
	Draw(cw);
	writethru++;
    }
#endif
    if ((rp = roomin(&hero)) == NULL) {
	msg("The corridor glows and then fades");
#ifdef	iris
	if (_iris) {
	    Draw(cw);
	    writethru++;
	    coloroff = COLSHIM;
	    for (i=Max(hero.x-1,0); i<=Min(hero.x+1,COLS-1); i++)
		for (j=Max(hero.y-1,0); j<=Min(hero.y+1,LINES-1); j++)
			if (cw->_firstch[j] == _NOCHANGE)
				cw->_firstch[j] = cw->_lastch[j] = i;
			else if (i < cw->_firstch[j])
				cw->_firstch[j] = i;
			else if (i > cw->_lastch[j])
				cw->_lastch[j] = i;

	    coloroff = COLNORM;
	    Draw(cw);
	    writethru--;
	}
#endif	iris
    } else {
	msg("The room is lit%s",
	  terse ? "" : " by a shimmering blue light");
#ifdef	iris
	if (_iris) {
	    coloroff = COLBLACK;
	    light(&hero);
	}
#endif	iris
	rp->r_flags &= ~ISDARK;
	/*
	 * Light the room and put the player back up
	 */
#ifdef	iris
	for (i=(_iris?50:1); i>0; i--)
#else
	for (i=(1); i>0; i--)
#endif	iris
	{
	    Draw(cw);
#ifdef	iris
	    if (_iris) {
		Coord x;
		Coord y;

		pushmatrix();
		if (!rnd(2))
		    coloroff = COLSHIM;
		else
		    coloroff = COLNORM;
		if (rnd(2))
			x = 3.;
		else
			x = -3.;
		if (rnd(2))
			y = 3.;
		else
			y = -3.;
		translate(x,y,(Coord)0.);
			/* can't rotate since can't rotate raster fonts */
	    }
#endif	iris
	    light(&hero);
#ifdef	COLOR
	    Dcolor(cw, PLAYER);
#endif
	    mvwaddch(cw, hero.y, hero.x, PLAYER);
#ifdef	COLOR
	    acolor(cw, Dforeground, Dbackground);
#endif
	    Draw(cw);
#ifdef	iris
	    if (_iris) {
/*		tick(6); */	/* wait 1/10 seconds except last iteration */
		coloroff = COLBLACK;
		light(&hero);
		Dcolor(cw, PLAYER);
		mvwaddch(cw, hero.y, hero.x, PLAYER);
		acolor(cw, Dforeground, Dbackground);
		Draw(cw);
		popmatrix();
	    }
#endif	iris
	}
    }
#ifdef	iris
    if (_iris) {
	writethru--;
	coloroff = COLNORM;
	clearok(curscr,TRUE);
	Draw(cw);
    }
#endif
}

/*
 * num:
 *	Figure out the plus number for armor/weapons
 */

char	*
num(n1, n2)
reg1 regd1	int	n1;
reg2 regd2	int	n2;
{
    static	char	numbuf[80];

    if (n1 == 0 && n2 == 0)
	return "+0";
    if (n2 == 0) {
/* #ifdef	V7 */
	sprintf(numbuf, "%s%d", n1 < 0 ? "" : plusstr, n1);
/* #else */
/* 	sprintf(numbuf, "%+d", n1); */
/* #endif */
    } else {
/* #ifdef	V7 */
	sprintf(numbuf, "%s%d,%s%d",
	  n1 < 0 ? "" : plusstr, n1,
	  n2 < 0 ? "" : plusstr, n2);
/* #else */
/*     	sprintf(numbuf, "%+d,%+d", n1, n2); */
/* #endif */
    }
    return numbuf;
}

/*
 * whatis:
 *	What a certain object is
 */

whatis()
{
    reg1 rega1	struct	object	*obj;
    reg2 rega2	struct	linked_list *item;
		char	*numbug;	/* debug num() */

    if ((item = get_item("identify", 0)) == NULL)
	return 0;		/* unused */
    obj = (struct object *) ldata(item);
    switch (obj->o_type) {
      case SCROLL:
	s_know[obj->o_which] = TRUE;
	if (s_guess[obj->o_which]) {
	    cfree(s_guess[obj->o_which]);
	    s_guess[obj->o_which] = NULL;
	}
      when POTION:
	p_know[obj->o_which] = TRUE;
	if (p_guess[obj->o_which]) {
	    cfree(p_guess[obj->o_which]);
	    p_guess[obj->o_which] = NULL;
	}
      when WAND:
	W_know[obj->o_which] = TRUE;
	obj->o_flags |= ISKNOW;
	if (W_guess[obj->o_which]) {
	    cfree(W_guess[obj->o_which]);
	    W_guess[obj->o_which] = NULL;
	}
      when WEAPON:
      case ARMOR:
	obj->o_flags |= ISKNOW;
/*
 * #ifdef	V7
 * 	msg("A %s%s AC %s",
 * #else
 * 	msg("A %s%+d AC %s",
 * #endif
 * 	  (obj->o_flags&ISCURSED) ? "cursed " : "",
 * #ifdef	V7
 * 	  num(obj->o_ac,0),
 * #else
 * 	  obj->o_ac,
 * #endif
 */
      when RING:
	r_know[obj->o_which] = TRUE;
	obj->o_flags |= ISKNOW;
	if (r_guess[obj->o_which]) {
	    cfree(r_guess[obj->o_which]);
	    r_guess[obj->o_which] = NULL;
	}
/*
 * 	  case 0:
 * 	    msg("Delicious food");
 * 	  when 1:
 * 	    msg("Yucky food");
 * 	  when 2:
 * 	    msg("Magical food");
 */
    }
    msg(inv_name(obj, FALSE));
    return 1;		/* item used */
}

/*
 * eat:
 *	She wants to eat something, so let her try
 */

eat()
{
    reg1 rega1	struct	linked_list *item;
    reg2 rega2	struct	object	*obj;

    if ((item = get_item("eat", FOOD)) == NULL)
	return;
    obj = (struct object *) ldata(item);
    if (obj->o_type != FOOD) {
	if (!terse)
	    msg("Ugh, you would get ill if you ate that");
	else
	    msg("That's Inedible");
	return;
    }
    inpack--;
    if (--obj->o_count < 1) {
	detach(pack, item);
	discard(item);
    }
    if (obj->o_which == 1)
				/* he ate his fruit */
	msg("Mmmm, that was a yummy %s",fruit);
    else
	if (rnd(100) > 70) {
				/* Man, he sure ate some baaaad stuff */
	    msg("Yuk, this food tastes awful");
	    pstats.s_exp++;
	} else
	    msg("Yum, that tasted good");
    /*
     * Shut down all possible hunger things (Ha!)
     */
    if ((food_left += HUNGERTIME + rnd(400) - 200) > STOMACHSIZE)
	food_left = STOMACHSIZE;
    hungry_state = 0;
    if (obj == cur_weapon)
	cur_weapon = NULL;
}

/*
 * Used to modify the player's strength
 * it keeps track of the highest it has been, just in case
 */

chg_str(amt)
reg1 regd1	int	amt;
{
    if (amt == 0)
	return;
    if (amt > 0) {
	while (amt--) {
	    if (pstats.s_str.st_str < 18)
		pstats.s_str.st_str++;
	    else if (pstats.s_str.st_add == 0)
		pstats.s_str.st_add = rnd(50) + 1;
	    else if (pstats.s_str.st_add <= 50)
		pstats.s_str.st_add = 51 + rnd(24);
	    else if (pstats.s_str.st_add <= 75)
		pstats.s_str.st_add = 76 + rnd(14);
	    else if (pstats.s_str.st_add <= 90)
		pstats.s_str.st_add = 91;
	    else if (pstats.s_str.st_add < 100)
		pstats.s_str.st_add++;
	}
	if (pstats.s_str.st_str > max_stats.s_str.st_str ||
	    (pstats.s_str.st_str == 18 &&
	     pstats.s_str.st_add > max_stats.s_str.st_add))
		max_stats.s_str = pstats.s_str;
    } else {
	while (amt++) {
	    if (pstats.s_str.st_str < 18 || pstats.s_str.st_add == 0)
		pstats.s_str.st_str--;
	    else if (pstats.s_str.st_add < 51)
		pstats.s_str.st_add = 0;
	    else if (pstats.s_str.st_add < 76)
		pstats.s_str.st_add = 1 + rnd(50);
	    else if (pstats.s_str.st_add < 91)
		pstats.s_str.st_add = 51 + rnd(25);
	    else if (pstats.s_str.st_add < 100)
		pstats.s_str.st_add = 76 + rnd(14);
	    else
		pstats.s_str.st_add = 91 + rnd(8);
	}
	if (pstats.s_str.st_str < 3)
	    pstats.s_str.st_str = 3;
    }
}

/*
 * add_haste:
 *	add a haste to the player
 */

add_haste(potion)
bool	potion;
{
    if (on(player, ISHASTE)) {
	msg("You faint from too much Speed.");
	no_command += rnd(8);
	extinguish(nohaste);
    } else {
	player.t_flags |= ISHASTE;
	if (potion)
	    fuse(nohaste, 0, rnd(4)+4, AFTER);
    }
}

/*
 * aggravate:
 *	aggravate all the monsters on this level
 */

aggravate()
{
    reg1 rega1	struct	linked_list *mi;

    for (mi = mlist; mi != NULL; mi = next(mi))
	runto(&((struct thing *) ldata(mi))->t_pos, &hero);
}

/* 
 * see if the object is one of the currently used items
 */
is_current(obj)
reg1 rega1	struct	object	*obj;
{
    if (obj == NULL)
	return FALSE;
    if (obj == cur_armor || obj == cur_weapon || obj == cur_ring[LEFT]
      || obj == cur_ring[RIGHT]) {
	msg(terse ? "In use." : "That's already in use.");
	return TRUE;
    }
    return FALSE;
}

/*
 * set up the direction co_ordinate for use in varios "prefix" commands
 */
get_dir()
{
    reg1 rega1	char	*prompt;
    reg2 regd1	bool	gotit;

    if (!terse)
	msg(prompt = "Which direction? ");
    else
	msg(prompt = "Direction: ");
    do {
	gotit = TRUE;
	switch (readchar()) {
	    case 'h': case'H': delta.y =  0; delta.x = -1;
	    when 'j': case'J': delta.y =  1; delta.x =  0;
	    when 'k': case'K': delta.y = -1; delta.x =  0;
	    when 'l': case'L': delta.y =  0; delta.x =  1;
	    when 'y': case'Y': delta.y = -1; delta.x = -1;
	    when 'u': case'U': delta.y = -1; delta.x =  1;
	    when 'b': case'B': delta.y =  1; delta.x = -1;
	    when 'n': case'N': delta.y =  1; delta.x =  1;
	    when ESCAPE: return FALSE;
	    otherwise:
		mpos = 0;
		msg(prompt);
		gotit = FALSE;
	}
    } until (gotit);
    if (on(player, ISHUH) && rnd(100) > 80)
	do {
	    delta.y = rnd(3) - 1;
	    delta.x = rnd(3) - 1;
	} while (delta.y == 0 && delta.x == 0);
    mpos = 0;
    return TRUE;
}

/*
 * Create -- Wizard command for getting anything he wants
 */

#ifndef	SMALL
createm()
{
    reg1 rega1	struct	linked_list *item;
    reg2 rega2	struct	object	*obj;
    reg3 regd1	char	ch;
    reg4 regd2	char	bless;
    reg5 regd3	int	i;

    wclear(hw);
#ifndef	SMALL
    wmove(hw, 1, 0);
    waddstr(hw, "* Gold\n");
    waddstr(hw, "! Potion\n");
    waddstr(hw, "? Scroll\n");
    waddstr(hw, ": Food\n");
    waddstr(hw, ") Weapon\n");
    waddstr(hw, "] Armor\n");
    waddstr(hw, "/ Wand\n");
    waddstr(hw, "= Ring\n");
    waddstr(hw, ", Amulet\n");
#endif
    item = new_item(sizeof *obj);
    obj = (struct object *) ldata(item);
    wmsg(hw,"Type of item:");
    obj->o_type = readchar();
    obj->o_count = 1;
#ifndef	SMALL
    wclear(hw);
    wmove(hw, 1, 0);
    switch (obj->o_type) {
	case SCROLL:
	    for (i=0; i<MAXSCROLLS; i++)
		wprintw(hw,"%x. A scroll of %s.\n",i,s_magic[i].mi_name);
        when POTION:
	    for (i=0; i<MAXPOTIONS; i++)
		wprintw(hw,"%x. A potion of %s(%s).\n",
		  i, p_magic[i].mi_name, p_colors[i]);
        when WAND:
	    for (i=0; i<MAXWANDS; i++)
		wprintw(hw,"%x. A wand of %s(%s).\n",
		  i, W_magic[i].mi_name, W_magic[i].mi_name);
        when RING:
	    for (i=0; i<MAXRINGS; i++)
		wprintw(hw,"%x. A ring of %s(%s)%s.\n",
		  i, r_magic[i].mi_name, r_magic[i].mi_name,
		  hand(obj));
	when FOOD:
		wprintw(hw,"0. good food\n");
		wprintw(hw,"1. bad food\n");
		wprintw(hw,"2. A single %s\n", fruit);
	when WEAPON:
	    for (i=0; i<MAXWEAPONS; i++)
		wprintw(hw,"%x. %s\n", i, w_names[i]);
	when ARMOR:
	    for (i=0; i<MAXARMORS; i++)
		wprintw(hw,"%x. %s\n", i, a_names[i]);
	when AMULET:
	    wprintw(hw,"The Amulet of Yendor\n");
	otherwise:
	    wprintw(hw,"Something bizarre %s\n", unctrl(obj->o_type));
    }
#endif
    mpos = 0;
    wmsg(hw,"Which %c do you want? (0-f)", obj->o_type);
    obj->o_which = (isdigit((ch = readchar())) ? ch - '0' : ch - 'a' + 10);
#ifndef	SMALL
    wclear(hw);
    wmove(hw, 1, 0);
    waddstr(hw, "+: plusgood\n");
    waddstr(hw, "-: plusungood (cursed)\n");
    waddstr(hw, "n: good\n");
#endif
    mpos = 0;
#ifdef	iris
    if (_iris)
	wmsg(hw,"Blessing?");
    else
#endif
	wmsg(hw,"Blessing? (+,-,n)");
    bless = readchar();
    if (bless == '-')
	obj->o_flags |= ISCURSED;
    if (obj->o_type == WEAPON) {
	init_weapon(obj, obj->o_which);
	if (bless == '-')
	    obj->o_hplus -= rnd(3)+1;
	if (bless == '+')
	    obj->o_hplus += rnd(3)+1;
    }
    if (obj->o_type == ARMOR) {
	obj->o_ac = a_class[obj->o_which];
	if (bless == '-')
	    obj->o_ac += rnd(3)+1;
	if (bless == '+')
	    obj->o_ac -= rnd(3)+1;
    }
    attach(pack, item);
    clearok(cw, TRUE);
    touchwin(cw);
    after = FALSE;
}
#endif	SMALL

/*
 * telport:
 *	Scotty, beam me up!
 */

teleport()
{
    reg1 regd1	int	rm;
		coord	c;

    c = hero;
    mvwaddch(cw, hero.y, hero.x, mvwinch(stdscr, hero.y, hero.x));
    do {
	rm = rnd_room();
	rnd_pos(&rooms[rm], &hero);
    } until(winat(hero.y, hero.x) == floor);
    light(&c);
    light(&hero);
    mvwaddch(cw, hero.y, hero.x, PLAYER);
    /*
     * turn off ISHELD in case teleportation was done while fighting
     * a Fungi
     */
    if (on(player, ISHELD)) {
	player.t_flags &= ~ISHELD;
	fung_hit = 0;
	strcpy(monsters['F'-'A'].m_stats.s_dmg, "000d0");
    }
    count = 0;
    running = FALSE;
    flushi();		/* flush typeahead */
    return rm;
}

overmerge(from,to)
reg1 rega1	WINDOW	*from;
reg2 rega2	WINDOW	*to;
{
    reg3 regd1	int i;
    reg4 regd2	int j;
    reg5 regd3	char ch;

    for (i = 0; i < LINES; i++)
	for (j = 0; j < COLS; j++) {
	    ch = mvwinch(from, i, j); 
	    if ((ch & 0177) != ' ') {
#ifdef	COLOR
		Dcolor(to, ch);
#endif
		mvwaddch(to, i, j, ch);
	    }
	}
#ifdef	COLOR
    acolor(to, Dforeground, Dbackground);
#endif
}
