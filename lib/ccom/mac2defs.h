/*	68000 Registers */

	/* registers */
# define D0 0
# define D1 1
# define D2 2
# define D3 3
# define D4 4
# define D5 5
# define D6 6
# define D7 7
# define A0 8
# define A1 9
# define A2 10
# define A3 11
# define A4 12
# define A5 13
# define A6 14
# define SP 15

# define SAVEREGION 8 /* number of bytes for save area */

# define BYTEOFF(x) ((x)&01)
# define wdal(k) (BYTEOFF(k)==0)
# define BITOOR(x) ((x)>>3)  /* bit offset to oreg offset */

# define REGSZ 16

# define TMPREG A6


# define STOARG(p)     /* just evaluate the arguments, and be done with it... */
# define STOFARG(p)
# define STOSTARG(p)
# define genfcall(a,b) gencall(a,b)


	/* shape for constants between -128 and 127 */
# define SCCON (SPECIAL+100)
	/* shape for constants between 0 and 32767 */
# define SICON (SPECIAL+101)
	/* shape for constants between 1 and 8 */
# define S8CON (SPECIAL+102)

# define MYREADER(p) myreader(p)
extern int fltused;
	/* calls can be nested on the 68000 */
# define NESTCALLS
