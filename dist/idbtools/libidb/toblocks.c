#include <sys/param.h>
#if R2300 || IP4
#include <sys/types.h>
#include <sys/sysmacros.h>
#else
#include <sys/fs.h>
#endif

#define		DIRECT		10
#if R2300 || IP4
#define		INDIR		128
#define		INSHFT		7
#else
#define		SECTORSHIFT	9
#define		SECTORSIZE	(1<<SECTORSHIFT)
#endif

long
toblocks (n)
	long		n;
{
	register long	blocks, t, tot;

#if R2300 || IP4
	blocks = tot = (n + NBPSCTR - 1) >> SCTRSHFT;
	if(blocks > DIRECT)
		tot += ((blocks - DIRECT - 1) >> INSHFT) + 1;
	if(blocks > DIRECT + INDIR)
		tot += ((blocks - DIRECT - INDIR - 1) >> (INSHFT * 2)) + 1;
	if(blocks > DIRECT + INDIR + INDIR*INDIR)
		tot++;
#else
	t = tot = (n + BSIZE - 1) / BSIZE;
	t /= 2;
	if (t > DIRECT) {
		tot += ((t - DIRECT - 1) >> NSHIFT) + 1;
		if (t > DIRECT + NINDIR) {
			tot += ((t - DIRECT - NINDIR - 1) >> (NSHIFT * 2)) + 1;
			if (t > DIRECT + NINDIR + NINDIR * NINDIR) {
				tot++;
			}
		}
	}
#endif
	return (tot);
}
