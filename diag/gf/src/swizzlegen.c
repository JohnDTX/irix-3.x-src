/* swizzletab.c
 */

main()
{
	register src,dest;

	printf("unsigned short swizzletab[] = {\n");

	for (src=0; src<256; src++) {
		dest = halfswiz(src);
		dest += halfswiz(src>>1) <<8;
		printf("0x%x,\n",dest);
	}
	printf("0};\n");
}

halfswiz(s)
	register s;
{
	register mask,dst,i;

	dst = 0;
	mask = 1;

	for (i=0; i<8; i++) {
		dst += s & mask;
		mask <<= 1;
		s >>= 1;
	}
	return(dst);
}
