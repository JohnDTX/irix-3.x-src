# define PM2

# include "pmII.h"
# include "Qglobals.h"

char x[512] = 0;
main()
{
    register int i;

    DIAG_DISPLAY(0xF);
    delay(1000);
    DIAG_DISPLAY(0xD);
    delay(1000);
    DIAG_DISPLAY(0xE);
    i = 0;
    for( ;; )
    {
	printf("hi %d\n",i);
	DIAG_DISPLAY((i&0xF));
	i--;
	delay(1000);
    }
}

delay(x)
    int x;
{
    register int n;
    while( --x >= 0 )
	for( n = 125; --n >= 0; )
	    ;
}
