# include	"../hdr/defines.h"

SCCSID(@(#)fmterr.c	5.1);

fmterr(pkt)
register struct packet *pkt;
{
	fclose(pkt->p_iop);
	sprintf(Error,"format error at line %u (co4)",pkt->p_slnno);
	fatal(Error);
}
