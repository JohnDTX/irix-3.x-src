/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

 /* t8.c: write out one line of output table */


# include "t..c"
# define realsplit ((ct=='a'||ct=='n') && table[nl][c].rcol)
int watchout;
int once, dv;
putline(i, nl)
	/* i is line number for deciding format */
	/* nl is line number for finding data   usually identical */
{
int c, lf, ct, form, lwid, vspf, ip, cmidx, exvspen, vforml;
int vct, chfont, uphalf;
char *s, *size, *fn;
watchout=vspf=exvspen=0;
if (i==0) once=0;
if (i==0 && ( allflg || boxflg || dboxflg))
	fullwide(0,   dboxflg? '=' : '-');
if (linestop[nl])
	fprintf(tabout, ".mk #%c\n", linestop[nl]+'a'-1);
if (instead[nl])
	{
	puts(instead[nl]);
	return;
	}
if (fullbot[nl])
	{
	switch (ct=fullbot[nl])
		{
		case '=':
		case '-':
			fullwide(nl,ct);
		}
	return;
	}
if (allflg && once>0 )
	fullwide(i,'-');
once=1;
lf = prev(nl);
fprintf (tabout, ".nr %d 0\n", S2);
for(c=0; c<ncol; c++)
	{
	s = table[nl][c].col;
	if (s==0) continue;
	if (vspen(s))
		continue;
	if (point(s)) continue;
	if (lf < 0 || !vspen(table[lf][c].col))
		{
		fprintf (tabout, ".if \\n(%c|u>\\n(%du .nr %d \\n(%c|u\n", s, S2, S2, s);
		}
	watchout=1;
	}
fprintf (tabout, ".nr %d \\n(%du+\\n(.du+2p\n", S2, S2);
for(c=0; c<ncol; c++)
	{
	if (instead[nl]==0 && fullbot[nl]==0)
	if (vspen(table[nl][c].col)) vspf=1;
	if (lf>=0)
		if (vspen(table[lf][c].col)) vspf=1;
	}
if (vspf)
	{
	fprintf(tabout, ".nr #^ \\n(\\*(#du\n");
	fprintf(tabout, ".nr #- \\n(#^\n"); /* current line position relative to bottom */
	}
vspf=0;
chfont=0;
for(c=0; c<ncol; c++)
	{
	s = table[nl][c].col;
	if (s==0) continue;
	chfont |= (int)(font[c][stynum[nl]]);
	if (point(s) ) continue;
	lf=prev(nl);
	if (lf>=0 && vspen(table[lf][c].col))
		{
		fprintf(tabout, ".if (\\n(%c|+\\n(^%c-1v)>\\n(#- .nr #- \\n(%c|+\\n(^%c-1v\n",s,'a'+c,s,'a'+c);
		fprintf (tabout, ".nr %d \\n(#-+1v+2p\n", S2);
		}
	}
runtabs(i, nl);
if (watchout)
	funnies (i, nl);
if (allh(i) && !pr1403)
	{
	fprintf(tabout, ".nr %d \\n(.v\n", SVS);
	fprintf(tabout, ".vs \\n(.vu-\\n(.sp\n");
	fprintf(tabout, ".nr 35 \\n(.vu\n");
	}
else
	fprintf(tabout, ".nr 35 1m\n");
if (chfont)
	fprintf(tabout, ".nr %2d \\n(.f\n", S1);
fprintf(tabout, "\\&");
vct = 0;
for(c=0; c<ncol; c++)
	{
	uphalf=0;
	if (watchout==0 && i+1<nlin && (lf=left(i,c, &lwid))>=0)
		{
		tohcol(c);
		drawvert(lf, i, c, lwid);
		vct += 2;
		}
	if (rightl && c+1==ncol) continue;
	vforml=i;
	for(lf=prev(nl); lf>=0 && vspen(table[lf][c].col); lf=prev(lf))
		vforml= lf;
	form= ctype(vforml,c);
	if (form != 's')
		{
		ct = reg(c,CLEFT);
		if (form=='a') ct = reg(c,CMID);
		if (form=='n' && table[nl][c].rcol && lused[c]==0) ct= reg(c,CMID);
		fprintf(tabout, "\\h'|\\n(%2su'", ct);
		}
	s= table[nl][c].col;
	fn = font[c][stynum[vforml]];
	size = csize[c][stynum[vforml]];
	if (*size==0)size=0;
	if ((flags[c][stynum[nl]] & HALFUP)!=0 && pr1403 == 0)
		uphalf=1;
	switch(ct=ctype(vforml, c))
		{
		case 'n':
		case 'a':
			if (table[nl][c].rcol)
				{
			   if (lused[c]) /*Zero field width*/
				{
				ip = prev(nl);
				if (ip>=0)
				if (vspen(table[ip][c].col))
					{
					if (exvspen==0)
						{
						fprintf(tabout, "\\v'-(\\n(\\*(#du-\\n(^%cu", c+'a');
						if (cmidx)
							fprintf(tabout, "-((\\n(#-u-\\n(^%cu)/2u)", c+'a');
						vct++;
						if (pr1403) /* must round to whole lines */
							fprintf(tabout, "/1v*1v");
						fprintf(tabout, "'");
						exvspen=1;
						}
					}
				fprintf(tabout, "%c%c",F1,F2);
				if (uphalf) fprintf(tabout, "\\u");
				puttext(s,fn,size);
				if (uphalf) fprintf(tabout, "\\d");
				fprintf(tabout, "%c",F1);
				}
				s= table[nl][c].rcol;
				form=1;
				break;
				}
		case 'c':
			form=3; break;
		case 'r':
			form=2; break;
		case 'l':
			form=1; break;
		case '-':
		case '=':
			if (real(table[nl][c].col))
				fprintf(stderr,"%s: line %d: Data ignored on table line %d\n", ifile, iline-1, i+1);
			makeline(i,c,ct);
			continue;
		default:
			continue;
		}
	if (realsplit ? rused[c]: used[c]) /*Zero field width*/
		{
		/* form: 1 left, 2 right, 3 center adjust */
		if (ifline(s))
			{
			makeline(i,c,ifline(s));
			continue;
			}
		if (filler(s))
			{
			fprintf(tabout, "\\l'|\\n(%2su\\&%s'", reg(c,CRIGHT), s+2);
			continue;
			}
		if (vspen (s))
			{
			vspf = 1;
			continue;
			}
		ip = prev(nl);
		cmidx = (flags[c][stynum[nl]] & (CTOP|CDOWN))==0;
		if (ip>=0)
		if (vspen(table[ip][c].col))
			{
			if (exvspen==0)
				{
				fprintf(tabout, "\\v'-(\\n(\\*(#du-\\n(^%cu", c+'a');
				if (cmidx)
					fprintf(tabout, "-((\\n(#-u-\\n(^%cu)/2u)", c+'a');
				vct++;
				if (pr1403) /* round to whole lines */
					fprintf(tabout, "/1v*1v");
				fprintf(tabout, "'");
				}
			}
		fprintf(tabout, "%c", F1);
		if (form!= 1)
			fprintf(tabout, "%c", F2);
		if (uphalf) fprintf(tabout, "\\u");
		puttext(s, fn, size);
		if (uphalf) fprintf(tabout, "\\d");
		if (form !=2)
			fprintf(tabout, "%c", F2);
		fprintf(tabout, "%c", F1);
		}
	ip = prev(nl);
	if (ip>=0)
	if (vspen(table[ip][c].col))
		{
		exvspen = (c+1 < ncol) && vspen(table[ip][c+1].col) &&
			(topat[c] == topat[c+1]) &&
			(cmidx == (flags[c+1] [stynum[nl]]&(CTOP|CDOWN)==0)) && (left(i,c+1,&lwid)<0);
		if (exvspen==0)
			{
			fprintf(tabout, "\\v'(\\n(\\*(#du-\\n(^%cu", c+'a');
			if (cmidx)
				fprintf(tabout, "-((\\n(#-u-\\n(^%cu)/2u)", c+'a');
			vct++;
			if (pr1403) /* round to whole lines */
				fprintf(tabout, "/1v*1v");
			fprintf(tabout, "'");
			}
		}
	else
		exvspen=0;
	/* if lines need to be split for gcos here is the place for a backslash */
	if (vct > 7 && c < ncol)
		{
		fprintf(tabout, "\n.sp-1\n\\&");
		vct=0;
		}
	}
fprintf(tabout, "\n");
if (allh(i) && !pr1403) fprintf(tabout, ".vs \\n(%du\n", SVS);
if (watchout)
	{
	fprintf (tabout, ".sp |\\n(%du\n", S2);
	for(c=dv=0; c<ncol; c++)
		{
		if (i+1< nlin && (lf=left(i,c,&lwid))>=0)
			{
			if (dv++ == 0)
				fprintf(tabout, ".sp -1v\n");
			tohcol(c);
			dv++;
			drawvert(lf, i, c, lwid);
			}
		}
	if (dv)
		{
		fprintf(tabout,"\n");
		fprintf (tabout, ".sp |\\n(%du\n", S2);
		}
	}
if (vspf)
	{
	for(c=0; c<ncol; c++)
		if (vspen(table[nl][c].col) && (nl==0 || (lf=prev(nl))<0 || !vspen(table[lf][c].col)))
			{
			fprintf(tabout, ".nr ^%c \\n(#^u\n", 'a'+c);
			topat[c]=nl;
			}
	}
}
puttext(s,fn, size)
	char *s, *size, *fn;
{
if (point(s))
	{
	putfont(fn);
	putsize(size);
	fprintf(tabout, "%s",s);
	if (*fn>0) fprintf(tabout, "\\f\\n(%2d", S1);
	if (size!=0) putsize("0");
	}
}
funnies( stl, lin)
{
/* write out funny diverted things */
int c, s, pl, lwid, ct;
char *fn;
fprintf(tabout, ".mk ##\n"); /* rmember current vertical position */
for(c=0; c<ncol; c++)
	{
	s = (int)table[lin][c].col;
	if (point(s)) continue;
	if (s==0) continue;
	fprintf(tabout, ".nr %d ", SIND);
	for(pl=stl; pl>=0 && !isalpha(ct=ctype(pl,c)); pl=prev(pl))
		;
	switch (ct)
		{
		case 'n':
		case 'c':
			fprintf(tabout, "(\\n(%2su+\\n(%2su-\\n(%c-u)/2u\n",reg(c,CLEFT),reg(c-1+ctspan(lin,c),CRIGHT), s);
			break;
		case 'l':
			fprintf(tabout, "\\n(%2su\n",reg(c,CLEFT));
			break;
		case 'a':
			fprintf(tabout, "\\n(%2su\n",reg(c,CMID));
			break;
		case 'r':
			fprintf(tabout, "\\n(%2su-\\n(%c-u\n", reg(c,CRIGHT), s);
			break;
		}
	fprintf(tabout, ".in +\\n(%du\n", SIND);
	pl = prev(stl);
	if (stl>0 && pl>=0 && vspen(table[pl][c].col))
		{
		fprintf(tabout, ".sp |\\n(^%cu\n", 'a'+c);
		if ((flags[c][stynum[stl]]&(CTOP|CDOWN))==0)
			{
			fprintf(tabout, ".nr %d \\n(#-u-\\n(^%c-\\n(%c|+1v\n",TMP, 'a'+c, s);
			fprintf(tabout, ".if \\n(%d>0 .sp \\n(%du/2u", TMP, TMP);
			if (pr1403) /* round */
				fprintf(tabout, "/1v*1v");
			fprintf(tabout, "\n");
			}
		}
	fprintf(tabout, ".ie \\n(%c|>\\n(.t .nr #& 1\n", s);
	fprintf(tabout, ".el .nr #& 0\n");
	fn=font[c][stynum[stl]];
	putfont(fn);
	fprintf(tabout, ".%c+\n",s);
	fprintf(tabout, ".#%%\n");
	fprintf(tabout, ".in -\\n(%du\n", SIND);
	if (*fn>0) putfont("P");
	fprintf(tabout, ".sp |\\n(##u\n");
	}
}
putfont(fn)
	char *fn;
{
if (fn && *fn)
	fprintf(tabout,  fn[1] ? "\\f(%.2s" : "\\f%.2s",  fn);
}
putsize(s)
	char *s;
{
if (s && *s)
	fprintf(tabout, "\\s%s",s);
}
