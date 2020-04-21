/*
 * 	makemap - 
 *		Make the color map using the clues provided by ~/.desktop
 *		most of the mextools need the colors mapped by this program.
 *
 *				Paul Haeberli - 1984
 *
 */
#include "gl.h"
#include "port.h"

main(argc,argv)
int argc;
char **argv;
{
    noport();
    winopen("makemap");
    if (argc>1)
	makemap(1);
    else
	makemap(0);
    gexit();
}

makemap(dotop)
int dotop;
{
    register int i, j, v;
    register int r, g, b, w;
    int planes;

/* map in the default colors */
    for (i=0; i<8; i++) {
	r = (i>>0)&1;
	g = (i>>1)&1;
	b = (i>>2)&1;
	gammapcolor(i,255*r,255*g,255*b);
    } 

/* restore all desktop colors from ~/.desktop */
    restorecolors();

/* get the color for cursor  . . . */

/* make a grey scale of 16 steps */
    for (i=0; i<16; i++) {
	v = i*16;
	gammapcolor(GREY(i),v,v,v);
    } 

/* make the first ordered color map */
    for (i=0; i<32; i++) {
	r = (i>>0) & 3;
	g = (i>>2) & 3;
  	b = (i>>4) & 1;
	r = (255*r)/3;
	g = (255*g)/3;
	b *= 255;
	gammapcolor(i+32,r,g,b);
    }
    planes = getplanes();

/* if there are more than 8 planes make a random color map 64 to 128 */
    if (planes > 8) 
	for (i=64; i<128; i++) 
	    gammapcolor(i,rand()&0xff,rand()&0xff,rand()&0xff);

/* if there are more than 8 planes make a ramp at 128 */
    if (planes > 8) 
	for (i=0; i<128; i++) 
	    gammapcolor(i+128,i<<1,i<<1,i<<1);

/* if there are more than 8 planes make an ordered color map at 256 */
    if (planes > 8) 
	for (i=0; i<256; i++) {
	    r = (i>>0) & 7;
	    g = (i>>3) & 7;
	    b = (i>>6) & 3;
	    r = (255*r)/7;
	    g = (255*g)/7;
	    b = (255*b)/3;
	    gammapcolor(i+256,r,g,b);
    	}

/* if there are more than 8 planes make it red from 512 to 1023 */
    if (planes > 8) 
	for (i=512; i<1024; i++) 
	    gammapcolor(i,200,0,0);
}
