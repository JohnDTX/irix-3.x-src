#include "gl.h"

main()
{
	unsigned short buf[200];
	register i,j,num;
	float *bufptr;

	ginit();
	feedback(buf, 200); 	/* turns on feedback mode, buf is the name
				of the feedback buffer and there is room
					for 200 16-bit values */
	pnt(1.0,2.0,-0.2);
	passthrough(1);  	/* store a marker in buf called "1" */
	pnt2(23.0,6.0);
	passthrough(2);  	/* store a marker called "2" */
	pnt2i(0x123,0x234);  	/* arguments are in hexidecimal */
	passthrough(3);  	/* store a marker called "3" */
	num = endfeedback(buf);
	/* the following section of the program prints out
		 the contents of the feedback buffer */
	for (i = 0; i < num; i++) {
		if (i % 8 == 0)
			printf("\n");
		printf(" %0.4x\t", buf[i]);
	}
	printf("\n");
	printf("\n");
	gexit();
}
