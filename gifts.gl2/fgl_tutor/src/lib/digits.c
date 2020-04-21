#include "gl.h"
#include "device.h"

static float scalesize;

/*---------------------------------------------------------------------------
 * Library to draw calculator like digits on object space. i.e. They scale.
 *---------------------------------------------------------------------------
 */

set_digit_scale(n)
/*---------------------------------------------------------------------------
 * Set the digits scale factor to n.
 *---------------------------------------------------------------------------
 */
float n;
{
    scalesize = n;
}

draw_segment(n)
/*---------------------------------------------------------------------------
 * Draw the number segment n, using the current matrix.
 *---------------------------------------------------------------------------
 */
int n;
{

    switch (n) {
	case 1:
	    pmv(.2, 12.0);
	    pdr(5.8, 12.0);
	    pdr(4.8, 11.0);
	    pdr(1.2, 11.0);
	    pclos();
	    break;
	case 2:
	    pmv(6.0, 6.2);
	    pdr(6.0, 11.8);
	    pdr(5.0, 10.8);
	    pdr(5.0, 7.2);
	    pclos();
	    break;
	case 3:
	    pmv(0.0, 6.2);
	    pdr(1.0, 7.2);
	    pdr(1.0, 10.8);
	    pdr(0.0, 11.8);
	    pclos();
	    break;
	case 4:
	    pmv(1.0, 5.4);
	    pdr(5.0, 5.4);
	    pdr(5.8, 6.0);
	    pdr(5.0, 6.6);
	    pdr(1.0, 6.6);
	    pdr(.2, 6.0);
	    pclos();
	    break;
	case 5:
	    pmv(6.0, .2);
	    pdr(6.0, 5.8);
	    pdr(5.0, 4.8);
	    pdr(5.0, 1.2);
	    pclos();
	    break;
	case 6:
	    pmv(0.0, .2);
	    pdr(1.0, 1.2);
	    pdr(1.0, 4.8);
	    pdr(0.0, 5.8);
	    pclos();
	    break;
	case 7:
	    pmv(.2, 0.0);
	    pdr(5.8, 0.0);
	    pdr(4.8, 1.0);
	    pdr(1.2, 1.0);
	    pclos();
	    break;
	case 8:
	    rectf(2.0, 8.0, 4.0, 10.0);
	    break;
	case 9:
	    rectf(2.0, 2.0, 4.0, 4.0);
	    break;
	case 10:
	    pmv(0.0, 5.8);
	    pdr(1.0, 6.8);
	    pdr(3.0, 0.0);
	    pclos();
	    break;
	case 11:
	    pmv(3.0, 0.0);
	    pdr(5.0, 6.8);
	    pdr(6.0, 5.8);
	    pclos();
	    break;
	case 12:
	    rectf(0.0, 1.0, 1.0, 0.0);
	    break;
	case 13:
	    pmv(0.0, 0.0);
	    pdr(1.0, 0.0);
	    pdr(-1.0, -1.0);
	    pclos();
	    break;
    }
}

draw_char(n)
/*---------------------------------------------------------------------------
 * Draw the character n using the current matrix.
 *---------------------------------------------------------------------------
 */
int n;
{

    pushmatrix();
    translate(1.0, 2.0, 0.0);
    switch(n){
	case '0':
	    draw_segment(1);
	    draw_segment(2);
	    draw_segment(3);
	    draw_segment(5);
	    draw_segment(6);
	    draw_segment(7);
	    break;
	case '1':
	    draw_segment(2);
	    draw_segment(5);
	    break;
	case '2':
	    draw_segment(1);
	    draw_segment(2);
	    draw_segment(4);
	    draw_segment(6);
	    draw_segment(7);
	    break;
	case '3':
	    draw_segment(1);
	    draw_segment(2);
	    draw_segment(4);
	    draw_segment(5);
	    draw_segment(7);
	    break;
	case '4':
	    draw_segment(3);
	    draw_segment(4);
	    draw_segment(2);
	    draw_segment(5);
	    break;
	case '5':
	    draw_segment(1);
	    draw_segment(3);
	    draw_segment(4);
	    draw_segment(5);
	    draw_segment(7);
	    break;
	case '6':
	    draw_segment(1);
	    draw_segment(3);
	    draw_segment(4);
	    draw_segment(5);
	    draw_segment(6);
	    draw_segment(7);
	    break;
	case '7':
	    draw_segment(1);
	    draw_segment(2);
	    draw_segment(5);
	    break;
	case '8':
	    draw_segment(1);
	    draw_segment(2);
	    draw_segment(3);
	    draw_segment(4);
	    draw_segment(5);
	    draw_segment(6);
	    draw_segment(7);
	    break;
	case '9':
	    draw_segment(1);
	    draw_segment(2);
	    draw_segment(3);
	    draw_segment(4);
	    draw_segment(5);
	    draw_segment(7);
	    break;
	case ':':
	    draw_segment(8);
	    draw_segment(9);
	    break;
	case 'V':
	    draw_segment(10);
	    draw_segment(11);
	    draw_segment(3);
	    draw_segment(2);
	    break;
	case '-':
	    draw_segment(4);
	    break;
	case ',':
	    draw_segment(12);
	    draw_segment(13);
	    break;
	case '[':
	    draw_segment(1);
	    draw_segment(3);
	    draw_segment(6);
	    draw_segment(7);
	    break;
	case ']':
	    draw_segment(1);
	    draw_segment(2);
	    draw_segment(5);
	    draw_segment(7);
	    break;
    }
    popmatrix();

    if (n == ',')
	translate(-12.0, 0.0, 0.0);

}

draw_string(strng)
/*---------------------------------------------------------------------------
 * Draw a string of characters.
 *---------------------------------------------------------------------------
 */
char *strng;
{
    register int i;

    i = 0;

    pushmatrix();
	scale(scalesize/8.0, scalesize/8.0, scalesize/8.0);
	while (strng[i] != '\0'){
	    draw_char(strng[i++]);
	    translate(8.0, 0.0, 0.0);
	}
    popmatrix();
}

