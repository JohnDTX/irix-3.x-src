changequote([,])

ifdef([PROF],
[
    define([ENTRY],
[
	.globl _$1
	.text
_$1:
	movl	#9999$,a0
	jbsr	mcount
	.data
9999$:	.long	0
	.text
]
    )
]
,
[
    define([ENTRY],
[
	.globl	_$1
	.text
_$1:
]
    )
]
)

ifdef([PROF],
[
    define([ASENTRY],
[
	.globl $1
	.text
$1:
	movl	#9999$,a0
	jbsr	mcount
	.data
9999$:	.long	0
	.text
]
    )
]
,
[
    define([ASENTRY],
[
	.globl	$1
	.text
$1:
]
    )
]
)

ifdef([PROF],
[
    define([RASENTRY],
[
	.globl $1
	.text
$1:
	pea	9999$
	jbsr	smcount
	addql	#4,sp
	.data
9999$:	.long	0
	.text
]
    )
]
,
[
    define([RASENTRY],
[
	.globl	$1
	.text
$1:
]
    )
]
)
