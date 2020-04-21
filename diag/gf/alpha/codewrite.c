/* codewrite.c  -- just write microcode onto Alpha FBC board
 */

#include "/usr/sun/include/pcmap.h"
#include "gfdev.h"

extern unsigned short ucode[1024][4];

codewrite()
{
  short i,wd;

  DEVflags = WRITEMICRO;
  for (i=0; i<1024; i++)
    for (wd=0; wd<4; wd++)
	DEVmicro(i,wd) = ucode[i][wd];
  cycle_input();
  cycle_output();
}


cycle_input()
  {
	DEVflags = WRITEMICRO & ~FORCEREQ_BIT_; ;
	DEVflags = WRITEMICRO;
  }

cycle_output()
  {
	DEVflags = WRITEMICRO & ~FORCEACK_BIT_ ;
	DEVflags = WRITEMICRO;
  }
