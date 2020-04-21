/* main.c 
 */

short dc_dcr = 0x3000;
int num;
char c;
int toterrs;
int mask;

main()
{
    int errs;

    mask = -1;
    toterrs = 0;
    num = 1;
    while (num != 0) {
	for ( ; num-- > 0; ) {
		errs = testall(1,mask);
		toterrs += errs;
		printf("\n   %d returned.  %d total errors\n",errs,toterrs);
	}
	printf("\nreps? ");
	num = getnum();
	printf("\nmask? ");
	mask = getnum();
    }
}

breakcheck()
{}


getnum()
{
	int num;

	num = 0;
	while ((c=getchar()) != 012)
		num = num * 10 + c - 0x30;	/* get a digit */
	if (num < 0) num = 0;
	return (num);
}
