program rotate;
(* test Pascal compatibility *)
(* This program draws a number of polygons, and rotates them at
    the same time.  Then it waits for a mouse click on the left or
    middle mouse buttons *)



	const
{$I /usr/include/gl2/pglconsts.h}
{$I /usr/include/gl2/pdevice.h}

	type
{$I pgltypes.h}

	var
		str1: string128;

(* New regime: don't use Colorarray as a type below *)
		arr:	array[0..3,0..2] of Icoord;
		t1, t2: Coord;
		loop: Short;
		nverts:	longint;

		dev, val, dev1, val1: Short;

#include <gl2/pglmacs.h>
{$I /usr/include/gl2/pglprocs.h}

begin

    arr[0,0]:=100; arr[0,1]:=100; arr[0,2]:=0;
    arr[1,0]:=175; arr[1,1]:=175; arr[1,2]:=100;
    arr[2,0]:=100; arr[2,1]:=250; arr[2,2]:=0;
    arr[3,0]:=550; arr[3,1]:=175; arr[3,2]:=100;

    str1 := 'Some Graphics';
    foreground;
    getport(str1);
    qdevice(LEFTMOUSE);
    qdevice(MIDDLEMOUSE);
    color(3);
    nverts := 4;

    t1:=6; t2:=7;
    ortho(-400, 1000, -400, 1000, -400, 1000);
    for loop := 0 to 100 do
    begin
	polyi(nverts, arr);
	translate (t1, t2, 0);
 	rotate (15, 'y');
    end;
    dev := qread(val);
    gexit
end.
