/* gf2features.c
 *	tests of new features
 *	GE port & clocks must work
 */

#include "m68000.h"
#include "gums.h"

extern unsigned short testvecin[];
extern short testvi,testvo;
extern short GEstatus;
extern short tokenint_occurred;
extern char line[];
extern short ix;

tokentest()
{
	short save = GEstatus;

	printf("token test: ");
	tokenint_occurred = 0;
	GEflags = GERESET1;
	buzz(100);
	GEflags = GERESET3 | ENABTOKENINT_BIT_;
	buzz(100);
	GEdata = 8;
	if (!(FBCflags & TOKEN_BIT_))
		{printf("no initial clear\n"); return;}
	GEflags = GEstatus = GERESET3 & ~ENABTOKENINT_BIT_;
	intlevel(2);
	buzz(10);
	if (tokenint_occurred)
		{printf("unprovoked interrupt\n"); return;}
	GEdata = 8;
	if (tokenint_occurred)
		{printf("unprovoked interrupt\n"); return;}
	GETOKEN = 8;
	if (FBCflags & TOKEN_BIT_)
		{printf("TOKEN_BIT_ doesn't go active\n"); return;}
	if (!tokenint_occurred)
		{printf("no interrupt\n"); return;}
	GEdata = 8;
	if (!(FBCflags & TOKEN_BIT_))
		{printf("can't clear\n"); return;}
	GEstatus = save;
	printf("OK\n");
}

fbcsend(reps)
	int reps;
{
	register done = testvi;
	register unsigned short *pv;
	register i;
	fsetup;

    if (reps==0) reps = 1;
    while (reps-- > 0) {
	pv = testvecin;
	for (i=0; i<done; i++) {
		fshort(*pv++);
		fwait();
	}
    }
}
