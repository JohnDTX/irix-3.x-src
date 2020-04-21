/*	zshade_scanline
 *
 *	zshade_scanline dumps out the current span into the frame
 *	buffer and doing z compares with the scratch rams zbuffer
 *
 */
#include "mas.h"
#include "fbcparams.h"
#include "fbc.h"
#include "consts.h"
#include "fbc.mic.h"
#include "scandefs.h"

/* registers used 
    X_ADDRESS,
    START_COLOR_HI
    START_COLOR_LO
    START_Z_HI
    START_Z_LO
    DEL_COLOR_HI
    DEL_COLOR_LO
    DEL_Z_HI
    DEL_Z_LO
    X_NOT_LOADED
    COUNT
    INIT_HI
    INIT_LO
    SAV_X_ADDRESS
    SAV_START_COLOR_HI
    SAV_START_COLOR_LO
    SAV_START_Z_HI
    SAV_START_Z_LO
*/

zshade_scanline()
{
newfile("zshadescan.c");

label(ZSHADESCANLINE)

    _NS /* get the xaddress */
	LOADREG(_SAV_X_ADDRESS, ALL16, MORE);
	LOADDI(INRJUST);
    _ES

    _NS /* get the startcolorhi */
	LOADREG(_SAV_START_COLOR_HI, ALL16, MORE);
	LOADDI(INRJUST);
    _ES

    _NS /* get the startcolorlo */
	LOADREG(_SAV_START_COLOR_LO, ALL16, MORE);
	LOADDI(INRJUST);
    _ES

    _NS /* get the startzhi */
	LOADREG(_SAV_START_Z_HI, ALL16, MORE);
	LOADDI(INRJUST);
    _ES

    _NS /* get the startzlo */
	LOADREG(_SAV_START_Z_LO, ALL16, MORE);
	LOADDI(INRJUST);
    _ES

    _NS /* get the delcolorhi */
	LOADREG(_DEL_COLOR_HI, ALL16, MORE);
	LOADDI(INRJUST);
    _ES

    _NS /* get the delcolorlo */
	LOADREG(_DEL_COLOR_LO, ALL16, MORE);
	LOADDI(INRJUST);
    _ES

    _NS /* get the delzhi */
	LOADREG(_DEL_Z_HI, ALL16, MORE);
	LOADDI(INRJUST);
    _ES

    _NS /* get the delzlo */
	LOADREG(_DEL_Z_LO, ALL16, MORE);
	LOADDI(INRJUST);
    _ES
 
    _NS
    	LOADREG(_COUNT, ALL16, NOMORE);
	LOADDI(INRJUST);
    _ES

    _NS /* get the count  call RESET_MASKS for multi-screen*/
	REGHOLD;
	SEQ(JSUB);
	NEXT(RESET_MASKS);
    _ES
    
label(MULTMASK_LOOP)
    _NS /* restore all values to be reused in multimasks */
	REGREG(RONLYOP, P0, _SAV_X_ADDRESS, _X_ADDRESS);
    _ES
    
    _NS
	REGREG(RONLYOP, P0, _SAV_START_COLOR_HI, _START_COLOR_HI);
    _ES
    
    _NS
	REGREG(RONLYOP, P0, _SAV_START_COLOR_LO, _START_COLOR_LO);
    _ES
    
    _NS
	REGREG(RONLYOP, P0, _SAV_START_Z_HI, _START_Z_HI);
    _ES
    
    _NS
	REGREG(RONLYOP, P0, _SAV_START_Z_LO, _START_Z_LO);
    _ES
    
    _NS /* load the y address each time */
	REGREG(RONLYOP, P0, _Y_ADDRESS, _Y_ADDRESS);
	DOTOOUTREG;
	BPCCMD(LOADYS);
    _ES
    
    _NS /* load the xend register with a big number */
	ALUOP(RONLYOP, P0);
	SETROP(0, ALL16);
	SETSOP(NONQOP, 0, RAMNOP);
	LOADDI(UCONST);
	CONST(0x7fff);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
	BPCCMD(LOADXE);
    _ES

    _NS /* load the yend register with 1024 */
	ALUOP(RONLYOP, P0);
	SETROP(0, ALL16);
	SETSOP(NONQOP, 0, RAMNOP);
	LOADDI(UCONST);
	FTOYANDQ(FF, OLDQ, REGWRD);
	CONST(1024);
	DOTOOUTREG;
	BPCCMD(LOADYE);
    _ES
    
    _NS /* get count to Q & DO (need count's register so save it in Q) */
	ALUOP(SONLYOP, P0);
	SETSOP(NONQOP, _COUNT, RAMNOP);
	FTOYANDQ(FF, LDQ, REGWRD);
	DOTOOUTREG;
    _ES

    _NS /* write the count to the counter, move the (x address)*2 for calc */
	/* x_not_loaded = x_address * 2  */
	ALUOP(RONLYOP, P0);
	SETROP(_X_ADDRESS, NONE);
	SETSOP(NONQOP, _X_NOT_LOADED, RAMNOP);
	FTOYANDQ(FAL, QL, REGWRE);
	LOADDI(OUTPUTCOUNT);
	SEQ(LDCT);
    _ES

    _NS /* calculate the address of the z's */
	/* x_not_loaded = 4094 - x_not_loaded  */
	ALUOP(SUBRSOP, P1);
	SETROP(0, ALL16);
	LOADDI(UCONST);
	CONST(4094);
	SETSOP(NONQOP, _X_NOT_LOADED, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE); 
	DOTOMAR(LOAD);
    _ES

label(SCAN_LINE_LOOP)
    _NS /* check the current z and the z in scratch */
	REGRAMCOMP(GT, _START_Z_LO, INC);
	PROPOUT16;
    _ES

    _NS /* now test the high byte */
	REGRAMCOMP(GT, _START_Z_HI, DEC);
	PROPIN;
    _ES

    _NS /* check the current z and the z in scratch */
	SEQ(JUMP);
	COND(IFOVF);
	NEXT(HANDLEOVERFLOW);
	REGRAMCOMP(GT, _START_Z_LO, INC);
	PROPOUT16;
    _ES

    _NS /* now test the high byte */
	REGRAMCOMP(GT, _START_Z_HI, HOLD);
	PROPIN;
    _ES

    _NS
	/* test xloaded */
	REGREG(RONLYOP, P0, _X_NOT_LOADED, _X_NOT_LOADED);
	SEQ(JUMP);
	COND(IFGT);
	NEXT(DONT_WRITE);
    _ES

label(WRITE_THE_Z)
    _NS /* write START_Z_HI */
	ALUOP(RONLYOP, P0);
	SETROP(_START_Z_HI, NONE);
	SETSOP(NONQOP, _START_Z_HI, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOMAR(DEC);
	SEQ(JUMP);
	COND(IFZ);
	NEXT(DONT_LOAD_X);
    _ES

    _NS /* load XS with X_ADDRESS */
	REGREG(RONLYOP, P0, _X_ADDRESS, _X_ADDRESS);
	DOTOOUTREG;
	BPCCMD(LOADXS);
    _ES

    _NS /* do the SETADDRS and clear the X_NOT_LOADED flag */
	REGREG(FLOWOP, P0, 0, _X_NOT_LOADED);
	DOTOOUTREG;
	BPCCMD(SETADDRS);
    _ES

label(DONT_LOAD_X)
    _NS /* write the low half of Z to scratch, and write the pixel */
	ALUOP(RONLYOP, P0);
	SETROP(_START_COLOR_HI, NONE);
	SETSOP(NONQOP, _START_Z_LO, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
	BPCCMD(DRAWPIXELAB);
	SEQ(JUMP);
	NEXT(SKIP_NO_WRITE);
    _ES

label(DONT_WRITE)
    _NS /* set X_NOT_LOADED and decrement the MAR */
	REGREG(FHIGHOP, P0, _X_NOT_LOADED, _X_NOT_LOADED);
	DOTOMAR(DEC);
    _ES

label(SKIP_NO_WRITE)
    _NS /* increment X_ADDRESS and DECMAR */
	REGREG(RONLYOP, P1, _X_ADDRESS, _X_ADDRESS);
	DOTOMAR(DEC);
    _ES

    _NS /* bump color and z along too */
	REGREG(ADDOP, P0, _DEL_COLOR_LO, _START_COLOR_LO);
	PROPOUT16;
	DOTOMAR(DEC); /* now points to the low of the next z */
    _ES

    _NS
	REGREG(ADDOP, P0, _DEL_COLOR_HI, _START_COLOR_HI);
	PROPIN;
    _ES

    _NS
	REGREG(ADDOP, P0, _DEL_Z_LO, _START_Z_LO);
	PROPOUT16;
    _ES

    _NS
	REGREG(ADDOP, P0, _DEL_Z_HI, _START_Z_HI);
	PROPIN;
	SEQ(RPCT);
	NEXT(SCAN_LINE_LOOP);
    _ES
    
    _NS /* restore count from Q & check for more masks in multimask mode */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(QOPERAND, _COUNT, RAMNOP);
	FTOYANDQ(FAR, QR, REGWRE);
	SEQ(JSUB);
	NEXT(NEW_MASK);
    _ES
    
    _NS /* if returned non-zero, repeat the scanline */
	REGREG(RONLYOP, P0, 0, 0);
	COND(IFNZ);
	SEQ(JUMP);
	NEXT(MULTMASK_LOOP);
    _ES
    
    _NS /* return to dispatch */
	GEOMENGDATA;
	SEQ(JUMP);
	NEXT(DISPATCH);
    _ES

label(HANDLEOVERFLOW)
    _NS /* test the sign of the result */
	REGRAMCOMP(LE, _START_Z_HI, HOLD);
	PROPIN;
    _ES

    _NS /* jump to write case if the sign bit is set */
	REGREG(RONLYOP, P0, _X_NOT_LOADED, _X_NOT_LOADED);
	COND(IFGT);
	SEQ(JUMP);
	NEXT(WRITE_THE_Z);
    _ES

    _NS /* jump to the no write case */
	REGREG(RONLYOP, P0, _X_NOT_LOADED, _X_NOT_LOADED);
	SEQ(JUMP);
	NEXT(DONT_WRITE);
    _ES
	
}

scanline_init()
{
label(ZSCAN_INIT)
    _NS /* get the address of the div table messed up flag */
	LOADMAR(_DIVTAB_VALID);
	CONST(_DIVTAB_VALID);
    _ES

    _NS /* clear the flag */
	ALUOP(FLOWOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, _Y_ADDRESS, RAMNOP);
	FTOYANDQ(FF, OLDQ, REGWRE);
    _ES

    _NS /* get the yaddress */
	ALUOP(RONLYOP, P0);
	SETROP(0, ALL16); LOADDI(INRJUST);
	SETSOP(NONQOP, _Y_ADDRESS, RAMWR);
	FTOYANDQ(FF, OLDQ, REGWRE);
    _ES

    _NS /* get the current configuration register */
	LOADMAR(_CONFIG+1);
	CONST(_CONFIG+1);
    _ES

    _NS
	ALUOP(ANDOP, P0);
	SETROP(0, ALL16);
	LOADDI(UCONST);
	CONST(0xe0ff);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, LDQ, REGWRD);
	DOTOOUTREG;
	BPCCMD(LOADCONFIG);
    _ES

    _NS  /* now load the font address of the polystipple */
	LOADMAR(_POLYSTIPADR);
	CONST(_POLYSTIPADR);
    _ES

    _NS  /* send it to the BPC */
	ALUOP(SONLYOP, P0);
	SETROP(0, NONE);
	SETSOP(NONQOP, 0, RAMRD);
	FTOYANDQ(FF, OLDQ, REGWRD);
	DOTOOUTREG;
	BPCCMD(LOADFA);
    _ES

    _NS /* load the mar with 4094 */
	IMMREG(RONLYOP, P0, 4094, _TEMP);
	CONST(4095);
	DOTOMAR(LOAD);
    _ES

    _NS /* load INIT_HI and INIT_LO */
	IMMREG(RONLYOP, P0, 0xffff, _INIT_LO);
	CONST(0xffff);
    _ES

    _NS
	IMMREG(RONLYOP, P0, 0x7fff, _INIT_HI);
	CONST(0x7fff);
    _ES

    _NS /* load the counter with 1024 */
	REGHOLD;
	LOADDI(UCONST);
	CONST(1024);
	SEQ(LDCT);
    _ES

label(SCAN_INIT_LOOP)
    _NS /* load the high word */
	RAM(RAMWR, _INIT_HI, DEC);
    _ES

    _NS /* load the low word */
	RAM(RAMWR, _INIT_LO, DEC);
	SEQ(RPCT);
	NEXT(SCAN_INIT_LOOP);
    _ES

    _NS
	GEOMENGDATA;
	SEQ(JUMP);
	NEXT(DISPATCH);
    _ES
}
