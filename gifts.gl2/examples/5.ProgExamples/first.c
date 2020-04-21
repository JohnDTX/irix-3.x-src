/* "first.c" */
#include "gl.h"

main()
{
    ginit();
    cursoff();
    color(BLACK);
    clear();
    color(RED);
    move2i(20, 20);
    draw2i(50, 20);
    draw2i(50, 50);
    draw2i(20, 50);
    draw2i(20, 20);
    gexit();
}
