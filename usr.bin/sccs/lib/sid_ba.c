# include	"../hdr/defines.h"

SCCSID(@(#)sid_ba.c	5.2);

char *
sid_ba(sp,p)
register struct sid *sp;
register char *p;
{
	sprintf(p,"%u.%u",sp->s_rel,sp->s_lev);
	while (*p++)
		;
	--p;
	if (sp->s_br) {
		sprintf(p,".%u.%u",sp->s_br,sp->s_seq);
		while (*p++)
			;
		--p;
	}
	return(p);
}
