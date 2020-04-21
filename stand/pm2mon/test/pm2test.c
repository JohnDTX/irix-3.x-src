extern int __charbuf;

main() 
{
	/* test the standalone get/put stuff */

	int i,c,c1;
	while (1) {
	printf("(p)uts,(g)ets,(e)xit:");
	c = getchar();
	putchar('\n');
		switch (c) {

			case 'p':	for (i=1000;--i;) putchar('Q');
						break;
			case 'g':	while (getchar() != 'q') ;
						break;

			case 'e':	return(0);

			default:	printf("got %c (%x)\n",c,c);

		}

		c1 = __charbuf;
		c = getchar();
		printf("charbuf was %c (%x), getchar was %c (%x)\n",c1,c1,c,c);

	}
}

