long	efs_wildchecksum = 0L;

long
efs_cksum(src, len)
	register unsigned short *src;
	int len;
{
	register long a;

	a = 0;
	while (--len >= 0) {
		a ^= *src++;
		a = (a << 1) | (a < 0);
	}
	if (a == efs_wildchecksum)
		a = ~a;
	return a;
}
