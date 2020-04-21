/* fbctext.c  -- text-related routines for devcmd.c
 *		supports GFALPHA,  GFBETA,  GF2
 */

#include "fbcld.h"
#include "gfdev.h"

extern char *field[][NENTRIES];	/* text prompts for ucode fields */
extern short _ucode[][4];
extern short num;
short val;
int i1,i2;

printfields()
{
  printf("\tALU = %s",field[ALU][(_ucode[num][1]&0x1E)>>1]);
  printf("\tDST = %s",field[DST][(_ucode[num][1]&0x1E0)>>5]);
  printf("\tCIN = %x",(_ucode[num][1]&0x200)>>9);
  printf("\n");
  printf("\tMXA = %s",field[MXA][(_ucode[num][1]&0xC00)>>10]);
  printf("\tMXB = %s",field[MXB][(_ucode[num][1]&1)]);
#if GF2 || DEVEL
  i1 = (_ucode[num][2]&0x800)>>11;
  i2 = (_ucode[num][2]&0x10)>>3;
  printf("\tRJU = %s",field[RJU][ i1 + i2 ]);
#else
  printf("\tRJU = %x",(_ucode[num][2]&0x800)>>11);
#endif
  printf("\tHIB = %x",(_ucode[num][2]&0x400)>>10);
  printf("\n");
  printf("\tRGA = %x",(_ucode[num][1]&0xF000)>>12);
  printf("\t\tRGB = %x",(_ucode[num][2]&0xF));
  printf("\t\tRAM = %s",field[RAM][(_ucode[num][2]&0x300)>>8]);
  printf("\n");
  printf("\tIOC = %s",field[IOC][(_ucode[num][2]&0xE0)>>5]);
#if GF2 || DEVEL
  printf("\tFBC = %s",field[FBC][(_ucode[num][3]&0xF8)>>3]);
#else
  printf("\tFBC = %s",field[FBC][(_ucode[num][3]&0x78)>>3]);
  printf("\tREV = %x",(_ucode[num][3]&0x80)>>7);
#endif
  printf("\n");
  printf("\tCDN = %s",field[CDN][_ucode[num][3]&7]);
  printf("\tSEQ = %s",field[SEQ][(_ucode[num][2]&0xF000)>>12]);
  printf("\tBRA = %x",(unsigned short)_ucode[num][0]);
#if GF2 || DEVEL
  printf("\n");
#else
  printf("\t\tCLK = %s",field[CLK][(_ucode[num][2]&0x10)>>4]);
#endif
}

dostore_text(wd)
short wd;
{
  val=getnum();
  switch(wd) {
  case CDN: REPLACE(val,3,07,07,0); break;
  case CIN: REPLACE(val,1,0x200,01,9); break;
  case SEQ: REPLACE(val,2,0xf000,0xf,12); break;
  case HIB: REPLACE(val,2,0x400,01,10); break;
  case RAM: REPLACE(val,2,0x300,03,8); break;
  case IOC: REPLACE(val,2,0xe0,07,5); break;
#if GF2 || DEVEL
  case FBC: REPLACE(val,3,0xF8,0x1f,3); break;
  case RJU: REPLACE(val,2,0x800,01,11);		/* rightjust  bit */
	    REPLACE(val,2,0x10,02,3); break;	/* clklong bit */
#else
  case FBC: REPLACE(val,3,0x78,0xf,3); break;
  case REV: REPLACE(val,3,0x80,01,7); break;
  case RJU: REPLACE(val,2,0x800,01,11); break;
  case CLK: REPLACE(val,2,0x10,01,4); break;
#endif
  case MXA: REPLACE(val,1,0xc00,03,10); break;
  case RGA: REPLACE(val,1,0xf000,0x0f,12); break;
  case RGB: REPLACE(val,2,0xf,0xf,0); break;
  case ALU: REPLACE(val,1,0x1e,0xf,1); break;
  case DST: REPLACE(val,1,0x1e0,0xf,5); break;
  case MXB: REPLACE(val,1,01,01,0); break;
  case BRA: REPLACE(val,0,0xffff,0xffff,0);
  }
}

printflagbits(flg)
  short flg;
{
  printf("\tGE REQ = %d ",!(flg&GEREQ_BIT_));
  printf("\tFBC REQ = %d ",!(flg&FBCREQ_BIT_));
#ifdef GFALPHA
  printf("\tFBC ACK = %d ",!(flg&FBCACK_BIT_));
  printf("\tBPC ACK = %d ",!(flg&BPCACK_BIT_));
#endif
#ifdef GF2
  printf("\n\tFI TRAP = %d ",!(flg&FITRAP_BIT_));
  printf(  "\tFO TRAP = %d ",!(flg&FOTRAP_BIT_));
  printf("\nGET = %d ",(flg&GET_BIT)!=0);
  printf("  FBC ACK = %d ",(flg&FBCACK_BIT)!=0);
  printf("  BPC ACK = %d ",(flg&BPCACK_BIT)!=0);
#endif
  printf("\n");

#ifdef GFALPHA
  printf("\tENAB DO = %d ",(flg&ENABDOFBD_BIT)!=0);
  printf("\tRETRACE = %d ",!(flg&VERTINT_BIT_));
#else
  printf("\tRETRACE = %d ",(flg&VERTINT_BIT)!=0);
#endif
  printf("\tVERT INTRPT = %d ",!(flg&NEWVERT_BIT_));
#ifdef GF2
  printf("\n\tTOKEN = %d",!(flg&TOKEN_BIT_));
#endif
}

printflaghelp()
{
   printf("  bits 3..0 (MAINT MAINTSEL2..0)\n\t0 float\n");
   printf("\t1 normal run\n\t2 read data out\n");
   printf("\t3 subst run\n\t4 read inp rjust\n");
   printf("\t5 normal run w/ flag\n\t6 read BPC (& cmd out)\n");
   printf("\t7 subst run w/ flag\n\t8 read inp rjust\n");
   printf("\t9 read data out\n\ta read microseq DI\n");
#ifdef GF2
  printf("\tb read MBus\n\tc read inp ljust\n");
  printf("\td read BPC\n\te wrt micro mode\n\tf read micro mode\n");
#else
   printf("\tb read MBus/wrt micro\n\tc read inp ljust\n");
   printf("\td read cmd & BPC\n");
#endif
#ifdef GFALPHA
   printf("\n\tbit 4\t SUBST\n");
   printf("\tbit 5\t-FORCE REQ\n\tbit 6\t-FORCE ACK\n\tbit 7\t-ENABVERTINT\n\n");
#else
   printf("\n  bit 4\t-FORCE REQ\n  bit 5\t-FORCE ACK\n  bit 6\t SUBSTIN\n  bit 7\t SUBSTOUT\n");
#endif
}

gshelp()
{
#ifdef GF2
printf("   1  -GE RESET\n   2  -SUBST BPC CODE\n   4  -ENAB FIFO INT\n");
printf("   8  -ENAB TRAP INT\n  10  -ENAB TOKEN INT\n  20  -ENAB VERT INT\n");
printf(" 400  ENAB FBC INT\n 800  AUTOCLEAR\n8000  -MICRO ACCESS\n\n");
#endif
}
