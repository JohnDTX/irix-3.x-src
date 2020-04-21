/*
 * Structure containing font info
 */
struct	fontchar {
	ushort	offset;
	char	w, h;
	char	xoff, yoff;
	short	width;
};

/*
 * this is a "parallel" struct defined to make font loading easier
 * and must be the same size as struct fontchar
 */
struct	ufontchar {
	long l1, l2;
};

/* sizes of the default font */
#define defont_ht		13
#define defont_nc		128
#define defont_nr		934
#define defont_height		15
#define defont_width		9
#define defont_descender	2

# ifdef KERNEL
# define GLYPHSPERFONT		128
extern struct	fontchar defont_font[];
extern char	defont_bits[];

/* font memory allocation information */
ushort	fo_baseaddr;
ushort	fo_freebytes;
# endif KERNEL
