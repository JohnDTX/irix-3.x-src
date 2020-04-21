#include "gl.h"
/* define the matrix that will transform the points 
		(this particular matrix does not change the points) */
float idmat[4][4] = {
	1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0};

main()
{
	unsigned short buf[200];
	register i,j,num;
	float *bufptr;

	ginit();
	pushmatrix();  	/* save the current transformation matrix */
	loadmatrix(idmat);  /* load the transformation matrix */
	feedback(buf,50);
	xfpt(1.0,2.0,3.0);
	xfpt4(5.0,6.0,7.0,3.14159);
	xfpt2i(7,8);
	num = endfeedback(buf);
	popmatrix();  /* restore original transformation matrix */
	/* print out buf */
	for (i = 0; i < num; i++) {
		if( i % 8 == 0)
			printf("\n");
		printf(" %0.4x\t", buf[i]);
	}
	printf("\n");
	printf("\n");
	/* print out floating-point versions of the coordinates */
	for (i = 0; i < 3; i++) {
		bufptr = (float *)(&buf[1 + i*9]);
		for (j = 0; j < 4; j++)
			printf(" %f\t", *bufptr++);
		printf("\n");
	}
	gexit();
}
