/*	@(#)erase.c	1.1	*/
extern vti;
erase(){
	int i;
	i=0401;
	write(vti,&i,2);
}
