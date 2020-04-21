/*	@(#)tab2631-c.c	1.1	*/
#define INCH 240
/*
HP 2631 Line Printer with col-only reverse line feed - in compressed mode
nroff driving tables
width and code tables
*/

struct termtable t2631c = {
/*bset*/	0,
/*breset*/	0,
/*Hor*/		INCH/17,
/*Vert*/	INCH/6,
/*Newline*/	INCH/6,
/*Char*/	INCH/17,
/*Em*/		INCH/17,
/*Halfline*/	INCH/12,
/*Adj*/		INCH/17,
/*twinit*/	"\033&k2S",
/*twrest*/	"\033E",
/*twnl*/	"\n",
/*hlr*/		"",
/*hlf*/		"",
/*flr*/		"\0337",
/*bdon*/	"",
/*bdoff*/	"",
/*iton*/	"",
/*itoff*/	"",
/*ploton*/	"",
/*plotoff*/	"",
/*up*/		"",
/*down*/	"",
/*right*/	"",
/*left*/	"",
/*codetab*/
#include "code.lp"
