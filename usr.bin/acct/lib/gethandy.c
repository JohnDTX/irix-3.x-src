#include <stdio.h>
#include "acctdef.h"

#define MAX_HOLS 20
#define HYFILE "/etc/holidays"
#define PNPFILE "pnp"

gethandy()
{
	char buf[80];
	int try;
	int count=0;
	FILE *hyfile;
	extern int thisyear, holidays[];

	if ( ( hyfile=fopen(HYFILE,"r")) == NULL )
		/* well, settle for btl 1980, I guess! */
		return;

	/* first get the year */
	if ( fgets(buf, 79, hyfile) != NULL ){
		if ( (try = atoi(buf)) > 1982 )
				thisyear = try; 
	}
	else{
		fclose( hyfile );
		return;
	}

	/* then get all the holidays */
	while ( fgets(buf, 79, hyfile) != NULL && count < MAX_HOLS-1 ){
		if ( (try = atoi(buf)) >= 0 )
			holidays[count++] = try;
	}

	holidays[count] = -1;
	fclose( hyfile );
	return;
}

getpnp()
{

	char buf[80];
	int t;
	char type [10];
	char de [10];
	int try;
	int hour, minute, second;
	int count=0;
	int nf;
	FILE *pnpfile;
	extern struct hours {
		int	h_sec;
		int	h_min;
		int	h_hour;
		int	h_type;		/* prime/nonprime of previous period */
	} h[];
	extern int daysend[];

	if ( ( pnpfile=fopen(PNPFILE,"r")) == NULL )
		/* well, settle for btl, I guess! */
		return;
	
/* in ../acctdef.h P and NP are defined as 0 and 1 resp.
 * we define here DE for daysend which fits in the same array 
 */
#define DE 2

	while ( fgets(buf, 79, pnpfile) != NULL  ){
		nf = sscanf(buf, "%s%d%d%d%s",
			type, &hour, &minute, &second, de  );
		

		if ( !strcmp( type, "P" ) )  t = P; else
		if ( !strcmp( type, "NP" ) ) t = NP; else
		if ( !strcmp( type, "DE" ) ) t = DE; else
		continue;

		h[t].h_sec = second;
		h[t].h_min = minute;
		h[t].h_hour = hour;
		switch (t){
			case P: h[t].h_type = NP;  break;
			case NP: h[t].h_type = P;  break;
			case DE:
				h[t].h_type = (!strcmp(de,"P"))?P:NP;
				daysend[0] = second;
				daysend[1] = minute;
				daysend[2] = hour;
				break;
		}
	

	}
	fclose( pnpfile );
	return;
}
