/* fbc.h   --- definitions for fbc microcode
 */

/* no-param function calls */

#define INTERRUPTHOST	_InterruptHost()
#define DOTOOUTREG	_DOToOutreg()
#define READBPCBUS	_ReadBPCBus()
#define SETLED		_SetLED()
#define GEOMENGDATA	_GeomEngData()
#define MULDIV		_MulDiv()
#define PROPIN		_PropIn()
#define PROPOUT12	_PropOut12()
#define PROPOUT16	_PropOut16()
#define CLRFLAG		_ClrFlag()


#define NEXT(n)		next(n)
#define CONST(n)	const(n)

