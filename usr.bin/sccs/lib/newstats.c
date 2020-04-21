# include	"../hdr/defines.h"

SCCSID(@(#)newstats.c	5.2);

newstats(pkt,strp,ch)
register struct packet *pkt;
register char *strp;
register char *ch;
{
	char fivech[6];
	repeat(fivech,ch,5);
	sprintf(strp,"%c%c %s/%s/%s\n",CTLCHAR,STATS,fivech,fivech,fivech);
	putline(pkt,strp);
}
