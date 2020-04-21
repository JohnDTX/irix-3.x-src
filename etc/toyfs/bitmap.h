# define BITMAPWORDSHIFT	3
# define BITMAPWORDMASK		((1<<BITMAPWORDSHIFT)-1)

# define BITMAPWORD(b,i)	((char *)(b))[(i)>>BITMAPWORDSHIFT]
# define BITMAPBIT(b,i)		(1<<((i)&BITMAPWORDMASK))

# define GETBIT(b,i) (BITMAPWORD(b,i) & BITMAPBIT(b,i))
# define CLRBIT(b,i) (BITMAPWORD(b,i) &= ~BITMAPBIT(b,i))
# define SETBIT(b,i) (BITMAPWORD(b,i) |= BITMAPBIT(b,i))
