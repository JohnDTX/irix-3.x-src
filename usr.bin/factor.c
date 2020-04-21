#ifndef	lint
char _Origin_[] = "System V";
/* $Source: /d2/3.7/src/usr.bin/RCS/factor.c,v $ */
static	char	Sccsid[] = "@(#)$Revision: 1.1 $";
/* $Date: 89/03/27 17:40:06 $ */
#endif

/*	@(#)factor.c	1.1	*/

/*	factor	COMPILE:	cc -O factor.c -s -n -lm -o factor	*/
/*
 * works with up to 14 digit numbers
 * running time is proportional to sqrt(n)
 * accepts arguments either as input or on command line
 * 0 input terminates processing
 */

long float _lmodf(), _lsqrt();
long float nn, vv;
long float huge = 1.0e14;
long float sq[] = {
	10., 2., 4., 2., 4., 6., 2., 6.,
	 4., 2., 4., 6., 6., 2., 6., 4.,
	 2., 6., 4., 6., 8., 4., 2., 4.,
	 2., 4., 8., 6., 4., 6., 2., 4.,
	 6., 2., 6., 6., 4., 2., 4., 6.,
	 2., 6., 4., 2., 4., 2.,10., 2.,
};

main(argc, argv)
int argc;
char *argv[];
{
	int test = 1;
	int ret;
	register int j;
	long float junk, temp;
	long float fr;
	long float ii;

	if(argc > 2){
		printf("Usage: factor number\n");
		exit(0);
	}
	if(argc == 2){
		ret = sscanf(argv[1], "%lf", &nn);
		test = 0;
		printf("%.0lf\n", nn);
		goto start;
	}
	while(test == 1){
		ret = scanf("%lf", &nn);
start:
		if((ret<1) || (nn == 0.0)){
			exit(0);
		}
		if((nn<0.0) || (nn>huge)){
			printf("Ouch!\n");
			continue;
		}
		fr = _lmodf(nn, &junk);
		if(fr != 0.0){
			printf("Not an integer!\n");
			continue;
		}
		vv = 1. + _lsqrt(nn);
		try((long float)2.0);
		try((long float)3.0);
		try((long float)5.0);
		try((long float)7.0);
		if (nn == 1.0)
			goto done;
		ii = 1.0;
		while(ii < vv){
			for(j=0; j<48; j++){
				ii += sq[j];
retry:
				if (ii >= vv)
					goto done;
				_lmodf(nn/ii, &temp);
				if(nn == temp*ii){
					printf("     %.0lf\n", ii);
					nn = nn/ii;
					vv = 1. + _lsqrt(nn);
					if (ii >= vv)
						goto done;
					goto retry;
				}
			}
		}
done:
		if(nn > 1.0){
			printf("     %.0lf\n", nn);
		}
		printf("\n");
	}
}

try(arg)
long float arg;
{
	long float temp;
retry:
	_lmodf(nn/arg, &temp);
	if(nn == temp*arg){
		printf("     %.0lf\n", arg);
		nn = nn/arg;
		vv = 1. + _lsqrt(nn);
		goto retry;
	}
	return;
}
