#include "complex.h"

extern	char	*sbrk(int);

main()
{
	complex c1;
	complex c2;
	complex c3;
	complex c4;

	printf("Brk before loop = %x\n", sbrk(0));
	for (int i = 0; i < 10000; i++) {
		c4 = c3 + c2 + c1;
	}
	printf("c1=%f.%f c2=%f.%f c3=%f.%f\n", c1, c2, c3);
	printf("Brk after loop = %x\n", sbrk(0));
}
