# include	"../hdr/defines.h"

SCCSID(@(#)del_ab.c	5.3);

del_ab(p,dt,pkt)
register char *p;
register struct deltab *dt;
struct packet *pkt;
{
	extern	char	*satoi();
	int n;
	extern char *Datep;
	char *sid_ab();

	if (*p++ != CTLCHAR)
		fmterr(pkt);
	if (*p++ != BDELTAB)
		return(*--p);
	NONBLANK(p);
	dt->d_type = *p++;
	NONBLANK(p);
	p = sid_ab(p,&dt->d_sid);
	NONBLANK(p);
	date_ab(p,&dt->d_datetime);
	p = Datep;
	NONBLANK(p);
	if ((n = index(p," ")) < 0)
		fmterr(pkt);
	strncpy(dt->d_pgmr,p,n);
	dt->d_pgmr[n] = 0;
	p += n + 1;
	NONBLANK(p);
	p = satoi(p,&dt->d_serial);
	NONBLANK(p);
	p = satoi(p,&dt->d_pred);
	if (*p != '\n')
		fmterr(pkt);
	return(BDELTAB);
}
