/* fbinit.c   -- initialize field mnemonic arrays etc.
 * 	calls initmacros()
 *	supports GF2, other
 */

#include "fbcld.h"

extern char *field[][NENTRIES];
extern char *cats[];

devinit()
{
	register char **cp;

#define f *cp++

cp = field[CDN];
  f="true"; f="neg"; f="ovr"; f="not zero"; f="zero";
  f="not neg"; f="not host flag"; f="false"; f=".";

/* when first element is ".", second element is numeric range  */

  field[CIN][0]="."; field[CIN][1]="0-1";
  field[HIB][0]="."; field[HIB][1]="0-1";
#if GF2 || DEVEL
  field[RJU][0]="lj,short (cin0,cout16)";
  field[RJU][1]="rj,short (cin0,cout12)";
  field[RJU][2]="lj,long (zbus,cout16)";
  field[RJU][3]="rj,long (dcout,cout12)";
  field[RJU][4]=".";
#else
  field[RJU][0]="."; field[RJU][1]="0-1";
#endif

cp = field[SEQ];
  f="jzer"; f="cjs"; f="jmap"; f="cjp"; f="push"; f="jsrp";
  f="vect"; f="cjrp"; f="loup(rfct)"; f="rpct"; f ="retn";
  f ="cjpp"; f ="ldct"; f ="test(loop)"; f ="cont"; f ="thwb";
  f = ".";

cp =  field[RAM];
 f="read reg"; f="wrt ram"; f="float"; f="read ram"; f=".";

cp =  field[IOC];
  f="---"; f="input"; f="2 (put)"; f="3 (inp,put)"; f="load out";
  f="load out & input"; f="send output"; f="input, send output";
  f=".";

#ifndef GF2
  field[CLK][0]="short"; field[CLK][1]="long"; field[CLK][2]=".";
#endif

cp = field[MXA];
  f="reg"; f="rt di"; f="left di"; f="all di";
  f=".";

  field[RGA][0]="."; field[RGA][1]="0-f";
  field[RGB][0]="."; field[RGB][1]="0-f";

cp = field[ALU];
  f="ones (spcl)"; f="s-r-1+c"; f="r-s-1+c"; f="r+s+c"; f="s+c";
  f="~s + c"; f="r+c"; f="~r + c"; f="zeros"; f="~r and s";
  f ="r xnor s"; f ="r xor s"; f ="r and s"; f ="r nor s";
  f ="r nand s"; f ="r or s";
  f = ".";

cp =  field[DST];
  f="af/2 (mul)"; f="lf/2"; f="af/2 q/2 (2cmul)"; f="lf/2 q/2";
  f="f (inc1,2)"; f="f q/2 nw (sm-2c)"; f="f ldq nw (last mul)";
  f="f ldq"; f="a2f (norm)"; f="l2f"; f ="a2f 2q (div1)";
  f ="l2f 2q"; f ="f nw (div)"; f ="f 2q nw"; f ="signx (divcor)";
  f ="f";
  f = ".";

  field[MXB][0]="ram/reg"; field[MXB][1]="q"; field[MXB][2]=".";

cp = field[FBC];
#ifdef UC3
  f="--- (clip ld)"; f="load MAR (font wrt)"; f="inc MAR (color ld)";
  f="dec MAR (nop)"; f="interrupt (word read)";
  f="set LED (word wrt)"; f="BPC read (rotate wd)";
  f="--- (xy adr ld)"; f="ld config (pixel wrt)"; f="ld ED (drawchar)";
  f ="ld EC (rect)"; f ="ld XS (clear)"; f ="ld XE (line 0)";
  f ="ld YS (line 1)"; f ="ld YE (line 2)"; f ="ld FM adr (line 3)";
  f = ".";

  field[REV][0]="."; field[REV][1]="0-1";

#endif
#ifdef UC4
  f="--- (rdfont)";  f="ld ED (wrtfont)";  f="ld EC (rdrept)";
  f="ld XS (setaddrs)";  f="ld XE (savewd)";  f="ld YS (drawwd)";
  f="ld YE (readlstip)";  f="ld FA (noop)";  f="ld SAF";
  f="ld SAI (drawchar)";  f ="ld EAF (fillrect)";
  f ="ld EAI (filltrap)";  f ="ld SDF (vec 0)";  f ="ld SDI (vec 1)";
  f ="ld EDF (vec 2)";  f ="ld EDI (vec 3)";
  f ="ld MODE (scrmaskX)";  f ="ld REPT (scrmaskY)";
  f ="ld CONFIG (---)";  f ="ld MAR (---)";  f ="inc MAR (colorCD)";
  f ="dec MAR (colorAB)";  f ="interrupt (setweCD)";
  f ="setLED (setweAB)";  f ="fbread (rdpixelCD)";
  f ="ld MAR,int (rdpixelAB)";  f ="inc MAR,int (drawpixCD)";
  f ="dec MAR,int (drawpixAB)";  f ="ld MAR,fbread (vec 0R)";
  f ="inc MAR,fbread (vec 1R)";  f ="dec MAR,fbread (vec 2R)";
  f ="clrflag (vec 3R)";
#endif
#undef f

  field[BRA][0]="."; field[BRA][1]="0-ffff";

  cats[CDN]="cdn"; cats[CIN]="cin"; cats[SEQ]="seq"; cats[RAM]="ram";
  cats[IOC]="ioc"; cats[MXA]="mxa"; cats[RGA]="rga";
  cats[RGB]="rgb"; cats[RJU]="rju"; cats[HIB]="hib"; cats[FBC]="fbc";
  cats[ALU]="alu"; cats[DST]="dst"; cats[MXB]="mxb"; cats[BRA]="bra";
#ifndef GF2
  cats[REV]="rev"; cats[CLK]="clk";
#endif

  initmacros();

}
