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
    Angle = Short;
    Screencoord = -2048..2047; 
    Icoord = longint;
    Scoord = Short;
    Coord = real;
    Strng = Packed array[1..128] of char;
    string128 = string[128];
    pstring128 = ^string128;
    pstring = ^Strng;

    Matrix = array[0..3,0..3] of real;

    Device = longint;

    Colorindex = 0..4095;

    RGBvalue =  Byte;

    Linestyle = longint;
    Texture = array[0..15] of Byte;
    Cursor = array[0..15] of Short;
    Fontchar = record
                offst: Short;
                w,h:  -128..127;
                xoff,yoff: -128..127;
                width: Short;
            end;

    Object = longint;
    Tag = longint;
    Offset = longint;


