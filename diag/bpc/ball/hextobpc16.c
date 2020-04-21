#include <stdio.h>

main () {

int	width, height;
int	hexval, hexval2;
int	fmaddr;
char	s [100];
int	bytewidth;
int	wordwidth;
int	even;
int	h, wc, bc;

scanf ("%s %x", s, &width);
printf ("charwidth = 0x%x;\n", width);

scanf ("%s %x", s, &height);
printf ("charheight = 0x%x;\n", height);

bytewidth = (width+7) / 8;
wordwidth = (width+15) / 16;
even = (bytewidth == 2*wordwidth);
    
fmaddr = 0x100;

printf ("LDFMADDR (0x100)\n");
printf ("REQUEST (UC_SETADDRS, 0)\n");

for (h=0; h<height; h++) {
	for (wc=1; wc<=wordwidth; wc++) {
		scanf ("%x", &hexval);
		if (wc<wordwidth || even)
			scanf ("%x", &hexval2);
		else
			hexval2 = 0;
		printf ("REQUEST (UC_WRITEFONT, 0x%02x%02x);\n",
			hexval, hexval2);
		}
	}
}
