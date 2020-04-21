	/* This is a template for your graphics programs.
	   Note that you start comments with a backslash-
	   asterisk and end them with an asterisk-backslash.
 	*/

	/* These first lines attach "include files" to your program.
	   "gl.h" is a file that defines the IRIS graphics
	   routines, along with various useful constants. "device.h"
	   defines a set of common input/output devices.
	*/
#include "gl.h"
#include "device.h"

	/*********************************************************
	"main" is the name of the main function in all C programs.
	This function reflects the basic graphics algorithm
	we discussed earlier.  It calls three other functions,
	which are defined below.  "main" loops infinitely; you
	exit the program by killing the graphics window with
	the right mouse button.
	*/
main()
{
	initialize();
	while(TRUE) {
		drawimage();
		checkinput();
		}
}

	/*********************************************************
	"initialize" sets various graphics modes (which will be
	discussed later).  Right now, all it does is create a
	graphics window.  Just before the "winopen" command,
	you can set the characteristics of the window.  In the
	beginning, all of your programs will use the entire screen.
	*/
initialize()
{
	prefposition(0, 1023, 0, 767);
	winopen("name_of_program");
}

	/*********************************************************
 	"drawimage" is the function you will learn about 
	first.  It contains the graphics routines
	that actually put things on the screen.
	The general form of this function is simple:
		select a color and draw something, select
		another color and draw something else,
		and so on.
	Right now, all this function does is clear the
	screen to black.
	*/
drawimage()
{
	color(BLACK);
	clear();
}

	/*****************************************************
	"checkinput" checks the event queue to see if anything
	has happened (e.g., has a mouse button been pushed or
	has the keyboard been touched?).  REDRAW is a special
	event that is put in the event queue by the window
	manager when the graphics window should be redrawn
	(e.g., another window has been moved so that this
	one is now visible).  Don't worry about how this
	function works for now.
	*/
checkinput()
{
	Device val;

	switch(qread(&val)) {
		case REDRAW:
			reshapeviewport();
			break;
		}
}
