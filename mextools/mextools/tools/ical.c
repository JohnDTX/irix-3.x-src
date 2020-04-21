/*
 *	ical - 
 *		A very simple desk calendar.  Use the left and middle mouse
 *		mouse buttons to go forward and backward in time.
 *	
 *				Paul Haeberli - 1985
 */
#include "gl.h"
#include "port.h"
#include "device.h"
#include <time.h>

char *monthnames[]= {
    "January", "February", "March", "April",
    "May", "June", "July", "August",
    "September", "October", "November", "December",
};

char *daynames[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

char monlength[] = {
    0,
    31, 29, 31, 30,
    31, 30, 31, 31,
    30, 31, 30, 31,
};

char tempstr[100];
struct tm *tm;
long thetime;

struct	caltime {
    int	day;
    int	month;
    int	year;
};

main(argc, argv)
char *argv[];
{
    short val;
    struct caltime today, show;

    thetime = time(0);
    tm = localtime(&thetime);
    today.day = tm->tm_mday;
    today.month = tm->tm_mon + 1;
    today.year = tm->tm_year + 1900;

    if (argc >= 2) {
	show.month = atoi(argv[1]);
	show.year = atoi(argv[2]);
	show.day = 1;
    } else 
	show = today;
    prefsize(260,185);
    winopen("cal");
    qdevice(LEFTMOUSE);
    qdevice(MIDDLEMOUSE);
    drawcal(&show, &today);
    while (1) {
	switch (qread(&val)) {
	    case LEFTMOUSE:
		if (val) {
		    show.month--;
		    if (show.month == 0) {
			show.year--;
			show.month = 12;
		    }
		    drawcal(&show, &today);
		}
		break;
	    case MIDDLEMOUSE:
		if (val) {
		    show.month++;
		    if (show.month == 13) {
			show.year++;
			show.month = 1;
		    }
		    drawcal(&show, &today);
		}
		break;
	    case REDRAW:
		drawcal(&show, &today);
		break;
	}
    }
}


drawcal(show, today)
struct caltime *show, *today;
{
    register i;
    int c;
    int ycoord;
    int samemonth;

    samemonth = 0;
    if ((today->year == show->year) && (today->month == show->month))
	    samemonth = 1;
    color(7);
    clear();
    color(0);

    color(GREY(8));
    move2i(20,163);
    draw2i(240,163);

    color(BLUE);
    cmov2i(20,165);
    charstr(monthnames[show->month-1]);
    color(RED);
    sprintf(tempstr,"%u",show->year);
    cmov2i(208,165);
    charstr(tempstr);

    color(GREY(8));
    move2i(20,143);
    draw2i(240,143);

    color(GREY(0));

    for (i=0; i<7; i++){
	cmov2i(24+32*i,145);
	charstr(daynames[i]);
    }

    c = dayofweek(show->month, show->year);
    ycoord = 125;
    for (i=1; i<=monlength[show->month]; i++) {
	cmov2i(24+32*c,ycoord);
	sprintf(tempstr,"%2d",i); 
	if ((i == show->day) && samemonth)
	    color(RED);
	charstr(tempstr);
	if ((i == show->day) && samemonth)
	    color(GREY(0));
	if (++c == 7) {
	    c = 0;
	    ycoord -= 20;
	}
    }
}

dayofweek(m, y)
{
    register d, i;

    d = jan1(y);
    if (((jan1(y+1)+7-d)%7) == 1)
	monlength[2] = 28;
    else
	monlength[2] = 29;
    for (i=1; i<m; i++)
	d += monlength[i];
    return d%7;
}

/*
 *	return day of the week
 *	of jan 1 of given year
 */
jan1(y)
int y;
{
    register int d;

    d = y+4+(y+3)/4;
    if (y > 1752) {
	d += 3;
        if (y > 1800) {
	    d -= (y-1701)/100;
	    d += (y-1601)/400;
        }
    }
    return d%7;
}
