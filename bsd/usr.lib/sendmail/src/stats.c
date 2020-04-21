/*
**  Sendmail
**  Copyright (c) 1983  Eric P. Allman
**  Berkeley, California
**
**  Copyright (c) 1983 Regents of the University of California.
**  All rights reserved.  The Berkeley software License Agreement
**  specifies the terms and conditions for redistribution.
*/

#ifndef lint
static char	SccsId[] = "@(#)stats.c	5.8 (Berkeley) 5/2/86";
#endif not lint

# include "sendmail.h"
# include "mailstats.h"

struct statistics	Stat;

#define ONE_K		1000		/* one thousand (twenty-four?) */
#define KBYTES(x)	(((x) + (ONE_K - 1)) / ONE_K)
/*
**  MARKSTATS -- mark statistics
*/

markstats(e, to)
	register ENVELOPE *e;
	register ADDRESS *to;
{
	if (to == NULL)
	{
		if (e->e_from.q_mailer != NULL)
		{
			Stat.stat_nf[e->e_from.q_mailer->m_mno]++;
			Stat.stat_bf[e->e_from.q_mailer->m_mno] +=
				KBYTES(CurEnv->e_msgsize);
		}
	}
	else
	{
		Stat.stat_nt[to->q_mailer->m_mno]++;
		Stat.stat_bt[to->q_mailer->m_mno] += KBYTES(CurEnv->e_msgsize);
	}
}
/*
**  POSTSTATS -- post statistics in the statistics file
**
**	Parameters:
**		sfile -- the name of the statistics file.
**
**	Returns:
**		none.
**
**	Side Effects:
**		merges the Stat structure with the sfile file.
*/

poststats(sfile)
	char *sfile;
{
	register int fd;
	struct statistics stat;
	extern off_t lseek();

	if (sfile == NULL)
		return;

	(void) time(&Stat.stat_itime);
	Stat.stat_size = sizeof Stat;

	fd = open(sfile, 2);
	if (fd < 0)
	{
		errno = 0;
		return;
	}
	if (read(fd, (char *) &stat, sizeof stat) == sizeof stat &&
	    stat.stat_size == sizeof stat)
	{
		/* merge current statistics into statfile */
		register int i;

		for (i = 0; i < MAXMAILERS; i++)
		{
			stat.stat_nf[i] += Stat.stat_nf[i];
			stat.stat_bf[i] += Stat.stat_bf[i];
			stat.stat_nt[i] += Stat.stat_nt[i];
			stat.stat_bt[i] += Stat.stat_bt[i];
		}
	}
	else
		bcopy((char *) &Stat, (char *) &stat, sizeof stat);

	/* write out results */
	(void) lseek(fd, (off_t) 0, 0);
	(void) write(fd, (char *) &stat, sizeof stat);
	(void) close(fd);
}
