/*
 * BSD compatibility routine
 *
 * Stub for 'getgroups' system call
 */

getgroups(nent, gps)
int *nent, *gps;
{
	/*
	|| SGI kernel doesn't support Berkeley groups
	*/
	*gps = getgid();
	*nent = 1;
	return(1);
}
