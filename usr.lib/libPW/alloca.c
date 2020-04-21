char *
alloca(nbytes)
unsigned nbytes;
{
#define notdef
#ifdef notdef
	static int firstime = 1;
	static char * block;

	if (firstime) {
		firstime = 0;
		return (block = (char *)xalloc(nbytes));
	}
	else {
		xfree(block);
		return (block = (char *)xalloc(nbytes));
	}
#else
	static char foo[2000];
	return(foo);
#endif
}
