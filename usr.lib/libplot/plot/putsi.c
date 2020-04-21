/*	@(#)putsi.c	1.1	*/
#include <stdio.h>
putsi(a){
	putc((char)a,stdout);
	putc((char)(a>>8),stdout);
}
