/*
 * $Source: /d2/3.7/src/sys/ipII/RCS/evec.h,v $
 * $Revision: 1.1 $
 * $Date: 89/03/27 17:30:43 $
 */
|
| exception vectors
|   The proms the live at the 1kb chunk of code starting at
|   PROM_BASE.
|
evecstart:
	.long	SRAM_BASE+SRAM_SZ	| reset: initial intr stack pointer
	.long	start			| reset: initial pc

|
| processor defined execptions
|
	.long	Xbuserr		| vector  2: bus error
	.long	Xaddrerr	| vector  3: address error 
	.long	Xtraps		| vector  4: illegal instruction
	.long	Xtraps		| vector  5: zero divide
	.long	Xtraps		| vector  6: CHK, CHK2 instruction
	.long	Xtraps		| vector  7: cpTRAPcc, TRAPcc, TRAPV instruction
	.long	Xtraps		| vector  8: privilege violation
	.long	Xtraps		| vector  9: trace
	.long	Xtraps		| vector 10: line 1010 emulator
	.long	Xtraps		| vector 11: line 1111 emulator
	.long	Xtraps		| vector 12: unassigned, reserved
	.long	Xtraps		| vector 13: coprocessor protocol violation
	.long	Xtraps		| vector 14: format error
	.long	Xtraps		| vector 15: uninitialized interrupt
	.long	Xtraps		| vector 16: unassigned, reserved
	.long	Xtraps		| vector 17: 	"
	.long	Xtraps		| vector 18: 	"
	.long	Xtraps		| vector 19: 	"
	.long	Xtraps		| vector 20: 	"
	.long	Xtraps		| vector 21: 	"
	.long	Xtraps		| vector 22: 	"
	.long	Xtraps		| vector 23: 	"
	.long	Xtraps		| vector 24: spurious interrupt 

|
| interrupt auto vectors
|   not used on the IP2
|
	.long	Xtraps		| vector 25: level 1 interrupt auto vector 
	.long	Xtraps		| vector 26: level 2 interrupt auto vector
	.long	Xtraps		| vector 27: level 3 interrupt auto vector 
	.long	Xtraps		| vector 28: level 4 interrupt auto vector 
	.long	Xtraps		| vector 29: level 5 interrupt auto vector 
	.long	Xtraps		| vector 30: level 6 interrupt auto vector 
	.long	Xtraps		| vector 31: level 7 interrupt auto vector 

|
| Trap instruction vectors
|
	.long	Xsyscall	| vector 32: trap 0 - used for system calls 
	.long	Xtraps		| vector 33: trap 1
	.long	Xtraps		| vector 34: trap 2
	.long	Xtraps		| vector 35: trap 3
	.long	Xtraps		| vector 36: trap 4
	.long	Xtraps		| vector 37: trap 5
	.long	Xtraps		| vector 38: trap 6
	.long	Xtraps		| vector 39: trap 7
	.long	Xtraps		| vector 40: trap 8
	.long	Xtraps		| vector 41: trap 9
	.long	Xtraps		| vector 42: trap 10
	.long	Xtraps		| vector 43: trap 11
	.long	Xtraps		| vector 44: trap 12
	.long	Xtraps		| vector 45: trap 13
	.long	Xtraps		| vector 46: trap 14
	.long	Xtraps		| vector 47: trap 15

	.long	Xtraps		| vector 48: unassigned, reserved
	.long	Xtraps		| vector 49: unassigned, reserved
	.long	Xtraps		| vector 50: unassigned, reserved
	.long	Xtraps		| vector 51: unassigned, reserved
	.long	Xtraps		| vector 52: unassigned, reserved
	.long	Xtraps		| vector 53: unassigned, reserved
	.long	Xtraps		| vector 54: unassigned, reserved
	.long	Xtraps		| vector 55: unassigned, reserved
	.long	Xtraps		| vector 56: unassigned, reserved
	.long	Xtraps		| vector 57: unassigned, reserved
	.long	Xtraps		| vector 58: unassigned, reserved
	.long	Xtraps		| vector 59: unassigned, reserved
	.long	Xtraps		| vector 60: unassigned, reserved
	.long	Xtraps		| vector 61: unassigned, reserved
	.long	Xtraps		| vector 62: unassigned, reserved
	.long	Xtraps		| vector 63: unassigned, reserved

|
| User defined vectors
|
	.long	Xtraps		| vector 64: not used
	.long	Xmbintr01	| vector 65: multibus 0 and 1 interrupt
	.long	Xmbintr2	| vector 66: multibus 2 interrupt
	.long	Xmbintr3	| vector 67: multibus 3 interrupt
	.long	Xmbintr4	| vector 68: multibus 4 interrupt
	.long	Xmbintr5	| vector 69: multibus 5 interrupt
	.long	Xmbintr6	| vector 70: multibus 6 interrupt
	.long	Xmbintr7	| vector 71: multibus 7 interrupt
	.long	Xtraps		| vector 72: not used
	.long	Xtraps		| vector 73: not used
	.long	Xtraps		| vector 74: not used
	.long	Xtraps		| vector 75: not used
	.long	Xtraps		| vector 76: not used
	.long	Xtraps		| vector 77: not used
	.long	Xtraps		| vector 78: not used
	.long	Xtraps		| vector 79: not used
	.long	Xduart0		| vector 80: duart0 (tty0 and 1)
	.long	Xduart1		| vector 81: duart1 (tty2 and 3)
	.long	Xtraps		| vector 82: external interrupt (from multibus)
	.long	Xclock		| vector 83: clock interrupt
	.long	Xtraps		| vector 84: not used
	.long	Xparity		| vector 85: parity error (NMI)
	.long	_mouseintr	| vector 86: mouse quadrature
	.long	_mouseintr	| vector 87:  not used, but it can be generated
				| by plugging in and out the mouse plug.
	.long	Xtraps		| vector 88:  not used
	.long	Xtraps		| vector 89:  not used
	.long	Xtraps		| vector 90:  not used
	.long	Xtraps		| vector 91:  not used
	.long	Xtraps		| vector 92:  not used
	.long	Xtraps		| vector 93:  not used
	.long	Xtraps		| vector 94:  not used
	.long	Xtraps		| vector 95:  not used
	.long	Xtraps		| vector 96:  not used
	.long	Xtraps		| vector 97:  not used
	.long	Xtraps		| vector 98:  not used
	.long	Xtraps		| vector 99:  not used
	.long	Xtraps		| vector 100: not used
	.long	Xtraps		| vector 101: not used
	.long	Xtraps		| vector 102: not used
	.long	Xtraps		| vector 103: not used
	.long	Xtraps		| vector 104: not used
	.long	Xtraps		| vector 105: not used
	.long	Xtraps		| vector 106: not used
	.long	Xtraps		| vector 107: not used
	.long	Xtraps		| vector 108: not used
	.long	Xtraps		| vector 109: not used
	.long	Xtraps		| vector 110: not used
	.long	Xtraps		| vector 111: not used
	.long	Xtraps		| vector 112: not used
	.long	Xtraps		| vector 113: not used
	.long	Xtraps		| vector 114: not used
	.long	Xtraps		| vector 115: not used
	.long	Xtraps		| vector 116: not used
	.long	Xtraps		| vector 117: not used
	.long	Xtraps		| vector 118: not used
	.long	Xtraps		| vector 119: not used
	.long	Xtraps		| vector 120: not used
	.long	Xtraps		| vector 121: not used
	.long	Xtraps		| vector 122: not used
	.long	Xtraps		| vector 123: not used
	.long	Xtraps		| vector 124: not used
	.long	Xtraps		| vector 125: not used
	.long	Xtraps		| vector 126: not used
	.long	Xtraps		| vector 127: not used
	.long	Xtraps		| vector 128: not used
	.long	Xtraps		| vector 129: not used
	.long	Xtraps		| vector 130: not used
	.long	Xtraps		| vector 131: not used
	.long	Xtraps		| vector 132: not used
	.long	Xtraps		| vector 133: not used
	.long	Xtraps		| vector 134: not used
	.long	Xtraps		| vector 135: not used
	.long	Xtraps		| vector 136: not used
	.long	Xtraps		| vector 137: not used
	.long	Xtraps		| vector 138: not used
	.long	Xtraps		| vector 139: not used
	.long	Xtraps		| vector 140: not used
	.long	Xtraps		| vector 141: not used
	.long	Xtraps		| vector 142: not used
	.long	Xtraps		| vector 143: not used
	.long	Xtraps		| vector 144: not used
	.long	Xtraps		| vector 145: not used
	.long	Xtraps		| vector 146: not used
	.long	Xtraps		| vector 147: not used
	.long	Xtraps		| vector 148: not used
	.long	Xtraps		| vector 149: not used
	.long	Xtraps		| vector 150: not used
	.long	Xtraps		| vector 151: not used
	.long	Xtraps		| vector 152: not used
	.long	Xtraps		| vector 153: not used
	.long	Xtraps		| vector 154: not used
	.long	Xtraps		| vector 155: not used
	.long	Xtraps		| vector 156: not used
	.long	Xtraps		| vector 157: not used
	.long	Xtraps		| vector 158: not used
	.long	Xtraps		| vector 159: not used
	.long	Xtraps		| vector 160: not used
	.long	Xtraps		| vector 161: not used
	.long	Xtraps		| vector 162: not used
	.long	Xtraps		| vector 163: not used
	.long	Xtraps		| vector 164: not used
	.long	Xtraps		| vector 165: not used
	.long	Xtraps		| vector 166: not used
	.long	Xtraps		| vector 167: not used
	.long	Xtraps		| vector 168: not used
	.long	Xtraps		| vector 169: not used
	.long	Xtraps		| vector 170: not used
	.long	Xtraps		| vector 171: not used
	.long	Xtraps		| vector 172: not used
	.long	Xtraps		| vector 173: not used
	.long	Xtraps		| vector 174: not used
	.long	Xtraps		| vector 175: not used
	.long	Xtraps		| vector 176: not used
	.long	Xtraps		| vector 177: not used
	.long	Xtraps		| vector 178: not used
	.long	Xtraps		| vector 179: not used
	.long	Xtraps		| vector 180: not used
	.long	Xtraps		| vector 181: not used
	.long	Xtraps		| vector 182: not used
	.long	Xtraps		| vector 183: not used
	.long	Xtraps		| vector 184: not used
	.long	Xtraps		| vector 185: not used
	.long	Xtraps		| vector 186: not used
	.long	Xtraps		| vector 187: not used
	.long	Xtraps		| vector 188: not used
	.long	Xtraps		| vector 189: not used
	.long	Xtraps		| vector 190: not used
	.long	Xtraps		| vector 191: not used
	.long	Xtraps		| vector 192: not used
	.long	Xtraps		| vector 193: not used
	.long	Xtraps		| vector 194: not used
	.long	Xtraps		| vector 195: not used
	.long	Xtraps		| vector 196: not used
	.long	Xtraps		| vector 197: not used
	.long	Xtraps		| vector 198: not used
	.long	Xtraps		| vector 199: not used
	.long	Xtraps		| vector 200: not used
	.long	Xtraps		| vector 201: not used
	.long	Xtraps		| vector 202: not used
	.long	Xtraps		| vector 203: not used
	.long	Xtraps		| vector 204: not used
	.long	Xtraps		| vector 205: not used
	.long	Xtraps		| vector 206: not used
	.long	Xtraps		| vector 207: not used
	.long	Xtraps		| vector 208: not used
	.long	Xtraps		| vector 209: not used
	.long	Xtraps		| vector 210: not used
	.long	Xtraps		| vector 211: not used
	.long	Xtraps		| vector 212: not used
	.long	Xtraps		| vector 213: not used
	.long	Xtraps		| vector 214: not used
	.long	Xtraps		| vector 215: not used
	.long	Xtraps		| vector 216: not used
	.long	Xtraps		| vector 217: not used
	.long	Xtraps		| vector 218: not used
	.long	Xtraps		| vector 219: not used
	.long	Xtraps		| vector 220: not used
	.long	Xtraps		| vector 221: not used
	.long	Xtraps		| vector 222: not used
	.long	Xtraps		| vector 223: not used
	.long	Xtraps		| vector 224: not used
	.long	Xtraps		| vector 225: not used
	.long	Xtraps		| vector 226: not used
	.long	Xtraps		| vector 227: not used
	.long	Xtraps		| vector 228: not used
	.long	Xtraps		| vector 229: not used
	.long	Xtraps		| vector 230: not used
	.long	Xtraps		| vector 231: not used
	.long	Xtraps		| vector 232: not used
	.long	Xtraps		| vector 233: not used
	.long	Xtraps		| vector 234: not used
	.long	Xtraps		| vector 235: not used
	.long	Xtraps		| vector 236: not used
	.long	Xtraps		| vector 237: not used
	.long	Xtraps		| vector 238: not used
	.long	Xtraps		| vector 239: not used
	.long	Xtraps		| vector 240: not used
	.long	Xtraps		| vector 241: not used
	.long	Xtraps		| vector 242: not used
	.long	Xtraps		| vector 243: not used
	.long	Xtraps		| vector 244: not used
	.long	Xtraps		| vector 245: not used
	.long	Xtraps		| vector 246: not used
	.long	Xtraps		| vector 247: not used
	.long	Xtraps		| vector 248: not used
	.long	Xtraps		| vector 249: not used
	.long	Xtraps		| vector 250: not used
	.long	Xtraps		| vector 251: not used
	.long	Xtraps		| vector 252: not used
	.long	Xtraps		| vector 253: not used
	.long	Xtraps		| vector 254: not used
	.long	Xtraps		| vector 255: not used
