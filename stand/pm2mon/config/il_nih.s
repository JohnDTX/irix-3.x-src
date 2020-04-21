	.globl start
start:
	.data
	.byte	0xf0		| config prom magic number 0
	.byte	0xc5		| config prom magic number 1
	.byte	0x0		| config prom low mode = interlaced
	.byte	0x30		| config prom high mode = non-interlaced
