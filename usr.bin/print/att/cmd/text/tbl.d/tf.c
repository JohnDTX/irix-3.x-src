/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

 /* tf.c: save and restore fill mode around table */
# include "t..c"
savefill()
{
/* remembers various things: fill mode, vs, ps in mac 35 (SF) */
fprintf(tabout, ".de %d\n",SF);
fprintf(tabout, ".ps \\n(.s\n");
fprintf(tabout, ".vs \\n(.vu\n");
fprintf(tabout, ".in \\n(.iu\n");
fprintf(tabout, ".if \\n(.u .fi\n");
fprintf(tabout, ".if \\n(.j .ad\n");
fprintf(tabout, ".if \\n(.j=0 .na\n");
fprintf(tabout, "..\n");
fprintf(tabout, ".nf\n");
/* set obx offset if useful */
fprintf(tabout, ".nr #~ 0\n");
fprintf(tabout, ".if \\n(.T .if n .nr #~ 0.6n\n");
}
rstofill()
{
fprintf(tabout, ".%d\n",SF);
}
endoff()
{
int i;
	for(i=0; i<MAXHEAD; i++)
		if (linestop[i])
			fprintf(tabout, ".nr #%c 0\n", 'a'+i);
	for(i=0; i<texct; i++)
		fprintf(tabout, ".rm %c+\n",texstr[i]);
fprintf(tabout, ".ch #f\n");
fprintf(tabout, "%s\n", last);
}
ifdivert()
{
fprintf(tabout, ".ds #d .d\n");
fprintf(tabout, ".if \\(ts\\n(.z\\(ts\\(ts .ds #d nl\n");
}
saveline()
{
fprintf(tabout, ".if \\n+(b.=1 .nr d. \\n(.c-\\n(c.-1\n");
linstart=iline;
}
restline()
{
fprintf(tabout,".if \\n-(b.=0 .nr c. \\n(.c-\\n(d.-%d\n", iline-linstart);
linstart = 0;
}
cleanfc()
{
fprintf(tabout, ".fc\n");
}
multipg()
{
int c;

fprintf (tabout, ".if \\(ts\\n(.z\\(ts\\(ts .wh \\n(nlu+\\n(.tu-2v #f\n");
fprintf (tabout, ".de #f\n");
fprintf (tabout, ".ie \\\\n(#& \\{ \\\n");
fprintf (tabout, ".\tnr #& 0\n");
fprintf (tabout, ".\tnr #| 1\n");
fprintf (tabout, ".\tdi #o \\}\n");
fprintf (tabout, ".el \\{ \\\n");
fprintf (tabout, ".\tch #f\n");
fprintf (tabout, ".\tnr T. 1\n");
fprintf (tabout, ".\tif t .sp 1v\n");
fprintf (tabout, ".\tT#\n");
fprintf (tabout, ".\tsp \\\\n(.tu\n");
fprintf (tabout, ".\tmk #T\n");
for (c=0; c<ncol; c++)
	fprintf (tabout, ".\tnr ^%c \\\\n(#T\n", 'a'+c);
fprintf (tabout, ".\tif \\\\n(#| \\{ \\\n");
fprintf (tabout, ".\t\tin 0\n");
fprintf (tabout, ".\t\t#+\n");
fprintf (tabout, ".\t\trm #+\n");
fprintf (tabout, ".\t\tmk %d\n", S2);
fprintf (tabout, ".\t\tnr #| 0\n");
fprintf (tabout, ".\t\tin \\}\n");
fprintf (tabout, ".\tif \\\\(ts\\\\n(.z\\\\(ts\\\\(ts .wh \\\\n(nlu+\\\\n(.tu-2v #f \\}\n");
fprintf (tabout, "..\n");
fprintf (tabout, ".de #%%\n");
fprintf (tabout, ".if \\\\(ts\\\\n(.z\\\\(ts#o\\\\(ts \\{ \\\n");
fprintf (tabout, ".\tsp |0u\n");
fprintf (tabout, ".\tin 0\n");
fprintf (tabout, ".\t#+\n");
fprintf (tabout, ".\tin\n");
fprintf (tabout, ".\tsp |\\\\n(.hu\n");
fprintf (tabout, ".\tdi\n");
fprintf (tabout, ".\trn #o #+ \\}\n");
fprintf (tabout, "..\n");
}
