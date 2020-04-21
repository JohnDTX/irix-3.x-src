(**************************************************************************
 *                                                                        *
 *          Copyright (C) 1984, Silicon Graphics, Inc.                    *
 *                                                                        *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *                                                                        *
 **************************************************************************)

(* pgltypes.h - type declarations section for Pascal graphics *)

    Byte =  0..255; 
    Short =  -32768..32767; 
    UnsignedShort = 0..65535;
    Angle = Short;
    Screencoord = -2048..2047; 
    Icoord = longint;
    Coord = real;
    Strng = Packed array[1..128] of char;
    string128 = string[128];
    pstrng = ^Strng;

    Matrix = array[0..3,0..3] of real;

    Device = UnsignedShort;

    Colorindex = UnsignedShort;
    RGBvalue =  UnsignedShort;

    Linestyle = UnsignedShort;
    Texture = array[0..15] of Byte;
    Cursor = array[0..15] of UnsignedShort;
    Fontchar = record
                offst: UnsignedShort;
                w,h: Byte;
                xoff,yoff: -128..127;
                width: Short;
            end;

    Object = longint;
    Tag = longint;
    Offset = longint;


    Coord4array = array[0..MAXARRAY,0..3] of Coord;
    Coord3array = array[0..MAXARRAY,0..2] of Coord;
    Coord2array = array[0..MAXARRAY,0..1] of Coord;
    Icoord3array = array[0..MAXARRAY,0..2] of Icoord;
    Icoord2array = array[0..MAXARRAY,0..1] of Icoord;
    Screenarray = array[0..MAXARRAY,0..2] of Screencoord;

    Boolarray = array[0..MAXARRAY] of Boolean;
    Colorarray = array[0..MAXARRAY] of Colorindex;
    RGBarray = array[0..MAXARRAY] of RGBvalue;

    Objarray = array[0..127] of Object;
    Fntchrarray = array[0..127] of Fontchar;
    Fontraster = array[0..MAXRASTER] of Byte;
    lbuffer = array[0..MAXOBJECTS] of Object;



