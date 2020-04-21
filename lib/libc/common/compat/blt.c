/*
 * C stub for old code which uses unisoft blt procedure
 */
blt(to, from, count)
{
	bcopy(from, to, count);
}
