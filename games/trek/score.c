static char ID[] = "@(#)score.c	1.1";

# include	"trek.h"

/**
 **	print out the current score
 **/

long score()
{
	register int		u;
	long			s, t;	/* should really be long */
	double			r;
	extern CVNTAB	Skitab[];

	printf("\n*** Your score:\n");
	s = t = Param.klingpwr / 4 * (u = Game.gkillk);;
	if (t != 0)
		printf("%d Klingons killed\t\t\t%6ld\n", u, t);
	r = Status.date - Initial.date;
	if (r < 1.0)
		r = 1.0;
	r = u / r;
	s += (t = 400 * r);
	if (t != 0)
		printf("Kill rate %.2f Klingons/stardate  \t%6ld\n", r, t);
	r = Status.kling;
	r /= u + 1;
	s += (t = -1000 * r);
	if (t != 0)
		printf("Penalty for %d klingons remaining\t%6ld\n", Status.kling, t);
	if (Move.endgame > 0)
	{
		s += (t = 100 * (u = Game.skill));
		printf("Bonus for winning a %s%s game   \t%6ld\n", Skitab[u - 1].abrev, Skitab[u - 1].full, t);
	}
	if (violations)
	{
		t = -10 * Game.skill * violats;
		printf ("Penalty for %d violations\t\t%6ld\n", violations, t);
		s += t;
	}
	if (Game.killed)
	{
		s -= 200;
		printf("Penalty for getting killed\t\t  -200\n");
	}
	s += (t = -100 * (u = Game.killb));
	if (t != 0)
		printf("%d starbases killed\t\t\t%6ld\n", u, t);
	s += (t = -100 * (u = Game.helps));
	if (t != 0)
		printf("%d calls for help\t\t\t%6ld\n", u, t);
	s += (t = -20 * (u = Game.kills));
	if (t != 0)
		printf("%d stars destroyed\t\t\t%6ld\n", u, t);
	s += (t = -150 * (u = Game.killinhab));
	if (t != 0)
		printf("%d inhabited starsystems destroyed\t%6ld\n", u, t);
	if (Status.ship != things[ENTERPRISE])
	{
		s -= 200;
		printf("penalty for abandoning ship\t\t  -200\n");
	}
	s += (t = 3 * (u = Game.captives));
	if (t != 0)
		printf("%d Klingons captured\t\t\t%6ld\n", u, t);
	s += (t = -(u = Game.deaths));
	if (t != 0)
		printf("%d casualties\t\t\t\t%6ld\n", u, t);
	printf("\n***  TOTAL\t\t\t\t%6ld\n", s);
	return (s);
}


violate ()
{
	if (!violat0)
	 printf ("Sulu: This is going to hurt your chances for a promotion.\n");
	violats = violat0 + violat1;
	violat0 = violat1;
	violat1 = violats;
}
