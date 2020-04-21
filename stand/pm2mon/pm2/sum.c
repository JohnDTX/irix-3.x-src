unsigned long checksum;

sum(num)
unsigned short *num;
{
	register unsigned long s;

	s = checksum;

	/* checksum the short num */
	s += *num;
	s &= 0xffff;
	s <<= 1;
	s |= (s >> 16);
	s &= 0xffff;

	checksum = s;
}
