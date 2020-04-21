program test;
(* test coersion of Pascal *)
	const
{$I pglconsts.h}
{$I pdevice.h}

	type
{$I pgltypes.h}

	var
		rate: Short;
		cidx: Colorindex;
		str1: string[128];
		r,g,b: RGBvalue;
#include "pglmacs.h"
{$I pglprocs.h}


begin
    r := 100; g := 100; b := 100;
    str1 := 'window name';
    rate := 32;
    cidx := 2;
    getport(str1);
    blink(rate, cidx, r, g, b);
end.
