/*	@(#)frame.c	1.1	*/
frame(n)
{
	extern vti;
	n=n&0377 | 02000;
	write(vti,&n,2);
}
