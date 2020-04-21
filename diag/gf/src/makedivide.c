main()
{
register long i, j, k;
float a, b, c, d;

    d = 2.0;		/* 2 ** 1 */
    d = d * d;		/* 2 ** 2 */
    c = d = d * d;	/* 2 ** 4 */
    d = d * d;		/* 2 ** 8 */
    d = c * d;		/* 2 ** 12 */
    printf("unsigned short dividetab[] = {\n");
    for(i=0; i<32; i++) {
	for(j=0; j<32; j++) {
	    if(j) {
		a = i;
		b = j;
		c = a/b;
	    } else
		c = 0.0;
	    c = c * d;	/* c <<= 12 */
	    c += 0.5;	/* round up */
	    k = c;	/* 4.12 fixed point */
	/* print 16 bits of integer, followed by 16 bits of fraction: */
	    printf("\t0x%x, 0x%x,", k>>12, k&0xfff);
	    if((j&3) == 0x3)
		printf("\n");
	}
	printf("\n");
    }
    printf("};\n");
}
