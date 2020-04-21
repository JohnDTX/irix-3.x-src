#include "defs.h"

MSG ODDADR;
MSG BADDAT;
MSG BADTXT;
struct map txtmap;
struct map datmap;
int wtflag;
char * errflg;
int errno;
int pid;
unsigned accessf();

/* file handling and access routines */

put(adr,space,value)
long int adr;
{
	accessf(WT,adr,space,value);
}

unsigned get(adr, space)
long int adr;
{
	register unsigned data = accessf(RD,adr,space,0);
	return((data>>16)&0xFFFF);
}

unsigned chkget(n, space)
long int n;
{
	register int w;

	w = get(n, space);
	chkerr();
	return(w);
}

unsigned 
accessf(mode,adr,space,value)
long int adr;
{
	int w, w1, pmode, rd, file, rmode;
	rd = mode==RD;

	if( space == NSP ){
		return(0);
	}

	if( pid /* tracing on? */)
	{
		if( (adr&01) && !rd ){
			error(ODDADR);
		}
		rmode = (space&DSP? RDUSER : RIUSER);
		pmode = (space&DSP?(rd?RDUSER:WDUSER):
		(rd?RIUSER:WIUSER));
		if (!rd)
		{
			w1 = ptrace(rmode, pid, adr, 0);
			value = (w1&0xFFFF) | (value&0xFFFF0000);
		}
		w = ptrace(pmode, pid, (adr&~01), value);
		if( adr&01)
		{
			w1 = ptrace(pmode, pid, (adr+1), value);
			w = ((w<<8)&0xFFFF0000) | ((w1>>8)&0xFFFF);
		}
		if( errno)
		{
			errflg = (space&DSP ? BADDAT : BADTXT);
		}
		return(w);
	}
	w = 0;
	if( mode==WT && wtflag==0)
	{
		error("not in write mode");
	}
	if( !chkmap(&adr,space))
	{
		return(0);
	}
	file=(space&DSP?datmap.ufd:txtmap.ufd);
	if( longseek(file,adr)==0 ||
	    (rd ? read(file,&w,2) : write(file,&value,2)) < 1)
	{
		errflg=(space&DSP?BADDAT:BADTXT);
	}
	return(w);

}

chkmap(adr,space)
register long int *adr;
register int space;
{
	register struct map * amap;
	amap=((space&DSP?&datmap:&txtmap));
	if( space&STAR || !within(*adr,amap->b1,amap->e1))
	{
		if( within(*adr,amap->b2,amap->e2))
		{
			*adr += (amap->f2)-(amap->b2);
		} else {
			errflg=(space&DSP?BADDAT:BADTXT);
			return(0);
		}
	} else {
		*adr += (amap->f1)-(amap->b1);
	}
	return(1);
}

within(adr,lbd,ubd)
long int adr, lbd, ubd;
{
	return(adr>=lbd && adr<ubd);
}
