
/*
 * ibswab() --
 * swap bytes; because of strange bus arch.
 */
ibswab(wp,n)
    register short *wp;
    register int n;
{
    register short iii;
    n ++;
    n >>= 1;
    while( --n >= 0 )
    {
	iii = *wp;
asm("	rolw #8,d6");
	*wp++ = iii;
	/*the C (slow) way
	iii=cp[0];cp[0]=cp[busfix(0)];cp[busfix(0)]=iii;
	cp += 2;
	 */
    }
}
