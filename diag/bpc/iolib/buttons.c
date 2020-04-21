/**************************************************************************
 *									  *
 * 		 Copyright (C) 1983, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/

/* File: buttons.c:  This file contains the initialization of the buttons[]
 * array for the SGI keyboard.  The format of the buttondata data structure
 * can be found in device.h.  The normal ascii part of the keyboard behaves
 * in the usual way.  Keys which send no ascii value normally are indicated
 * with a 0xff entry.  The entries for the extra keys (pad keys and arrow
 * keys) have the high-order bit turned on.  In the case of the pad keys,
 * the code is just the standard ascii code with that extra bit.  The arrow
 * keys have a more arbitrary code.
 *
 * The rationale for adding a high order bit to the ascii values of the pad
 * standard ascii characters is that the user always has access to the standard
 * ascii values using the keys along the top of the keyboard.  If one desires to
 * use the keypad as a standard ascii device, the program merely needs to strip
 * off a bit.  Since the bit is present, however, the user effectively has 14
 * extra keys without descending into the depths of the up-down encoding.  With
 * the up-down encoding, anything can still be done.
 */

#include "devdata.h"


buttondata kbuttons[] = {
	{0, 0, 0, 0},
	{0x80, 0x80, 0x80, 0x80},	/* BUTTON0 - BREAKKEY */
	{0xff, 0xff, 0xff, 0xff},	/* BUTTON1 - SETUPKEY */
	{0xff, 0xff, 0xff, 0xff},	/* BUTTON2 - CTRLKEY */
	{0xff, 0xff, 0xff, 0xff},	/* BUTTON3 - CAPSLOCKKEY */
	{0xff, 0xff, 0xff, 0xff},	/* BUTTON4 - RIGHTSHIFTKEY */
	{0xff, 0xff, 0xff, 0xff},	/* BUTTON5 - LEFTSHIFTKEY */
	{0x1b, 0x1b, 0x1b, 0x1b},	/* BUTTON6 - ESCKEY */
	{0x31, 0x21, 0x31, 0x21},	/* BUTTON7 - ONEKEY */
	{0x09, 0x09, 0x09, 0x09},	/* BUTTON8 - TABKEY */
	{0x71, 0x51, 0x11, 0x11},	/* BUTTON9 - QKEY */
	{0x61, 0x41, 0x01, 0x01},	/* BUTTON10 - AKEY */
	{0x73, 0x53, 0x13, 0x13},	/* BUTTON11 - SKEY */
	{0xff, 0xff, 0xff, 0xff},	/* BUTTON12 - NOSCRLKEY */
	{0x32, 0x40, 0x32, 0x00},	/* BUTTON13 - TWOKEY */
	{0x33, 0x23, 0x33, 0x23},	/* BUTTON14 - THREEKEY */
	{0x77, 0x57, 0x17, 0x17},	/* BUTTON15 - WKEY */
	{0x65, 0x45, 0x05, 0x05},	/* BUTTON16 - EKEY */
	{0x64, 0x44, 0x04, 0x04},	/* BUTTON17 - DKEY */
	{0x66, 0x46, 0x06, 0x06},	/* BUTTON18 - FKEY */
	{0x7a, 0x5a, 0x1a, 0x1a},	/* BUTTON19 - ZKEY */
	{0x78, 0x58, 0x18, 0x18},	/* BUTTON20 - XKEY */
	{0x34, 0x24, 0x34, 0x24},	/* BUTTON21 - FOURKEY */
	{0x35, 0x25, 0x35, 0x25},	/* BUTTON22 - FIVEKEY */
	{0x72, 0x52, 0x12, 0x12},	/* BUTTON23 - RKEY */
	{0x74, 0x54, 0x14, 0x14},	/* BUTTON24 - TKEY */
	{0x67, 0x47, 0x07, 0x07},	/* BUTTON25 - GKEY */
	{0x68, 0x48, 0x08, 0x08},	/* BUTTON26 - HKEY */
	{0x63, 0x43, 0x03, 0x03},	/* BUTTON27 - CKEY */
	{0x76, 0x56, 0x16, 0x16},	/* BUTTON28 - VKEY */
	{0x36, 0x5e, 0x36, 0x1e},	/* BUTTON29 - SIXKEY */
	{0x37, 0x26, 0x37, 0x26},	/* BUTTON30 - SEVENKEY */
	{0x79, 0x59, 0x19, 0x19},	/* BUTTON31 - YKEY */
	{0x75, 0x55, 0x15, 0x15},	/* BUTTON32 - UKEY */
	{0x6a, 0x4a, 0x0a, 0x0a},	/* BUTTON33 - JKEY */
	{0x6b, 0x4b, 0x0b, 0x0b},	/* BUTTON34 - KKEY */
	{0x62, 0x42, 0x02, 0x02},	/* BUTTON35 - BKEY */
	{0x6e, 0x4e, 0x0e, 0x0e},	/* BUTTON36 - NKEY */
	{0x38, 0x2a, 0x38, 0x2a},	/* BUTTON37 - EIGHTKEY */
	{0x39, 0x28, 0x39, 0x28},	/* BUTTON38 - NINEKEY */
	{0x69, 0x49, 0x09, 0x09},	/* BUTTON39 - IKEY */
	{0x6f, 0x4f, 0x0f, 0x0f},	/* BUTTON40 - OKEY */
	{0x6c, 0x4c, 0x0c, 0x0c},	/* BUTTON41 - LKEY */
	{0x3b, 0x3a, 0x3b, 0x3a},	/* BUTTON42 - SEMICOLONKEY */
	{0x6d, 0x4d, 0x0d, 0x0d},	/* BUTTON43 - MKEY */
	{0x2c, 0x3c, 0x2c, 0x3c},	/* BUTTON44 - COMMAKEY */
	{0x30, 0x29, 0x30, 0x29},	/* BUTTON45 - ZEROKEY */
	{0x2d, 0x5f, 0x1f, 0x5f},	/* BUTTON46 - MINUSKEY */
	{0x70, 0x50, 0x10, 0x10},	/* BUTTON47 - PKEY */
	{0x5b, 0x7b, 0x1b, 0x1b},	/* BUTTON48 - LEFTBRACKET */
	{0x27, 0x22, 0x27, 0x22},	/* BUTTON49 - QUOTEKEY */
	{0x0d, 0x0d, 0x0d, 0x0d},	/* BUTTON50 - RETURNKEY */
	{0x2e, 0x3e, 0x2e, 0x3e},	/* BUTTON51 - PERIODKEY */
	{0x2f, 0x3f, 0x2f, 0x3f},	/* BUTTON52 - VIRGULEKEY */
	{0x3d, 0x2b, 0x3d, 0x2b},	/* BUTTON53 - EQUALKEY */
	{0x60, 0x7e, 0x60, 0x7e},	/* BUTTON54 - ACCENTGRAVEKEY */
	{0x5d, 0x7d, 0x1d, 0x7d},	/* BUTTON55 - RIGHTBRACKETKEY */
	{0x5c, 0x7c, 0x1c, 0x7c},	/* BUTTON56 - BACKSLASHKEY */
	{0xb1, 0xb1, 0xb1, 0xb1},	/* BUTTON57 - PADONEKEY */
	{0xb0, 0xb0, 0xb0, 0xb0},	/* BUTTON58 - PADZEROKEY */
	{0x0a, 0x0a, 0x0a, 0x0a},	/* BUTTON59 - LINEFEEDKEY */
	{0x08, 0x08, 0x08, 0x08},	/* BUTTON60 - BACKSPACEKEY */
	{0x7f, 0x7f, 0x7f, 0x7f},	/* BUTTON61 - DELETEKEY */
	{0xb4, 0xb4, 0xb4, 0xb4},	/* BUTTON62 - PADFOURKEY */
	{0xb2, 0xb2, 0xb2, 0xb2},	/* BUTTON63 - PADTWOKEY */
	{0xb3, 0xb3, 0xb3, 0xb3},	/* BUTTON64 - PADTHREEKEY */
	{0xae, 0xae, 0xae, 0xae},	/* BUTTON65 - PADPERIODKEY */
	{0xb7, 0xb7, 0xb7, 0xb7},	/* BUTTON66 - PADSEVENKEY */
	{0xb8, 0xb8, 0xb8, 0xb8},	/* BUTTON67 - PADEIGHTKEY */
	{0xb5, 0xb5, 0xb5, 0xb5},	/* BUTTON68 - PADFIVEKEY */
	{0xb6, 0xb6, 0xb6, 0xb6},	/* BUTTON69 - PADSIXKEY */
	{0x82, 0x82, 0x82, 0x82},	/* BUTTON70 - PADPF2KEY */
	{0x81, 0x81, 0x81, 0x81},	/* BUTTON71 - PADPF1KEY */
	{0x88, 0x88, 0x88, 0x88},	/* BUTTON72 - LEFTARROWKEY */
	{0x86, 0x86, 0x86, 0x86},	/* BUTTON73 - DOWNARROWKEY */
	{0xb9, 0xb9, 0xb9, 0xb9},	/* BUTTON74 - PADNINEKEY */
	{0xad, 0xad, 0xad, 0xad},	/* BUTTON75 - PADMINUSKEY */
	{0xac, 0xac, 0xac, 0xac},	/* BUTTON76 - PADCOMMAKEY */
	{0x84, 0x84, 0x84, 0x84},	/* BUTTON77 - PADPF4KEY */
	{0x83, 0x83, 0x83, 0x83},	/* BUTTON78 - PADPF3KEY */
	{0x87, 0x87, 0x87, 0x87},	/* BUTTON79 - RIGHTARROWKEY */
	{0x85, 0x85, 0x85, 0x85},	/* BUTTON80 - UPARROWKEY */
	{0x8d, 0x8d, 0x8d, 0x8d},	/* BUTTON81 - PADENTERKEY */
	{0x20, 0x20, 0x20, 0x20},	/* BUTTON82 - SPACKKEY */

	{0, 0, 0, 0},	/* BUTTON83 - UNDEFINED */
	{0, 0, 0, 0},	/* BUTTON84 - UNDEFINED */
	{0, 0, 0, 0},	/* BUTTON85 - UNDEFINED */
	{0, 0, 0, 0},	/* BUTTON86 - UNDEFINED */
	{0, 0, 0, 0},	/* BUTTON87 - UNDEFINED */
	{0, 0, 0, 0},	/* BUTTON88 - UNDEFINED */
	{0, 0, 0, 0},	/* BUTTON89 - UNDEFINED */
	{0, 0, 0, 0},	/* BUTTON90 - UNDEFINED */
	{0, 0, 0, 0},	/* BUTTON91 - UNDEFINED */
	{0, 0, 0, 0},	/* BUTTON92 - UNDEFINED */
	{0, 0, 0, 0},	/* BUTTON93 - UNDEFINED */
	{0, 0, 0, 0},	/* BUTTON94 - UNDEFINED */
	{0, 0, 0, 0},	/* BUTTON95 - UNDEFINED */
	{0, 0, 0, 0},	/* BUTTON96 - UNDEFINED */
	{0, 0, 0, 0},	/* BUTTON97 - UNDEFINED */
	{0, 0, 0, 0},	/* BUTTON98 - UNDEFINED */
	{0, 0, 0, 0},	/* BUTTON99 - UNDEFINED */

	{0xff, 0xff, 0xff, 0xff},	/* Mouse button 1 */
	{0xff, 0xff, 0xff, 0xff},	/* Mouse button 2 */
	{0xff, 0xff, 0xff, 0xff},	/* Mouse button 3 */

	{0, 0, 0, 0},	/* BUTTON103 - UNDEFINED */
	{0, 0, 0, 0},	/* BUTTON104 - UNDEFINED */
	{0, 0, 0, 0},	/* BUTTON105 - UNDEFINED */
	{0, 0, 0, 0},	/* BUTTON106 - UNDEFINED */
	{0, 0, 0, 0},	/* BUTTON107 - UNDEFINED */
	{0, 0, 0, 0},	/* BUTTON108 - UNDEFINED */
	{0, 0, 0, 0},	/* BUTTON109 - UNDEFINED */

	{0, 0, 0, 0},	/* BUTTON110 - SWITCH0  */
	{0, 0, 0, 0},	/* BUTTON111 - SWITCH1 */
	{0, 0, 0, 0},	/* BUTTON112 - SWITCH2 */
	{0, 0, 0, 0},	/* BUTTON113 - SWITCH3 */
	{0, 0, 0, 0},	/* BUTTON114 - SWITCH4 */
	{0, 0, 0, 0},	/* BUTTON115 - SWITCH5 */
	{0, 0, 0, 0},	/* BUTTON116 - SWITCH6 */
	{0, 0, 0, 0},	/* BUTTON117 - SWITCH7 */
	{0, 0, 0, 0},	/* BUTTON118 - SWITCH8 */
	{0, 0, 0, 0},	/* BUTTON119 - SWITCH9 */
	{0, 0, 0, 0},	/* BUTTON120 - SWITCH10 */
	{0, 0, 0, 0},	/* BUTTON121 - SWITCH11 */
	{0, 0, 0, 0},	/* BUTTON122 - SWITCH12 */
	{0, 0, 0, 0},	/* BUTTON123 - SWITCH13 */
	{0, 0, 0, 0},	/* BUTTON124 - SWITCH14 */
	{0, 0, 0, 0},	/* BUTTON125 - SWITCH15 */
	{0, 0, 0, 0},	/* BUTTON126 - SWITCH16 */
	{0, 0, 0, 0},	/* BUTTON127 - SWITCH17 */
	{0, 0, 0, 0},	/* BUTTON128 - SWITCH18 */
	{0, 0, 0, 0},	/* BUTTON129 - SWITCH19 */
	{0, 0, 0, 0},	/* BUTTON130 - SWITCH20 */
	{0, 0, 0, 0},	/* BUTTON131 - SWITCH21 */
	{0, 0, 0, 0},	/* BUTTON132 - SWITCH22 */
	{0, 0, 0, 0},	/* BUTTON133 - SWITCH23 */
	{0, 0, 0, 0},	/* BUTTON134 - SWITCH24 */
	{0, 0, 0, 0},	/* BUTTON135 - SWITCH25 */
	{0, 0, 0, 0},	/* BUTTON136 - SWITCH26 */
	{0, 0, 0, 0},	/* BUTTON137 - SWITCH27 */
	{0, 0, 0, 0},	/* BUTTON138 - SWITCH28 */
	{0, 0, 0, 0},	/* BUTTON139 - SWITCH29 */
	{0, 0, 0, 0},	/* BUTTON140 - SWITCH30 */
	{0, 0, 0, 0},	/* BUTTON141 - SWITCH31 */

	{0, 0, 0, 0}	/* BUTTON142 - UNDEFINED */

	};


