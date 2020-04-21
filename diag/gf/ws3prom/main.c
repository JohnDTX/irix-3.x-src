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
	printf("\nreps? ");
	num = getnum();
	printf("\nmask? ");
	mask = getnum();

	if (mask==0)
		while(1) {gesend(0xff08); gesend(8);}
	else for ( ; num-- > 0; ) {
		errs = testall(1,mask);
		toterrs += errs;
		printf("\n   %d returned.  %d total errors\n",errs,toterrs);
	}
    }
}

breakcheck()
{}


getnum()
{
	int num;

	num = 0;
	while ((c=getchar()) != 012 && c != 015)
		num = num * 10 + c - 0x30;	/* get a digit */
	if (num < 0) num = 0;
	printf("    (0x%x)\n",num);
	return (num);
}
