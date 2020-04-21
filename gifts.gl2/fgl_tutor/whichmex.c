#include <stdio.h>
#include <gl.h>
#include <device.h>

main ()
{
    int planes;

    ginit ();
    doublebuffer ();
    gconfig ();
    
    planes = getplanes ();

    singlebuffer ();
    gconfig ();
    gexit ();

    if (planes == 12) {       /*  24 bitplane machine	*/
	system ("/usr/bin/mex");
    }
    else {
	system ("/usr/bin/mex -d");
    }
}
