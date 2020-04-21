                        	.data
       0000 0004        	.comm	outfile,0x4
                        	.even
000DA4                  testlim:
000DA4 0000 07D0        	.long	0x7d0
                        	.even
000DA8                  testmode:
000DA8 0000 0000        	.long	0x0
       0000 0004        	.comm	pgearray,0x4
                        	.even
                        	.globl	testin
000DAC                  testin:
000DAC 0000 0000        	.long	0x0
000DB0 0000 0000        	.long	testgsi
000DB4 0000 0000        	.long	testmm1
000DB8 0000 0000        	.long	testmm2
000DBC 0000 0000        	.long	testmm3
000DC0 0000 0000        	.long	testmm4
000DC4 0000 0000        	.long	testcl1
000DC8 0000 0000        	.long	testcl2
000DCC 0000 0000        	.long	testcl3
000DD0 0000 0000        	.long	testcl4
000DD4 0000 0000        	.long	testcl5
000DD8 0000 0000        	.long	testcl6
000DDC 0000 0000        	.long	testsc1
000DE0 0000 0000        	.long	testgso
                        	.even
                        	.globl	testout
000DE4                  testout:
000DE4 0000 0000        	.long	0x0
000DE8 0000 0000        	.long	testmm1
000DEC 0000 0000        	.long	testmm2
000DF0 0000 0000        	.long	testmm3
000DF4 0000 0000        	.long	testmm4
000DF8 0000 0000        	.long	testcl1
000DFC 0000 0000        	.long	testcl2
000E00 0000 0000        	.long	testcl3
000E04 0000 0000        	.long	testcl4
000E08 0000 0000        	.long	testcl5
000E0C 0000 0000        	.long	testcl6
000E10 0000 0000        	.long	testsc1
000E14 0000 0000        	.long	testgso
000E18 0000 0000        	.long	0x0
                        	.even
                        	.globl	test10in
000E1C                  test10in:
000E1C 0000 0000        	.long	0x0
000E20 0000 0000        	.long	testgsi
000E24 0000 0000        	.long	testmm1
000E28 0000 0000        	.long	testmm2
000E2C 0000 0000        	.long	testmm3
000E30 0000 0000        	.long	testcl5
000E34 0000 0000        	.long	testmm4
000E38 0000 0000        	.long	testcl1
000E3C 0000 0000        	.long	testcl2
000E40 0000 0000        	.long	testcl3
000E44 0000 0000        	.long	testcl4
000E48 0000 0000        	.long	testcl4
000E4C 0000 0000        	.long	test10sc1
                        	.even
                        	.globl	test10out
000E50                  test10out:
000E50 0000 0000        	.long	0x0
000E54 0000 0000        	.long	testmm1
000E58 0000 0000        	.long	testmm2
000E5C 0000 0000        	.long	testmm3
000E60 0000 0000        	.long	testmm4
000E64 0000 0000        	.long	testcl6
000E68 0000 0000        	.long	testcl1
000E6C 0000 0000        	.long	testcl2
000E70 0000 0000        	.long	testcl3
000E74 0000 0000        	.long	testcl4
000E78 0000 0000        	.long	testcl5
000E7C 0000 0000        	.long	test10sc1
000E80 0000 0000        	.long	test10gso
000E84 0000 0000        	.long	0x0
                        	.even
                        	.globl	masterconfig
000E88                  masterconfig:
000E88 0000             	.word	0
000E8A 0009             	.word	9
000E8C 000A             	.word	10
000E8E 000B             	.word	11
000E90 000C             	.word	12
000E92 0010             	.word	16
000E94 0011             	.word	17
000E96 0012             	.word	18
000E98 0013             	.word	19
000E9A 0014             	.word	20
000E9C 0015             	.word	21
000E9E 0020             	.word	32
000EA0 0021             	.word	33
                        	.even
                        	.globl	masterconfig10
000EA2                  masterconfig10:
000EA2 0000             	.word	0
000EA4 0009             	.word	9
000EA6 000A             	.word	10
000EA8 000B             	.word	11
000EAA 000C             	.word	12
000EAC 0015             	.word	21
000EAE 0010             	.word	16
000EB0 0011             	.word	17
000EB2 0012             	.word	18
000EB4 0013             	.word	19
000EB6 0014             	.word	20
000EB8 0020             	.word	32
000EBA 0021             	.word	33
                        	.even
                        	.globl	useconfig
000EBC                  useconfig:
000EBC 0000             	.word	0
000EBE 0B09             	.word	2825
000EC0 0A0A             	.word	2570
000EC2 090B             	.word	2315
000EC4 080C             	.word	2060
000EC6 0710             	.word	1808
000EC8 0611             	.word	1553
000ECA 0512             	.word	1298
000ECC 0413             	.word	1043
000ECE 0314             	.word	788
000ED0 0215             	.word	533
000ED2 0120             	.word	288
000ED4 0021             	.word	33
                        	.even
                        	.globl	delaycount
000ED6                  delaycount:
000ED6 03E8             	.word	1000
                        	.even
                        	.globl	printconfig
000ED8                  printconfig:
000ED8 0000             	.word	0
                        	.text
                        	.globl	ge
000000                  ge:
000000 4E56 FFF0        	link	a6,#-.F1
000004 48EE 20C0 FFF0   	moveml	#.S1,a6@(-.F1)
                        | A1 = 8
00000A 2A7C 0040 1000   	movl	#0x401000,a5
000010 3039 0000 0000   	movw	ix,d0
000016 48C0             	extl	d0
000018 0680 0000 0000   	addl	#line,d0
00001E 2040             	movl	d0,a0
000020 1010             	movb	a0@,d0
000022 4880             	extw	d0
000024 48C0             	extl	d0
000026 5279 0000 0000   	addqw	#0x1,ix
00002C 0480 0000 003F   	subl	#0x3f,d0
000032 0C80 0000 0038   	cmpl	#56,d0
000038 6200 0668        	jhi	.L174
00003C D040             	addw	d0,d0
00003E 303B 0006        	movw	pc@(6,d0:w),d0
000042 4EFB 0002        	jmp	pc@(2,d0:w)
       0000 0046        .L189 = .
000046 0076             	.word	.L89-.L189
000048 065C             	.word	.L174-.L189
00004A 065C             	.word	.L174-.L189
00004C 04F2             	.word	.L163-.L189
00004E 0444             	.word	.L153-.L189
000050 047A             	.word	.L158-.L189
000052 065C             	.word	.L174-.L189
000054 065C             	.word	.L174-.L189
000056 065C             	.word	.L174-.L189
000058 065C             	.word	.L174-.L189
00005A 065C             	.word	.L174-.L189
00005C 065C             	.word	.L174-.L189
00005E 065C             	.word	.L174-.L189
000060 04E2             	.word	.L162-.L189
000062 065C             	.word	.L174-.L189
000064 065C             	.word	.L174-.L189
000066 065C             	.word	.L174-.L189
000068 0238             	.word	.L128-.L189
00006A 065C             	.word	.L174-.L189
00006C 065C             	.word	.L174-.L189
00006E 065C             	.word	.L174-.L189
000070 0498             	.word	.L159-.L189
000072 065C             	.word	.L174-.L189
000074 065C             	.word	.L174-.L189
000076 065C             	.word	.L174-.L189
000078 065C             	.word	.L174-.L189
00007A 065C             	.word	.L174-.L189
00007C 065C             	.word	.L174-.L189
00007E 065C             	.word	.L174-.L189
000080 065C             	.word	.L174-.L189
000082 065C             	.word	.L174-.L189
000084 065C             	.word	.L174-.L189
000086 065C             	.word	.L174-.L189
000088 065C             	.word	.L174-.L189
00008A 007E             	.word	.L91-.L189
00008C 065C             	.word	.L174-.L189
00008E 065C             	.word	.L174-.L189
000090 016A             	.word	.L110-.L189
000092 017A             	.word	.L111-.L189
000094 00EA             	.word	.L102-.L189
000096 065C             	.word	.L174-.L189
000098 065C             	.word	.L174-.L189
00009A 0402             	.word	.L152-.L189
00009C 0088             	.word	.L93-.L189
00009E 065C             	.word	.L174-.L189
0000A0 065C             	.word	.L174-.L189
0000A2 03A6             	.word	.L149-.L189
0000A4 065C             	.word	.L174-.L189
0000A6 065C             	.word	.L174-.L189
0000A8 019A             	.word	.L114-.L189
0000AA 065C             	.word	.L174-.L189
0000AC 065C             	.word	.L174-.L189
0000AE 0130             	.word	.L106-.L189
0000B0 0264             	.word	.L131-.L189
0000B2 065C             	.word	.L174-.L189
0000B4 018A             	.word	.L112-.L189
0000B6 035E             	.word	.L148-.L189
0000B8 6000 0738        	jra	.L86
0000BC                  .L89:
0000BC 6100 0A74        	jbsr	gehelp
0000C0 6000 0730        	jra	.L86
0000C4                  .L91:
0000C4 4EB9 0000 0000   	jbsr	gepa
0000CA 6000 0726        	jra	.L86
0000CE                  .L93:
0000CE 4A79 0000 0000   	tstw	initdone
0000D4 6612             	jne	.L94
0000D6 4879 0000 0EDA   	pea	.L96
0000DC                  .L20015:
0000DC 4EB9 0000 0000   	jbsr	printf
0000E2                  .L20014:
0000E2 588F             	addql	#4,sp
0000E4 6000 070C        	jra	.L86
0000E8                  .L94:
0000E8 4AB9 0000 0DA8   	tstl	testmode
0000EE 6718             	jeq	.L97
0000F0 4879 0000 0001   	pea	0x1
0000F6                  .L20019:
0000F6 4EB9 0000 0000   	jbsr	getnum
0000FC 2F00             	movl	d0,sp@-
0000FE 6100 0826        	jbsr	testGE
000102 508F             	addql	#8,sp
000104 6000 06EC        	jra	.L86
000108                  .L97:
000108 4EB9 0000 0000   	jbsr	getnum
00010E 3C00             	movw	d0,d6
000110 4879 0000 0001   	pea	0x1
000116                  .L20026:
000116 4EB9 0000 0000   	jbsr	getnum
00011C 2F00             	movl	d0,sp@-
00011E 3006             	movw	d6,d0
000120 48C0             	extl	d0
000122 2F00             	movl	d0,sp@-
000124 6100 06D6        	jbsr	configureGE
000128 DEFC 000C        	addw	#12,sp
00012C 6000 06C4        	jra	.L86
000130                  .L102:
000130 7000             	moveq	#0,d0
000132 3039 0000 0000   	movw	GEstatus,d0
000138 2F00             	movl	d0,sp@-
00013A 7000             	moveq	#0,d0
00013C 3039 001F 2C00   	movw	0x1f2c00,d0
000142 2F00             	movl	d0,sp@-
000144 4879 0000 0EE7   	pea	.L103
00014A 4EB9 0000 0000   	jbsr	printf
000150 DEFC 000C        	addw	#12,sp
000154 3039 0000 0000   	movw	ix,d0
00015A 48C0             	extl	d0
00015C 0680 0000 0000   	addl	#line,d0
000162 2040             	movl	d0,a0
000164 0C10 0074        	cmpb	#0x74,a0@
000168 6600 0688        	jne	.L86
00016C 4EB9 0000 0000   	jbsr	printgflags
000172 6000 067E        	jra	.L86
000176                  .L106:
000176 3039 0000 0000   	movw	ix,d0
00017C 48C0             	extl	d0
00017E 0680 0000 0000   	addl	#line,d0
000184 2040             	movl	d0,a0
000186 0C10 003F        	cmpb	#0x3f,a0@
00018A 660A             	jne	.L107
00018C 4EB9 0000 0000   	jbsr	gshelp
000192 6000 065E        	jra	.L86
000196                  .L107:
000196 4EB9 0000 0000   	jbsr	getnum
00019C 33C0 0000 0000   	movw	d0,GEstatus
0001A2 33F9 0000 0000   	movw	GEstatus,0x1f2c00
       001F 2C00        
0001AC 6000 0644        	jra	.L86
0001B0                  .L110:
0001B0 4EB9 0000 0000   	jbsr	getnum
0001B6 33C0 0040 1000   	movw	d0,0x401000
0001BC 6000 0634        	jra	.L86
0001C0                  .L111:
0001C0 4EB9 0000 0000   	jbsr	getnum
0001C6 33C0 0040 0000   	movw	d0,0x400000
0001CC 6000 0624        	jra	.L86
0001D0                  .L112:
0001D0 4EB9 0000 0000   	jbsr	getnum
0001D6 2F00             	movl	d0,sp@-
0001D8 6100 0B10        	jbsr	vectsend
0001DC 6000 FF04        	jra	.L20014
0001E0                  .L114:
0001E0 23FC 0000 0000   	movl	#testgsi,pgearray
       0000 0000        
0001EA 3039 0000 0000   	movw	GEmask,d0
0001F0 0280 0000 1FFE   	andl	#0x1ffe,d0
0001F6 0C40 1FFE        	cmpw	#0x1ffe,d0
0001FA 664C             	jne	.L115
0001FC 23FC 0000 0000   	movl	#testgso,outfile
       0000 0000        
000206                  .L120:
000206 2079 0000 0000   	movl	pgearray,a0
00020C 0C50 04D3        	cmpw	#0x4d3,a0@
000210 6700 05E0        	jeq	.L86
000214 2E3C 0010 0000   	movl	#0x100000,d7
00021A                  .L124:
00021A 2007             	movl	d7,d0
00021C 5387             	subql	#0x1,d7
00021E 4A80             	tstl	d0
000220 670E             	jeq	.L123
000222 3039 001F 2C00   	movw	0x1f2c00,d0
000228 0280 0000 8000   	andl	#0x8000,d0
00022E 66EA             	jne	.L124
000230                  .L123:
000230 4A87             	tstl	d7
000232 6E3A             	jgt	.L126
000234 4879 0000 0F18   	pea	.L127
00023A                  .L20029:
00023A 4EB9 0000 0000   	jbsr	printf
000240 588F             	addql	#4,sp
000242                  .L20034:
000242 7000             	moveq	#0,d0
000244 6000 05AC        	jra	.L86
000248                  .L115:
000248 3039 0000 0000   	movw	GEmask,d0
00024E 0280 0000 1FFE   	andl	#0x1ffe,d0
000254 0C40 1EF6        	cmpw	#0x1ef6,d0
000258 660C             	jne	.L117
00025A 23FC 0000 0000   	movl	#test10gso,outfile
       0000 0000        
000264 60A0             	jra	.L120
000266                  .L117:
000266 4879 0000 0F06   	pea	.L119
00026C 60CC             	jra	.L20029
00026E                  .L126:
00026E 2079 0000 0000   	movl	pgearray,a0
000274 3A90             	movw	a0@,a5@
000276 54B9 0000 0000   	addql	#0x2,pgearray
00027C 6088             	jra	.L120
00027E                  .L128:
00027E 23FC 0000 0000   	movl	#testpass,pgearray
       0000 0000        
000288 23FC 0000 0000   	movl	#testpass,outfile
       0000 0000        
000292                  .L129:
000292 2079 0000 0000   	movl	pgearray,a0
000298 0C50 04D3        	cmpw	#0x4d3,a0@
00029C 6700 0554        	jeq	.L86
0002A0 3A90             	movw	a0@,a5@
0002A2 54B9 0000 0000   	addql	#0x2,pgearray
0002A8 60E8             	jra	.L129
0002AA                  .L131:
0002AA 4EB9 0000 0000   	jbsr	getnum
0002B0 3D40 FFFE        	movw	d0,a6@(-2)
0002B4 4EB9 0000 0000   	jbsr	getnum
0002BA 3D40 FFFC        	movw	d0,a6@(-4)
0002BE B06E FFFE        	cmpw	a6@(-2),d0
0002C2 6C06             	jge	.L132
0002C4 3D6E FFFE FFFC   	movw	a6@(-2),a6@(-4)
0002CA                  .L132:
0002CA 13FC 0078 0000   	movb	#0x78,intcmd
       0000             
0002D2 6000 0084        	jra	.L135
0002D6                  .L20003:
0002D6 302E FFFE        	movw	a6@(-2),d0
0002DA 48C0             	extl	d0
0002DC 7209             	moveq	#0x9,d1
0002DE E3A0             	asll	d1,d0
0002E0 0680 0000 0000   	addl	#drawtests,d0
0002E6 23C0 0000 0000   	movl	d0,pgearray
0002EC 302E FFFE        	movw	a6@(-2),d0
0002F0 48C0             	extl	d0
0002F2 EF80             	asll	#0x7,d0
0002F4 0680 0000 0000   	addl	#expect,d0
0002FA 23C0 0000 0000   	movl	d0,pintdata
000300 2040             	movl	d0,a0
000302 33D0 0000 0000   	movw	a0@,expecting_interrupt
000308 54B9 0000 0000   	addql	#0x2,pintdata
00030E 6038             	jra	.L136
000310                  .L20001:
000310 2E3C 0010 0000   	movl	#0x100000,d7
000316                  .L140:
000316 2007             	movl	d7,d0
000318 5387             	subql	#0x1,d7
00031A 4A80             	tstl	d0
00031C 670E             	jeq	.L139
00031E 3039 001F 2C00   	movw	0x1f2c00,d0
000324 0280 0000 8000   	andl	#0x8000,d0
00032A 66EA             	jne	.L140
00032C                  .L139:
00032C 4A87             	tstl	d7
00032E 6E0A             	jgt	.L142
000330 4879 0000 0F1E   	pea	.L143
000336 6000 FF02        	jra	.L20029
00033A                  .L142:
00033A 2079 0000 0000   	movl	pgearray,a0
000340 3A90             	movw	a0@,a5@
000342 54B9 0000 0000   	addql	#0x2,pgearray
000348                  .L136:
000348 2079 0000 0000   	movl	pgearray,a0
00034E 0C50 04D3        	cmpw	#0x4d3,a0@
000352 66BC             	jne	.L20001
000354 526E FFFE        	addqw	#0x1,a6@(-2)
000358                  .L135:
000358 302E FFFE        	movw	a6@(-2),d0
00035C B06E FFFC        	cmpw	a6@(-4),d0
000360 6F00 FF74        	jle	.L20003
000364 4A79 0000 0000   	tstw	expecting_interrupt
00036A 672E             	jeq	.L144
00036C 4879 0000 0064   	pea	0x64
000372 4EB9 0000 0000   	jbsr	buzz
000378 588F             	addql	#4,sp
00037A 4A79 0000 0000   	tstw	expecting_interrupt
000380 6718             	jeq	.L144
000382 3039 0000 0000   	movw	expecting_interrupt,d0
000388 48C0             	extl	d0
00038A 2F00             	movl	d0,sp@-
00038C 4879 0000 0F24   	pea	.L147
000392 4EB9 0000 0000   	jbsr	printf
000398 508F             	addql	#8,sp
00039A                  .L144:
00039A 4279 0000 0000   	clrw	expecting_interrupt
0003A0 6000 0450        	jra	.L86
0003A4                  .L148:
0003A4 3039 0000 0000   	movw	ix,d0
0003AA 48C0             	extl	d0
0003AC 0680 0000 0000   	addl	#line,d0
0003B2 2040             	movl	d0,a0
0003B4 0C10 0072        	cmpb	#0x72,a0@
0003B8 6622             	jne	.L10000
0003BA 3039 0000 0ED6   	movw	delaycount,d0
0003C0 48C0             	extl	d0
0003C2 E580             	asll	#0x2,d0
0003C4 3239 0000 0ED6   	movw	delaycount,d1
0003CA 48C1             	extl	d1
0003CC D081             	addl	d1,d0
0003CE 0680 0000 44D7   	addl	#0x44d7,d0
0003D4 0280 0000 03FF   	andl	#0x3ff,d0
0003DA 6006             	jra	.L10001
0003DC                  .L10000:
0003DC 4EB9 0000 0000   	jbsr	getnum
0003E2                  .L10001:
0003E2 33C0 0000 0ED6   	movw	d0,delaycount
0003E8 6000 0408        	jra	.L86
0003EC                  .L149:
0003EC 4EB9 0000 0000   	jbsr	getnum
0003F2 7209             	moveq	#0x9,d1
0003F4 E3A0             	asll	d1,d0
0003F6 0680 0000 0000   	addl	#bustests,d0
0003FC 23C0 0000 0000   	movl	d0,pgearray
000402 33FC 0073 001F   	movw	#0x73,0x1f2400
       2400             
00040A                  .L150:
00040A 2079 0000 0000   	movl	pgearray,a0
000410 0C50 04D3        	cmpw	#0x4d3,a0@
000414 6700 03DC        	jeq	.L86
000418 33D0 001F 2800   	movw	a0@,0x1f2800
00041E 54B9 0000 0000   	addql	#0x2,pgearray
000424 33FC 0063 001F   	movw	#0x63,0x1f2400
       2400             
00042C 33FC 0073 001F   	movw	#0x73,0x1f2400
       2400             
000434 3039 0000 0ED6   	movw	delaycount,d0
00043A 48C0             	extl	d0
00043C 2F00             	movl	d0,sp@-
00043E 4EB9 0000 0000   	jbsr	buzz
000444 588F             	addql	#4,sp
000446 60C2             	jra	.L150
000448                  .L152:
000448 33FC 843F 0000   	movw	#-31681,GEstatus
       0000             
000450 33F9 0000 0000   	movw	GEstatus,0x1f2c00
       001F 2C00        
00045A 4879 0000 0064   	pea	0x64
000460 4EB9 0000 0000   	jbsr	buzz
000466 588F             	addql	#4,sp
000468 33FC 8036 0000   	movw	#-32714,GEstatus
       0000             
000470 33F9 0000 0000   	movw	GEstatus,0x1f2c00
       001F 2C00        
00047A 4279 0000 0000   	clrw	outx
000480 4239 0000 0000   	clrb	intcmd
000486 6000 036A        	jra	.L86
00048A                  .L153:
00048A 4A79 0000 0000   	tstw	initdone
000490 660A             	jne	.L154
000492 4879 0000 0F3B   	pea	.L155
000498 6000 FC42        	jra	.L20015
00049C                  .L154:
00049C 4AB9 0000 0DA8   	tstl	testmode
0004A2 670A             	jeq	.L156
0004A4 4879 0000 0000   	pea	0x0
0004AA 6000 FC4A        	jra	.L20019
0004AE                  .L156:
0004AE 4EB9 0000 0000   	jbsr	getnum
0004B4 3C00             	movw	d0,d6
0004B6 4879 0000 0000   	pea	0x0
0004BC 6000 FC58        	jra	.L20026
0004C0                  .L158:
0004C0 3039 0000 0ED8   	movw	printconfig,d0
0004C6 48C0             	extl	d0
0004C8 7201             	moveq	#0x1,d1
0004CA 9280             	subl	d0,d1
0004CC 33C1 0000 0ED8   	movw	d1,printconfig
0004D2 13FC 0070 0000   	movb	#0x70,intcmd
       0000             
0004DA 6000 0316        	jra	.L86
0004DE                  .L159:
0004DE 4EB9 0000 0000   	jbsr	getnum
0004E4 3D40 FFFE        	movw	d0,a6@(-2)
0004E8 42B9 0000 0DA8   	clrl	testmode
0004EE 4A6E FFFE        	tstw	a6@(-2)
0004F2 6714             	jeq	.L161
0004F4 302E FFFE        	movw	a6@(-2),d0
0004F8 48C0             	extl	d0
0004FA 2F00             	movl	d0,sp@-
0004FC 6100 04E4        	jbsr	inpipe
000500 588F             	addql	#4,sp
000502 4A80             	tstl	d0
000504 6700 02EC        	jeq	.L86
000508                  .L161:
000508 302E FFFE        	movw	a6@(-2),d0
00050C 48C0             	extl	d0
00050E 23C0 0000 0DA8   	movl	d0,testmode
000514 4A80             	tstl	d0
000516 6704             	jeq	.L10002
000518 7054             	moveq	#0x54,d0
00051A 6002             	jra	.L10003
00051C                  .L10002:
00051C 7021             	moveq	#0x21,d0
00051E                  .L10003:
00051E 13C0 0000 0000   	movb	d0,prompt
000524 6000 02CC        	jra	.L86
000528                  .L162:
000528 4EB9 0000 0000   	jbsr	getnum
00052E 23C0 0000 0DA4   	movl	d0,testlim
000534 6000 02BC        	jra	.L86
000538                  .L163:
000538 4A79 0000 0000   	tstw	initdone
00053E 660A             	jne	.L164
000540 4879 0000 0F48   	pea	.L165
000546 6000 FCF2        	jra	.L20029
00054A                  .L164:
00054A 4EB9 0000 0000   	jbsr	getnum
000550 3D40 FFFE        	movw	d0,a6@(-2)
000554 302E FFFE        	movw	a6@(-2),d0
000558 48C0             	extl	d0
00055A 2F00             	movl	d0,sp@-
00055C 6100 0484        	jbsr	inpipe
000560 588F             	addql	#4,sp
000562 4A80             	tstl	d0
000564 661E             	jne	.L166
000566                  .L20041:
000566 4879 0000 0000   	pea	0x0
00056C 4879 0000 000D   	pea	0xd
000572 6100 03B2        	jbsr	testGE
000576 508F             	addql	#8,sp
000578 33FC 0BAD 0040   	movw	#0xbad,0x401000
       1000             
000580 6000 FCC0        	jra	.L20034
000584                  .L166:
000584 3039 0000 0000   	movw	GEmask,d0
00058A 0280 0000 1FFE   	andl	#0x1ffe,d0
000590 0C40 1FFE        	cmpw	#0x1ffe,d0
000594 6710             	jeq	.L10004
000596 302E FFFE        	movw	a6@(-2),d0
00059A 48C0             	extl	d0
00059C E580             	asll	#0x2,d0
00059E 0680 0000 0E1C   	addl	#test10in,d0
0005A4 600E             	jra	.L20005
0005A6                  .L10004:
0005A6 302E FFFE        	movw	a6@(-2),d0
0005AA 48C0             	extl	d0
0005AC E580             	asll	#0x2,d0
0005AE 0680 0000 0DAC   	addl	#testin,d0
0005B4                  .L20005:
0005B4 2040             	movl	d0,a0
0005B6 2010             	movl	a0@,d0
0005B8 23C0 0000 0000   	movl	d0,pgearray
0005BE 4EB9 0000 0000   	jbsr	getnum
0005C4 3D40 FFFC        	movw	d0,a6@(-4)
0005C8 302E FFFC        	movw	a6@(-4),d0
0005CC B06E FFFE        	cmpw	a6@(-2),d0
0005D0 6C06             	jge	.L167
0005D2 3D6E FFFE FFFC   	movw	a6@(-2),a6@(-4)
0005D8                  .L167:
0005D8 302E FFFC        	movw	a6@(-4),d0
0005DC 48C0             	extl	d0
0005DE 2F00             	movl	d0,sp@-
0005E0 6100 0400        	jbsr	inpipe
0005E4 588F             	addql	#4,sp
0005E6 4A80             	tstl	d0
0005E8 6700 FC58        	jeq	.L20034
0005EC 3039 0000 0000   	movw	GEmask,d0
0005F2 0280 0000 1FFE   	andl	#0x1ffe,d0
0005F8 0C40 1FFE        	cmpw	#0x1ffe,d0
0005FC 6710             	jeq	.L10006
0005FE 302E FFFC        	movw	a6@(-4),d0
000602 48C0             	extl	d0
000604 E580             	asll	#0x2,d0
000606 0680 0000 0E50   	addl	#test10out,d0
00060C 600E             	jra	.L20007
00060E                  .L10006:
00060E 302E FFFC        	movw	a6@(-4),d0
000612 48C0             	extl	d0
000614 E580             	asll	#0x2,d0
000616 0680 0000 0DE4   	addl	#testout,d0
00061C                  .L20007:
00061C 2040             	movl	d0,a0
00061E 2010             	movl	a0@,d0
000620 23C0 0000 0000   	movl	d0,outfile
000626 2F00             	movl	d0,sp@-
000628 2039 0000 0000   	movl	pgearray,d0
00062E 2F00             	movl	d0,sp@-
000630 4879 0000 0F55   	pea	.L169
000636 4EB9 0000 0000   	jbsr	printf
00063C DEFC 000C        	addw	#12,sp
000640 4AB9 0000 0DA8   	tstl	testmode
000646 6716             	jeq	.L170
000648 4879 0000 0000   	pea	0x0
00064E 302E FFFE        	movw	a6@(-2),d0
000652 48C0             	extl	d0
000654 2F00             	movl	d0,sp@-
000656 6100 02CE        	jbsr	testGE
00065A 508F             	addql	#8,sp
00065C 601E             	jra	.L171
00065E                  .L170:
00065E 4879 0000 0000   	pea	0x0
000664 302E FFFC        	movw	a6@(-4),d0
000668 48C0             	extl	d0
00066A 2F00             	movl	d0,sp@-
00066C 302E FFFE        	movw	a6@(-2),d0
000670 48C0             	extl	d0
000672 2F00             	movl	d0,sp@-
000674 6100 0186        	jbsr	configureGE
000678 DEFC 000C        	addw	#12,sp
00067C                  .L171:
00067C 4EB9 0000 0000   	jbsr	getnum
000682 3D40 FFFE        	movw	d0,a6@(-2)
000686 4A6E FFFE        	tstw	a6@(-2)
00068A 6606             	jne	.L172
00068C 3D7C 0046 FFFE   	movw	#0x46,a6@(-2)
000692                  .L172:
000692 302E FFFE        	movw	a6@(-2),d0
000696 48C0             	extl	d0
000698 2F00             	movl	d0,sp@-
00069A 6100 03D2        	jbsr	Bsend
00069E 6000 FA42        	jra	.L20014
0006A2                  .L174:
0006A2 4A79 0000 0000   	tstw	initdone
0006A8 660A             	jne	.L175
0006AA 4879 0000 0F6A   	pea	.L176
0006B0 6000 FB88        	jra	.L20029
0006B4                  .L175:
0006B4 33FC 0001 0000   	movw	#0x1,fastest
       0000             
0006BC 5379 0000 0000   	subqw	#0x1,ix
0006C2 4EB9 0000 0000   	jbsr	getnum
0006C8 3D40 FFFE        	movw	d0,a6@(-2)
0006CC 302E FFFE        	movw	a6@(-2),d0
0006D0 48C0             	extl	d0
0006D2 2F00             	movl	d0,sp@-
0006D4 6100 030C        	jbsr	inpipe
0006D8 588F             	addql	#4,sp
0006DA 4A80             	tstl	d0
0006DC 6700 FE88        	jeq	.L20041
0006E0 3039 0000 0000   	movw	GEmask,d0
0006E6 0280 0000 1FFE   	andl	#0x1ffe,d0
0006EC 0C40 1FFE        	cmpw	#0x1ffe,d0
0006F0 6710             	jeq	.L10008
0006F2 302E FFFE        	movw	a6@(-2),d0
0006F6 48C0             	extl	d0
0006F8 E580             	asll	#0x2,d0
0006FA 0680 0000 0E1C   	addl	#test10in,d0
000700 600E             	jra	.L20009
000702                  .L10008:
000702 302E FFFE        	movw	a6@(-2),d0
000706 48C0             	extl	d0
000708 E580             	asll	#0x2,d0
00070A 0680 0000 0DAC   	addl	#testin,d0
000710                  .L20009:
000710 2040             	movl	d0,a0
000712 2010             	movl	a0@,d0
000714 23C0 0000 0000   	movl	d0,pgearray
00071A 4EB9 0000 0000   	jbsr	getnum
000720 3D40 FFFC        	movw	d0,a6@(-4)
000724 302E FFFC        	movw	a6@(-4),d0
000728 B06E FFFE        	cmpw	a6@(-2),d0
00072C 6C06             	jge	.L178
00072E 3D6E FFFE FFFC   	movw	a6@(-2),a6@(-4)
000734                  .L178:
000734 302E FFFC        	movw	a6@(-4),d0
000738 48C0             	extl	d0
00073A 2F00             	movl	d0,sp@-
00073C 6100 02A4        	jbsr	inpipe
000740 588F             	addql	#4,sp
000742 4A80             	tstl	d0
000744 6700 FAFC        	jeq	.L20034
000748 3039 0000 0000   	movw	GEmask,d0
00074E 0280 0000 1FFE   	andl	#0x1ffe,d0
000754 0C40 1FFE        	cmpw	#0x1ffe,d0
000758 6710             	jeq	.L10010
00075A 302E FFFC        	movw	a6@(-4),d0
00075E 48C0             	extl	d0
000760 E580             	asll	#0x2,d0
000762 0680 0000 0E50   	addl	#test10out,d0
000768 600E             	jra	.L20011
00076A                  .L10010:
00076A 302E FFFC        	movw	a6@(-4),d0
00076E 48C0             	extl	d0
000770 E580             	asll	#0x2,d0
000772 0680 0000 0DE4   	addl	#testout,d0
000778                  .L20011:
000778 2040             	movl	d0,a0
00077A 2010             	movl	a0@,d0
00077C 23C0 0000 0000   	movl	d0,outfile
000782 2F00             	movl	d0,sp@-
000784 2039 0000 0000   	movl	pgearray,d0
00078A 2F00             	movl	d0,sp@-
00078C 4879 0000 0F77   	pea	.L180
000792 4EB9 0000 0000   	jbsr	printf
000798 DEFC 000C        	addw	#12,sp
00079C 4246             	clrw	d6
00079E 6044             	jra	.L181
0007A0                  .L20013:
0007A0 2079 0000 0000   	movl	pgearray,a0
0007A6 0C50 04D3        	cmpw	#0x4d3,a0@
0007AA 6746             	jeq	.L86
0007AC 2E3C 0010 0000   	movl	#0x100000,d7
0007B2                  .L185:
0007B2 2007             	movl	d7,d0
0007B4 5387             	subql	#0x1,d7
0007B6 4A80             	tstl	d0
0007B8 670E             	jeq	.L184
0007BA 3039 001F 2C00   	movw	0x1f2c00,d0
0007C0 0280 0000 8000   	andl	#0x8000,d0
0007C6 66EA             	jne	.L185
0007C8                  .L184:
0007C8 4A87             	tstl	d7
0007CA 6E0A             	jgt	.L187
0007CC 4879 0000 0F8C   	pea	.L188
0007D2 6000 FA66        	jra	.L20029
0007D6                  .L187:
0007D6 2079 0000 0000   	movl	pgearray,a0
0007DC 3A90             	movw	a0@,a5@
0007DE 54B9 0000 0000   	addql	#0x2,pgearray
0007E4                  .L181:
0007E4 5246             	addqw	#0x1,d6
0007E6 3006             	movw	d6,d0
0007E8 48C0             	extl	d0
0007EA B0B9 0000 0DA4   	cmpl	testlim,d0
0007F0 6DAE             	jlt	.L20013
0007F2                  .L86:
0007F2 4CEE 20C0 FFF0   	moveml	a6@(-.F1),#0x20c0
0007F8 4E5E             	unlk	a6
0007FA 4E75             	rts
       0000 0010        .F1 = 16
       0000 20C0        .S1 = 8384
                        | M1 = 144
                        	.globl	configureGE
0007FC                  configureGE:
0007FC 4E56 FFF4        	link	a6,#-.F2
000800 2F07             	movl	d7,sp@-
000802 2F06             	movl	d6,sp@-
000804 3E2E 000E        	movw	a6@(14),d7
                        | A2 = 20
000808 3039 0000 0000   	movw	GEmask,d0
00080E 0280 0000 1FFE   	andl	#0x1ffe,d0
000814 0C40 1FFE        	cmpw	#0x1ffe,d0
000818 6708             	jeq	.L10012
00081A 203C 0000 0EA2   	movl	#masterconfig10,d0
000820 6006             	jra	.L10013
000822                  .L10012:
000822 203C 0000 0E88   	movl	#masterconfig,d0
000828                  .L10013:
000828 2D40 FFFC        	movl	d0,a6@(-4)
00082C 4A6E 000A        	tstw	a6@(0xa)
000830 6608             	jne	.L191
000832 3D7C 0001 000A   	movw	#0x1,a6@(0xa)
000838 7E0C             	moveq	#0xc,d7
00083A                  .L191:
00083A BE6E 000A        	cmpw	a6@(0xa),d7
00083E 6C04             	jge	.L192
000840 3E2E 000A        	movw	a6@(0xa),d7
000844                  .L192:
000844 302E 000A        	movw	a6@(0xa),d0
000848 48C0             	extl	d0
00084A 2F00             	movl	d0,sp@-
00084C 6100 0194        	jbsr	inpipe
000850 588F             	addql	#4,sp
000852 4A80             	tstl	d0
000854 6710             	jeq	.L10014
000856 3007             	movw	d7,d0
000858 48C0             	extl	d0
00085A 2F00             	movl	d0,sp@-
00085C 6100 0184        	jbsr	inpipe
000860 588F             	addql	#4,sp
000862 4A80             	tstl	d0
000864 6606             	jne	.L193
000866                  .L10014:
000866 7000             	moveq	#0,d0
000868 6000 00B4        	jra	.L190
00086C                  .L193:
00086C 7C01             	moveq	#0x1,d6
00086E 6066             	jra	.L196
000870                  .L20043:
000870 BC6E 000A        	cmpw	a6@(0xa),d6
000874 6D22             	jlt	.L197
000876 BC47             	cmpw	d7,d6
000878 6E1E             	jgt	.L197
00087A 3006             	movw	d6,d0
00087C 48C0             	extl	d0
00087E E380             	asll	#0x1,d0
000880 D0AE FFFC        	addl	a6@(-4),d0
000884 2040             	movl	d0,a0
000886 3006             	movw	d6,d0
000888 48C0             	extl	d0
00088A E380             	asll	#0x1,d0
00088C 0680 0000 0EBC   	addl	#useconfig,d0
000892 2240             	movl	d0,a1
000894 3290             	movw	a0@,a1@
000896 6012             	jra	.L198
000898                  .L197:
000898 3006             	movw	d6,d0
00089A 48C0             	extl	d0
00089C E380             	asll	#0x1,d0
00089E 0680 0000 0EBC   	addl	#useconfig,d0
0008A4 2040             	movl	d0,a0
0008A6 30BC 0038        	movw	#0x38,a0@
0008AA                  .L198:
0008AA 4A79 0000 0ED8   	tstw	printconfig
0008B0 6722             	jeq	.L194
0008B2 3006             	movw	d6,d0
0008B4 48C0             	extl	d0
0008B6 E380             	asll	#0x1,d0
0008B8 0680 0000 0EBC   	addl	#useconfig,d0
0008BE 2040             	movl	d0,a0
0008C0 7000             	moveq	#0,d0
0008C2 3010             	movw	a0@,d0
0008C4 2F00             	movl	d0,sp@-
0008C6 4879 0000 0F92   	pea	.L200
0008CC 4EB9 0000 0000   	jbsr	printf
0008D2 508F             	addql	#8,sp
0008D4                  .L194:
0008D4 5246             	addqw	#0x1,d6
0008D6                  .L196:
0008D6 0C46 000D        	cmpw	#0xd,d6
0008DA 6D94             	jlt	.L20043
0008DC 4A79 0000 0ED8   	tstw	printconfig
0008E2 670E             	jeq	.L201
0008E4 4879 0000 000A   	pea	0xa
0008EA 4EB9 0000 0000   	jbsr	putchar
0008F0 588F             	addql	#4,sp
0008F2                  .L201:
0008F2 4A6E 0012        	tstw	a6@(0x12)
0008F6 670E             	jeq	.L203
0008F8 4879 0000 0EBC   	pea	useconfig
0008FE 4EB9 0000 0000   	jbsr	justconfigure
000904 600C             	jra	.L20044
000906                  .L203:
000906 4879 0000 0EBC   	pea	useconfig
00090C 4EB9 0000 0000   	jbsr	smartconfigure
000912                  .L20044:
000912 588F             	addql	#4,sp
000914 33F9 0000 0000   	movw	GEstatus,0x1f2c00
       001F 2C00        
00091E                  .L190:
00091E 2C1F             	movl	sp@+,d6
000920 2E1F             	movl	sp@+,d7
000922 4E5E             	unlk	a6
000924 4E75             	rts
       0000 000C        .F2 = 12
       0000 00C0        .S2 = 192
                        | M2 = 140
                        	.globl	testGE
000926                  testGE:
000926 4E56 FFF8        	link	a6,#-.F3
00092A 2F07             	movl	d7,sp@-
                        | A3 = 16
00092C 3039 0000 0000   	movw	GEmask,d0
000932 0280 0000 1FFE   	andl	#0x1ffe,d0
000938 0C40 1FFE        	cmpw	#0x1ffe,d0
00093C 6708             	jeq	.L10015
00093E 203C 0000 0EA2   	movl	#masterconfig10,d0
000944 6006             	jra	.L10016
000946                  .L10015:
000946 203C 0000 0E88   	movl	#masterconfig,d0
00094C                  .L10016:
00094C 2D40 FFFC        	movl	d0,a6@(-4)
000950 7E01             	moveq	#0x1,d7
000952 6014             	jra	.L210
000954                  .L20046:
000954 3007             	movw	d7,d0
000956 48C0             	extl	d0
000958 E380             	asll	#0x1,d0
00095A 0680 0000 0EBC   	addl	#useconfig,d0
000960 2040             	movl	d0,a0
000962 30BC 0038        	movw	#0x38,a0@
000966 5247             	addqw	#0x1,d7
000968                  .L210:
000968 0C47 000D        	cmpw	#0xd,d7
00096C 6DE6             	jlt	.L20046
00096E 4A6E 000A        	tstw	a6@(0xa)
000972 663C             	jne	.L211
000974 2039 0000 0DA8   	movl	testmode,d0
00097A                  .L20055:
00097A E380             	asll	#0x1,d0
00097C D0AE FFFC        	addl	a6@(-4),d0
000980 2040             	movl	d0,a0
000982 2039 0000 0DA8   	movl	testmode,d0
000988 E380             	asll	#0x1,d0
00098A 0680 0000 0EBC   	addl	#useconfig,d0
000990 2240             	movl	d0,a1
000992 3290             	movw	a0@,a1@
000994 4A6E 000E        	tstw	a6@(0xe)
000998 672A             	jeq	.L215
00099A 4879 0000 0EBC   	pea	useconfig
0009A0 4EB9 0000 0000   	jbsr	justconfigure
0009A6 6028             	jra	.L20047
0009A8                  .L20057:
0009A8 302E 000A        	movw	a6@(0xa),d0
0009AC 48C0             	extl	d0
0009AE 60CA             	jra	.L20055
0009B0                  .L211:
0009B0 302E 000A        	movw	a6@(0xa),d0
0009B4 48C0             	extl	d0
0009B6 2F00             	movl	d0,sp@-
0009B8 6128             	jbsr	inpipe
0009BA 588F             	addql	#4,sp
0009BC 4A80             	tstl	d0
0009BE 66E8             	jne	.L20057
0009C0 7000             	moveq	#0,d0
0009C2 6018             	jra	.L207
0009C4                  .L215:
0009C4 4879 0000 0EBC   	pea	useconfig
0009CA 4EB9 0000 0000   	jbsr	smartconfigure
0009D0                  .L20047:
0009D0 588F             	addql	#4,sp
0009D2 33F9 0000 0000   	movw	GEstatus,0x1f2c00
       001F 2C00        
0009DC                  .L207:
0009DC 2E1F             	movl	sp@+,d7
0009DE 4E5E             	unlk	a6
0009E0 4E75             	rts
       0000 0008        .F3 = 8
       0000 0080        .S3 = 128
                        | M3 = 136
                        	.globl	inpipe
0009E2                  inpipe:
0009E2 4E56 FFFC        	link	a6,#-.F4
                        | A4 = 12
0009E6 1D79 0000 0000   	movb	intcmd,a6@(-1)
       FFFF             
0009EE 4239 0000 0000   	clrb	intcmd
0009F4 0C6E 000D 000A   	cmpw	#0xd,a6@(0xa)
0009FA 676C             	jeq	.L20058
0009FC 0C6E 0001 000A   	cmpw	#0x1,a6@(0xa)
000A02 6D08             	jlt	.L10017
000A04 0C6E 000C 000A   	cmpw	#0xc,a6@(0xa)
000A0A 6F4C             	jle	.L219
000A0C                  .L10017:
000A0C 302E 000A        	movw	a6@(0xa),d0
000A10 48C0             	extl	d0
000A12 2F00             	movl	d0,sp@-
000A14 4879 0000 0F97   	pea	.L220
000A1A 6028             	jra	.L20062
000A1C                  .L20064:
000A1C 302E 000A        	movw	a6@(0xa),d0
000A20 48C0             	extl	d0
000A22 223C 0000 2000   	movl	#0x2000,d1
000A28 E0A1             	asrl	d0,d1
000A2A 3039 0000 0000   	movw	GEmask,d0
000A30 48C0             	extl	d0
000A32 C280             	andl	d0,d1
000A34 662A             	jne	.L221
000A36 302E 000A        	movw	a6@(0xa),d0
000A3A 48C0             	extl	d0
000A3C 2F00             	movl	d0,sp@-
000A3E 4879 0000 0FA8   	pea	.L222
000A44                  .L20062:
000A44 4EB9 0000 0000   	jbsr	printf
000A4A 508F             	addql	#8,sp
000A4C 13FC 0061 0000   	movb	#0x61,intcmd
       0000             
000A54 7000             	moveq	#0,d0
000A56 6012             	jra	.L217
000A58                  .L219:
000A58 4AB9 0000 0DA8   	tstl	testmode
000A5E 67BC             	jeq	.L20064
000A60                  .L221:
000A60 13EE FFFF 0000   	movb	a6@(-1),intcmd
       0000             
000A68                  .L20058:
000A68 7001             	moveq	#0x1,d0
000A6A                  .L217:
000A6A 4E5E             	unlk	a6
000A6C 4E75             	rts
       0000 0004        .F4 = 4
       0000 0000        .S4 = 0
                        | M4 = 140
                        	.globl	Bsend
000A6E                  Bsend:
000A6E 4E56 FFF0        	link	a6,#-.F5
000A72 48EE 30A0 FFF0   	moveml	#.S5,a6@(-.F5)
000A78 3E2E 000A        	movw	a6@(10),d7
                        | A5 = 12
000A7C 2A7C 0040 1000   	movl	#0x401000,a5
000A82 3039 0000 0000   	movw	devstatus,d0
000A88 48C0             	extl	d0
000A8A 0080 0000 0040   	orl	#0x40,d0
000A90 33C0 0000 0000   	movw	d0,devstatus
000A96 33F9 0000 0000   	movw	devstatus,0x1f2400
       001F 2400        
000AA0 0247 FFFE        	andw	#-2,d7
000AA4 4245             	clrw	d5
000AA6 6006             	jra	.L226
000AA8                  .L20066:
000AA8 3ABC 0008        	movw	#0x8,a5@
000AAC 5245             	addqw	#0x1,d5
000AAE                  .L226:
000AAE BA47             	cmpw	d7,d5
000AB0 6DF6             	jlt	.L20066
000AB2 3007             	movw	d7,d0
000AB4 4440             	negw	d0
000AB6 33C0 0000 0000   	movw	d0,outx
000ABC 600E             	jra	.L227
000ABE                  .L20068:
000ABE 2079 0000 0000   	movl	pgearray,a0
000AC4 3A90             	movw	a0@,a5@
000AC6 54B9 0000 0000   	addql	#0x2,pgearray
000ACC                  .L227:
000ACC 3039 001F 2C00   	movw	0x1f2c00,d0
000AD2 0280 0000 8000   	andl	#0x8000,d0
000AD8 67E4             	jeq	.L20068
000ADA 4879 0000 03E8   	pea	0x3e8
000AE0 4EB9 0000 0000   	jbsr	buzz
000AE6 588F             	addql	#4,sp
000AE8 3039 0000 0000   	movw	devstatus,d0
000AEE 48C0             	extl	d0
000AF0 0280 FFFF FFBF   	andl	#-65,d0
000AF6 33C0 0000 0000   	movw	d0,devstatus
000AFC 33F9 0000 0000   	movw	devstatus,0x1f2400
       001F 2400        
000B06 3A3C 04D3        	movw	#0x4d3,d5
000B0A 2879 0000 0000   	movl	pgearray,a4
000B10 33FC 0001 0000   	movw	#0x1,fastest
       0000             
000B18 6002             	jra	.L229
000B1A                  .L20070:
000B1A 3A9C             	movw	a4@+,a5@
000B1C                  .L229:
000B1C 3014             	movw	a4@,d0
000B1E B045             	cmpw	d5,d0
000B20 66F8             	jne	.L20070
000B22 4279 0000 0000   	clrw	fastest
000B28 4CEE 30A0 FFF0   	moveml	a6@(-.F5),#0x30a0
000B2E 4E5E             	unlk	a6
000B30 4E75             	rts
       0000 0010        .F5 = 16
       0000 30A0        .S5 = 12448
                        | M5 = 136
                        	.globl	gehelp
000B32                  gehelp:
000B32 4E56 FFF8        	link	a6,#-.F6
000B36 2F07             	movl	d7,sp@-
000B38 2F06             	movl	d6,sp@-
                        | A6 = 8
000B3A 4879 0000 0FBF   	pea	.L232
000B40 4EB9 0000 0000   	jbsr	printf
000B46 588F             	addql	#4,sp
000B48 4879 0000 0FDF   	pea	.L233
000B4E 4EB9 0000 0000   	jbsr	printf
000B54 588F             	addql	#4,sp
000B56 4879 0000 1014   	pea	.L234
000B5C 4EB9 0000 0000   	jbsr	printf
000B62 588F             	addql	#4,sp
000B64 4879 0000 104A   	pea	.L235
000B6A 4EB9 0000 0000   	jbsr	printf
000B70 588F             	addql	#4,sp
000B72 4879 0000 106B   	pea	.L236
000B78 4EB9 0000 0000   	jbsr	printf
000B7E 588F             	addql	#4,sp
000B80 4879 0000 1091   	pea	.L237
000B86 4EB9 0000 0000   	jbsr	printf
000B8C 588F             	addql	#4,sp
000B8E 4879 0000 10C2   	pea	.L238
000B94 4EB9 0000 0000   	jbsr	printf
000B9A 588F             	addql	#4,sp
000B9C 4879 0000 10D5   	pea	.L239
000BA2 4EB9 0000 0000   	jbsr	printf
000BA8 588F             	addql	#4,sp
000BAA 4879 0000 10E9   	pea	.L240
000BB0 4EB9 0000 0000   	jbsr	printf
000BB6 588F             	addql	#4,sp
000BB8 4879 0000 1124   	pea	.L241
000BBE 4EB9 0000 0000   	jbsr	printf
000BC4 588F             	addql	#4,sp
000BC6 4879 0000 1146   	pea	.L242
000BCC 4EB9 0000 0000   	jbsr	printf
000BD2 588F             	addql	#4,sp
000BD4 4879 0000 1164   	pea	.L243
000BDA 4EB9 0000 0000   	jbsr	printf
000BE0 588F             	addql	#4,sp
000BE2 4879 0000 117D   	pea	.L244
000BE8 4EB9 0000 0000   	jbsr	printf
000BEE 588F             	addql	#4,sp
000BF0 4879 0000 1194   	pea	.L245
000BF6 4EB9 0000 0000   	jbsr	printf
000BFC 588F             	addql	#4,sp
000BFE 4879 0000 11A9   	pea	.L246
000C04 4EB9 0000 0000   	jbsr	printf
000C0A 588F             	addql	#4,sp
000C0C 4879 0000 11E2   	pea	.L247
000C12 4EB9 0000 0000   	jbsr	printf
000C18 588F             	addql	#4,sp
000C1A 4879 0000 1206   	pea	.L248
000C20 4EB9 0000 0000   	jbsr	printf
000C26 588F             	addql	#4,sp
000C28 4879 0000 1237   	pea	.L249
000C2E 4EB9 0000 0000   	jbsr	printf
000C34 588F             	addql	#4,sp
000C36 4879 0000 1247   	pea	.L250
000C3C 4EB9 0000 0000   	jbsr	printf
000C42 588F             	addql	#4,sp
000C44 4EB9 0000 0000   	jbsr	getchar
000C4A 4879 0000 000A   	pea	0xa
000C50 4EB9 0000 0000   	jbsr	putchar
000C56 588F             	addql	#4,sp
000C58 7E01             	moveq	#0x1,d7
000C5A 6042             	jra	.L254
000C5C                  .L20072:
000C5C 7C00             	moveq	#0,d6
000C5E 2007             	movl	d7,d0
000C60 E580             	asll	#0x2,d0
000C62 0680 0000 0DE4   	addl	#testout,d0
000C68 2040             	movl	d0,a0
000C6A 23D0 0000 0000   	movl	a0@,outfile
000C70                  .L257:
000C70 2006             	movl	d6,d0
000C72 5286             	addql	#0x1,d6
000C74 E380             	asll	#0x1,d0
000C76 D0B9 0000 0000   	addl	outfile,d0
000C7C 2040             	movl	d0,a0
000C7E 0C50 04D3        	cmpw	#0x4d3,a0@
000C82 66EC             	jne	.L257
000C84 2006             	movl	d6,d0
000C86 5380             	subql	#0x1,d0
000C88 2F00             	movl	d0,sp@-
000C8A 2F07             	movl	d7,sp@-
000C8C 4879 0000 124F   	pea	.L258
000C92 4EB9 0000 0000   	jbsr	printf
000C98 DEFC 000C        	addw	#12,sp
000C9C 5287             	addql	#0x1,d7
000C9E                  .L254:
000C9E 2007             	movl	d7,d0
000CA0 E580             	asll	#0x2,d0
000CA2 0680 0000 0DE4   	addl	#testout,d0
000CA8 2040             	movl	d0,a0
000CAA 4A90             	tstl	a0@
000CAC 66AE             	jne	.L20072
000CAE 7C00             	moveq	#0,d6
000CB0 23FC 0000 0000   	movl	#test10gso,outfile
       0000 0000        
000CBA                  .L261:
000CBA 2006             	movl	d6,d0
000CBC 5286             	addql	#0x1,d6
000CBE E380             	asll	#0x1,d0
000CC0 D0B9 0000 0000   	addl	outfile,d0
000CC6 2040             	movl	d0,a0
000CC8 0C50 04D3        	cmpw	#0x4d3,a0@
000CCC 66EC             	jne	.L261
000CCE 2006             	movl	d6,d0
000CD0 5380             	subql	#0x1,d0
000CD2 2F00             	movl	d0,sp@-
000CD4 4879 0000 125E   	pea	.L262
000CDA 4EB9 0000 0000   	jbsr	printf
000CE0 508F             	addql	#8,sp
000CE2 2C1F             	movl	sp@+,d6
000CE4 2E1F             	movl	sp@+,d7
000CE6 4E5E             	unlk	a6
000CE8 4E75             	rts
       0000 0008        .F6 = 8
       0000 00C0        .S6 = 192
                        | M6 = 144
                        	.globl	vectsend
000CEA                  vectsend:
000CEA 4E56 FFE8        	link	a6,#-.F7
000CEE 48EE 30F0 FFE8   	moveml	#.S7,a6@(-.F7)
000CF4 2E2E 0008        	movl	a6@(8),d7
                        | A7 = 12
000CF8 2A7C 0040 1000   	movl	#0x401000,a5
000CFE 3039 0000 0000   	movw	testvi,d0
000D04 48C0             	extl	d0
000D06 2800             	movl	d0,d4
000D08 4A87             	tstl	d7
000D0A 6602             	jne	.L264
000D0C 7E01             	moveq	#0x1,d7
000D0E                  .L264:
000D0E 4A79 0000 0000   	tstw	interrupts
000D14 671C             	jeq	.L269
000D16                  .L266:
000D16 2007             	movl	d7,d0
000D18 5387             	subql	#0x1,d7
000D1A 4A80             	tstl	d0
000D1C 6F7A             	jle	.L268
000D1E 287C 0000 0000   	movl	#testvecin,a4
000D24 2004             	movl	d4,d0
000D26 5380             	subql	#0x1,d0
000D28 2A00             	movl	d0,d5
000D2A                  vvvv:
000D2A 3A9C             	movw	a4@+,a5@
000D2C 51CD FFFC        	dbf	d5,vvvv
000D30 60E4             	jra	.L266
000D32                  .L269:
000D32 2007             	movl	d7,d0
000D34 5387             	subql	#0x1,d7
000D36 4A80             	tstl	d0
000D38 6F5E             	jle	.L263
000D3A 23FC 0000 0000   	movl	#testvecin,pgearray
       0000 0000        
000D44 23FC 0000 0000   	movl	#testvecout,outfile
       0000 0000        
000D4E 7A00             	moveq	#0,d5
000D50                  .L273:
000D50 BA84             	cmpl	d4,d5
000D52 6CDE             	jge	.L269
000D54 2C3C 0010 0000   	movl	#0x100000,d6
000D5A                  .L276:
000D5A 2006             	movl	d6,d0
000D5C 5386             	subql	#0x1,d6
000D5E 4A80             	tstl	d0
000D60 670E             	jeq	.L275
000D62 3039 001F 2C00   	movw	0x1f2c00,d0
000D68 0280 0000 8000   	andl	#0x8000,d0
000D6E 66EA             	jne	.L276
000D70                  .L275:
000D70 4A86             	tstl	d6
000D72 6E12             	jgt	.L278
000D74 4879 0000 1274   	pea	.L279
000D7A 4EB9 0000 0000   	jbsr	printf
000D80 588F             	addql	#4,sp
000D82 7000             	moveq	#0,d0
000D84 6012             	jra	.L263
000D86                  .L278:
000D86 2079 0000 0000   	movl	pgearray,a0
000D8C 3A90             	movw	a0@,a5@
000D8E 54B9 0000 0000   	addql	#0x2,pgearray
000D94 5285             	addql	#0x1,d5
000D96 60B8             	jra	.L273
000D98                  .L268:
000D98                  .L263:
000D98 4CEE 30F0 FFE8   	moveml	a6@(-.F7),#0x30f0
000D9E 4E5E             	unlk	a6
000DA0 4E75             	rts
       0000 0018        .F7 = 24
       0000 30F0        .S7 = 12528
                        | M7 = 136
                        	.data
000EDA                  .L96:
000EDA 69 6E 69 74 20   	.ascii	"init first!\012"
       66 69 72 73 74   
       21 0A            
000EE6 00               	.byte	0
000EE7                  .L103:
000EE7 72 65 61 64 20   	.ascii	"read = %04x    written = %04x\012"
       3D 20 25 30 34   
       78 20 20 20 20   
       77 72 69 74 74   
       65 6E 20 3D 20   
       25 30 34 78 0A   
000F05 00               	.byte	0
000F06                  .L119:
000F06 77 72 6F 6E 67   	.ascii	"wrong no. of GEs\012"
       20 6E 6F 2E 20   
       6F 66 20 47 45   
       73 0A            
000F17 00               	.byte	0
000F18                  .L127:
000F18 68 75 6E 67 0A   	.ascii	"hung\012"
000F1D 00               	.byte	0
000F1E                  .L143:
000F1E 68 75 6E 67 0A   	.ascii	"hung\012"
000F23 00               	.byte	0
000F24                  .L147:
000F24 25 64 20 69 6E   	.ascii	"%d interrupts missing\012"
       74 65 72 72 75   
       70 74 73 20 6D   
       69 73 73 69 6E   
       67 0A            
000F3A 00               	.byte	0
000F3B                  .L155:
000F3B 69 6E 69 74 20   	.ascii	"init first!\012"
       66 69 72 73 74   
       21 0A            
000F47 00               	.byte	0
000F48                  .L165:
000F48 69 6E 69 74 20   	.ascii	"init first!\012"
       66 69 72 73 74   
       21 0A            
000F54 00               	.byte	0
000F55                  .L169:
000F55 2A 69 6E 3D 25   	.ascii	"*in=%x *out=outfile\012"
       78 20 2A 6F 75   
       74 3D 6F 75 74   
       66 69 6C 65 0A   
000F69 00               	.byte	0
000F6A                  .L176:
000F6A 69 6E 69 74 20   	.ascii	"init first!\012"
       66 69 72 73 74   
       21 0A            
000F76 00               	.byte	0
000F77                  .L180:
000F77 2A 69 6E 3D 25   	.ascii	"*in=%x *out=outfile\012"
       78 20 2A 6F 75   
       74 3D 6F 75 74   
       66 69 6C 65 0A   
000F8B 00               	.byte	0
000F8C                  .L188:
000F8C 68 75 6E 67 0A   	.ascii	"hung\012"
000F91 00               	.byte	0
000F92                  .L200:
000F92 25 78 20 20      	.ascii	"%x  "
000F96 00               	.byte	0
000F97                  .L220:
000F97 62 61 64 20 63   	.ascii	"bad chip no. %d\012"
       68 69 70 20 6E   
       6F 2E 20 25 64   
       0A               
000FA7 00               	.byte	0
000FA8                  .L222:
000FA8 63 68 69 70 20   	.ascii	"chip %d not installed\012"
       25 64 20 6E 6F   
       74 20 69 6E 73   
       74 61 6C 6C 65   
       64 0A            
000FBE 00               	.byte	0
000FBF                  .L232:
000FBF 20 20 20 61 63   	.ascii	"   accelerator test vector <n>\012"
       63 65 6C 65 72   
       61 74 6F 72 20   
       74 65 73 74 20   
       76 65 63 74 6F   
       72 20 3C 6E 3E   
       0A               
000FDE 00               	.byte	0
000FDF                  .L233:
000FDF 20 20 20 42 65   	.ascii	"   Better test of GE 1..c[-1..c]"
       74 74 65 72 20   
       74 65 73 74 20   
       6F 66 20 47 45   
       20 31 2E 2E 63   
       5B 2D 31 2E 2E   
       63 5D            
000FFF 20 5B 23 70 72   	.ascii	" [#prefix wds] test\012"
       65 66 69 78 20   
       77 64 73 5D 20   
       74 65 73 74 0A   
001013 00               	.byte	0
001014                  .L234:
001014 20 20 20 43 6F   	.ascii	"   Configure pipe [0..c]  [to 0."
       6E 66 69 67 75   
       72 65 20 70 69   
       70 65 20 5B 30   
       2E 2E 63 5D 20   
       20 5B 74 6F 20   
       30 2E            
001034 2E 63 5D 20 63   	.ascii	".c] chip(s) isolated\012"
       68 69 70 28 73   
       29 20 69 73 6F   
       6C 61 74 65 64   
       0A               
001049 00               	.byte	0
00104A                  .L235:
00104A 20 20 20 64 61   	.ascii	"   data word send down pipe <n>\012"
       74 61 20 77 6F   
       72 64 20 73 65   
       6E 64 20 64 6F   
       77 6E 20 70 69   
       70 65 20 3C 6E   
       3E 0A            
00106A 00               	.byte	0
00106B                  .L236:
00106B 20 20 20 65 20   	.ascii	"   e - data word <n> sent with T"
       2D 20 64 61 74   
       61 20 77 6F 72   
       64 20 3C 6E 3E   
       20 73 65 6E 74   
       20 77 69 74 68   
       20 54            
00108B 4F 4B 45 4E 0A   	.ascii	"OKEN\012"
001090 00               	.byte	0
001091                  .L237:
001091 20 20 20 44 69   	.ascii	"   Display config when gC called"
       73 70 6C 61 79   
       20 63 6F 6E 66   
       69 67 20 77 68   
       65 6E 20 67 43   
       20 63 61 6C 6C   
       65 64            
0010B1 20 28 6D 6F 64   	.ascii	" (mode toggles)\012"
       65 20 74 6F 67   
       67 6C 65 73 29   
       0A               
0010C1 00               	.byte	0
0010C2                  .L238:
0010C2 20 20 20 66 6C   	.ascii	"   flag reg print\012"
       61 67 20 72 65   
       67 20 70 72 69   
       6E 74 0A         
0010D4 00               	.byte	0
0010D5                  .L239:
0010D5 20 20 20 69 6E   	.ascii	"   initialize pipe\012"
       69 74 69 61 6C   
       69 7A 65 20 70   
       69 70 65 0A      
0010E8 00               	.byte	0
0010E9                  .L240:
0010E9 20 20 20 6A 75   	.ascii	"   just config [<n> chips] as pa"
       73 74 20 63 6F   
       6E 66 69 67 20   
       5B 3C 6E 3E 20   
       63 68 69 70 73   
       5D 20 61 73 20   
       70 61            
001109 73 73 65 72 73   	.ascii	"ssers (no fifo/ga config)\012"
       20 28 6E 6F 20   
       66 69 66 6F 2F   
       67 61 20 63 6F   
       6E 66 69 67 29   
       0A               
001123 00               	.byte	0
001124                  .L241:
001124 20 20 20 4C 69   	.ascii	"   Limit GE test to <n> words in"
       6D 69 74 20 47   
       45 20 74 65 73   
       74 20 74 6F 20   
       3C 6E 3E 20 77   
       6F 72 64 73 20   
       69 6E            
001144 0A               	.ascii	"\012"
001145 00               	.byte	0
001146                  .L242:
001146 20 20 20 6D 75   	.ascii	"   multibus picture test <n>\012"
       6C 74 69 62 75   
       73 20 70 69 63   
       74 75 72 65 20   
       74 65 73 74 20   
       3C 6E 3E 0A      
001163 00               	.byte	0
001164                  .L243:
001164 20 20 20 50 61   	.ascii	"   PassThru test vector\012"
       73 73 54 68 72   
       75 20 74 65 73   
       74 20 76 65 63   
       74 6F 72 0A      
00117C 00               	.byte	0
00117D                  .L244:
00117D 20 20 20 73 74   	.ascii	"   store flag reg <n>\012"
       6F 72 65 20 66   
       6C 61 67 20 72   
       65 67 20 3C 6E   
       3E 0A            
001193 00               	.byte	0
001194                  .L245:
001194 20 20 20 74 65   	.ascii	"   test picture <n>\012"
       73 74 20 70 69   
       63 74 75 72 65   
       20 3C 6E 3E 0A   
0011A8 00               	.byte	0
0011A9                  .L246:
0011A9 20 20 20 54 65   	.ascii	"   Test vec for specific GE (0 f"
       73 74 20 76 65   
       63 20 66 6F 72   
       20 73 70 65 63   
       69 66 69 63 20   
       47 45 20 28 30   
       20 66            
0011C9 6F 72 20 61 6C   	.ascii	"or all, <n> for chip n)\012"
       6C 2C 20 3C 6E   
       3E 20 66 6F 72   
       20 63 68 69 70   
       20 6E 29 0A      
0011E1 00               	.byte	0
0011E2                  .L247:
0011E2 20 20 20 77 61   	.ascii	"   wait <n> between 'm' test wor"
       69 74 20 3C 6E   
       3E 20 62 65 74   
       77 65 65 6E 20   
       27 6D 27 20 74   
       65 73 74 20 77   
       6F 72            
001202 64 73 0A         	.ascii	"ds\012"
001205 00               	.byte	0
001206                  .L248:
001206 20 20 20 31 2E   	.ascii	"   1..c[-1..c] send vector for s"
       2E 63 5B 2D 31   
       2E 2E 63 5D 20   
       73 65 6E 64 20   
       76 65 63 74 6F   
       72 20 66 6F 72   
       20 73            
001226 70 65 63 69 66   	.ascii	"pecific chip(s)\012"
       69 63 20 63 68   
       69 70 28 73 29   
       0A               
001236 00               	.byte	0
001237                  .L249:
001237 20 20 20 76 65   	.ascii	"   vector test\012"
       63 74 6F 72 20   
       74 65 73 74 0A   
001246 00               	.byte	0
001247                  .L250:
001247 6D 6F 72 65 2E   	.ascii	"more..."
       2E 2E            
00124E 00               	.byte	0
00124F                  .L258:
00124F 67 25 78 3A 20   	.ascii	"g%x: %d words\012"
       25 64 20 77 6F   
       72 64 73 0A      
00125D 00               	.byte	0
00125E                  .L262:
00125E 31 30 2D 63 68   	.ascii	"10-chip gp: %d words\012"
       69 70 20 67 70   
       3A 20 25 64 20   
       77 6F 72 64 73   
       0A               
001273 00               	.byte	0
001274                  .L279:
001274 68 75 6E 67 0A   	.ascii	"hung\012"
001279 00               	.byte	0
