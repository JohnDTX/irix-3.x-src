program test;

	const

{$I /usr/include/gl2/pglconsts.h}
{$I /usr/include/gl2/pdevice.h}

	type

{$I /usr/include/gl2/pgltypes.h}

	var
		rate: Short;
		cidx: Colorindex;
		str1: string[128];
		r,g,b: RGBvalue;
#include <gl2/pglmacs.h>
{$I /usr/include/gl2/pglprocs.h}


begin
    r := 100; g := 100; b := 100;
    str1 := 'window name';
    rate := 32;
    cidx := 2;
    getport(str1);
    blink(rate, cidx, r, g, b);
end.
