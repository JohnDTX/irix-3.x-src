#include <stdio.h>
#include "acctdef.h"

/*
 *	prime(0) and nonprime(1) times during a day
 *	for BTL, prime time is 9AM to 5PM
 */
int thisyear, holidays[20];
struct hours {
	int	h_sec;
	int	h_min;
	int	h_hour;
	int	h_type;		/* prime/nonprime of previous period */
} h[] = {
	0,	0,	9,	NP,	/* 9AM, BTL prime starts */
	0,	0,	17,	P,	/* 5PM, BTL prime ends */
	60,	59,	23,	NP,	/* daysend */
	-1
};
int	daysend[]	= {60, 59, 23};
main(){
	int i;
	getpnp();
	for ( i=0; i< 3; i++ )
		printf("%d: hour %d minute %d second %d after %d\n", i,
			h[i].h_hour, h[i].h_min, h[i].h_sec, h[i].h_type);
	printf("end: hour %d minute %d second %d after %d\n",
			daysend[2], daysend[1], daysend[0] );
			
}
