/* devcmdsb.c  -- assorted subroutines for devcmd	*/

#include "pcmap.h"
#include "fbcld.h"
#include "gfdev.h"

extern unsigned short _ucode[][4];
extern char line[];	/* command line buffer */
extern short intoccurred;
extern short expecting_interrupt;
extern short expecting_output;
extern short devstatus;	/* copy of currently written status reg */
extern short ix;	/* command line index */
extern char cmd,which,how;
extern short num;	/* current ucode addr */
extern short val;	/* field designator for dostore */
extern short low,high;	/* limits for block store */
extern char bpset[];

extern char *field[][NENTRIES];		/* text prompts for ucode fields */

extern char *cats[NFIELDS];		/* names of fields */

getnum()
{
  int n=0;

  while (!validnum() && line[ix] != '\n') ix++;
  while (validnum())
    {
	if (line[ix] >= '0' && line[ix] <= '9')
		 n = n*16 + line[ix++] - '0';
	else n = n*16 + 10 + line[ix++] - 'a';
    }
  return(n);
}


validnum()
{ return(line[ix]>='0' && line[ix]<='9' || line[ix]>='a' && line[ix]<='f');}


match()	/* recognize 3-letter field name */
{
  short i;
  short s = -1;
  char *pc;

  for (i=0; i<NFIELDS; i++)
    {
	pc = cats[i];
	if (*pc==line[ix] && *(pc+1)==line[ix+1] && *(pc+2)==line[ix+2])
		s=i;
    }
  ix += 3;
  return(s);
}


printm()
{
    char *bpstr = "";

  if ( how == '.' || line[ix] == '.' )
    {
	if (line[ix] == '.') ++ix;
	if (line[ix] == '+') num += getnum();
	if (line[ix] == '-') num -= getnum();
    }
  else if ( how == ',' || line[ix] == ',') ++num;
  else num=getnum();

  if (cmd=='o') return(1);	/* o command just opens location	*/

#ifdef GF2
  if (bpset[num]) bpstr = " bp";
#endif
  if (how == 't')
    {
	printf("%x%s",num,bpstr);
	printfields();
    }
  else printf("%x%s/  %x  %x  %x  %x\n",num,bpstr,_ucode[num][0],
				_ucode[num][1],_ucode[num][2],_ucode[num][3]);
}


storem()
{
  short wdno;

  if (how == 'n')
    {
	if ((wdno = getnum()) >= 0 && wdno < 4)
		dostore_num();
	else printf("no such word no.\n");
    }
  else if (how == '?')
    {
	for (wdno=0; wdno < NFIELDS; wdno++) printf("%s  ",cats[wdno]);
	printf("\n");
    }
  else if ((wdno = match()) >= 0)		  /* assume how = 't' */
    {
	while (line[ix] == '?') printchoices(wdno);
	dostore_text(wdno);
    }
  else printf("no such field\n");
}


dostore_num(wd)
short wd;
{

  switch(which) {
    case 'm': _ucode[num][wd] = getnum(); break;
    case 'b': val = getnum();
	getlohi();
	for (; low <= high; ++low) _ucode[low][wd] = val;
	break;
    }
}


REPLACE(value,word,mask,range,shift)
short value,word,mask,range,shift;
{
  short new;

  new = shift<0 ? (value & range)>>-shift : (value & range)<<shift;
  switch(which) {
    case 'm':  _ucode[num][word] = _ucode[num][word] & ~mask | new;
	break;
    case 'b': getlohi();
	for (; low<=high; ++low)
		_ucode[low][word] = _ucode[low][word] & ~mask | new;
	break;
    }
}


getlohi()
{
	low = getnum(); low = low<0 ? 0 : low;
	high = getnum(); high = high>1023 ? 1023 : high;
}


printflag()
{
  unsigned short flg;

  flg = FBCflags;
  if (how == 't')
    {
	printflagbits(flg);
	printf("\tFBC INTERRUPT = %d",!(flg&INTERRUPT_BIT_));
	printf("\n\texpecting_interrupt = %d",expecting_interrupt);
	printf("\n\texpecting_output = %d\n",expecting_output);
	intoccurred = 0;
    }
  printf("\tread = %02x \twritten = %x\n",flg,devstatus);
}


storeflag()
{
  if (how == 'n') FBCflags = devstatus = getnum();
  else if (how == '?') printflaghelp();
  else printf("bad mode\n");
}


printchoices(wd)
  short wd;
{
  short i;

  if (*field[wd][0] == '.')
	printf("\tnumeric %s\n",field[wd][1]);
  else for (i=0; i<NENTRIES && *field[wd][i] != '.'; i++) {
	  printf("\t%x  %s\n",i,field[wd][i]);
	  if (i==16) {
		printf("--more--");
		getchar();
		putchar('\n');
	  }
  }
  for (i=0; i<ix;i++) putchar(line[i]);	/* retype cmd input */
  ix=0;
  getlin();	/* wait for value input*/
}


printgflags()
{
    register short flg = GEflags;
    register short i;

#if GFBETA | GF2
    if (flg & LOWATER_BIT) printf("LOWATER  ");
    if (flg & HIWATER_BIT) printf("HIWATER  ");
    if (flg & FIFOINT_BIT) printf("FIFOINT  ");
    if (flg & TRAPINT_BIT)
	printf("TRAP: ");
    flg = flg>>3;
    for (i=1; i<13; i++) {
	if (flg & 1) printf("%d ",i);
	flg = flg>>1;
    }
#endif GFBETA
    putchar('\n');
}

