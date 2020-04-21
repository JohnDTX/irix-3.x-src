#include "gl.h"

main()
{
	unsigned short buf[200];
	register i,j,num;
	float *bufptr;

	ginit();
	setdepth(0,10); /* set the near clipping plane to 0 and the
				far clipping plane to 10 */
	depthcue(1);  /* turn on depth-cue mode */
	feedback(buf,200);
	pnt(1.0,2.0,1.0);
	passthrough(1);
	pnt(1.0,2.0,-1.0);
	passthrough(2);
	pnt2i(0x123,0x234); /* arguments are in hexidecimal */
	passthrough(3);
	num = endfeedback(buf);
	depthcue(0);  /* turn off depth-cue mode */
	/* this section prints out the contents of buf */
	for (i = 0; i < num; i++) {
		if (i % 8 == 0)
			printf("\n");
		printf(" %0.4x\t", buf[i]);
	}
	printf("\n");
	printf("\n");
	gexit();
}
